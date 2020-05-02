#include "AuthEventHandler.h"

#include <sqlpp11/select.h>

#include "../api/Resources.h"
#include "../communication/CookieVerify.h"
#include "../database/UsersTable.h"
#include "../utility/Logger.h"

constexpr std::chrono::system_clock::duration AuthEventHandler::s_tokenExpiration;

AuthEventHandler::AuthEventHandler(
    DBHandler& dbHandler, WebsocketCommunication& sockComm, const Authenticator& authenticator)
    : m_dbHandler(&dbHandler), m_sockComm(&sockComm), m_authenticator(&authenticator)
{}

PostEventState AuthEventHandler::operator()(const WebsocketCommunication::EventVariant& e)
{
    if (absl::holds_alternative<Events::SocketMessageEvent>(e))
    {
        return HandleSocketMessage(absl::get<Events::SocketMessageEvent>(e));
    }
    return PostEventState::notHandled;
}

PostEventState AuthEventHandler::HandleSocketMessage(const Events::SocketMessageEvent& event)
{
    try
    {
        const nlohmann::json& payload = event.GetJsonPayload();
        auto it = payload.find("login");
        if (it == payload.end())
        {
            return PostEventState::notHandled;
        }
        const nlohmann::json& login = *it;
        const std::string& username = login.at("username");
        const std::string& password = login.at("password");
        std::string token = ValidateLogin(username, password);
        if (token.empty())
        {
            m_sockComm->Send(event.GetConnection(), nlohmann::json{{"authFailed", true}});
        }
        else
        {
            m_sockComm->Send(event.GetConnection(),
                nlohmann::json{{"idToken", token},
                    {"expiresIn", std::chrono::duration_cast<std::chrono::seconds>(s_tokenExpiration).count()}});

            auto& db = m_dbHandler->GetDatabase();
            UsersTable users;

            auto result = db(select(users.userId, users.picture).from(users).where(users.userName == username));

            if (!result.empty())
            {
                auto& front = result.front();
                int64_t user_id = front.userId;
                std::vector<uint8_t> blob(front.picture.blob, front.picture.blob + front.picture.len);
                std::string blob_str(blob.begin(), blob.end());
                m_sockComm->Send(
                    event.GetConnection(), nlohmann::json{{"user", username}, {"userid", user_id}, {"pic", blob_str}});
            }
        }
        // \todo Consume event to avoid too many other handlers from seeing the password
        // This is definitely not secure
        // TODO: Implement privacy in websocket messages
        return PostEventState::handled | PostEventState::consumeEvent;
    }
    catch (const std::exception& e)
    {
        Res::Logger().Error("AuthEventHandler", std::string("Exception while processing message: ") + e.what());
    }
    return PostEventState::notHandled;
}

std::string AuthEventHandler::ValidateLogin(const std::string& username, const std::string& password)
{
    auto& db = m_dbHandler->GetDatabase();
    UsersTable users;
    auto result = db(select(users.userPwhash, users.userId).from(users).where(users.userName == username));
    if (!result.empty())
    {
        auto& front = result.front();
        std::string hash = front.userPwhash;
        if (m_authenticator->ValidatePassword(password, hash))
        {
            nlohmann::json token{{"iss", "sh-auth-event-handler"}, {"sub", std::to_string(front.userId)}};
            return m_authenticator->CreateJWTToken(std::move(token));
        }
    }
    return std::string();
}

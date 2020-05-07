#include "ProfileSocketHandler.h"

#include <cryptopp/base64.h>
#include <cryptopp/hmac.h>
#include <cryptopp/osrng.h> // AutoSeededRandomPool
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>
#include <sqlpp11/select.h>
#include <sqlpp11/update.h>

#include "../communication/CookieVerify.h"
#include "../database/UsersTable.h"

PostEventState ProfileSocketHandler::operator()(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel)
{
    if (absl::holds_alternative<Events::SocketMessageEvent>(event))
    {
        return HandleSocketMessage(absl::get<Events::SocketMessageEvent>(event), channel);
    }
    return PostEventState::notHandled;
}

PostEventState ProfileSocketHandler::HandleSocketMessage(
    const Events::SocketMessageEvent& event, WebsocketChannel& channel)
{
    try
    {
        auto& db = m_dbHandler->GetDatabase();
        UsersTable users;

        const nlohmann::json& payload = event.GetJsonPayload();
        auto it = payload.find("command");
        if (it == payload.end())
        {
            return PostEventState::notHandled;
        }

        const std::string& command = *it;
        if (command == s_checkPassword)
        {
            Res::Logger().Debug("ProfileSocketHandler", "Checking password");
            if (event.GetUser().has_value())
            {
                const int64_t userId = event.GetUser().value().IActuallyReallyNeedTheIntegerNow();
                bool password_valid = ValidatePassword(userId, payload.at("pw"));
                channel.Send(
                    event.GetConnection(), nlohmann::json {{"pw", payload.at("pw")}, {"valid", password_valid}});

                return PostEventState::handled;
            }
            return PostEventState::error;
        }
        else if (command == s_changePassword)
        {
            Res::Logger().Debug("ProfileSocketHandler", "Changing password");
            PostEventState return_state = PostEventState::error;
            if (event.GetUser().has_value())
            {
                bool success = false;
                const int64_t userId = event.GetUser().value().IActuallyReallyNeedTheIntegerNow();
                bool password_valid = ValidatePassword(userId, payload.at("pw").at("old"));
                //! \todo Need to check if admin or have user system
                std::string new_password = payload.at("pw").at("new");
                if (password_valid && new_password == payload.at("pw").at("rep"))
                {
                    std::string hash = m_authenticator->CreatePasswordHash(new_password);

                    // test password hash before writing
                    if (m_authenticator->ValidatePassword(new_password, hash))
                    {
                        db(update(users).set(users.userPwhash = hash).where(users.userId == userId));
                        success = true;
                    }
                    else
                    {
                        Res::Logger().Severe("Generated Password hash could not be validated");
                    }
                }

                channel.Send(
                    event.GetConnection(), nlohmann::json {{"pw", payload.at("pw").at("old")}, {"success", success}});
                return_state = PostEventState::handled;
            }
            return return_state;
        }
        else if (command == s_changePicture)
        {
            Res::Logger().Debug("ProfileSocketHandler", "Changing picture");
            PostEventState return_state = PostEventState::error;
            if (event.GetUser().has_value() && payload.find("pic") != payload.end())
            {
                std::string picture = payload.at("pic");
                const int64_t userId = event.GetUser().value().IActuallyReallyNeedTheIntegerNow();
                db(update(users).set(users.picture = picture).where(users.userId == userId));
                /// \todo Need to broadcast to only this user and not everyone!!!
                channel.Broadcast(nlohmann::json {{"userid", userId}, {"pic", picture}});
                return_state = PostEventState::handled;
            }
            return return_state;
        }
        else if (command == s_getUser)
        {
            Res::Logger().Debug("ProfileSocketHandler", "Get user");
            PostEventState return_state = PostEventState::error;
            if (event.GetUser().has_value())
            {
                const int64_t userId = event.GetUser().value().IActuallyReallyNeedTheIntegerNow();
                auto result = db(select(users.userName, users.picture).from(users).where(users.userId == userId));
                if (!result.empty())
                {
                    auto& front = result.front();
                    std::string username = front.userName;
                    std::vector<uint8_t> blob(front.picture.blob, front.picture.blob + front.picture.len);
                    std::string blob_str(blob.begin(), blob.end());
                    return_state = PostEventState::handled;
                    channel.Send(event.GetConnection(),
                        nlohmann::json {{"user", username}, {"userid", userId}, {"pic", blob_str}});
                }
            }
            return return_state;
        }
        else if (command == s_getPicture)
        {
            Res::Logger().Debug("ProfileSocketHandler", "Get picture");
            PostEventState return_state = PostEventState::error;
            if (event.GetUser().has_value())
            {
                const int64_t userId = event.GetUser().value().IActuallyReallyNeedTheIntegerNow();
                auto result = db(select(users.picture).from(users).where(users.userId == userId));
                if (result.empty())
                {
                    auto& front = result.front();
                    std::string blob_str(front.picture.blob, front.picture.blob + front.picture.len);
                    return_state = PostEventState::handled;
                    channel.Send(event.GetConnection(), nlohmann::json {{"pic", blob_str}});
                }
            }
            return return_state;
        }
    }
    catch (const std::exception& e)
    {
        Res::Logger().Error("ProfileSocketHandler", std::string("Exception while processing message: ") + e.what());
        return PostEventState::error;
    }
    return PostEventState::notHandled;
}

bool ProfileSocketHandler::ValidatePassword(const int64_t userid, const std::string& password)
{
    auto& db = m_dbHandler->GetDatabase();
    UsersTable users;
    auto result = db(select(users.userPwhash).from(users).where(users.userId == userid));
    if (!result.empty())
    {
        std::string hash = result.front().userPwhash;
        return m_authenticator->ValidatePassword(password, hash);
    }
    return false;
}

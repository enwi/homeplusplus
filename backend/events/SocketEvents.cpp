#include "SocketEvents.h"

#include "../communication/Authenticator.h"

Events::SocketMessageEvent Events::SocketMessageEvent::Parse(
    connection_hdl hdl, message_ptr msg, const Authenticator* authenticator)
{
    nlohmann::json payload;
    absl::optional<UserId> user;
    // Check if msg contains json
    if (nlohmann::json::accept(msg->get_payload()))
    {
        payload = nlohmann::json::parse(msg->get_payload());
        if (authenticator)
        {
            auto idTokenIt = payload.find("idToken");
            if (idTokenIt != payload.end())
            {
                nlohmann::json idToken = authenticator->ValidateJWTToken(*idTokenIt);
                if (idToken != nullptr && idToken.count("sub") != 0)
                {
                    user = UserId(std::stoll(idToken["sub"].get<std::string>()));
                }
            }
        }
    }
    else
    {
        payload = msg->get_payload();
    }
    return SocketMessageEvent(std::move(hdl), std::move(msg), std::move(payload), std::move(user));
}

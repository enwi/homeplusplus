#ifndef _SOCKET_MESSAGE_EVENT_H
#define _SOCKET_MESSAGE_EVENT_H

#include <json.hpp>

#include "EventSystem.h"

#include "../api/User.h"
#include "../config/WebsocketConfig.h"
#include "absl/types/optional.h"

// Strange error
#ifdef GetMessage
#undef GetMessage
#endif

class Authenticator;
namespace Events
{
    class SocketConnectEvent
    {
    public:
        explicit SocketConnectEvent(websocketpp::connection_hdl hdl, absl::optional<UserId> user)
            : m_hdl(hdl), m_user(std::move(user))
        {}

        websocketpp::connection_hdl GetConnection() const { return m_hdl; }
        absl::optional<UserId> GetUser() const { return m_user; };

    private:
        websocketpp::connection_hdl m_hdl;
        absl::optional<UserId> m_user;
    };

    class SocketDisconnectEvent
    {
    public:
        explicit SocketDisconnectEvent(websocketpp::connection_hdl hdl, absl::optional<UserId> user)
            : m_hdl(hdl), m_user(std::move(user))
        {}

        websocketpp::connection_hdl GetConnection() const { return m_hdl; }
        absl::optional<UserId> GetUser() const { return m_user; };

    private:
        websocketpp::connection_hdl m_hdl;
        absl::optional<UserId> m_user;
    };

    class SocketMessageEvent
    {
    public:
        using connection_hdl = websocketpp::connection_hdl;
        using message_ptr = WebsocketConfig::server::message_ptr;
        using message_type = typename message_ptr::element_type;

        SocketMessageEvent(connection_hdl hdl, message_ptr msg, nlohmann::json payload, absl::optional<UserId> user)
            : m_hdl(hdl), m_msg(std::move(msg)), m_payload(std::move(payload)), m_user(std::move(user))
        {}

        websocketpp::connection_hdl GetConnection() const { return m_hdl; }
        const message_type& GetMessage() const { return *m_msg; }
        // Message payload in json form
        // If message payload is not convertible, contains string with payload
        const nlohmann::json& GetJsonPayload() const { return m_payload; }
        absl::optional<UserId> GetUser() const { return m_user; };

        // Authenticator can be nullptr
        static SocketMessageEvent Parse(connection_hdl hdl, message_ptr msg, const Authenticator* authenticator);

    private:
        websocketpp::connection_hdl m_hdl;
        message_ptr m_msg;
        nlohmann::json m_payload;
        absl::optional<UserId> m_user;
    };

} // namespace Events

#endif
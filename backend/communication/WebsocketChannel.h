#ifndef WEBSOCKET_CHANNEL_H
#define WEBSOCKET_CHANNEL_H

#include <set>
#include <vector>

#include <absl/strings/string_view.h>
#include <absl/types/variant.h>

#include "../communication/Authenticator.h"
#include "../events/SocketEvents.h"

class WebsocketChannel
{
public:
    using connection_hdl = Events::SocketMessageEvent::connection_hdl;
    using EventVariant
        = absl::variant<Events::SocketConnectEvent, Events::SocketMessageEvent, Events::SocketDisconnectEvent>;
    enum class RequireAuth
    {
        noAuth,
        auth
    };

    WebsocketChannel(class WebsocketCommunication& communication, absl::string_view name, RequireAuth requireAuth);

    void OnChannelEvent(const absl::variant<Events::SocketMessageEvent, Events::SocketDisconnectEvent>& e);

    // Broadcasts a message to all connections subscribed to this channel
    // json by value so channel name can be added
    void Broadcast(nlohmann::json value);
    // Sends a message to the specified connection. Does not require connection to be subscribed
    // json by value so channel name can be added
    void Send(connection_hdl connection, nlohmann::json value);
    // Sends a binary message to the specified connection. Does not require connection to be subscribed
    void SendBytes(connection_hdl connection, const void* data, std::size_t length);

    void Subscribe(connection_hdl connection);
    void Unsubscribe(connection_hdl connection);
    // Returs whether clients have subscribe. Can be used to avoid unneccessary operations
    bool HasSubscribers() const { return m_clients.size() > 0; }

    // Add event handler to handle events
    // If evHandler accepts socketConnect or socketDisconnect events,
    // it is also notified of subscriptions and unsubscriptions
    void AddEventHandler(std::function<PostEventState(const EventVariant&, WebsocketChannel&)> evHandler);

    const std::string& GetName() const { return m_name; }

private:
    bool CheckAuthenticated(const Events::SocketMessageEvent& message);

private:
    class WebsocketCommunication* m_communication;
    std::string m_name;
    std::set<connection_hdl, std::owner_less<connection_hdl>> m_clients;
    EventEmitter<EventVariant, WebsocketChannel&> m_eventEmitter;
    RequireAuth m_requireAuth;
};

#endif

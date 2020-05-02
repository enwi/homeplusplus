#include "WebsocketChannel.h"

#include "WebsocketCommunication.h"

#include "../api/Resources.h"
#include "../utility/Logger.h"

WebsocketChannel::WebsocketChannel(
    WebsocketCommunication& communication, absl::string_view name, RequireAuth requireAuth)
    : m_communication(&communication), m_name(name), m_requireAuth(requireAuth)
{}

void WebsocketChannel::OnChannelEvent(const absl::variant<Events::SocketMessageEvent, Events::SocketDisconnectEvent>& e)
{
    try
    {
        if (absl::holds_alternative<Events::SocketDisconnectEvent>(e))
        {
            const Events::SocketDisconnectEvent& disconnectEvent = absl::get<Events::SocketDisconnectEvent>(e);
            auto it = m_clients.find(disconnectEvent.GetConnection());
            if (it != m_clients.end())
            {
                Unsubscribe(disconnectEvent.GetConnection());
            }
        }
        else if (absl::holds_alternative<Events::SocketMessageEvent>(e))
        {
            const Events::SocketMessageEvent& messageEvent = absl::get<Events::SocketMessageEvent>(e);
            if (CheckAuthenticated(messageEvent))
            {
                const nlohmann::json& json = messageEvent.GetJsonPayload();

                auto commandIt = json.find("command");
                if (commandIt != json.end())
                {
                    if (*commandIt == "subscribeChannel")
                    {
                        Subscribe(messageEvent.GetConnection());
                        return;
                    }
                    else if (*commandIt == "unsubscribeChannel")
                    {
                        Unsubscribe(messageEvent.GetConnection());
                        return;
                    }
                }
                m_eventEmitter.EmitEvent(messageEvent, *this);
            }
        }
    }
    catch (const std::exception& e)
    {
        Res::Logger().Error("WebsocketChannel", std::string("Exception while processing event: ") + e.what());
    }
}

void WebsocketChannel::Broadcast(nlohmann::json value)
{
    value["channel"] = m_name;
    for (const auto& connection : m_clients)
    {
        m_communication->Send(connection, value);
    }
}

void WebsocketChannel::Send(Events::SocketMessageEvent::connection_hdl connection, nlohmann::json value)
{
    // Value is copied so channel can be added
    value["channel"] = m_name;
    m_communication->Send(connection, value);
}

void WebsocketChannel::SendBytes(connection_hdl connection, const void* data, std::size_t length)
{
    // TODO: Encode channel information in binary data
    m_communication->SendBytes(connection, data, length);
}

void WebsocketChannel::Subscribe(WebsocketCommunication::connection_hdl connection)
{
    m_clients.insert(connection);
    Res::Logger().Debug("Client " + std::to_string(reinterpret_cast<intptr_t>(connection.lock().get()))
        + " subscribed to channel " + m_name);
    m_eventEmitter.EmitEvent(Events::SocketConnectEvent{connection, absl::nullopt}, *this);
}

void WebsocketChannel::Unsubscribe(WebsocketCommunication::connection_hdl connection)
{
    m_clients.erase(connection);
    Res::Logger().Debug("Client " + std::to_string(reinterpret_cast<intptr_t>(connection.lock().get()))
        + " unsubscribed from channel " + m_name);
    m_eventEmitter.EmitEvent(Events::SocketDisconnectEvent{connection, absl::nullopt}, *this);
}

void WebsocketChannel::AddEventHandler(std::function<PostEventState(const EventVariant&, WebsocketChannel&)> handler)
{
    m_eventEmitter.AddHandler(std::move(handler));
}

bool WebsocketChannel::CheckAuthenticated(const Events::SocketMessageEvent& e)
{
    if (m_requireAuth == RequireAuth::auth)
    {
        if (!e.GetUser().has_value())
        {
            Res::Logger().Warning("WebsocketChannel", "Unauthenticated websocket request");
            m_communication->Send(e.GetConnection(), nlohmann::json{{"error", "notAuthenticated"}});
            return false;
        }
    }
    return true;
}
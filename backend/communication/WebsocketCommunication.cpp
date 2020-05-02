#include "WebsocketCommunication.h"

#include <algorithm>
#include <stdexcept>

#include "CookieVerify.h"

#include "../api/Resources.h"
#include "../events/SocketEvents.h"
#include "../utility/Logger.h"

WebsocketCommunication::WebsocketCommunication(const Authenticator* authenticator) : m_authenticator(authenticator)
{
    // Must init now, because otherwise no callbacks can be queued
    m_server.init_asio();
}

void WebsocketCommunication::Send(connection_hdl hdl, const nlohmann::json& msg)
{
    m_server.send(hdl, msg.dump(), websocketpp::frame::opcode::text);
}

void WebsocketCommunication::SendBytes(connection_hdl hdl, const void* data, size_t len)
{
    m_server.send(hdl, data, len, websocketpp::frame::opcode::binary);
}

void WebsocketCommunication::Broadcast(const nlohmann::json& msg)
{
    for (connection_hdl hdl : m_connections)
    {
        m_server.send(hdl, msg.dump(), websocketpp::frame::opcode::text);
    }
}

void WebsocketCommunication::Close(
    connection_hdl hdl, websocketpp::close::status::value code, const std::string& reason)
{
    m_server.close(hdl, code, reason);
}

void WebsocketCommunication::Start()
{
    m_thread = std::thread(&WebsocketCommunication::Run, this);
}

void WebsocketCommunication::Stop()
{
    m_server.get_io_service().post([this] {
        m_server.stop_listening();
        for (connection_hdl c : m_connections)
        {
            try
            {
                m_server.close(c, websocketpp::close::status::going_away, "Server shutdown");
            }
            catch (const websocketpp::exception& e)
            {
                Res::Logger().Warning("Failed to close websocke connection: " + e.m_msg);
            }
        }
    });
    if (m_thread.joinable())
    {
        // Wait until the thread finishes if it is still running
        m_thread.join();
    }
    Res::Logger().Info("WebsocketCommunication thread stopped");
}

void WebsocketCommunication::AddEventHandler(std::function<PostEventState(const EventVariant&)> handler)
{
    m_server.get_io_service().post([this, handler]() mutable { m_eventEmitter.AddHandler(std::move(handler)); });
}

void WebsocketCommunication::AddChannel(WebsocketChannel channel)
{
    m_channels.try_emplace(channel.GetName(), std::move(channel));
}

WebsocketChannel& WebsocketCommunication::GetChannel(absl::string_view name)
{
    return m_channels.at(name);
}

void WebsocketCommunication::OnOpen(connection_hdl hdl)
{
    m_connections.insert(hdl);
    m_eventEmitter.EmitEvent(Events::SocketConnectEvent(hdl, absl::nullopt));
}

void WebsocketCommunication::OnClose(connection_hdl hdl)
{
    auto pos = m_connections.find(hdl);
    // Check if this is not a close caused by missing authentication, in which case hdl is not in the set
    if (pos != m_connections.end())
    {
        m_connections.erase(pos);
        Events::SocketDisconnectEvent event(hdl, absl::nullopt);
        for (auto& c : m_channels)
        {
            c.second.OnChannelEvent(event);
        }
        m_eventEmitter.EmitEvent(Events::SocketDisconnectEvent(hdl, absl::nullopt));
    }
}

void WebsocketCommunication::OnMessage(connection_hdl hdl, server::message_ptr msg)
{
    Res::Logger().Debug(
        "Message from " + std::to_string(reinterpret_cast<intptr_t>(hdl.lock().get())) + ":" + msg->get_payload());

    Events::SocketMessageEvent event = Events::SocketMessageEvent::Parse(hdl, msg, m_authenticator);
    const nlohmann::json& json = event.GetJsonPayload();
    auto it = json.find("channel");
    if (it != json.end())
    {
        auto channelIt = m_channels.find(*it);
        if (channelIt == m_channels.end())
        {
            Res::Logger().Warning("WebsocketCommunication", "Command to unknown channel will be ignored");
        }
        else
        {
            channelIt->second.OnChannelEvent(event);
        }
    }
    else
    {
        // General event handlers only receive events not belonging to a channel
        m_eventEmitter.EmitEvent(event);
    }
}

void WebsocketCommunication::Run()
{
    using namespace std::placeholders;
    try
    {
        m_server.set_reuse_addr(true);
        m_server.set_message_handler([this](auto _1, auto _2) { this->OnMessage(_1, _2); });
        m_server.set_open_handler([this](auto _1) { this->OnOpen(_1); });
        m_server.set_close_handler([this](auto _1) { this->OnClose(_1); });
        m_server.listen(9002);
        m_server.start_accept();
        m_server.run();
    }
    catch (websocketpp::lib::error_code ec)
    {
        Res::Logger().Severe(std::string("Error code in WebsocketCommunication thread: ") + ec.message());
        Res::Logger().Severe("Terminating WebsocketCommunication thread!");
        // Re-throw
        throw;
    }
    catch (const std::exception& e)
    {
        Res::Logger().Severe(std::string("Exception in WebsocketCommunication thread: ") + e.what());
        Res::Logger().Severe("Terminating WebsocketCommunication thread!");
        // Re-throw
        throw;
    }
}

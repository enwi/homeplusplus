#ifndef _WEBSOCKET_COMMUNICATION_H
#define _WEBSOCKET_COMMUNICATION_H
#include <mutex>
#include <set>
#include <thread>

#include <absl/container/flat_hash_map.h>
#include <json.hpp>

#include "Authenticator.h"
#include "WebsocketChannel.h"

#include "../config/WebsocketConfig.h"
#include "../events/EventSystem.h"
#include "../events/SocketEvents.h"
#include "../utility/Active.h"

// Class to handle websocket connections
class WebsocketCommunication
{
public:
    friend class TestWebsocketCommunication;
    using server = WebsocketConfig::server;
    using connection_hdl = websocketpp::connection_hdl;

    using EventVariant
        = absl::variant<Events::SocketConnectEvent, Events::SocketDisconnectEvent, Events::SocketMessageEvent>;

public:
    explicit WebsocketCommunication(const Authenticator* authenticator);

    // Not copy- or movable because thread requires permanent address
    WebsocketCommunication(WebsocketCommunication&&) = delete;
    WebsocketCommunication& operator=(WebsocketCommunication&&) = delete;

    // Sends text message to a connection
    void Send(connection_hdl hdl, const nlohmann::json& msg);
    // Sends binary data to a connection
    void SendBytes(connection_hdl hdl, const void* data, size_t len);
    // Sends text message to all connections
    void Broadcast(const nlohmann::json& msg);

    // Closes a connection
    void Close(connection_hdl hdl, websocketpp::close::status::value code, const std::string& reason);

    // Launches a thread which handles socket communication
    void Start();
    // Stops communication thread if running
    void Stop();

    void AddEventHandler(std::function<PostEventState(const EventVariant&)> handler);

    // Does nothing if same channel name already exists
    void AddChannel(WebsocketChannel channel);

    // Throws if not found
    WebsocketChannel& GetChannel(absl::string_view name);

private:
    // Fires SOCKET_CONNECT Event
    void OnOpen(connection_hdl hdl);
    // Fires SOCKET_DISCONNECT Event
    void OnClose(connection_hdl hdl);
    // Fires SOCKET_MESSAGE Event
    void OnMessage(connection_hdl hdl, server::message_ptr msg);

    // Called by thread
    void Run();

private:
    const Authenticator* m_authenticator;
    server m_server;
    std::thread m_thread;
    std::set<connection_hdl, std::owner_less<connection_hdl>> m_connections;
    EventEmitter<EventVariant> m_eventEmitter;
    absl::flat_hash_map<std::string, WebsocketChannel> m_channels;
};

// Retrieves a predetermined websocket channel, because they do not have stable addresses
class WebsocketChannelAccessor
{
public:
    WebsocketChannelAccessor(WebsocketCommunication& sockComm, absl::string_view name)
        : m_sockComm(&sockComm), m_name(name)
    {}

    WebsocketChannel& Get() { return m_sockComm->GetChannel(m_name); }

private:
    WebsocketCommunication* m_sockComm;
    std::string m_name;
};

#endif

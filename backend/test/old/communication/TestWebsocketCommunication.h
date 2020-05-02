#ifndef TEST_WEBSOCKET_COMMUNICATION_H
#define TEST_WEBSOCKET_COMMUNICATION_H

#include <gtest/gtest.h>

#include "../mocks/MockWebsocketServer.h"
#include "communication/WebsocketCommunication.h"

// This class is in a header so that other tests can acces WebsocketCommunication private members
class TestWebsocketCommunication
{
public:
    using connection_hdl = WebsocketCommunication::connection_hdl;
    MockWebsocketServer& GetServer() { return ws.m_server; }
    // Forward private methods
    bool WSValidateConnection(connection_hdl hdl) { return ws.ValidateConnection(hdl); }
    void WSOnOpen(connection_hdl hdl) { ws.OnOpen(hdl); }
    void WSOnClose(connection_hdl hdl) { ws.OnClose(hdl); }
    void WSOnMessage(connection_hdl hdl, WebsocketCommunication::server::message_ptr msg) { ws.OnMessage(hdl, msg); }
    void WSRun() { ws.Run(); }
    std::set<connection_hdl, std::owner_less<connection_hdl>>& GetConnections() { return ws.m_connections; }

public:
    WebsocketCommunication ws;
};

#endif
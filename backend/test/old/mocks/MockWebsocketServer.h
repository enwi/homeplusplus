#ifndef MOCK_WEBSOCKET_SERVER_H
#define MOCK_WEBSOCKET_SERVER_H

#include <gmock/gmock.h>
#include <websocketpp/server.hpp>

class MockWebsocketMessage
{
public:
    MOCK_CONST_METHOD0(get_payload, const std::string&());
};

class MockWebsocketConnection
{
public:
    MOCK_CONST_METHOD1(get_request_header, const std::string&(const std::string&));
    MOCK_METHOD2(set_status, void(websocketpp::http::status_code::value, const std::string&));
    MOCK_METHOD2(close, void(websocketpp::close::status::value, const std::string&));
};

// Mock for websocketpp::server, mocking is done by template in WebsocketConfig.h
class MockWebsocketServer
{
public:
    using connection_hdl = websocketpp::connection_hdl;
    using message_ptr = std::shared_ptr<MockWebsocketMessage>;
    using connection_ptr = std::shared_ptr<MockWebsocketConnection>;
    using message_handler = std::function<void(connection_hdl, message_ptr)>;
    using open_handler = std::function<void(connection_hdl)>;
    using close_handler = std::function<void(connection_hdl)>;
    using validate_handler = std::function<bool(connection_hdl)>;

    // Send string
    MOCK_METHOD3(send, void(connection_hdl, const std::string&, websocketpp::frame::opcode::value));
    // Send raw data
    MOCK_METHOD4(send, void(connection_hdl, const void*, std::size_t, websocketpp::frame::opcode::value));

    MOCK_METHOD1(get_con_from_hdl, connection_ptr(connection_hdl));

    MOCK_METHOD0(stop_listening, void());

    MOCK_METHOD3(close, void(connection_hdl, websocketpp::close::status::value, const std::string&));

    MOCK_METHOD0(init_asio, void());
    MOCK_METHOD1(set_reuse_addr, void(bool));
    MOCK_METHOD1(set_message_handler, void(message_handler));
    MOCK_METHOD1(set_open_handler, void(open_handler));
    MOCK_METHOD1(set_close_handler, void(close_handler));
    MOCK_METHOD1(set_validate_handler, void(validate_handler));
    MOCK_METHOD1(listen, void(int));
    MOCK_METHOD0(start_accept, void());
    MOCK_METHOD0(run, void());
};

#endif
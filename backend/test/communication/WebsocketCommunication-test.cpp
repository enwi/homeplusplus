#include <gtest/gtest.h>

#include "TestWebsocketCommunication.h"

#include "../mocks/MockWebsocketServer.h"
#include "api/Resources.h"
#include "communication/WebsocketCommunication.h"
#include "events/SocketEvents.h"
#include "utility/Logger.h"

// Creates a test fixture from TestWebsocketCommunication
class WebsocketCommunicationTest : public ::testing::Test, public TestWebsocketCommunication
{};

TEST_F(WebsocketCommunicationTest, Send)
{
    using namespace ::testing;
    MockWebsocketServer& server = GetServer();

    {
        MockWebsocketServer::connection_hdl h;
        const nlohmann::json msg = "test websocket message";
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h.lock(); }), msg.dump(), websocketpp::frame::opcode::text));
        ws.Send(h, msg);
    }

    {
        std::shared_ptr<int> p = std::make_shared<int>(12);
        MockWebsocketServer::connection_hdl h = p;
        const nlohmann::json msg = {{"test websocket message 2", "abc"}, {"value", 3}, {"array", {1, 2, 3, 4, 5}}};
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h.lock(); }), msg.dump(), websocketpp::frame::opcode::text));
        ws.Send(h, msg);
    }
    // Throwing is forwarded
    {
        std::shared_ptr<int> p = std::make_shared<int>(23);
        MockWebsocketServer::connection_hdl h = p;
        const nlohmann::json msg = "test websocket message";
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h.lock(); }), msg.dump(), websocketpp::frame::opcode::text))
            .WillOnce(Throw(std::exception()));
        EXPECT_THROW(ws.Send(h, msg), std::exception);
    }
}

TEST_F(WebsocketCommunicationTest, SendBytes)
{
    using namespace ::testing;
    MockWebsocketServer& server = GetServer();

    {
        MockWebsocketServer::connection_hdl h;
        const std::vector<uint8_t> msg{0xAA, 0x00, 0xFF, 0x12, 0xA0};
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h.lock(); }), msg.data(), msg.size(),
                websocketpp::frame::opcode::binary));
        ws.SendBytes(h, msg.data(), msg.size());
    }

    {
        std::shared_ptr<int> p = std::make_shared<int>(12);
        MockWebsocketServer::connection_hdl h = p;
        const std::vector<uint8_t> msg{0xAA, 0x00, 0xFF, 0x12, 0xA0};
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h.lock(); }), msg.data(), msg.size(),
                websocketpp::frame::opcode::binary));
        ws.SendBytes(h, msg.data(), msg.size());
    }
    // Throwing is forwarded
    {
        std::shared_ptr<int> p = std::make_shared<int>(23);
        MockWebsocketServer::connection_hdl h{};
        const std::vector<uint8_t> msg{0xAA, 0x00, 0xFF, 0x12, 0xA0};
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h.lock(); }), msg.data(), msg.size(),
                websocketpp::frame::opcode::binary))
            .WillOnce(Throw(std::exception()));
        EXPECT_THROW(ws.SendBytes(h, msg.data(), msg.size()), std::exception);
    }
}

TEST_F(WebsocketCommunicationTest, Broadcast)
{
    using namespace ::testing;
    MockWebsocketServer& server = GetServer();

    // No connections known
    {
        const nlohmann::json msg = "test websocket message";
        EXPECT_CALL(server, send(_, _, _)).Times(0);
        ws.Broadcast(msg);
        Mock::VerifyAndClearExpectations(&server);
    }

    // Connections
    std::shared_ptr<int> p1 = std::make_shared<int>(12);
    MockWebsocketServer::connection_hdl h1 = p1;
    std::shared_ptr<int> p2 = std::make_shared<int>(23);
    MockWebsocketServer::connection_hdl h2 = p2;
    GetConnections().insert(h1);
    GetConnections().insert(h2);

    {
        const nlohmann::json msg = "test websocket message";
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h1.lock(); }), msg.dump(), websocketpp::frame::opcode::text));
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h2.lock(); }), msg.dump(), websocketpp::frame::opcode::text));
        ws.Broadcast(msg);
        Mock::VerifyAndClearExpectations(&server);
    }
    // Throwing is forwarded
    {

        const nlohmann::json msg = "test websocket message 2";
        // Depending on order in set, this might be called or not
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h1.lock(); }), msg.dump(), websocketpp::frame::opcode::text))
            .Times(AtMost(1));
        EXPECT_CALL(server,
            send(Truly([&](auto p) { return p.lock() == h2.lock(); }), msg.dump(), websocketpp::frame::opcode::text))
            .WillOnce(Throw(std::exception()));
        EXPECT_THROW(ws.Broadcast(msg), std::exception);
        Mock::VerifyAndClearExpectations(&server);
    }
}

TEST_F(WebsocketCommunicationTest, OnOpen)
{
    using namespace ::testing;

    using EventVariant = WebsocketCommunication::EventVariant;

    MockFunction<PostEventState(const EventVariant&)> eventHandler;
    ON_CALL(eventHandler, Call(_)).WillByDefault(Return(PostEventState::handled));

    std::set<connection_hdl, std::owner_less<connection_hdl>>& connections = GetConnections();

    ws.AddEventHandler(eventHandler.AsStdFunction());

    EXPECT_THAT(connections, IsEmpty());
    std::shared_ptr<int> p1 = std::make_shared<int>(12);
    MockWebsocketServer::connection_hdl h1 = p1;
    std::shared_ptr<int> p2 = std::make_shared<int>(23);
    MockWebsocketServer::connection_hdl h2 = p2;

    MockFunction<void(int)> check;
    {
        InSequence s;
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketConnectEvent>(e))
            {
                return false;
            }
            return absl::get<Events::SocketConnectEvent>(e).GetConnection().lock() == h1.lock();
        })));
        EXPECT_CALL(check, Call(1));
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketConnectEvent>(e))
            {
                return false;
            }
            return absl::get<Events::SocketConnectEvent>(e).GetConnection().lock() == h2.lock();
        })));
        EXPECT_CALL(check, Call(2));
    }

    WSOnOpen(h1);
    EXPECT_THAT(connections, UnorderedElementsAre(Truly([&](auto p) { return p.lock() == h1.lock(); })));
    check.Call(1);

    WSOnOpen(h2);
    EXPECT_THAT(connections,
        UnorderedElementsAre(Truly([&](auto p) { return p.lock() == h1.lock(); }),
            Truly([&](auto p) { return p.lock() == h2.lock(); })));
    check.Call(2);
}

TEST_F(WebsocketCommunicationTest, OnClose)
{
    using namespace ::testing;
    using EventVariant = WebsocketCommunication::EventVariant;

    MockFunction<PostEventState(const EventVariant&)> eventHandler;
    ON_CALL(eventHandler, Call(_)).WillByDefault(Return(PostEventState::handled));

    ws.AddEventHandler(eventHandler.AsStdFunction());

    std::set<connection_hdl, std::owner_less<connection_hdl>>& connections = GetConnections();

    EXPECT_THAT(connections, IsEmpty());
    std::shared_ptr<int> p1 = std::make_shared<int>(12);
    MockWebsocketServer::connection_hdl h1 = p1;
    std::shared_ptr<int> p2 = std::make_shared<int>(23);
    MockWebsocketServer::connection_hdl h2 = p2;

    connections.insert(h1);
    connections.insert(h2);

    MockFunction<void(int)> check;
    {
        InSequence s;
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketDisconnectEvent>(e))
            {
                return false;
            }
            return absl::get<Events::SocketDisconnectEvent>(e).GetConnection().lock() == h1.lock();
        })));
        EXPECT_CALL(check, Call(1));
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketDisconnectEvent>(e))
            {
                return false;
            }
            return absl::get<Events::SocketDisconnectEvent>(e).GetConnection().lock() == h2.lock();
        })));
        EXPECT_CALL(check, Call(2));
    }

    WSOnClose(h1);
    EXPECT_THAT(connections, UnorderedElementsAre(Truly([&](auto p) { return p.lock() == h2.lock(); })));
    check.Call(1);
    WSOnClose(h2);
    EXPECT_THAT(connections, IsEmpty());
    check.Call(2);
}

TEST_F(WebsocketCommunicationTest, OnMessage)
{
    using namespace ::testing;
    using EventVariant = WebsocketCommunication::EventVariant;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockWebsocketServer::connection_ptr p = std::make_shared<MockWebsocketConnection>();
    MockWebsocketServer::connection_hdl h = p;

    MockWebsocketServer::message_ptr msg = std::make_shared<MockWebsocketMessage>();

    MockFunction<PostEventState(const EventVariant&)> eventHandler;
    ON_CALL(eventHandler, Call(_)).WillByDefault(Return(PostEventState::handled));

    std::string msgPayload = "\"test\"";
    std::string parsedMsgPayload = "test";
    EXPECT_CALL(*msg, get_payload()).WillRepeatedly(ReturnRef(msgPayload));
    {
        // Test without authentication
        ws.AddEventHandler(eventHandler.AsStdFunction());
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketMessageEvent>(e))
            {
                return false;
            }
            auto& casted = absl::get<Events::SocketMessageEvent>(e);
            return casted.GetConnection().lock() == h.lock() && (&casted.GetMessage()) == msg.get()
                && !casted.GetUser().has_value() && casted.GetJsonPayload() == parsedMsgPayload;
        })));
        WSOnMessage(h, msg);

        Mock::VerifyAndClearExpectations(&eventHandler);
    }
    {
        // Test with authentication
        Authenticator a;
        WSSetAuthenticator(&a);
        const int userId = 1;
        nlohmann::json payload{{"message", "msg"},
            {"idToken", a.CreateJWTToken(nlohmann::json{{"iss", "test"}, {"sub", std::to_string(userId)}})}};
        auto authenticatedMsg = std::make_shared<MockWebsocketMessage>(payload);

        InSequence s;
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketMessageEvent>(e))
            {
                return false;
            }
            auto& casted = absl::get<Events::SocketMessageEvent>(e);
            return casted.GetConnection().lock() == h.lock() && (&casted.GetMessage()) == msg.get()
                && !casted.GetUser().has_value() && casted.GetJsonPayload() == parsedMsgPayload;
        })));
        WSOnMessage(h, msg);
        EXPECT_CALL(eventHandler, Call(Truly([&](const EventVariant& e) {
            if (!absl::holds_alternative<Events::SocketMessageEvent>(e))
            {
                return false;
            }
            auto& casted = absl::get<Events::SocketMessageEvent>(e);
            return casted.GetConnection().lock() == h.lock() && (&casted.GetMessage()) == authenticatedMsg.get()
                && casted.GetUser().value().IActuallyReallyNeedTheIntegerNow() == userId
                && casted.GetJsonPayload() == payload;
        })));
        WSOnMessage(h, authenticatedMsg);

        Mock::VerifyAndClearExpectations(&eventHandler);
    }
}

TEST_F(WebsocketCommunicationTest, Run)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockWebsocketServer& server = GetServer();

    // Normal
    {
        InSequence s;
        EXPECT_CALL(server, set_reuse_addr(true));
        EXPECT_CALL(server, set_message_handler(NotNull()));
        EXPECT_CALL(server, set_open_handler(NotNull()));
        EXPECT_CALL(server, set_close_handler(NotNull()));
        EXPECT_CALL(server, listen(9002));
        EXPECT_CALL(server, start_accept());

        EXPECT_CALL(server, run());

        WSRun();
    }
    // Exception is re-thrown to terminate the thread
    {
        InSequence s;
        EXPECT_CALL(server, set_reuse_addr(true));
        EXPECT_CALL(server, set_message_handler(NotNull()));
        EXPECT_CALL(server, set_open_handler(NotNull()));
        EXPECT_CALL(server, set_close_handler(NotNull()));
        EXPECT_CALL(server, listen(9002));
        EXPECT_CALL(server, start_accept());

        EXPECT_CALL(server, run()).WillOnce(Throw(std::exception()));

        EXPECT_THROW(WSRun(), std::exception);
    }
}

TEST_F(WebsocketCommunicationTest, Stop)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockWebsocketServer& server = GetServer();

    std::set<connection_hdl, std::owner_less<connection_hdl>>& connections = GetConnections();
    auto p1 = std::make_shared<MockWebsocketConnection>();
    MockWebsocketServer::connection_hdl h1 = p1;
    auto p2 = std::make_shared<MockWebsocketConnection>();
    MockWebsocketServer::connection_hdl h2 = p2;

    connections.insert(h1);
    connections.insert(h2);

    // Normal
    {
        EXPECT_CALL(server, stop_listening());

        EXPECT_CALL(server,
            close(Truly([&](auto p) { return p.lock() == h1.lock(); }), websocketpp::close::status::going_away,
                "Server shutdown"));
        EXPECT_CALL(server,
            close(Truly([&](auto p) { return p.lock() == h2.lock(); }), websocketpp::close::status::going_away,
                "Server shutdown"));

        ws.Stop();
        Mock::VerifyAndClearExpectations(&server);
    }
    // Connection throws, not forwarded
    {
        EXPECT_CALL(server, stop_listening());

        EXPECT_CALL(server,
            close(Truly([&](auto p) { return p.lock() == h1.lock(); }), websocketpp::close::status::going_away,
                "Server shutdown"))
            .WillOnce(Throw(websocketpp::exception("test")));
        EXPECT_CALL(server,
            close(Truly([&](auto p) { return p.lock() == h2.lock(); }), websocketpp::close::status::going_away,
                "Server shutdown"))
            .WillOnce(Throw(websocketpp::exception("test")));

        EXPECT_NO_THROW(ws.Stop());
        Mock::VerifyAndClearExpectations(&server);
    }
}
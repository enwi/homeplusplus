#include <gtest/gtest.h>

#include "TestWebsocketCommunication.h"

#include "../mocks/MockWebsocketServer.h"
#include "api/Resources.h"
#include "communication/WebsocketChannel.h"
#include "utility/Logger.h"

TEST(WebsocketChannel, GetName)
{
    using namespace ::testing;

    WebsocketCommunication wc {nullptr};

    {
        WebsocketChannel c(wc, "channel1", WebsocketChannel::RequireAuth::noAuth);
        EXPECT_EQ(c.GetName(), "channel1");
    }
    {
        WebsocketChannel c(wc, "", WebsocketChannel::RequireAuth::noAuth);
        EXPECT_EQ(c.GetName(), "");
    }
}

TEST(WebsocketChannel, HasSubscribers)
{
    using namespace ::testing;

    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    WebsocketCommunication wc {nullptr};
    WebsocketChannel channel(wc, "", WebsocketChannel::RequireAuth::noAuth);

    EXPECT_FALSE(channel.HasSubscribers());

    auto c1 = std::make_shared<MockWebsocketConnection>();
    auto c2 = std::make_shared<MockWebsocketConnection>();

    channel.Subscribe(c1);
    EXPECT_TRUE(channel.HasSubscribers());
    channel.Subscribe(c2);
    EXPECT_TRUE(channel.HasSubscribers());
    channel.Unsubscribe(c1);
    EXPECT_TRUE(channel.HasSubscribers());
    channel.Unsubscribe(c2);
    EXPECT_FALSE(channel.HasSubscribers());
}

TEST(WebsocketChannel, AddEventHandler)
{
    using namespace ::testing;
    using EventVariant = WebsocketChannel::EventVariant;

    WebsocketCommunication wc {nullptr};
    const std::string channelName = "channel1";
    WebsocketChannel channel(wc, channelName, WebsocketChannel::RequireAuth::noAuth);

    MockFunction<PostEventState(const EventVariant&, WebsocketChannel&)> eventHandler;
    ON_CALL(eventHandler, Call(_, _)).WillByDefault(Return(PostEventState::handled));

    channel.AddEventHandler(eventHandler.AsStdFunction());

    EXPECT_CALL(eventHandler, Call(_, Ref(channel)));
    nlohmann::json payload = {{"channel", channelName}, {"data", "test"}};
    channel.OnChannelEvent(Events::SocketMessageEvent(WebsocketCommunication::connection_hdl(),
        std::make_shared<MockWebsocketMessage>(payload), payload, absl::nullopt));
}

TEST(WebsocketChannel, Send)
{
    using namespace ::testing;

    TestWebsocketCommunication testWC;
    const std::string channelName = "channel1";
    WebsocketChannel channel(testWC.ws, channelName, WebsocketChannel::RequireAuth::noAuth);

    nlohmann::json message {{"test", "value"}, {"other", "data"}};
    nlohmann::json expected = message;
    expected["channel"] = channelName;

    auto connection = std::make_shared<MockWebsocketConnection>();
    WebsocketChannel::connection_hdl hdl = connection;

    EXPECT_CALL(testWC.GetServer(),
        send(Truly([&](WebsocketChannel::connection_hdl h) { return hdl.lock() == h.lock(); }), expected.dump(),
            websocketpp::frame::opcode::text));

    channel.Send(hdl, message);
}

TEST(WebsocketChannel, SendBytes)
{
    using namespace ::testing;

    TestWebsocketCommunication testWC;
    const std::string channelName = "channel1";
    WebsocketChannel channel(testWC.ws, channelName, WebsocketChannel::RequireAuth::noAuth);

    std::vector<char> data = {0x12, 0x25, 0x52, 0x35, 0x15, 0x60};

    auto connection = std::make_shared<MockWebsocketConnection>();
    WebsocketChannel::connection_hdl hdl = connection;

    EXPECT_CALL(testWC.GetServer(),
        send(Truly([&](WebsocketChannel::connection_hdl h) { return hdl.lock() == h.lock(); }), data.data(),
            data.size(), websocketpp::frame::opcode::binary));

    channel.SendBytes(hdl, data.data(), data.size());
}

TEST(WebsocketChannel, Broadcast)
{
    using namespace ::testing;

    TestWebsocketCommunication testWC;
    const std::string channelName = "channel1";
    WebsocketChannel channel(testWC.ws, channelName, WebsocketChannel::RequireAuth::noAuth);

    nlohmann::json message {{"test", "value"}, {"other", "data"}};
    nlohmann::json expected = message;
    expected["channel"] = channelName;

    auto c1 = std::make_shared<MockWebsocketConnection>();
    auto c2 = std::make_shared<MockWebsocketConnection>();
    {

        EXPECT_CALL(testWC.GetServer(), send(_, _, _, _)).Times(0);
        channel.Broadcast(message);
        Mock::VerifyAndClearExpectations(&testWC.GetServer());
    }

    channel.Subscribe(c1);
    {
        EXPECT_CALL(testWC.GetServer(),
            send(Truly([&](WebsocketChannel::connection_hdl h) { return h.lock() == c1; }), expected.dump(),
                websocketpp::frame::opcode::text));
        channel.Broadcast(message);
        Mock::VerifyAndClearExpectations(&testWC.GetServer());
    }
    channel.Subscribe(c2);
    {
        EXPECT_CALL(testWC.GetServer(),
            send(Truly([&](WebsocketChannel::connection_hdl h) { return h.lock() == c1; }), expected.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_CALL(testWC.GetServer(),
            send(Truly([&](WebsocketChannel::connection_hdl h) { return h.lock() == c2; }), expected.dump(),
                websocketpp::frame::opcode::text));
        channel.Broadcast(message);
        Mock::VerifyAndClearExpectations(&testWC.GetServer());
    }
    channel.Unsubscribe(c1);
    {
        EXPECT_CALL(testWC.GetServer(),
            send(Truly([&](WebsocketChannel::connection_hdl h) { return h.lock() == c2; }), expected.dump(),
                websocketpp::frame::opcode::text));
        channel.Broadcast(message);
        Mock::VerifyAndClearExpectations(&testWC.GetServer());
    }
}

TEST(WebsocketChannel, SubscribeUnsubscribe)
{
    using namespace ::testing;
    using EventVariant = WebsocketChannel::EventVariant;

    WebsocketCommunication wc {nullptr};
    const std::string channelName = "channel1";
    WebsocketChannel channel(wc, channelName, WebsocketChannel::RequireAuth::noAuth);

    MockFunction<PostEventState(const EventVariant&, WebsocketChannel&)> eventHandler;
    ON_CALL(eventHandler, Call(_, _)).WillByDefault(Return(PostEventState::handled));

    channel.AddEventHandler(eventHandler.AsStdFunction());

    auto c1 = std::make_shared<MockWebsocketConnection>();

    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketConnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));

    channel.Subscribe(c1);

    Mock::VerifyAndClearExpectations(&eventHandler);

    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketDisconnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));

    channel.Unsubscribe(c1);
}

TEST(WebsocketChannel, OnChannelEvent)
{
    using namespace ::testing;
    using EventVariant = WebsocketChannel::EventVariant;

    WebsocketCommunication wc {nullptr};
    WebsocketChannel channel(wc, "channel1", WebsocketChannel::RequireAuth::noAuth);

    MockFunction<PostEventState(const EventVariant&, WebsocketChannel&)> eventHandler;
    ON_CALL(eventHandler, Call(_, _)).WillByDefault(Return(PostEventState::handled));

    channel.AddEventHandler(eventHandler.AsStdFunction());

    auto c1 = std::make_shared<MockWebsocketConnection>();

    // Subscribe message properly subscribes channel
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketConnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    nlohmann::json payload1 {{"command", "subscribeChannel"}};
    channel.OnChannelEvent(
        Events::SocketMessageEvent(c1, std::make_shared<MockWebsocketMessage>(payload1), payload1, absl::nullopt));

    EXPECT_TRUE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&eventHandler);

    // Unsubscribe message properly unsubscribes channel
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketDisconnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    nlohmann::json payload2 {{"command", "unsubscribeChannel"}};
    channel.OnChannelEvent(
        Events::SocketMessageEvent(c1, std::make_shared<MockWebsocketMessage>(payload2), payload2, absl::nullopt));

    EXPECT_FALSE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&eventHandler);
    // Suppress warning about uninteresting call
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketConnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    channel.Subscribe(c1);
    // Disconnect properly unsubscribes channel
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketDisconnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));

    channel.OnChannelEvent(Events::SocketDisconnectEvent(c1, absl::nullopt));
    EXPECT_FALSE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&eventHandler);

    // Disconnect on not subscribed client does not trigger an event
    EXPECT_CALL(eventHandler, Call(_, _)).Times(0);

    channel.OnChannelEvent(Events::SocketDisconnectEvent(c1, absl::nullopt));
}

TEST(WebsocketChannel, AuthenticatedChannelEvent)
{
    using namespace ::testing;
    using EventVariant = WebsocketChannel::EventVariant;

    TestWebsocketCommunication wc;
    WebsocketChannel channel(wc.ws, "channel1", WebsocketChannel::RequireAuth::auth);

    MockFunction<PostEventState(const EventVariant&, WebsocketChannel&)> eventHandler;
    ON_CALL(eventHandler, Call(_, _)).WillByDefault(Return(PostEventState::handled));

    channel.AddEventHandler(eventHandler.AsStdFunction());

    auto c1 = std::make_shared<MockWebsocketConnection>();

    // Authenticated subscribe message properly subscribes channel
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketConnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    nlohmann::json subscribePayload {{"command", "subscribeChannel"}};
    channel.OnChannelEvent(Events::SocketMessageEvent(
        c1, std::make_shared<MockWebsocketMessage>(subscribePayload), subscribePayload, UserId::Dummy()));

    EXPECT_TRUE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&eventHandler);

    // Authenticated unsubscribe message properly unsubscribes channel
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketDisconnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    nlohmann::json unsubscribePayload {{"command", "unsubscribeChannel"}};
    channel.OnChannelEvent(Events::SocketMessageEvent(
        c1, std::make_shared<MockWebsocketMessage>(unsubscribePayload), unsubscribePayload, UserId::Dummy()));

    EXPECT_FALSE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&eventHandler);
    // Suppress warning about uninteresting call
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketConnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    channel.Subscribe(c1);
    // Disconnect properly unsubscribes channel (event unauthenticated)
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketDisconnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));

    channel.OnChannelEvent(Events::SocketDisconnectEvent(c1, absl::nullopt));
    EXPECT_FALSE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&eventHandler);

    std::string error = nlohmann::json {{"error", "notAuthenticated"}}.dump();

    // Unauthenticated subscribe message triggers error
    EXPECT_CALL(eventHandler, Call(_, _)).Times(0);
    EXPECT_CALL(wc.GetServer(),
        send(Truly([&](const WebsocketChannel::connection_hdl c) { return c.lock() == c1; }), error,
            websocketpp::frame::opcode::text));
    channel.OnChannelEvent(Events::SocketMessageEvent(
        c1, std::make_shared<MockWebsocketMessage>(subscribePayload), subscribePayload, absl::nullopt));

    EXPECT_FALSE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&wc.GetServer());
    Mock::VerifyAndClearExpectations(&eventHandler);

    // Unauthenticated unsubscribe message triggers error
    // Suppress warning about uninteresting call
    EXPECT_CALL(eventHandler,
        Call(Truly([&](const EventVariant& v) {
            return absl::get<Events::SocketConnectEvent>(v).GetConnection().lock() == c1;
        }),
            Ref(channel)));
    channel.Subscribe(c1);
    EXPECT_CALL(eventHandler, Call(_, _)).Times(0);
    EXPECT_CALL(wc.GetServer(),
        send(Truly([&](const WebsocketChannel::connection_hdl c) { return c.lock() == c1; }), error,
            websocketpp::frame::opcode::text));
    channel.OnChannelEvent(Events::SocketMessageEvent(
        c1, std::make_shared<MockWebsocketMessage>(unsubscribePayload), unsubscribePayload, absl::nullopt));
    EXPECT_TRUE(channel.HasSubscribers());

    Mock::VerifyAndClearExpectations(&wc.GetServer());
}
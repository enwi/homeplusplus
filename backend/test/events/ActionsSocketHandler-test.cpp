#include <gtest/gtest.h>

#include "../communication/TestWebsocketCommunication.h"
#include "../mocks/MockActionImpl.h"
#include "../mocks/MockActionSerialize.h"
#include "../mocks/MockDeviceSerialize.h"
#include "api/DeviceRegistry.h"
#include "database/DBDeviceSerialize.h"
#include "events/ActionsSocketHandler.h"

class ActionsSocketHandlerTest : public ::testing::Test
{
public:
    ActionsSocketHandlerTest()
        : storage(actionSerialize, eventEmitter),
          channel(websocket.ws, "actions", WebsocketChannel::RequireAuth::noAuth),
          deviceReg(deviceSer, deviceEvents, propertyEvents),
          handler(actionReg, storage, deviceReg, WebsocketChannelAccessor(websocket.ws, "notifications"))
    {
        websocket.ws.AddChannel(WebsocketChannel(websocket.ws, "notifications", WebsocketChannel::RequireAuth::noAuth));
        eventEmitter.AddHandler(eventHandler.AsStdFunction());
        Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
        Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    }

    MockActionSerialize actionSerialize;
    TestWebsocketCommunication websocket;
    WebsocketChannel channel;
    ::testing::MockFunction<PostEventState(const Events::ActionChangeEvent&)> eventHandler;
    EventEmitter<Events::ActionChangeEvent> eventEmitter;
    EventEmitter<Events::DeviceChangeEvent> deviceEvents;
    EventEmitter<Events::DevicePropertyChangeEvent> propertyEvents;
    SubActionRegistry actionReg;
    ActionStorage storage;
    MockDeviceSerialize deviceSer;
    DeviceRegistry deviceReg;
    ActionsSocketHandler handler;
};

TEST_F(ActionsSocketHandlerTest, CallOperator)
{
    EXPECT_EQ(PostEventState::notHandled,
        handler(Events::SocketConnectEvent(websocketpp::connection_hdl(), absl::nullopt), channel));
    EXPECT_EQ(PostEventState::notHandled,
        handler(Events::SocketDisconnectEvent(websocketpp::connection_hdl(), absl::nullopt), channel));
    EXPECT_EQ(PostEventState::notHandled,
        handler(
            Events::SocketMessageEvent(websocketpp::connection_hdl(), nullptr, {{"no_command", "t"}}, absl::nullopt),
            channel));
}

TEST_F(ActionsSocketHandlerTest, AddAction)
{
    using namespace ::testing;
    {
        // missing actionJSON
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_addAction}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    {
        // invalid actionJSON
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_addAction}, {"actionJSON", "asdgioje"}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    {
        // valid actionJSON, invalid user
        ::Action action{35234, "test action", "icon", 0x02345, {}};
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_addAction}, {"actionJSON", action.ToJson()}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    {
        // valid actionJSON
        ::Action action{35234, "test action", "icon", 0x02345, {}};
        UserId user{0x2346324};
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_addAction}, {"actionJSON", action.ToJson()}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, user};
        InSequence s;
        // Return same action id to simplify event matching
        EXPECT_CALL(
            actionSerialize, AddAction(Truly([&](const ::Action& a) { return a.ToJson() == action.ToJson(); }), user))
            .WillOnce(Return(action.GetId()));
        EXPECT_CALL(eventHandler, Call(Truly([&](const Events::ActionChangeEvent& e) {
            return e.GetOld().GetId() == 0 && e.GetChanged().ToJson() == action.ToJson()
                && e.GetChangedFields() == Events::ActionFields::ADD;
        })));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(ActionsSocketHandlerTest, GetActions)
{
    using namespace ::testing;
    using Action = ::Action;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const nlohmann::json payload{{"command", ActionsSocketHandler::s_getActions}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const UserId user{0x2689267};
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // empty list
        const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"actions", nlohmann::json::array()}};
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAllActions(_, user)).WillOnce(Return(std::vector<Action>()));
        EXPECT_CALL(websocket.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&actionSerialize);
    Mock::VerifyAndClearExpectations(&websocket.GetServer());
    {
        // single action
        Action result{0x243646, "test action", "icon", 0x251, {}};
        const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"action", result.ToJson()}};
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAllActions(_, user)).WillOnce(Return(std::vector<Action>{result}));
        EXPECT_CALL(websocket.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&actionSerialize);
    Mock::VerifyAndClearExpectations(&websocket.GetServer());
    {
        // multiple actions
        std::vector<Action> result{{0x243646, "test action", "icon", 0x251, {}},
            {0x15, "test action2", "icon2", 0x168961, {}}, {0x235, "test action3", "agsf", 196, {}, false}};
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAllActions(_, user)).WillOnce(Return(result));
        for (const Action& a : result)
        {
            const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"action", a.ToJson()}};
            EXPECT_CALL(websocket.GetServer(),
                send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }),
                    expectedResponse.dump(), websocketpp::frame::opcode::text));
        }
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(ActionsSocketHandlerTest, GetAction)
{
    using namespace ::testing;
    using Action = ::Action;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const UserId user{0x2689267};
    {
        // missing id
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_getAction}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const uint64_t actionId = 3;
    const nlohmann::json payload{{"command", ActionsSocketHandler::s_getAction}, {"id", actionId}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // action found
        Action action{actionId, "name", "icon", 1253, {}};
        const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"action", action.ToJson()}};
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAction(actionId, user)).WillOnce(Return(action));
        EXPECT_CALL(websocket.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&actionSerialize);
    Mock::VerifyAndClearExpectations(&websocket.GetServer());
    {
        // action not found
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAction(actionId, user)).WillOnce(Return(absl::nullopt));
        EXPECT_CALL(websocket.GetServer(), send(_, _, _)).Times(0);
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(ActionsSocketHandlerTest, DeleteAction)
{
    using namespace ::testing;
    using Action = ::Action;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const UserId user{0x2689267};
    {
        // missing id
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_deleteAction}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const uint64_t actionId = 3;
    const nlohmann::json payload{{"command", ActionsSocketHandler::s_deleteAction}, {"id", actionId}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // action found
        Action action{actionId, "name", "icon", 1253, {}};
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAction(actionId, user)).WillOnce(Return(action));
        EXPECT_CALL(actionSerialize, RemoveAction(actionId, user));
        EXPECT_CALL(eventHandler, Call(Truly([&](const Events::ActionChangeEvent& e) {
            return e.GetOld().GetId() == actionId && e.GetChangedFields() == Events::ActionFields::REMOVE;
        })));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&actionSerialize);
    Mock::VerifyAndClearExpectations(&eventHandler);
    {
        // action not found
        InSequence s;
        EXPECT_CALL(actionSerialize, GetAction(actionId, user)).WillOnce(Return(absl::nullopt));
        EXPECT_CALL(actionSerialize, RemoveAction(_, Matcher<UserId>(_))).Times(0);
        EXPECT_CALL(eventHandler, Call(_)).Times(0);
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(ActionsSocketHandlerTest, ExecAction)
{
    using namespace ::testing;
    using Action = ::Action;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const UserId user{235};
    // missing ID
    {
        const nlohmann::json payload{{"command", ActionsSocketHandler::s_execAction}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const uint64_t actionId = 3;
    const nlohmann::json payload{{"command", ActionsSocketHandler::s_execAction}, {"id", actionId}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // action not found
        EXPECT_CALL(actionSerialize, GetAction(actionId, user)).WillOnce(Return(absl::nullopt));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&actionSerialize);
    {
        // action found
        const std::size_t type = 1;
        auto subAction = std::make_unique<MockActionImpl>(type);
        EXPECT_CALL(*subAction, Execute(_, _, _, user, 0));
        Action action{actionId, "action", "icon", 0x1253, {SubAction(std::move(subAction))}};
        EXPECT_CALL(actionSerialize, GetAction(actionId, user)).WillOnce(Return(action));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(ActionsSocketHandlerTest, GetActionTypes)
{
    using namespace ::testing;
    using Action = ::Action;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    actionReg.RegisterDefaultSubActions();
    const nlohmann::json payload{{"command", ActionsSocketHandler::s_getActionTypes}};
    const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
    nlohmann::json actionTypes = nlohmann::json::array();
    const std::vector<SubActionInfo>& registered = actionReg.GetRegistered();
    for (std::size_t i = 0; i < registered.size(); ++i)
    {
        if (registered[i] != nullptr)
        {
            actionTypes.push_back(nlohmann::json{{"id", i}, {"name", registered[i].name}});
        }
    }
    const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"actionTypes", actionTypes}};
    EXPECT_CALL(websocket.GetServer(),
        send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
            websocketpp::frame::opcode::text));
    EXPECT_EQ(PostEventState::handled, handler(event, channel));
}

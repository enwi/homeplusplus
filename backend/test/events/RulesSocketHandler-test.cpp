#include <gtest/gtest.h>

#include "../communication/TestWebsocketCommunication.h"
#include "../mocks/MockRuleSerialize.h"
#include "events/RulesSocketHandler.h"

class RulesSocketHandlerTest : public ::testing::Test
{
public:
    RulesSocketHandlerTest()
        : storage(ruleSerialize, eventEmitter),
          channel(websocket.ws, "rules", WebsocketChannel::RequireAuth::noAuth),
          handler(storage)
    {
        eventEmitter.AddHandler(eventHandler.AsStdFunction());
        Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
        Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
        Res::ConditionRegistry().RegisterDefaultConditions();
    }

    MockRuleSerialize ruleSerialize;
    TestWebsocketCommunication websocket;
    WebsocketChannel channel;
    ::testing::MockFunction<PostEventState(const Events::RuleChangeEvent&)> eventHandler;
    EventEmitter<Events::RuleChangeEvent> eventEmitter;
    RuleStorage storage;
    RulesSocketHandler handler;
};

TEST_F(RulesSocketHandlerTest, CallOperator)
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

TEST_F(RulesSocketHandlerTest, AddRule)
{
    using namespace ::testing;
    {
        // missing ruleJSON
        const nlohmann::json payload{{"command", RulesSocketHandler::s_addRule}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    {
        // invalid ruleJSON
        const nlohmann::json payload{{"command", RulesSocketHandler::s_addRule}, {"ruleJSON", "asdgioje"}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    {
        // valid ruleJSON, invalid user
        Rule rule{35234, "test rule", "icon", 0x02345, {}, ::Action{125, "test rule", "icon", 8916, {}}};
        const nlohmann::json payload{{"command", RulesSocketHandler::s_addRule}, {"ruleJSON", rule.ToJson()}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    {
        // valid ruleJSON
        Rule rule{35234, "test rule", "icon", 0x02345, Res::ConditionRegistry().GetCondition(0),
            ::Action{125, "test rule", "icon", 8916, {}}};
        UserId user{0x2346324};
        const nlohmann::json payload{{"command", RulesSocketHandler::s_addRule}, {"ruleJSON", rule.ToJson()}};
        const Events::SocketMessageEvent event{websocketpp::connection_hdl(), nullptr, payload, user};
        InSequence s;
        EXPECT_CALL(ruleSerialize, AddRule(Truly([&](const Rule& r) { return r.ToJson() == rule.ToJson(); }), user));
        EXPECT_CALL(eventHandler, Call(Truly([&](const Events::RuleChangeEvent& e) {
            return e.GetOld().GetId() == 0 && e.GetChanged().ToJson() == rule.ToJson()
                && e.GetChangedFields() == Events::RuleFields::ADD;
        })));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(RulesSocketHandlerTest, GetRules)
{
    using namespace ::testing;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const nlohmann::json payload{{"command", RulesSocketHandler::s_getRules}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const UserId user{0x3618912};
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // empty list
        const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"rules", nlohmann::json::array()}};
        InSequence s;
        EXPECT_CALL(ruleSerialize, GetAllRules(_, user)).WillOnce(Return(std::vector<Rule>()));
        EXPECT_CALL(websocket.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&ruleSerialize);
    Mock::VerifyAndClearExpectations(&websocket.GetServer());
    {
        // single rule
        Rule result{35234, "test rule", "icon", 0x02345, Res::ConditionRegistry().GetCondition(0),
            ::Action{125, "test rule", "icon", 8916, {}}};
        const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"rule", result.ToJson()}};
        InSequence s;
        EXPECT_CALL(ruleSerialize, GetAllRules(_, user)).WillOnce(Return(std::vector<Rule>{result}));
        EXPECT_CALL(websocket.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&ruleSerialize);
    Mock::VerifyAndClearExpectations(&websocket.GetServer());
    {
        std::vector<Rule> result{{35234, "test rule", "icon", 0x02345, Res::ConditionRegistry().GetCondition(0),
                                     ::Action{125, "test rule", "icon", 8916, {}}},
            {28946, "test rule 2", "icon2", 0x6892, Res::ConditionRegistry().GetCondition(0),
                ::Action{125, "test rule2", "icon", 32152, {}}}};
        InSequence s;
        EXPECT_CALL(ruleSerialize, GetAllRules(_, user)).WillOnce(Return(result));
        for (const Rule& r : result)
        {
            const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"rule", r.ToJson()}};
            EXPECT_CALL(websocket.GetServer(),
                send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }),
                    expectedResponse.dump(), websocketpp::frame::opcode::text));
        }
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(RulesSocketHandlerTest, GetRule)
{
    using namespace ::testing;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const UserId user{0x4186934};
    {
        // missing id
        const nlohmann::json payload{{"command", RulesSocketHandler::s_getRule}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const uint64_t ruleId = 6344;
    const nlohmann::json payload{{"command", RulesSocketHandler::s_getRule}, {"ruleId", ruleId}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // rule found
        Rule rule{ruleId, "name", "icon", 990629, Res::ConditionRegistry().GetCondition(0),
            ::Action{892634, "name", "icon", 89, {}}};
        const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"rule", rule.ToJson()}};
        InSequence s;
        EXPECT_CALL(ruleSerialize, GetRule(ruleId, user)).WillOnce(Return(rule));
        EXPECT_CALL(websocket.GetServer(),
            send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
                websocketpp::frame::opcode::text));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
    Mock::VerifyAndClearExpectations(&ruleSerialize);
    Mock::VerifyAndClearExpectations(&websocket.GetServer());
    {
        // rule not found
        InSequence s;
        EXPECT_CALL(ruleSerialize, GetRule(ruleId, user)).WillOnce(Return(absl::nullopt));
        EXPECT_CALL(websocket.GetServer(), send(_, _, _)).Times(0);
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(RulesSocketHandlerTest, RemoveRule)
{
    using namespace ::testing;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const UserId user{0x4186934};
    {
        // missing id
        const nlohmann::json payload{{"command", RulesSocketHandler::s_removeRule}};
        const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const uint64_t ruleId = 3;
    const nlohmann::json payload{{"command", RulesSocketHandler::s_removeRule}, {"ruleId", ruleId}};
    {
        // missing user
        const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
        EXPECT_EQ(PostEventState::error, handler(event, channel));
    }
    const Events::SocketMessageEvent event{hdl, nullptr, payload, user};
    {
        // correct request
        InSequence s;
        EXPECT_CALL(ruleSerialize, RemoveRule(ruleId, user));
        EXPECT_CALL(eventHandler, Call(Truly([&](const Events::RuleChangeEvent& e) {
            return e.GetOld().GetId() == ruleId && e.GetChangedFields() == Events::RuleFields::REMOVE;
        })));
        EXPECT_EQ(PostEventState::handled, handler(event, channel));
    }
}

TEST_F(RulesSocketHandlerTest, GetConditionTypes)
{
    using namespace ::testing;
    auto connection = std::make_shared<MockWebsocketConnection>();
    websocketpp::connection_hdl hdl = connection;
    const nlohmann::json payload{{"command", RulesSocketHandler::s_getConditionTypes}};
    const Events::SocketMessageEvent event{hdl, nullptr, payload, absl::nullopt};
    nlohmann::json conditionTypes = nlohmann::json::array();
    const std::vector<RuleConditions::RuleConditionInfo>& registered = Res::ConditionRegistry().GetRegistered();
    for (std::size_t i = 0; i < registered.size(); ++i)
    {
        if (registered[i] != nullptr)
        {
            conditionTypes.push_back(nlohmann::json{{"id", i}, {"name", registered[i].name}});
        }
    }
    const nlohmann::json expectedResponse{{"channel", channel.GetName()}, {"conditionTypes", conditionTypes}};
    EXPECT_CALL(websocket.GetServer(),
        send(Truly([&](websocketpp::connection_hdl h) { return h.lock() == connection; }), expectedResponse.dump(),
            websocketpp::frame::opcode::text));
	EXPECT_EQ(PostEventState::handled, handler(event, channel));
}
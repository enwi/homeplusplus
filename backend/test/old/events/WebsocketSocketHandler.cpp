#include "events/WebsocketSocketHandler.h"

#include <gtest/gtest.h>

#include "../communication/TestWebsocketCommunication.h"
#include "../mocks/MockActionImpl.h"
#include "../mocks/MockActionSerialize.h"
#include "../mocks/MockDBHandler.h"
#include "../mocks/MockDevice.h"
#include "../mocks/MockDeviceAPI.h"
#include "../mocks/MockEventHandler.h"
#include "../mocks/MockNodeCommunication.h"
#include "../mocks/MockNodeManager.h"
#include "../mocks/MockNodeSerialize.h"
#include "../mocks/MockRuleSerialize.h"
#include "../mocks/MockWebsocketServer.h"
#include "api/DeviceAPI.h"
#include "api/DeviceRegistry.h"
#include "database/DBActionSerialize.h"
#include "database/DBNodeSerialize.h"
#include "database/DBRuleSerialize.h"

TEST(WebsocketSocketHandler, ProcessMessageAddActor)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    {
        Actor a(23, 0, "name", "loc", 23, 4);
        nlohmann::json json = a.ToJson();
        const std::string msg = ws.s_addActor + ";23;"s + json.dump();

        EXPECT_CALL(nm, AddActor(23, a));

        ws.ProcessMessage(hdl, msg);
    }
    // NodeId parse error
    {
        Actor a(23, 0, "name", "loc", 23, 4);
        nlohmann::json json = a.ToJson();
        const std::string msg = ws.s_addActor + ";asdf;"s + json.dump();
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Actor parse error
    {
        const std::string msg = ws.s_addActor + ";23;"s + "asdfqwer189{";
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Actor missing
    {
        const std::string msg = ws.s_addActor + ";23"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_addActor + ";"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageAddSensor)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    {
        Sensor s(23, 0, "name", "loc", 23, 4, 3);
        nlohmann::json json = s.ToJson();
        const std::string msg = ws.s_addSensor + ";23;"s + json.dump();

        EXPECT_CALL(nm, AddSensor(23, s));

        ws.ProcessMessage(hdl, msg);
    }
    // NodeId parse error
    {
        Sensor s(23, 0, "name", "loc", 23, 4, 3);
        nlohmann::json json = s.ToJson();
        const std::string msg = ws.s_addSensor + ";asdf;"s + json.dump();
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Sensor parse error
    {
        const std::string msg = ws.s_addSensor + ";23;"s + "asdfqwer189{";
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Sensor missing
    {
        const std::string msg = ws.s_addSensor + ";23"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_addSensor + ";"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageRemoveSensor)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    {
        const std::string msg = ws.s_removeSensor + ";23;10"s;

        EXPECT_CALL(nm, RemoveSensor(23, 10));

        ws.ProcessMessage(hdl, msg);
    }
    // NodeId parse error
    {
        const std::string msg = ws.s_removeSensor + ";asdf;10"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // SensorId parse error
    {
        const std::string msg = ws.s_removeSensor + ";23;asdf"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // SensorId missing
    {
        const std::string msg = ws.s_removeSensor + ";23"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_removeSensor + ";"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageRemoveActor)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    {
        const std::string msg = ws.s_removeActor + ";23;10"s;

        EXPECT_CALL(nm, RemoveActor(23, 10));

        ws.ProcessMessage(hdl, msg);
    }
    // NodeId parse error
    {
        const std::string msg = ws.s_removeActor + ";asdf;10"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // ActorId parse error
    {
        const std::string msg = ws.s_removeActor + ";23;asdf"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // ActorId missing
    {
        const std::string msg = ws.s_removeActor + ";23"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_removeActor + ";"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageAddRule)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    auto eh = std::make_shared<MockEventHandler>();
    CleanupEventHandler ceh(Res::EventSystem(), eh);
    EXPECT_CALL(*eh, ShouldExecuteOn(_)).Times(AnyNumber());

    Res::EventSystem().AddHandler(eh);
    try
    {
        Res::ConditionRegistry().Register(
            RuleConditions::RuleConditionInfo{
                [](std::size_t i) { return std::make_unique<RuleConditions::RuleConstantCondition>(i); }, ""},
            3);
        Rule r{0, "n", "i", 234, std::make_unique<RuleConditions::RuleConstantCondition>(3, false),
            ::Action{0, "n", "i", 0234, {}, false}, true};
        nlohmann::json json = r.ToJson();
        std::ostringstream sstream;
        sstream << json;
        const std::string msg = ws.s_addRule + ";"s + sstream.str();

        EXPECT_CALL(*eh,
            HandleEvent(Truly([&](const EventBase& event) {
                if (event.GetType() != EventTypes::ruleChange)
                {
                    return false;
                }
                const Events::RuleChangeEvent& casted = dynamic_cast<const Events::RuleChangeEvent&>(event);
                return casted.GetChangedFields() == Events::RuleFields::ADD;
            }),
                Ref(Res::EventSystem())));

        ws.ProcessMessage(hdl, msg);

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(eh.get());
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    // Json parse error
    {
        const std::string msg = ws.s_addRule + ";asdf{2341o"s;
        EXPECT_CALL(*eh, HandleEvent(_, _)).Times(0);
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Rule parse error (valid json)
    {
        const std::string msg = ws.s_addRule + R"(;{ "i am": "valid" })"s;
        EXPECT_CALL(*eh, HandleEvent(_, _)).Times(0);
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_addRule + ";"s;
        EXPECT_CALL(*eh, HandleEvent(_, _)).Times(0);
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetRules)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // No result
    {
        const std::string msg = ws.s_getRules;

        EXPECT_CALL(rs, GetAllRules()).WillOnce(Return(std::vector<Rule>()));

        EXPECT_CALL(wc.GetServer(), send(_, "NOT_FOUND\n", _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&rs);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // One result
    {
        const std::string msg = ws.s_getRules;

        const std::vector<Rule> results
            = {Rule{3, "hi", "ic", 532, nullptr, ::Action(3, "n", "i", 432, {}, false), true}};

        EXPECT_CALL(rs, GetAllRules()).WillOnce(Return(results));

        EXPECT_CALL(wc.GetServer(), send(_, "Rule:" + results[0].ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&rs);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Multiple results
    {
        const std::string msg = ws.s_getRules;

        const std::vector<Rule> results
            = {Rule{3, "hi", "ic", 532, nullptr, ::Action(3, "n", "i", 432, {}, false), true},
                Rule{4, "a", "b", 3, nullptr, ::Action(4, "n", "i2", 23, {}, false), false}};

        InSequence s;
        EXPECT_CALL(rs, GetAllRules()).WillOnce(Return(results));

        EXPECT_CALL(wc.GetServer(), send(_, "Rule:" + results[0].ToJson().dump() + '\n', _));
        EXPECT_CALL(wc.GetServer(), send(_, "Rule:" + results[1].ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&rs);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetRule)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // Rule not found
    {
        const std::size_t ruleId = 23;
        const std::string msg = ws.s_getRule + ";"s + std::to_string(ruleId);

        EXPECT_CALL(rs, GetRule(ruleId)).WillOnce(Throw(std::out_of_range("test")));

        EXPECT_CALL(wc.GetServer(), send(_, _, _)).Times(0);

        // TODO: Change this to not throw
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);

        Mock::VerifyAndClearExpectations(&rs);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Argument missing
    {
        const std::string msg = ws.s_getRule;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Argument not convertible to int
    {
        const std::string msg = ws.s_getRule + ";asf23"s;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Rule found
    {
        const std::size_t ruleId = 23;
        const std::string msg = ws.s_getRule + ";"s + std::to_string(ruleId);

        Rule result{ruleId, "n", "i", 23, nullptr, ::Action(), true};

        EXPECT_CALL(rs, GetRule(ruleId)).WillOnce(Return(result));

        EXPECT_CALL(wc.GetServer(), send(_, "Rule:" + result.ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&rs);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageRemoveRule)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // Rule not found
    {
        const std::size_t ruleId = 23;
        const std::string msg = ws.s_removeRule + ";"s + std::to_string(ruleId);

        EXPECT_CALL(rs, GetRule(ruleId)).WillOnce(Throw(std::out_of_range("test")));

        // TODO: Change this to not throw
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);

        Mock::VerifyAndClearExpectations(&rs);
    }
    // Argument missing
    {
        const std::string msg = ws.s_removeRule;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Argument not convertible to int
    {
        const std::string msg = ws.s_removeRule + ";asf23"s;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Rule found
    {
        const std::size_t ruleId = 23;
        const std::string msg = ws.s_removeRule + ";"s + std::to_string(ruleId);

        Rule result{ruleId, "n", "i", 23, nullptr, ::Action(), true};

        auto eh = std::make_shared<MockEventHandler>();
        {
            InSequence s;
            EXPECT_CALL(rs, GetRule(ruleId)).WillOnce(Return(result));
            EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::ruleChange));
            EXPECT_CALL(*eh,
                HandleEvent(Truly([&](const EventBase& e) {
                    if (e.GetType() != EventTypes::ruleChange)
                    {
                        return false;
                    }
                    const auto& casted = dynamic_cast<const Events::RuleChangeEvent&>(e);
                    return casted.GetChangedFields() == Events::RuleFields::REMOVE
                        && casted.GetOld().ToJson() == result.ToJson()
                        && casted.GetChanged().ToJson() == Rule().ToJson();
                }),
                    Ref(Res::EventSystem())));
        }
        Res::EventSystem().AddHandler(eh);
        CleanupEventHandler cleanup(Res::EventSystem(), eh);

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&rs);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageSetActorValue)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Range;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    auto pApi = std::make_unique<MockDeviceAPI>();
    MockDeviceAPI& api = *pApi;
    reg.RegisterDeviceAPI(std::move(pApi));
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // Device not found
    {
        const std::size_t deviceId = 3;
        const std::size_t actorId = 1;
        const std::string value = "test";

        const std::string msg
            = ws.s_setActorValue + ";"s + std::to_string(deviceId) + ";" + std::to_string(actorId) + ";" + value;

        EXPECT_CALL(api, GetDevice(deviceId)).WillOnce(Return(ByMove(nullptr)));

        EXPECT_NO_THROW(ws.ProcessMessage(hdl, msg));

        Mock::VerifyAndClearExpectations(&api);
    }
    // No args
    {
        const std::string msg = ws.s_setActorValue;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Actor id missing
    {
        const std::size_t deviceId = 3;
        const std::string msg = ws.s_setActorValue + ";"s + std::to_string(deviceId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
    }
    // Value missing
    {
        const std::size_t deviceId = 3;
        const std::size_t actorId = 1;
        const std::string msg = ws.s_setActorValue + ";"s + std::to_string(deviceId) + ";" + std::to_string(actorId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
    }
    // Device not convertible
    {
        const std::size_t actorId = 1;
        const std::string value = "12";
        const std::string msg = ws.s_setActorValue + ";asdf;"s + std::to_string(actorId) + ";" + value;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Actor not convertible
    {
        const std::size_t deviceId = 3;
        const std::string value = "12";
        const std::string msg = ws.s_setActorValue + ";"s + std::to_string(deviceId) + ";asdf;" + value;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Actor not found (none)
    {
        const std::size_t deviceId = 3;
        const std::size_t actorId = 1;
        const std::string value = "test";

        const std::string msg
            = ws.s_setActorValue + ";"s + std::to_string(deviceId) + ";" + std::to_string(actorId) + ";" + value;

        auto result = std::make_unique<MockDevice>();
        MockDevice& dev = *result;

        EXPECT_CALL(api, GetDevice(deviceId)).WillOnce(Return(ByMove(std::move(result))));

        EXPECT_CALL(dev, GetActors()).WillOnce(Return(Range<IActor>()));

        EXPECT_NO_THROW(ws.ProcessMessage(hdl, msg));

        Mock::VerifyAndClearExpectations(&api);
    }
    // Actor not found (too few)
    {
        const std::size_t deviceId = 3;
        const std::size_t actorId = 1;
        const std::string value = "test";

        const std::string msg
            = ws.s_setActorValue + ";"s + std::to_string(deviceId) + ";" + std::to_string(actorId) + ";" + value;

        auto result = std::make_unique<MockDevice>();
        MockDevice& dev = *result;
        std::vector<Actor> actors = {Actor()};

        EXPECT_CALL(api, GetDevice(deviceId)).WillOnce(Return(ByMove(std::move(result))));

        EXPECT_CALL(dev, GetActors()).WillOnce(Return(Range<IActor>{actors.begin(), actors.end()}));

        EXPECT_NO_THROW(ws.ProcessMessage(hdl, msg));

        Mock::VerifyAndClearExpectations(&api);
    }
    // Actor found
    {
        const std::size_t deviceId = 3;
        const std::size_t actorId = 1;
        const std::string value = "test";

        const std::string msg
            = ws.s_setActorValue + ";"s + std::to_string(deviceId) + ";" + std::to_string(actorId) + ";" + value;

        auto result = std::make_unique<MockDevice>();
        MockDevice& dev = *result;
        std::vector<MockActor> actors(actorId + 1);

        EXPECT_CALL(api, GetDevice(deviceId)).WillOnce(Return(ByMove(std::move(result))));

        EXPECT_CALL(dev, GetActors()).WillOnce(Return(Range<IActor>{actors.begin(), actors.end()}));
        EXPECT_CALL(actors[actorId], Set(value));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&api);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetDevices)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Range;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    auto pApi = std::make_unique<MockDeviceAPI>();
    MockDeviceAPI& api = *pApi;
    reg.RegisterDeviceAPI(std::move(pApi));
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // No result
    {
        const std::string msg = ws.s_getDevices;

        EXPECT_CALL(api, GetDevices()).WillOnce(Return(Range<IDevice>{}));

        EXPECT_CALL(wc.GetServer(), send(_, "NOT_FOUND\n", _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // One result
    {
        const std::string msg = ws.s_getDevices;

        std::vector<NodeData> results = {NodeData{3, "hi", "ic", {}, {}, "s", NodePath{2, 3}}};

        EXPECT_CALL(api, GetDevices()).WillOnce(Return(Range<IDevice>{results.begin(), results.end()}));

        EXPECT_CALL(wc.GetServer(), send(_, "Node:" + results[0].ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Multiple results
    {
        const std::string msg = ws.s_getDevices;

        std::vector<NodeData> results
            = {NodeData{3, "hi", "ic", {}, {}, "s", NodePath{2, 3}}, NodeData{4, "a", "b", {}, {}, "c", NodePath{2}}};

        InSequence s;
        EXPECT_CALL(api, GetDevices()).WillOnce(Return(Range<IDevice>{results.begin(), results.end()}));

        EXPECT_CALL(wc.GetServer(), send(_, "Node:" + results[0].ToJson().dump() + '\n', _));
        EXPECT_CALL(wc.GetServer(), send(_, "Node:" + results[1].ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetDevice)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    auto pApi = std::make_unique<MockDeviceAPI>();
    MockDeviceAPI& api = *pApi;
    reg.RegisterDeviceAPI(std::move(pApi));
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // No result
    {
        const std::size_t deviceId = 3;
        const std::string msg = ws.s_getDevice + ";"s + std::to_string(deviceId);

        EXPECT_CALL(api, GetDevice(deviceId)).WillOnce(Return(ByMove(nullptr)));

        EXPECT_CALL(wc.GetServer(), send(_, "NOT_FOUND\n", _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Arg missing
    {
        const std::string msg = ws.s_getDevice;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Arg not convertible to int
    {
        const std::string msg = ws.s_getDevice + ";asf234"s;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Result
    {
        const std::size_t deviceId = 3;
        const std::string msg = ws.s_getDevice + ";"s + std::to_string(deviceId);

        NodeData result{3, "hi", "ic", {}, {}, "s", NodePath{2, 3}};

        EXPECT_CALL(api, GetDevice(deviceId)).WillOnce(Return(ByMove(std::make_unique<NodeData>(result))));

        EXPECT_CALL(wc.GetServer(), send(_, "Node:" + result.ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&api);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageAddAction)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Action;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    auto eh = std::make_shared<MockEventHandler>();
    CleanupEventHandler ceh(Res::EventSystem(), eh);
    EXPECT_CALL(*eh, ShouldExecuteOn(_)).Times(AnyNumber());

    Res::EventSystem().AddHandler(eh);
    try
    {
        Action a{0, "n", "i", 234, {}, true};
        nlohmann::json json = a.ToJson();
        std::ostringstream sstream;
        sstream << json;
        const std::string msg = ws.s_addAction + ";"s + sstream.str();

        EXPECT_CALL(*eh,
            HandleEvent(Truly([&](const EventBase& event) {
                if (event.GetType() != EventTypes::actionChange)
                {
                    return false;
                }
                const Events::ActionChangeEvent& casted = dynamic_cast<const Events::ActionChangeEvent&>(event);
                return casted.GetChangedFields() == Events::ActionFields::ADD;
            }),
                Ref(Res::EventSystem())));

        ws.ProcessMessage(hdl, msg);

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(eh.get());
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    // Json parse error
    {
        const std::string msg = ws.s_addAction + ";asdf{2341o"s;
        EXPECT_CALL(*eh, HandleEvent(_, _)).Times(0);
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Action parse error (valid json)
    {
        const std::string msg = ws.s_addAction + R"(;{ "i am": "valid" })"s;
        EXPECT_CALL(*eh, HandleEvent(_, _)).Times(0);
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_addAction + ";"s;
        EXPECT_CALL(*eh, HandleEvent(_, _)).Times(0);
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetActions)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Action;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // No result
    {
        const std::string msg = ws.s_getActions;

        EXPECT_CALL(as, GetAllActions()).WillOnce(Return(std::vector<Action>()));

        EXPECT_CALL(wc.GetServer(), send(_, "NOT_FOUND\n", _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&as);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // One result
    {
        const std::string msg = ws.s_getActions;

        const std::vector<Action> results = {Action(3, "n", "i", 432, {}, false)};

        EXPECT_CALL(as, GetAllActions()).WillOnce(Return(results));

        EXPECT_CALL(wc.GetServer(), send(_, "Action:" + results[0].ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&as);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Multiple results
    {
        const std::string msg = ws.s_getActions;

        const std::vector<Action> results = {Action(3, "n", "i", 432, {}, false), Action(4, "n", "i2", 23, {}, false)};

        InSequence s;
        EXPECT_CALL(as, GetAllActions()).WillOnce(Return(results));

        EXPECT_CALL(wc.GetServer(), send(_, "Action:" + results[0].ToJson().dump() + '\n', _));
        EXPECT_CALL(wc.GetServer(), send(_, "Action:" + results[1].ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&as);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetAction)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Action;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // Action not found
    {
        const std::size_t actionId = 23;
        const std::string msg = ws.s_getAction + ";"s + std::to_string(actionId);

        EXPECT_CALL(as, GetAction(actionId)).WillOnce(Throw(std::out_of_range("test")));

        EXPECT_CALL(wc.GetServer(), send(_, _, _)).Times(0);

        // TODO: Change this to not throw
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);

        Mock::VerifyAndClearExpectations(&as);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Argument missing
    {
        const std::string msg = ws.s_getAction;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Argument not an int
    {
        const std::string msg = ws.s_getAction + ";af"s;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Action found
    {
        const std::size_t actionId = 23;
        const std::string msg = ws.s_getAction + ";"s + std::to_string(actionId);

        Action result{actionId, "n", "i", 23, {}, true};

        EXPECT_CALL(as, GetAction(actionId)).WillOnce(Return(result));

        EXPECT_CALL(wc.GetServer(), send(_, "Action:" + result.ToJson().dump() + '\n', _));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&as);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageDeleteAction)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Action;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // Action not found
    {
        const std::size_t actionId = 23;
        const std::string msg = ws.s_deleteAction + ";"s + std::to_string(actionId);

        EXPECT_CALL(as, GetAction(actionId)).WillOnce(Throw(std::out_of_range("test")));

        // TODO: Change this to not throw
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);

        Mock::VerifyAndClearExpectations(&as);
    }
    // Argument missing
    {
        const std::string msg = ws.s_deleteAction;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Argument not convertible to int
    {
        const std::string msg = ws.s_deleteAction + ";asf23"s;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Action found
    {
        const std::size_t actionId = 23;
        const std::string msg = ws.s_deleteAction + ";"s + std::to_string(actionId);

        Action result{actionId, "n", "i", 23, {}, true};

        auto eh = std::make_shared<MockEventHandler>();
        {
            InSequence s;
            EXPECT_CALL(as, GetAction(actionId)).WillOnce(Return(result));
            EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actionChange));
            EXPECT_CALL(*eh,
                HandleEvent(Truly([&](const EventBase& e) {
                    if (e.GetType() != EventTypes::actionChange)
                    {
                        return false;
                    }
                    const auto& casted = dynamic_cast<const Events::ActionChangeEvent&>(e);
                    return casted.GetChangedFields() == Events::ActionFields::REMOVE
                        && casted.GetOld().ToJson() == result.ToJson()
                        && casted.GetChanged().ToJson() == Action().ToJson();
                }),
                    Ref(Res::EventSystem())));
        }
        Res::EventSystem().AddHandler(eh);
        CleanupEventHandler cleanup(Res::EventSystem(), eh);

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&as);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageExecAction)
{
    using namespace ::testing;
    using namespace std::string_literals;
    using ::Action;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    std::unique_ptr<class MockDeviceAPI> mdapi = std::make_unique<class MockDeviceAPI>();
    reg.RegisterDeviceAPI(std::move(mdapi));
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // Action not found
    {
        const std::size_t actionId = 23;
        const std::string msg = ws.s_execAction + ";"s + std::to_string(actionId);

        EXPECT_CALL(as, GetAction(actionId)).WillOnce(Throw(std::out_of_range("test")));

        EXPECT_CALL(wc.GetServer(), send(_, _, _)).Times(0);

        // TODO: Change this to not throw
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);

        Mock::VerifyAndClearExpectations(&as);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Argument missing
    {
        const std::string msg = ws.s_execAction;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Argument not an int
    {
        const std::string msg = ws.s_execAction + ";d1asf"s;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Action found
    {
        const std::size_t actionId = 23;
        const std::string msg = ws.s_execAction + ";"s + std::to_string(actionId);

        std::shared_ptr<MockActionImpl> impl = std::make_shared<MockActionImpl>(3);
        Action result{actionId, "n", "i", 23, {SubAction(impl)}, true};

        EXPECT_CALL(as, GetAction(actionId)).WillOnce(Return(result));

        EXPECT_CALL(*impl, Execute(Ref(db), Ref(wc.ws), Ref(reg), 0));

        ws.ProcessMessage(hdl, msg);

        Mock::VerifyAndClearExpectations(&as);
    }
}

// TODO: TEST(WebsocketSocketHandler, ProcessMessageFindNodes)
// TODO: TEST(WebsocketSocketHandler, ProcessMessageAddNode)

TEST(WebsocketSocketHandler, ProcessMessageRemoveNode)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    {
        const std::string msg = ws.s_removeNode + ";23"s;

        EXPECT_CALL(nm, RemoveNode(23));

        ws.ProcessMessage(hdl, msg);
    }
    // NodeId parse error
    {
        const std::string msg = ws.s_removeNode + ";asdf"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Empty string after command
    {
        const std::string msg = ws.s_removeNode + ";"s;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetSensorData)
{
    using namespace ::testing;
    using namespace std::string_literals;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    MockDBHandler db;
    db.UseDefaults();
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    // No args
    {
        const std::string msg = ws.s_getSensorData;

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Only mode
    {
        const int mode = 0;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
    }
    // Only up to startTime
    {
        const int mode = 0;
        const long long startTime = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
    }
    // Only up to endTime
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
    }
    // Only up to nodeId
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
    }
    // Mode not convertible
    {
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const std::string msg = ws.s_getSensorData + ";sadf;"s + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // StartTime not convertible
    {
        const int mode = 0;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";asdf;" + std::to_string(endTime)
            + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // EndTime not convertible
    {
        const int mode = 0;
        const long long startTime = 32;
        const int nodeId = 2;
        const int sensorId = 1;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime)
            + ";asfd;" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // NodeId not convertible
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const int sensorId = 1;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";asdf;" + std::to_string(sensorId);

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // SensorId not convertible
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";asdf";

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::invalid_argument);
    }
    // Sensor not found
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        {
            InSequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData"));
            EXPECT_CALL(db, GetROStatement("SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;"));
            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 0 bytes
            EXPECT_CALL(wc.GetServer(), send(_, _, 0, _));
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Mode 0, no data
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const int64_t sensorUID = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        const char* sensorSelectStatement = "SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;";
        auto pStS = db.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;

        {
            Sequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData")).InSequence(s);
            EXPECT_CALL(db, GetROStatement(sensorSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStS.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(1));
                // Statement has result
                EXPECT_CALL(stS, Step()).InSequence(s).After(bound).WillOnce(Return(MockStatement::row));
                EXPECT_CALL(stS, GetInt64(0)).InSequence(s).WillOnce(Return(sensorUID));
            }
            EXPECT_CALL(db,
                GetROStatement(
                    "SELECT changetime, val FROM sensor_log WHERE sensor_uid = ?1 AND changetime BETWEEN ?2 AND ?3;"))
                .InSequence(s);

            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 0 bytes
            EXPECT_CALL(wc.GetServer(), send(_, _, 0, _)).InSequence(s);
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Mode 0, data
    {
        const int mode = 0;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const int64_t sensorUID = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        const char* sensorSelectStatement = "SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;";
        auto pStS = db.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;
        const char* dataSelectStatement
            = "SELECT changetime, val FROM sensor_log WHERE sensor_uid = ?1 AND changetime BETWEEN ?2 AND ?3;";
        auto pStD = db.GetMockedStatement(dataSelectStatement);
        MockStatement& stD = pStD.second;

        {
            Sequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData")).InSequence(s);
            EXPECT_CALL(db, GetROStatement(sensorSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStS.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(1));
                // Statement has result
                EXPECT_CALL(stS, Step()).InSequence(s).After(bound).WillOnce(Return(MockStatement::row));
                EXPECT_CALL(stS, GetInt64(0)).InSequence(s).WillOnce(Return(sensorUID));
            }
            EXPECT_CALL(db, GetROStatement(dataSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStD.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stD, BindInt64(1, sensorUID)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stD, BindInt64(2, startTime)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stD, BindInt64(3, endTime)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stD, GetColumnCount()).WillRepeatedly(Return(2));
                EXPECT_CALL(stD, Step())
                    .InSequence(s)
                    .After(bound)
                    .WillOnce(Return(MockStatement::row))
                    .WillOnce(Return(MockStatement::done));
                // Changetime
                EXPECT_CALL(stD, GetInt64(0)).WillOnce(Return(23));
                // Value
                EXPECT_CALL(stD, GetInt64(1)).WillOnce(Return(1));
            }

            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 1 value
            EXPECT_CALL(wc.GetServer(), send(_, _, 1 * sizeof(std::pair<long long, long long>), _)).InSequence(s);
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Mode 1, no data
    {
        const int mode = 1;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const int64_t sensorUID = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        const char* sensorSelectStatement = "SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;";
        auto pStS = db.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;

        {
            Sequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData")).InSequence(s);
            EXPECT_CALL(db, GetROStatement(sensorSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStS.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(1));
                // Statement has result
                EXPECT_CALL(stS, Step()).InSequence(s).After(bound).WillOnce(Return(MockStatement::row));
                EXPECT_CALL(stS, GetInt64(0)).InSequence(s).WillOnce(Return(sensorUID));
            }
            EXPECT_CALL(db,
                GetROStatement(
                    "SELECT changetime, val FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = "
                    "sensor_log_id WHERE sensor_uid = ?1 AND changetime BETWEEN ?2 AND ?3 "
                    "UNION SELECT changetime,val FROM sensor_log WHERE sensor_uid = ?1 AND changetime BETWEEN "
                    "min(?3, max(?2, (SELECT changetime FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid "
                    "= sensor_log_compressed.sensor_log_id WHERE sensor_uid = ?1 ORDER BY changetime DESC LIMIT 1))) "
                    "AND ?3;"))
                .InSequence(s);

            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 0 bytes
            EXPECT_CALL(wc.GetServer(), send(_, _, 0, _)).InSequence(s);
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Mode 1, data
    {
        const int mode = 1;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const int64_t sensorUID = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        const char* sensorSelectStatement = "SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;";
        auto pStS = db.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;
        const char* dataSelectStatement
            = "SELECT changetime, val FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = sensor_log_id "
              "WHERE sensor_uid = ?1 AND changetime BETWEEN ?2 AND ?3 "
              "UNION SELECT changetime,val FROM sensor_log WHERE sensor_uid = ?1 AND changetime BETWEEN "
              "min(?3, max(?2, (SELECT changetime FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = "
              "sensor_log_compressed.sensor_log_id WHERE sensor_uid = ?1 ORDER BY changetime DESC LIMIT 1))) AND ?3;";
        auto pStD = db.GetMockedStatement(dataSelectStatement);
        MockStatement& stD = pStD.second;

        {
            Sequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData")).InSequence(s);
            EXPECT_CALL(db, GetROStatement(sensorSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStS.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(1));
                // Statement has result
                EXPECT_CALL(stS, Step()).InSequence(s).After(bound).WillOnce(Return(MockStatement::row));
                EXPECT_CALL(stS, GetInt64(0)).InSequence(s).WillOnce(Return(sensorUID));
            }
            EXPECT_CALL(db, GetROStatement(dataSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStD.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stD, BindInt64(1, sensorUID)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stD, BindInt64(2, startTime)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stD, BindInt64(3, endTime)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stD, GetColumnCount()).WillRepeatedly(Return(2));
                EXPECT_CALL(stD, Step())
                    .InSequence(s)
                    .After(bound)
                    .WillOnce(Return(MockStatement::row))
                    .WillOnce(Return(MockStatement::done));
                // Changetime
                EXPECT_CALL(stD, GetInt64(0)).WillOnce(Return(23));
                // Value
                EXPECT_CALL(stD, GetInt64(1)).WillOnce(Return(1));
            }

            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 1 value
            EXPECT_CALL(wc.GetServer(),
                send(_, Truly([](const void* p) {
                    const uint8_t* c = reinterpret_cast<const uint8_t*>(p);
                    // Big endian
                    return c[0] == 0 && c[1] == 0 && c[2] == 0 && c[3] == 0 && c[4] == 0 && c[5] == 0 && c[6] == 0
                        && c[7] == 23 && c[8] == 0 && c[9] == 0 && c[10] == 0 && c[11] == 0 && c[12] == 0 && c[13] == 0
                        && c[14] == 0 && c[15] == 1;
                }),
                    1 * sizeof(std::pair<long long, long long>), _))
                .InSequence(s);
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Mode 2, no data
    {
        const int mode = 2;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const int64_t sensorUID = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        const char* sensorSelectStatement = "SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;";
        auto pStS = db.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;

        {
            Sequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData")).InSequence(s);
            EXPECT_CALL(db, GetROStatement(sensorSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStS.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(1));
                // Statement has result
                EXPECT_CALL(stS, Step()).InSequence(s).After(bound).WillOnce(Return(MockStatement::row));
                EXPECT_CALL(stS, GetInt64(0)).InSequence(s).WillOnce(Return(sensorUID));
            }
            EXPECT_CALL(db,
                GetROStatement(
                    "SELECT changetime, val FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = "
                    "sensor_log_id WHERE sensor_uid = ?1 AND changetime BETWEEN ?2 AND ?3 "
                    "UNION SELECT changetime,val FROM sensor_log WHERE sensor_uid = ?1 AND changetime BETWEEN "
                    "min(?3, max(?2, (SELECT changetime FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid "
                    "= sensor_log_compressed.sensor_log_id WHERE sensor_uid = ?1 ORDER BY changetime DESC LIMIT 1))) "
                    "AND ?3;"))
                .InSequence(s);

            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 0 bytes
            EXPECT_CALL(wc.GetServer(), send(_, _, 0, _)).InSequence(s);
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Mode 2, data
    {
        const int mode = 2;
        const long long startTime = 32;
        const long long endTime = 35;
        const int nodeId = 2;
        const int sensorId = 1;
        const int64_t sensorUID = 32;
        const std::string msg = ws.s_getSensorData + ";"s + std::to_string(mode) + ";" + std::to_string(startTime) + ";"
            + std::to_string(endTime) + ";" + std::to_string(nodeId) + ";" + std::to_string(sensorId);

        const char* sensorSelectStatement = "SELECT sensor_uid FROM sensors WHERE node_id = ?1 AND sensor_id = ?2;";
        auto pStS = db.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;
        const char* dataSelectStatement
            = "SELECT changetime, val FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = sensor_log_id "
              "WHERE sensor_uid = ?1 AND changetime BETWEEN ?2 AND ?3 "
              "UNION SELECT changetime,val FROM sensor_log WHERE sensor_uid = ?1 AND changetime BETWEEN "
              "min(?3, max(?2, (SELECT changetime FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = "
              "sensor_log_compressed.sensor_log_id WHERE sensor_uid = ?1 ORDER BY changetime DESC LIMIT 1))) AND ?3;";
        auto pStD = db.GetMockedStatement(dataSelectStatement);
        MockStatement& stD = pStD.second;

        {
            Sequence s;
            EXPECT_CALL(db, GetROSavepoint("WebsocketSocketHandler_GetSensorData")).InSequence(s);
            EXPECT_CALL(db, GetROStatement(sensorSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStS.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(1));
                // Statement has result
                EXPECT_CALL(stS, Step()).InSequence(s).After(bound).WillOnce(Return(MockStatement::row));
                EXPECT_CALL(stS, GetInt64(0)).InSequence(s).WillOnce(Return(sensorUID));
            }
            EXPECT_CALL(db, GetROStatement(dataSelectStatement)).InSequence(s).WillOnce(Return(ByMove(pStD.first)));

            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stD, BindInt64(1, sensorUID)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stD, BindInt64(2, startTime)).WillOnce(Return(MockDatabase::ok));
                bound += EXPECT_CALL(stD, BindInt64(3, endTime)).WillOnce(Return(MockDatabase::ok));
                EXPECT_CALL(stD, GetColumnCount()).WillRepeatedly(Return(2));
                EXPECT_CALL(stD, Step())
                    .InSequence(s)
                    .After(bound)
                    .WillOnce(Return(MockStatement::row))
                    .WillOnce(Return(MockStatement::done));
                // Changetime
                EXPECT_CALL(stD, GetInt64(0)).WillOnce(Return(23));
                // Value
                EXPECT_CALL(stD, GetInt64(1)).WillOnce(Return(1));
            }

            db.ExpectROSavepointRelease("WebsocketSocketHandler_GetSensorData");

            // Send 1 value
            EXPECT_CALL(wc.GetServer(),
                send(_, Truly([](const void* p) {
                    const uint8_t* c = reinterpret_cast<const uint8_t*>(p);
                    // Big endian
                    return c[0] == 0 && c[1] == 0 && c[2] == 0 && c[3] == 0 && c[4] == 0 && c[5] == 0 && c[6] == 0
                        && c[7] == 23 && c[8] == 0 && c[9] == 0 && c[10] == 0 && c[11] == 0 && c[12] == 0 && c[13] == 0
                        && c[14] == 0 && c[15] == 1;
                }),
                    1 * sizeof(std::pair<long long, long long>), _))
                .InSequence(s);
        }

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetActionTypes)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    try
    {
        // None registered
        {
            const std::string msg = ws.s_getActionTypes;

            // Just in case something was left
            Res::ActionRegistry().RemoveAll();

            EXPECT_CALL(wc.GetServer(), send(_, "NOT_FOUND\n", _));

            ws.ProcessMessage(hdl, msg);

            Mock::VerifyAndClearExpectations(&wc.GetServer());
        }
        // One registered
        {
            const std::string msg = ws.s_getActionTypes;

            const std::string configName = "test_config_0.txt";
            // Callback is never used
            Res::ActionRegistry().Register(
                SubActionInfo{[](std::size_t) { return SubActionImpl::Ptr(); }, configName}, 0);

            nlohmann::json value = {{"id", 0}, {"name", "test_config_0"}};

            EXPECT_CALL(wc.GetServer(), send(_, "ActionType:" + value.dump() + '\n', _));

            ws.ProcessMessage(hdl, msg);

            Res::ActionRegistry().RemoveAll();

            Mock::VerifyAndClearExpectations(&wc.GetServer());
        }
        // Multiple registered (with gaps)
        {
            const std::string msg = ws.s_getActionTypes;

            const std::string configName0 = "test_config_0.txt";
            const std::string configName3 = "test_config_3.txt";
            // Callback is never used
            Res::ActionRegistry().Register(
                SubActionInfo{[](std::size_t) { return SubActionImpl::Ptr(); }, configName0}, 0);
            Res::ActionRegistry().Register(
                SubActionInfo{[](std::size_t) { return SubActionImpl::Ptr(); }, configName3}, 3);

            // file extension is removed
            nlohmann::json value0 = {{"id", 0}, {"name", "test_config_0"}};
            nlohmann::json value3 = {{"id", 3}, {"name", "test_config_3"}};

            {
                InSequence s;
                EXPECT_CALL(wc.GetServer(), send(_, "ActionType:" + value0.dump() + '\n', _));
                EXPECT_CALL(wc.GetServer(), send(_, "ActionType:" + value3.dump() + '\n', _));
            }

            ws.ProcessMessage(hdl, msg);

            Res::ActionRegistry().RemoveAll();

            Mock::VerifyAndClearExpectations(&wc.GetServer());
        }
    }
    catch (...)
    {
        Res::ActionRegistry().RemoveAll();
        throw;
    }
}

TEST(WebsocketSocketHandler, ProcessMessageGetConditionTypes)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    try
    {
        // None registered
        {
            const std::string msg = ws.s_getConditionTypes;

            // Just in case something was left
            Res::ConditionRegistry().RemoveAll();

            EXPECT_CALL(wc.GetServer(), send(_, "NOT_FOUND\n", _));

            ws.ProcessMessage(hdl, msg);

            Mock::VerifyAndClearExpectations(&wc.GetServer());
        }
        // One registered
        {
            const std::string msg = ws.s_getConditionTypes;

            const std::string configName = "test_config_0.txt";
            // Callback is never used
            Res::ConditionRegistry().Register(
                RuleConditions::RuleConditionInfo{[](std::size_t) { return RuleConditions::Ptr(); }, configName}, 0);

            nlohmann::json value = {{"id", 0}, {"name", "test_config_0"}};

            EXPECT_CALL(wc.GetServer(), send(_, "ConditionType:" + value.dump() + '\n', _));

            ws.ProcessMessage(hdl, msg);

            Res::ConditionRegistry().RemoveAll();

            Mock::VerifyAndClearExpectations(&wc.GetServer());
        }
        // Multiple registered (with gaps)
        {
            const std::string msg = ws.s_getConditionTypes;

            const std::string configName0 = "test_config_0.txt";
            const std::string configName3 = "test_config_3.txt";
            // Callback is never used
            Res::ConditionRegistry().Register(
                RuleConditions::RuleConditionInfo{[](std::size_t) { return RuleConditions::Ptr(); }, configName0}, 0);
            Res::ConditionRegistry().Register(
                RuleConditions::RuleConditionInfo{[](std::size_t) { return RuleConditions::Ptr(); }, configName3}, 3);

            // file extension is removed
            nlohmann::json value0 = {{"id", 0}, {"name", "test_config_0"}};
            nlohmann::json value3 = {{"id", 3}, {"name", "test_config_3"}};

            {
                InSequence s;
                EXPECT_CALL(wc.GetServer(), send(_, "ConditionType:" + value0.dump() + '\n', _));
                EXPECT_CALL(wc.GetServer(), send(_, "ConditionType:" + value3.dump() + '\n', _));
            }

            ws.ProcessMessage(hdl, msg);

            Res::ConditionRegistry().RemoveAll();

            Mock::VerifyAndClearExpectations(&wc.GetServer());
        }
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}

TEST(WebsocketSocketHandler, ProcessMessageSetNode)
{
    using namespace ::testing;
    using namespace std::string_literals;
    MockDBHandler db;
    db.UseDefaults();
    WebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    MockNodeSerialize ns;
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;
    // No args
    {
        const std::string msg = ws.s_setNode;
        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::logic_error);
    }
    // Node not found
    {
        const uint16_t nodeId = 2;
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId);

        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(NodeData::EmptyNode()));
        db.ExpectSavepointRelease("WebsocketSocketHandler_SetNode");

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Node found, field missing
    {
        const uint16_t nodeId = 2;
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId);

        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(NodeData()));
        db.ExpectSavepointRollback("WebsocketSocketHandler_SetNode");

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Field 0 (node name and location), args missing
    {
        const uint16_t nodeId = 2;
        const int field = 0;
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId) + ";" + std::to_string(field);

        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(NodeData()));
        db.ExpectSavepointRollback("WebsocketSocketHandler_SetNode");

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Field 0 (node name and location), arg missing
    {
        const uint16_t nodeId = 2;
        const int field = 0;
        const std::string name = "name";
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId) + ";" + std::to_string(field) + ";" + name;

        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(NodeData()));
        db.ExpectSavepointRollback("WebsocketSocketHandler_SetNode");

        EXPECT_THROW(ws.ProcessMessage(hdl, msg), std::out_of_range);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Field 0 (node name and location)
    {
        const uint16_t nodeId = 2;
        const int field = 0;
        const std::string name = "name";
        const std::string location = "location";
        const std::string msg
            = ws.s_setNode + ";"s + std::to_string(nodeId) + ";" + std::to_string(field) + ";" + name + ";" + location;

        NodeData result{nodeId, "n", "l", {}, {}, "s", NodePath(1, 1)};
        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(result));
        db.ExpectSavepointRelease("WebsocketSocketHandler_SetNode");

        auto eh = std::make_shared<MockEventHandler>();
        {
            EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::nodeChange));
            EXPECT_CALL(*eh,
                HandleEvent(Truly([&](const EventBase& e) {
                    if (e.GetType() != EventTypes::nodeChange)
                    {
                        return false;
                    }
                    const auto& casted = dynamic_cast<const Events::NodeChangeEvent&>(e);
                    return casted.GetChangedFields() == Events::NodeFields::NAME && casted.GetOld() == result
                        && casted.GetChanged().GetName() == name && casted.GetChanged().GetLocation() == location;
                }),
                    Ref(Res::EventSystem())));
        }
        Res::EventSystem().AddHandler(eh);
        CleanupEventHandler cleanup(Res::EventSystem(), eh);

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Field 1 (sensor)
    {
        const uint16_t nodeId = 2;
        const int field = 1;
        const uint8_t sensorId = 1;
        const Sensor sensor{nodeId, sensorId, "n", "l", 0, 0, 0};
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId) + ";" + std::to_string(field) + ";"
            + std::to_string(sensorId) + ";" + sensor.ToJson().dump();

        NodeData result{nodeId, "n", "l", {}, {}, "s", NodePath(1, 1)};
        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(result));
        EXPECT_CALL(nm, ChangeSensor(nodeId, sensorId, sensor));
        db.ExpectSavepointRelease("WebsocketSocketHandler_SetNode");

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
        Mock::VerifyAndClearExpectations(&nm);
    }
    // Field 2 (actor)
    {
        const uint16_t nodeId = 2;
        const int field = 2;
        const uint8_t actorId = 1;
        const Actor actor{nodeId, actorId, "n", "l", 0, 0};
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId) + ";" + std::to_string(field) + ";"
            + std::to_string(actorId) + ";" + actor.ToJson().dump();

        NodeData result{nodeId, "n", "l", {}, {}, "s", NodePath(1, 1)};
        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(result));
        EXPECT_CALL(nm, ChangeActor(nodeId, actorId, actor));
        db.ExpectSavepointRelease("WebsocketSocketHandler_SetNode");

        ws.ProcessMessage(hdl, msg);
        Mock::VerifyAndClearExpectations(&db);
        Mock::VerifyAndClearExpectations(&ns);
        Mock::VerifyAndClearExpectations(&nm);
    }
    // Unknown field
    {
        const uint16_t nodeId = 3;
        const std::string msg = ws.s_setNode + ";"s + std::to_string(nodeId) + ";5";

        EXPECT_CALL(db, GetSavepoint("WebsocketSocketHandler_SetNode"));
        EXPECT_CALL(ns, GetNodeById(nodeId)).WillOnce(Return(NodeData()));
        db.ExpectSavepointRelease("WebsocketSocketHandler_SetNode");

        EXPECT_NO_THROW(ws.ProcessMessage(hdl, msg));
    }
}

TEST(WebsocketSocketHandler, HandleSocketMessage)
{
    using namespace ::testing;
    using namespace std::string_literals;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    MockDBHandler db;
    TestWebsocketCommunication wc;
    DeviceRegistry reg;
    MockNodeManager nm;
    MockNodeCommunication nc{234};
    DBNodeSerialize ns{db};
    DBActionSerialize as{db};
    DBRuleSerialize rs{db, as};
    WebsocketSocketHandler ws{db, wc.ws, reg, nm, NewNodeFinder(nc, ns), ns, as, rs};
    websocketpp::connection_hdl hdl;

    {
        Actor a(23, 0, "name", "loc", 23, 4);
        nlohmann::json json = a.ToJson();
        std::ostringstream str;
        str << json;
        const std::string msg = ws.s_addActor + ";23;"s + str.str() + "\n";

        auto msgPtr = std::make_shared<MockWebsocketMessage>();

        EXPECT_CALL(*msgPtr, get_payload()).Times(AtLeast(1)).WillRepeatedly(ReturnRef(msg));

        EXPECT_CALL(nm, AddActor(23, a));

        ws.HandleSocketMessage(Events::SocketMessageEvent(hdl, msgPtr));
        Mock::VerifyAndClearExpectations(&nm);
    }
    // Exception caught
    {
        Actor a(23, 0, "name", "loc", 23, 4);
        nlohmann::json json = a.ToJson();
        std::ostringstream str;
        str << json;
        const std::string msg = ws.s_addActor + ";23;"s + str.str() + "\n";

        auto msgPtr = std::make_shared<MockWebsocketMessage>();

        {
            InSequence s;
            EXPECT_CALL(*msgPtr, get_payload()).Times(AtLeast(1)).WillRepeatedly(ReturnRef(msg));
            EXPECT_CALL(nm, AddActor(23, a)).WillOnce(Throw(std::runtime_error("test")));
#ifndef NDEBUG
            EXPECT_CALL(wc.GetServer(), send(_, "Alert:Exception while processing message: test\n", _));
#endif
        }
        EXPECT_NO_THROW(ws.HandleSocketMessage(Events::SocketMessageEvent(hdl, msgPtr)));
    }
}

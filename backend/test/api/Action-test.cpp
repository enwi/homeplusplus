#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sqlpp11/insert.h>

#include "../mocks/MockActionImpl.h"
#include "../mocks/MockActionSerialize.h"
#include "../mocks/MockDeviceSerialize.h"
#include "api/Action.h"
#include "api/ActionStorage.h"
#include "api/DeviceRegistry.h"
#include "api/SubActionImpls.h"
#include "database/ActionsTable.h"

TEST(Action, DefaultConstructor)
{
    {
        Action a;
        EXPECT_EQ(0, a.GetId());
        EXPECT_EQ("", a.GetName());
        EXPECT_EQ("", a.GetIcon());
        EXPECT_EQ(0, a.GetColor());
        EXPECT_EQ(false, a.GetVisibility());
        EXPECT_TRUE(a.GetSubActions().empty());
    }
    {
        Action a{};
        EXPECT_EQ(0, a.GetId());
        EXPECT_EQ("", a.GetName());
        EXPECT_EQ("", a.GetIcon());
        EXPECT_EQ(0, a.GetColor());
        EXPECT_EQ(false, a.GetVisibility());
        EXPECT_TRUE(a.GetSubActions().empty());
    }
}

TEST(Action, Constructor)
{
    std::vector<SubAction> subActions{SubAction(SubActionImpls::Notification())};
    {
        Action a(32, "name", "icon", 0xAABBCCDD, subActions);
        EXPECT_EQ(32, a.GetId());
        EXPECT_EQ("name", a.GetName());
        EXPECT_EQ("icon", a.GetIcon());
        EXPECT_EQ(0xAABBCCDD, a.GetColor());
        EXPECT_EQ(true, a.GetVisibility());
        EXPECT_EQ(subActions, a.GetSubActions());
    }
    {
        Action a(32, "name", "icon", 0xAABBCCDD, subActions, false);
        EXPECT_EQ(32, a.GetId());
        EXPECT_EQ("name", a.GetName());
        EXPECT_EQ("icon", a.GetIcon());
        EXPECT_EQ(0xAABBCCDD, a.GetColor());
        EXPECT_EQ(false, a.GetVisibility());
        EXPECT_EQ(subActions, a.GetSubActions());
    }
    {
        Action a(32, "name", "icon", 0xAABBCCDD, subActions, true);
        EXPECT_EQ(32, a.GetId());
        EXPECT_EQ("name", a.GetName());
        EXPECT_EQ("icon", a.GetIcon());
        EXPECT_EQ(0xAABBCCDD, a.GetColor());
        EXPECT_EQ(true, a.GetVisibility());
        EXPECT_EQ(subActions, a.GetSubActions());
    }
}

TEST(Action, SetId)
{
    Action a;
    a.SetId(3);
    EXPECT_EQ(3, a.GetId());
    a.SetId(4);
    EXPECT_EQ(4, a.GetId());
}

TEST(Action, SetName)
{
    Action a;
    a.SetName("a");
    EXPECT_EQ("a", a.GetName());
    a.SetName("b");
    EXPECT_EQ("b", a.GetName());
}

TEST(Action, SetIcon)
{
    Action a;
    a.SetIcon("a");
    EXPECT_EQ("a", a.GetIcon());
    a.SetIcon("b");
    EXPECT_EQ("b", a.GetIcon());
}

TEST(Action, SetColor)
{
    Action a;
    a.SetColor(3);
    EXPECT_EQ(3, a.GetColor());
    a.SetColor(4);
    EXPECT_EQ(4, a.GetColor());
}

TEST(Action, SetActions)
{
    Action a;
    std::vector<SubAction> subActions{SubAction(SubActionImpls::Notification())};
    a.SetActions(subActions);
    EXPECT_EQ(subActions, a.GetSubActions());
    std::vector<SubAction> subActions2{
        SubAction(SubActionImpls::RecursiveAction()), SubAction(SubActionImpls::DeviceSet())};
    a.SetActions(subActions2);
    EXPECT_EQ(subActions2, a.GetSubActions());
}

TEST(Action, SetVisible)
{
    Action a;
    a.SetVisible(true);
    EXPECT_TRUE(a.GetVisibility());
    a.SetVisible(false);
    EXPECT_FALSE(a.GetVisibility());
}

TEST(Action, ToJson)
{

    {
        Action a;
        EXPECT_EQ(nlohmann::json({{"id", 0}, {"name", ""}, {"icon", ""}, {"color", 0}, {"visible", false},
                      {"subActions", nlohmann::json::array()}}),
            a.ToJson());
    }
    {
        Action a{3, "name", "icon", 0xAAFFEE, {}, true};
        EXPECT_EQ(nlohmann::json({{"id", 3}, {"name", "name"}, {"icon", "icon"}, {"color", 0xAAFFEE}, {"visible", true},
                      {"subActions", nlohmann::json::array()}}),
            a.ToJson());
    }
    {
        std::vector<SubAction> subActions{SubAction(std::make_shared<SubActionImpls::RecursiveAction>()),
            SubAction(std::make_shared<SubActionImpls::DeviceSet>())};
        Action a{3, "name", "icon", 0xAAFFEE, subActions, true};
        EXPECT_EQ(nlohmann::json({{"id", 3}, {"name", "name"}, {"icon", "icon"}, {"color", 0xAAFFEE}, {"visible", true},
                      {"subActions", {subActions[0].ToJSON(), subActions[1].ToJSON()}}}),
            a.ToJson());
    }
}

TEST(Action, Parse)
{
    using namespace ::testing;
    using Action = ::Action;
    SubActionRegistry reg{};
    {
        nlohmann::json json{
            // Id field missing (defaulted to 0)
            {"name", "a"}, {"icon", "b"}, {"color", 2345}, {"subActions", nlohmann::json::array()}
            // Visible field missing (defaulted to true)
        };
        Action a = Action::Parse(json, reg);
        EXPECT_EQ(0, a.GetId());
        EXPECT_EQ("a", a.GetName());
        EXPECT_EQ("b", a.GetIcon());
        EXPECT_EQ(2345, a.GetColor());
        EXPECT_EQ(true, a.GetVisibility());
        EXPECT_TRUE(a.GetSubActions().empty());
    }
    {
        nlohmann::json json{{"id", 23}, {"name", "a"}, {"icon", "b"}, {"color", 2345},
            {"subActions", nlohmann::json::array()}, {"visible", false}};
        Action a = Action::Parse(json, reg);
        EXPECT_EQ(23, a.GetId());
        EXPECT_EQ("a", a.GetName());
        EXPECT_EQ("b", a.GetIcon());
        EXPECT_EQ(2345, a.GetColor());
        EXPECT_EQ(false, a.GetVisibility());
        EXPECT_TRUE(a.GetSubActions().empty());
    }
    {
        nlohmann::json json{{"id", 23}, {"name", "a"}, {"icon", "b"}, {"color", 2345},
            {"subActions", nlohmann::json::array()}, {"visible", false}};
        EXPECT_EQ(json, Action::Parse(json, reg).ToJson());
    }
    {
        nlohmann::json json{{"id", 23},
            // Name missing
            {"icon", "b"}, {"color", 2345}, {"subActions", nlohmann::json::array()}, {"visible", false}};
        EXPECT_THROW(Action::Parse(json, reg), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"id", 23}, {"name", "a"},
            // Icon missing
            {"color", 2345}, {"subActions", nlohmann::json::array()}, {"visible", false}};
        EXPECT_THROW(Action::Parse(json, reg), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"id", 23}, {"name", "a"}, {"icon", "b"},
            // Color missing
            {"subActions", nlohmann::json::array()}, {"visible", false}};
        EXPECT_THROW(Action::Parse(json, reg), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"id", 23}, {"name", "a"}, {"icon", "b"}, {"color", 2345},
            // SubActions missing
            {"visible", false}};
        EXPECT_THROW(Action::Parse(json, reg), nlohmann::json::exception);
    }
    std::shared_ptr<MockActionImpl> i{std::make_shared<MockActionImpl>()};
    reg.Register(SubActionInfo([=](size_t) { return i; }, "config"), 0);
    {
        i->UseDefaultJson();
        EXPECT_CALL(*i, ToJSONImpl()).Times(1);
        nlohmann::json implJson = i->ToJSON();
        nlohmann::json json{{"id", 23}, {"name", "a"}, {"icon", "b"}, {"color", 2345},
            {"subActions", {implJson, implJson}}, {"visible", false}};
        EXPECT_CALL(*i, ParseImpl(JsonWrapper(implJson))).Times(2);
        Action a = Action::Parse(json, reg);
        EXPECT_EQ(23, a.GetId());
        EXPECT_EQ("a", a.GetName());
        EXPECT_EQ("b", a.GetIcon());
        EXPECT_EQ(2345, a.GetColor());
        EXPECT_EQ(false, a.GetVisibility());
        // Cannot compare pointers for equality, so only size
        EXPECT_EQ(2, a.GetSubActions().size());
    }
    reg.Remove(0);
}

TEST(Action, Execute)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    std::shared_ptr<MockActionImpl> i = std::make_shared<MockActionImpl>();
    ::Action a{0, "a", "b", 0, {SubAction(i), SubAction(i)}, true};
    MockActionSerialize actionSer;
    EventEmitter<Events::ActionChangeEvent> event;
    ActionStorage storage{actionSer, event};
    WebsocketCommunication wc{ nullptr };
    WebsocketChannel ws{wc, "", WebsocketChannel::RequireAuth::noAuth};
	MockDeviceSerialize deviceSer;
	EventEmitter<Events::DeviceChangeEvent> events;
	EventEmitter<Events::DevicePropertyChangeEvent> pEvents;
	DeviceRegistry dr(deviceSer, events, pEvents);
	UserId user = UserId{ 2346 };
    EXPECT_CALL(*i, Execute(Ref(storage), Ref(ws), Ref(dr), user, 0)).Times(2);
    a.Execute(storage, ws, dr, user);
    // Recursion depth exceeded
    EXPECT_CALL(*i, Execute(_, _, _, _, _)).Times(0);
    a.Execute(storage, ws, dr, user, ::Action::s_maxRecursion + 1);
}

TEST(SubActionRegistry, GetImpl)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    SubActionRegistry reg;

    reg.Register(SubActionInfo{[](uint64_t s) { return std::make_shared<MockActionImpl>(s); }, "config filename"}, 0);
    // Get registered
    EXPECT_NE(nullptr, reg.GetImpl(0));
    // Get not registered
    EXPECT_THROW(reg.GetImpl(1), std::out_of_range);
}

TEST(SubActionRegistry, ParseJson)
{
    using namespace ::testing;
    SubActionRegistry reg;

    std::shared_ptr<MockActionImpl> i = std::make_shared<MockActionImpl>(0);

    reg.Register(SubActionInfo{[&](uint64_t) { return i; }, "config filename"}, 0);
    nlohmann::json json{{"type", 0}, {"data", "some test data"}};
    EXPECT_CALL(*i, ParseImpl(JsonWrapper(json)));
    reg.Parse(json);
}

TEST(SubActionRegistry, ParseDb)
{
    using namespace ::testing;
    SubActionRegistry reg;

    std::shared_ptr<MockActionImpl> i = std::make_shared<MockActionImpl>(0);

    reg.Register(SubActionInfo{[&](uint64_t) { return i; }, "name"}, 0);

    DBHandler dbHandler{":memory:"};
    SubActionsTable subActions;
    ActionsTable actions;
    auto& db = dbHandler.GetDatabase();
    db.execute(ActionsTable::createStatement);
    db.execute(SubActionsTable::createStatement);
    // action to satisfy foreign key
    const uint64_t id = 1;
    db(insert_into(actions).set(actions.actionId = id));
    db(insert_into(subActions)
            .set(subActions.subActionId = 123, subActions.actionId = id, subActions.actionType = 0,
                subActions.data = sqlpp::null));

    auto t = sqlpp::start_transaction(db);
    UserHeldTransaction transaction{UserId{0x236}, t};
    EXPECT_CALL(*i, Parse(Ref(db), _, Ref(transaction)));
    auto result = db(select(subActions.actionType, subActions.data,
        subActions.timeout, subActions.transition)
                         .from(subActions)
                         .unconditionally());
    reg.Parse(db, result.front(), transaction);
    t.commit();
}

TEST(SubActionRegistry, RegisterDefaultSubActions)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    SubActionRegistry reg;

    reg.RegisterDefaultSubActions();
    EXPECT_NE(nullptr, dynamic_cast<SubActionImpls::DeviceSet*>(reg.GetImpl(0).get()));
    EXPECT_NE(nullptr, dynamic_cast<SubActionImpls::DeviceToggle*>(reg.GetImpl(1).get()));
    EXPECT_NE(nullptr, dynamic_cast<SubActionImpls::Notification*>(reg.GetImpl(2).get()));
    EXPECT_NE(nullptr, dynamic_cast<SubActionImpls::RecursiveAction*>(reg.GetImpl(5).get()));
    EXPECT_THROW(reg.GetImpl(3), std::out_of_range);
    EXPECT_THROW(reg.GetImpl(4), std::out_of_range);
    // Call again
    reg.Remove(5);
    // Register without empty should not change anything
    reg.RegisterDefaultSubActions();
    EXPECT_THROW(reg.GetImpl(5), std::out_of_range);
}

TEST(SubActionRegistry, GetRegistered)
{
    using namespace ::testing;
    SubActionRegistry reg;

    reg.RegisterDefaultSubActions();
    const std::vector<SubActionInfo>& v = reg.GetRegistered();
    EXPECT_EQ(6, v.size());
    EXPECT_NE(nullptr, v.at(0).function);
    EXPECT_NE(nullptr, v.at(1).function);
    EXPECT_NE(nullptr, v.at(2).function);
    EXPECT_EQ(nullptr, v.at(3).function);
    EXPECT_EQ(nullptr, v.at(4).function);
    EXPECT_NE(nullptr, v.at(5).function);
}

TEST(SubAction, Execute)
{
    using namespace ::testing;
    std::shared_ptr<MockActionImpl> impl = std::make_shared<MockActionImpl>(0);
    SubAction a{impl};

    MockActionSerialize actionSer;
    EventEmitter<Events::ActionChangeEvent> emitter;
    ActionStorage storage{actionSer, emitter};
    WebsocketCommunication wc{ nullptr };
    WebsocketChannel ws{wc, "", WebsocketChannel::RequireAuth::noAuth};
	MockDeviceSerialize deviceSer;
	EventEmitter<Events::DeviceChangeEvent> events;
	EventEmitter<Events::DevicePropertyChangeEvent> pEvents;
    DeviceRegistry dr(deviceSer, events, pEvents);
	UserId user{ 2346 };
    EXPECT_CALL(*impl, Execute(Ref(storage), Ref(ws), Ref(dr), user,0));
    a.Execute(storage, ws, dr,user);
}

TEST(SubAction, ToJson)
{
    using namespace ::testing;
    std::shared_ptr<MockActionImpl> impl = std::make_shared<MockActionImpl>(0);
    SubAction a{impl};
    EXPECT_CALL(*impl, ToJSONImpl()).Times(2).WillRepeatedly(Return(JsonWrapper(nlohmann::json{"data", "test"})));
    EXPECT_EQ(impl->ToJSON(), a.ToJSON());
}


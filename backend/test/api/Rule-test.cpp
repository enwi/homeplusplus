#include <gtest/gtest.h>

#include "../mocks/MockRuleCondition.h"
#include "../mocks/MockRuleSerialize.h"
#include "api/Rule.h"
#include "api/SubActionImpls.h"
#include "database/DBRuleSerialize.h"

TEST(Rule, DefaultConstructor)
{
    {
        Rule r;
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ("", r.GetName());
        EXPECT_EQ("", r.GetIcon());
        EXPECT_EQ(0, r.GetColor());
        EXPECT_TRUE(r.IsEnabled());
        EXPECT_FALSE(r.HasCondition());
        const Action& a = r.GetEffect();
        EXPECT_EQ(0, a.GetId());
        EXPECT_EQ("", a.GetName());
        EXPECT_EQ("", a.GetIcon());
        EXPECT_EQ(0, a.GetColor());
        EXPECT_FALSE(a.GetVisibility());
    }
    {
        Rule r {};
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ("", r.GetName());
        EXPECT_EQ("", r.GetIcon());
        EXPECT_EQ(0, r.GetColor());
        EXPECT_TRUE(r.IsEnabled());
        EXPECT_FALSE(r.HasCondition());
        const Action& a = r.GetEffect();
        EXPECT_EQ(0, a.GetId());
        EXPECT_EQ("", a.GetName());
        EXPECT_EQ("", a.GetIcon());
        EXPECT_EQ(0, a.GetColor());
        EXPECT_FALSE(a.GetVisibility());
        EXPECT_TRUE(a.GetSubActions().empty());
    }
}

TEST(Rule, Constructor)
{
    {
        RuleConditions::Ptr condition = std::make_unique<RuleConditions::RuleConstantCondition>(0, false);
        RuleConditions::RuleCondition* c = condition.get();
        Rule r {2, "name", "icon", 0xAAFFEE00, std::move(condition),
            Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ("name", r.GetName());
        EXPECT_EQ("icon", r.GetIcon());
        EXPECT_EQ(0xAAFFEE00, r.GetColor());
        EXPECT_FALSE(r.IsEnabled());
        ASSERT_TRUE(r.HasCondition());
        EXPECT_EQ(c, &r.GetCondition());
        const Action& a = r.GetEffect();
        EXPECT_EQ(1, a.GetId());
        EXPECT_EQ("actionName", a.GetName());
        EXPECT_EQ("actionIcon", a.GetIcon());
        EXPECT_EQ(0xFF, a.GetColor());
        EXPECT_FALSE(a.GetVisibility());
    }
    {
        Rule r {2, "name", "icon", 0xAAFFEE00, nullptr, Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ("name", r.GetName());
        EXPECT_EQ("icon", r.GetIcon());
        EXPECT_EQ(0xAAFFEE00, r.GetColor());
        EXPECT_FALSE(r.IsEnabled());
        EXPECT_FALSE(r.HasCondition());
        const Action& a = r.GetEffect();
        EXPECT_EQ(1, a.GetId());
        EXPECT_EQ("actionName", a.GetName());
        EXPECT_EQ("actionIcon", a.GetIcon());
        EXPECT_EQ(0xFF, a.GetColor());
        EXPECT_FALSE(a.GetVisibility());
    }
    {
        RuleConditions::Ptr condition = std::make_unique<RuleConditions::RuleConstantCondition>(0, false);
        RuleConditions::RuleCondition* c = condition.get();
        Rule r {2, "name", "icon", 0xAAFFEE00, std::move(condition),
            Action(1, "actionName", "actionIcon", 0xFF, {}, false)};
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ("name", r.GetName());
        EXPECT_EQ("icon", r.GetIcon());
        EXPECT_EQ(0xAAFFEE00, r.GetColor());
        EXPECT_TRUE(r.IsEnabled());
        ASSERT_TRUE(r.HasCondition());
        EXPECT_EQ(c, &r.GetCondition());
        const Action& a = r.GetEffect();
        EXPECT_EQ(1, a.GetId());
        EXPECT_EQ("actionName", a.GetName());
        EXPECT_EQ("actionIcon", a.GetIcon());
        EXPECT_EQ(0xFF, a.GetColor());
        EXPECT_FALSE(a.GetVisibility());
    }
}

TEST(Rule, CopyConstructor)
{
    Res::ConditionRegistry().Register(
        RuleConditions::RuleConditionInfo {
            [](std::size_t i) { return std::make_unique<RuleConditions::RuleConstantCondition>(i); }, "test"},
        0);
    try
    {
        {
            RuleConditions::Ptr condition = std::make_unique<RuleConditions::RuleConstantCondition>(0, false);
            RuleConditions::RuleCondition* c = condition.get();
            Rule source {2, "name", "icon", 0xAAFFEE00, std::move(condition),
                Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};

            Rule r = source;
            EXPECT_EQ(2, r.GetId());
            EXPECT_EQ("name", r.GetName());
            EXPECT_EQ("icon", r.GetIcon());
            EXPECT_EQ(0xAAFFEE00, r.GetColor());
            EXPECT_FALSE(r.IsEnabled());
            ASSERT_TRUE(r.HasCondition());
            EXPECT_EQ(c->ToJson(), r.GetCondition().ToJson());
            const Action& a = r.GetEffect();
            EXPECT_EQ(1, a.GetId());
            EXPECT_EQ("actionName", a.GetName());
            EXPECT_EQ("actionIcon", a.GetIcon());
            EXPECT_EQ(0xFF, a.GetColor());
            EXPECT_FALSE(a.GetVisibility());
        }
        {
            Rule source {
                2, "name", "icon", 0xAAFFEE00, nullptr, Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};
            Rule r = source;
            EXPECT_EQ(2, r.GetId());
            EXPECT_EQ("name", r.GetName());
            EXPECT_EQ("icon", r.GetIcon());
            EXPECT_EQ(0xAAFFEE00, r.GetColor());
            EXPECT_FALSE(r.IsEnabled());
            EXPECT_FALSE(r.HasCondition());
            const Action& a = r.GetEffect();
            EXPECT_EQ(1, a.GetId());
            EXPECT_EQ("actionName", a.GetName());
            EXPECT_EQ("actionIcon", a.GetIcon());
            EXPECT_EQ(0xFF, a.GetColor());
            EXPECT_FALSE(a.GetVisibility());
        }
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    Res::ConditionRegistry().RemoveAll();
}

TEST(Rule, CopyAssignment)
{
    Res::ConditionRegistry().Register(
        RuleConditions::RuleConditionInfo {
            [](std::size_t i) { return std::make_unique<RuleConditions::RuleConstantCondition>(i); }, "test"},
        0);
    try
    {
        {
            RuleConditions::Ptr condition = std::make_unique<RuleConditions::RuleConstantCondition>(0, false);
            RuleConditions::RuleCondition* c = condition.get();
            Rule source {2, "name", "icon", 0xAAFFEE00, std::move(condition),
                Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};

            Rule r {1, "", "", 0, std::make_unique<RuleConditions::RuleConstantCondition>(), Action(), true};
            r = source;

            EXPECT_EQ(2, r.GetId());
            EXPECT_EQ("name", r.GetName());
            EXPECT_EQ("icon", r.GetIcon());
            EXPECT_EQ(0xAAFFEE00, r.GetColor());
            EXPECT_FALSE(r.IsEnabled());
            ASSERT_TRUE(r.HasCondition());
            EXPECT_EQ(c->ToJson(), r.GetCondition().ToJson());
            const Action& a = r.GetEffect();
            EXPECT_EQ(1, a.GetId());
            EXPECT_EQ("actionName", a.GetName());
            EXPECT_EQ("actionIcon", a.GetIcon());
            EXPECT_EQ(0xFF, a.GetColor());
            EXPECT_FALSE(a.GetVisibility());
        }
        {
            Rule source {
                2, "name", "icon", 0xAAFFEE00, nullptr, Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};

            Rule r {1, "", "", 0, std::make_unique<RuleConditions::RuleConstantCondition>(), Action(), true};
            r = source;

            EXPECT_EQ(2, r.GetId());
            EXPECT_EQ("name", r.GetName());
            EXPECT_EQ("icon", r.GetIcon());
            EXPECT_EQ(0xAAFFEE00, r.GetColor());
            EXPECT_FALSE(r.IsEnabled());
            EXPECT_FALSE(r.HasCondition());
            const Action& a = r.GetEffect();
            EXPECT_EQ(1, a.GetId());
            EXPECT_EQ("actionName", a.GetName());
            EXPECT_EQ("actionIcon", a.GetIcon());
            EXPECT_EQ(0xFF, a.GetColor());
            EXPECT_FALSE(a.GetVisibility());
        }
        // Move assignment
        {
            Rule source {
                2, "name", "icon", 0xAAFFEE00, nullptr, Action(1, "actionName", "actionIcon", 0xFF, {}, false), false};

            Rule r {1, "", "", 0, std::make_unique<RuleConditions::RuleConstantCondition>(), Action(), true};
            r = std::move(source);

            EXPECT_EQ(2, r.GetId());
            EXPECT_EQ("name", r.GetName());
            EXPECT_EQ("icon", r.GetIcon());
            EXPECT_EQ(0xAAFFEE00, r.GetColor());
            EXPECT_FALSE(r.IsEnabled());
            EXPECT_FALSE(r.HasCondition());
            const Action& a = r.GetEffect();
            EXPECT_EQ(1, a.GetId());
            EXPECT_EQ("actionName", a.GetName());
            EXPECT_EQ("actionIcon", a.GetIcon());
            EXPECT_EQ(0xFF, a.GetColor());
            EXPECT_FALSE(a.GetVisibility());
        }
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    Res::ConditionRegistry().RemoveAll();
}

TEST(Rule, SetId)
{
    Rule r;
    r.SetId(3);
    EXPECT_EQ(3, r.GetId());
    r.SetId(5);
    EXPECT_EQ(5, r.GetId());
}

TEST(Rule, SetName)
{
    Rule r;
    r.SetName("a");
    EXPECT_EQ("a", r.GetName());
    r.SetName("b");
    EXPECT_EQ("b", r.GetName());
}

TEST(Rule, SetIcon)
{
    Rule r;
    r.SetIcon("a");
    EXPECT_EQ("a", r.GetIcon());
    r.SetIcon("b");
    EXPECT_EQ("b", r.GetIcon());
}

TEST(Rule, SetEnabled)
{
    Rule r;
    r.SetEnabled(false);
    EXPECT_EQ(false, r.IsEnabled());
    r.SetEnabled(true);
    EXPECT_EQ(true, r.IsEnabled());
}

TEST(Rule, SetCondition)
{
    Rule r;
    {
        RuleConditions::Ptr c = std::make_unique<RuleConditions::RuleConstantCondition>(1, true);
        RuleConditions::RuleCondition* pc = c.get();
        r.SetCondition(std::move(c));
        ASSERT_TRUE(r.HasCondition());
        EXPECT_EQ(pc, &r.GetCondition());
        EXPECT_EQ(pc, &::testing::Const(r).GetCondition());
    }
    {
        RuleConditions::Ptr c = std::make_unique<RuleConditions::RuleConstantCondition>(3, false);
        RuleConditions::RuleCondition* pc = c.get();
        r.SetCondition(std::move(c));
        ASSERT_TRUE(r.HasCondition());
        EXPECT_EQ(pc, &r.GetCondition());
        EXPECT_EQ(pc, &::testing::Const(r).GetCondition());
    }
    {
        r.SetCondition(nullptr);
        EXPECT_FALSE(r.HasCondition());
    }
}

TEST(Rule, SetEffect)
{
    Rule r;
    Action action {2, "n", "i", 0xFF, {}, false};
    r.SetEffect(action);
    {
        const Action& a = r.GetEffect();
        EXPECT_EQ(2, a.GetId());
        EXPECT_EQ("n", a.GetName());
        EXPECT_EQ("i", a.GetIcon());
        EXPECT_EQ(0xFF, a.GetColor());
        EXPECT_FALSE(a.GetVisibility());
    }
    Action action2 {3, "a", "b", 0, {}, true};
    r.SetEffect(action2);
    {
        const Action& a = r.GetEffect();
        EXPECT_EQ(3, a.GetId());
        EXPECT_EQ("a", a.GetName());
        EXPECT_EQ("b", a.GetIcon());
        EXPECT_EQ(0, a.GetColor());
        EXPECT_TRUE(a.GetVisibility());
    }
}

TEST(Rule, ToJson)
{
    {
        Rule r;
        nlohmann::json json = {{"id", 0}, {"name", ""}, {"icon", ""}, {"color", 0}, {"condition", nullptr},
            {"effect", Action().ToJson()}, {"enabled", true}};
        EXPECT_EQ(json, r.ToJson());
    }
    {
        Action a {1, "test", "testi", 1234,
            {SubAction(SubActionImpls::Notification::Create(1, 2, "a", std::chrono::seconds(1), false))}, false};
        RuleConditions::Ptr c = std::make_unique<RuleConditions::RuleConstantCondition>(3, false);
        RuleConditions::RuleCondition* pc = c.get();
        Rule r {2, "n", "i", 4, std::move(c), a, false};
        nlohmann::json json = {{"id", 2}, {"name", "n"}, {"icon", "i"}, {"color", 4}, {"condition", pc->ToJson()},
            {"effect", a.ToJson()}, {"enabled", false}};
        EXPECT_EQ(json, r.ToJson());
    }
}

TEST(Rule, Parse)
{
    Res::ConditionRegistry().Register(
        RuleConditions::RuleConditionInfo {
            [](std::size_t) { return std::make_unique<RuleConditions::RuleConstantCondition>(0, false); }, "test"},
        0);
    {
        nlohmann::json json {// Id field missing (defaulted to 0)
            {"name", "a"}, {"icon", "b"}, {"color", 10},
            {"condition", RuleConditions::RuleConstantCondition(0, false).ToJson()}, {"effect", Action().ToJson()},
            {"enabled", false}};
        Rule r = Rule::Parse(json);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ("a", r.GetName());
        EXPECT_EQ("b", r.GetIcon());
        EXPECT_EQ(10, r.GetColor());
        EXPECT_FALSE(r.IsEnabled());
        EXPECT_EQ(RuleConditions::RuleConstantCondition(0, false).ToJson(), r.GetCondition().ToJson());
    }
    {
        nlohmann::json json {{"id", 4}, {"name", "a"}, {"icon", "b"}, {"color", 10},
            {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()}, {"effect", Action().ToJson()},
            {"enabled", true}};
        Rule r = Rule::Parse(json);
        EXPECT_EQ(4, r.GetId());
        EXPECT_EQ("a", r.GetName());
        EXPECT_EQ("b", r.GetIcon());
        EXPECT_EQ(10, r.GetColor());
        EXPECT_EQ(RuleConditions::RuleConstantCondition(0, true).ToJson(), r.GetCondition().ToJson());
        EXPECT_TRUE(r.IsEnabled());
    }
    {
        nlohmann::json json {{"id", 4}, {"name", "a"}, {"icon", "b"}, {"color", 10},
            {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()}, {"effect", Action().ToJson()},
            {"enabled", true}};
        EXPECT_EQ(json, Rule::Parse(json).ToJson());
    }
    {
        nlohmann::json json {{"id", 4},
            // Name missing
            {"icon", "b"}, {"color", 10}, {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()},
            {"effect", Action().ToJson()}, {"enabled", true}};
        EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 4}, {"name", "a"},
            // Icon missing
            {"color", 10}, {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()},
            {"effect", Action().ToJson()}, {"enabled", true}};
        EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 4}, {"name", "a"}, {"icon", "b"},
            // Color missing
            {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()}, {"effect", Action().ToJson()},
            {"enabled", true}};
        EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 4}, {"name", "a"}, {"icon", "b"}, {"color", 10},
            // Condition missing
            {"effect", Action().ToJson()}, {"enabled", true}};
        EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 4}, {"name", "a"}, {"icon", "b"}, {"color", 10},
            {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()},
            // Effect missing
            {"enabled", true}};
        EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {
            {"id", 4}, {"name", "a"}, {"icon", "b"}, {"color", 10},
            {"condition", RuleConditions::RuleConstantCondition(0, true).ToJson()}, {"effect", Action().ToJson()},
            // Enabled missing
        };
        EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
    }
    {
        {
            nlohmann::json json {{"id", 4}, {"name", "a"}, {"icon", "b"}, {"color", 10}, {"condition", "wrong type"},
                {"effect", Action().ToJson()}, {"enabled", true}};
            EXPECT_THROW(Rule::Parse(json), nlohmann::json::exception);
        }
    }
}

TEST(Rule, IsSatisfied)
{
    using namespace ::testing;
    using ::Action;
    {
        Rule r {1, "", "", 0, std::make_unique<MockRuleCondition>(1), Action()};
        EXPECT_CALL(dynamic_cast<MockRuleCondition&>(r.GetCondition()), IsSatisfied()).WillOnce(Return(true));
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        Rule r {1, "", "", 0, std::make_unique<MockRuleCondition>(1), Action()};
        EXPECT_CALL(dynamic_cast<MockRuleCondition&>(r.GetCondition()), IsSatisfied()).WillOnce(Return(false));
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        // Not enabled rule is not satisfied
        Rule r {1, "", "", 0, std::make_unique<MockRuleCondition>(1), Action(), false};
        EXPECT_CALL(dynamic_cast<MockRuleCondition&>(r.GetCondition()), IsSatisfied()).Times(0);
        EXPECT_FALSE(r.IsSatisfied());
    }
}

TEST(RuleConditionRegistry, GetCondition)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    RuleConditions::Registry reg;

    reg.Register(
        RuleConditions::RuleConditionInfo {
            [](std::size_t s) { return std::make_unique<MockRuleCondition>(s); }, "config filename"},
        0);
    // Get registered
    EXPECT_NE(nullptr, reg.GetCondition(0));
    // Get not registered
    EXPECT_THROW(reg.GetCondition(1), std::out_of_range);
}

TEST(RuleConditionRegistry, ParseJson)
{
    using namespace ::testing;
    RuleConditions::Registry reg;

    std::unique_ptr<MockRuleCondition> c = std::make_unique<MockRuleCondition>(0);
    MockRuleCondition& cr = *c;

    reg.Register(RuleConditions::RuleConditionInfo {[&](std::size_t) { return std::move(c); }, "config filename"}, 0);
    nlohmann::json json {{"type", 0}, {"data", "some test data"}};

    MockConditionSerialize cSer;
    // Careful! c is moved from after call to parse!
    EXPECT_CALL(cr, ParseImpl(Ref(reg), JsonWrapper(json)));
    EXPECT_EQ(&cr, reg.ParseCondition(json).get());
}

/*TEST(RuleConditionRegistry, ParseDb)
{
    using namespace ::testing;
    RuleConditions::Registry reg;

    std::unique_ptr<MockRuleCondition> pC = std::make_unique<MockRuleCondition>(0);
    MockRuleCondition& c = *pC;
    std::shared_ptr<MockStatement> pSt = std::make_shared<MockStatement>("test statement;");

    EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(2));
    // Column 1 is type
    EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(0));

    reg.Register(RuleConditions::RuleConditionInfo{[&](std::size_t) { return std::move(pC); }, "config filename"}, 0);
    DBResult result{pSt};
    MockDBHandler db;
    DBRuleConditionSerialize cSer{db};
    EXPECT_CALL(c, Parse(Ref(cSer), Ref(reg), A<DBResult>()));
    EXPECT_EQ(&c, reg.ParseCondition(cSer, result).get());
}*/

TEST(RuleConditionRegistry, RegisterDefaultConditions)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    RuleConditions::Registry reg;

    reg.RegisterDefaultConditions();
    EXPECT_NE(nullptr, dynamic_cast<RuleConditions::RuleConstantCondition*>(reg.GetCondition(0).get()));
    EXPECT_NE(nullptr, dynamic_cast<RuleConditions::RuleCompareCondition*>(reg.GetCondition(1).get()));
    EXPECT_NE(nullptr, dynamic_cast<RuleConditions::RuleTimeCondition*>(reg.GetCondition(2).get()));
    EXPECT_NE(nullptr, dynamic_cast<RuleConditions::RuleDeviceCondition*>(reg.GetCondition(3).get()));
    EXPECT_THROW(reg.GetCondition(4), std::out_of_range);
    // Call again
    reg.Remove(3);
    // Register without empty should not change anything
    reg.RegisterDefaultConditions();
    EXPECT_THROW(reg.GetCondition(3), std::out_of_range);
}

TEST(RuleConditionRegistry, GetRegistered)
{
    RuleConditions::Registry reg;

    reg.RegisterDefaultConditions();
    const std::vector<RuleConditions::RuleConditionInfo>& v = reg.GetRegistered();
    EXPECT_EQ(4, v.size());
    EXPECT_NE(nullptr, v.at(0).function);
    EXPECT_NE(nullptr, v.at(1).function);
    EXPECT_NE(nullptr, v.at(2).function);
    EXPECT_NE(nullptr, v.at(3).function);
}
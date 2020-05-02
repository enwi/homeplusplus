#include <gtest/gtest.h>

#include "../mocks/MockRuleSerialize.h"
#include "api/RuleStorage.h"

class RuleStorageTest : public ::testing::Test
{
public:
    RuleStorageTest() : storage(ruleSer, eventEmitter)
    {
        eventEmitter.AddHandler(handler.AsStdFunction());
        Res::ConditionRegistry().RegisterDefaultConditions();
    }
    ~RuleStorageTest() { Res::ConditionRegistry().RemoveAll(); }
    MockRuleSerialize ruleSer;
    EventEmitter<Events::RuleChangeEvent> eventEmitter;
    ::testing::MockFunction<PostEventState(const Events::RuleChangeEvent&)> handler;
    RuleStorage storage;
};

TEST_F(RuleStorageTest, AddRule)
{
    using namespace ::testing;
    using Action = ::Action;
    Rule rule{293, "name", "icon", 289, Res::ConditionRegistry().GetCondition(0),
        Action{352, "name", "icon", 126, {}}}; // ruleSer throws
    UserId user{234693};
    // ruleSer throws
    {
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_CALL(ruleSer, AddRule(rule, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.AddRule(rule, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&ruleSer);
    // success
    {
        const uint64_t ruleId = 3264;
        Rule copy = rule;
        copy.SetId(ruleId);
        InSequence s;
        EXPECT_CALL(ruleSer, AddRule(rule, user)).WillOnce(Invoke([&](Rule& r, UserId u) { r.SetId(ruleId); }));
        EXPECT_CALL(handler, Call(Truly([&](const Events::RuleChangeEvent& e) {
            return e.GetChanged() == copy && e.GetChangedFields() == Events::RuleFields::ADD;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.AddRule(rule, user);
    }
}

TEST_F(RuleStorageTest, UpdateRule)
{
    using namespace ::testing;
    using Action = ::Action;
    Rule rule{293, "name", "icon", 289, Res::ConditionRegistry().GetCondition(0),
        Action{352, "name", "icon", 126, {}}}; // ruleSer throws
    UserId user{234693};
    // ruleSer throws
    {
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_CALL(ruleSer, UpdateRule(rule, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.UpdateRule(rule, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&ruleSer);
    // success
    {
        const uint64_t ruleId = 3264;
        Rule copy = rule;
        copy.SetId(ruleId);
        InSequence s;
        EXPECT_CALL(ruleSer, UpdateRule(rule, user)).WillOnce(Invoke([&](Rule& r, UserId u) { r.SetId(ruleId); }));
        EXPECT_CALL(handler, Call(Truly([&](const Events::RuleChangeEvent& e) {
            return e.GetChanged() == copy && e.GetChangedFields() == Events::RuleFields::ALL;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.UpdateRule(rule, user);
    }
}

TEST_F(RuleStorageTest, UpdateRuleHeader)
{
    using namespace ::testing;
    using Action = ::Action;
    Rule rule{293, "name", "icon", 289, Res::ConditionRegistry().GetCondition(0),
        Action{352, "name", "icon", 126, {}}}; // ruleSer throws
    UserId user{234693};
    // ruleSer throws
    {
        EXPECT_CALL(ruleSer, AddRuleOnly(rule, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_THROW(storage.UpdateRuleHeader(rule, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&ruleSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(ruleSer, AddRuleOnly(rule, user));
        EXPECT_CALL(handler, Call(Truly([&](const Events::RuleChangeEvent& e) {
            return e.GetChanged() == rule && e.GetChangedFields() == Events::RuleFields::NAME;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.UpdateRuleHeader(rule, user);
    }
}

TEST_F(RuleStorageTest, GetRule)
{
    using namespace ::testing;
    using Action = ::Action;
    const uint64_t ruleId = 28916;
    Rule rule{
        ruleId, "name", "icon", 289, Res::ConditionRegistry().GetCondition(0), Action{352, "name", "icon", 126, {}}};
    UserId user{234693};
    // ruleSer throws
    {
        EXPECT_CALL(ruleSer, GetRule(ruleId, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.GetRule(ruleId, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&ruleSer);
    // success
    {
        EXPECT_CALL(ruleSer, GetRule(ruleId, user)).WillOnce(Return(rule));
        EXPECT_EQ(rule, storage.GetRule(ruleId, user));
    }
}

TEST_F(RuleStorageTest, GetAllRules)
{
    using namespace ::testing;
    using Action = ::Action;
    std::vector<Rule> rules{
        {3246, "name", "icon", 289, Res::ConditionRegistry().GetCondition(0), Action{352, "name", "icon", 126, {}}},
        {23849, "name2", "icon2", 892, Res::ConditionRegistry().GetCondition(0),
            Action{2364, "name", "icon", 829, {}}}};
    UserId user{234693};
    // ruleSer throws
    {
        EXPECT_CALL(ruleSer, GetAllRules(_, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.GetAllRules(Filter(), user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&ruleSer);
    // success
    {
        EXPECT_CALL(ruleSer, GetAllRules(_, user)).WillOnce(Return(rules));
        EXPECT_EQ(rules, storage.GetAllRules(Filter(), user));
    }
}

TEST_F(RuleStorageTest, RemoveRule)
{
    using namespace ::testing;
    const uint64_t ruleId = 235;
    UserId user{22646};
    // ruleSer throws
    {
        EXPECT_CALL(ruleSer, RemoveRule(ruleId, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_THROW(storage.RemoveRule(ruleId, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&ruleSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(ruleSer, RemoveRule(ruleId, user));
        EXPECT_CALL(handler, Call(Truly([&](const Events::RuleChangeEvent& e) {
            return e.GetOld().GetId() == ruleId && e.GetChangedFields() == Events::RuleFields::REMOVE;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.RemoveRule(ruleId, user);
    }
}

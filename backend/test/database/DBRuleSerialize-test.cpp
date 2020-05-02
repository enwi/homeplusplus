#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>

#include "../mocks/MockRuleCondition.h"
#include "database/DBActionSerialize.h"
#include "database/DBRuleSerialize.h"
#include "database/RulesTable.h"

#include "api/rule_conditions.pb.h"

using namespace ::RuleConditions;

class DBRuleConditionSerializeTest : public ::testing::Test
{
public:
    DBRuleConditionSerializeTest() : dbHandler(":memory:"), db(dbHandler.GetDatabase()), cs(dbHandler)
    {
        db.execute(RulesTable::createStatement);
        db.execute(RuleConditionsTable::createStatement);
    }

    UserId user{0x326};
    RulesTable rules;
    RuleConditionsTable ruleConditions;
    DBHandler dbHandler;
    DBHandler::DatabaseConnection& db;
    DBRuleConditionSerialize cs;
};

class DBRuleSerializeTest : public ::testing::Test
{
public:
    DBRuleSerializeTest() : dbHandler(":memory:"), db(dbHandler.GetDatabase()), as(dbHandler), rs(dbHandler, as)
    {
        db.execute(ActionsTable::createStatement);
        db.execute(SubActionsTable::createStatement);
        db.execute(RulesTable::createStatement);
        db.execute(RuleConditionsTable::createStatement);
        Res::ConditionRegistry().RegisterDefaultConditions();
    }

    void ClearDatabase()
    {
        db(remove_from(rules).unconditionally());
        db(remove_from(actions).unconditionally());
        db(remove_from(ruleConditions).unconditionally());
    }

    ~DBRuleSerializeTest() { Res::ConditionRegistry().RemoveAll(); }

    ActionsTable actions;
    RulesTable rules;
    RuleConditionsTable ruleConditions;
    DBHandler dbHandler;
    DBHandler::DatabaseConnection& db;
    DBActionSerialize as;
    DBRuleSerialize rs;
};

TEST_F(DBRuleConditionSerializeTest, RemoveRuleCondition)
{
    using namespace ::testing;

    // No childs
    {
        const uint64_t condId = 2;
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = condId, ruleConditions.conditionType = 1));
        RuleConstantCondition c{1, false};
        c.SetId(condId);

        cs.RemoveRuleCondition(c, user);
        auto result
            = db(select(ruleConditions.conditionId).from(ruleConditions).where(ruleConditions.conditionId == condId));
        EXPECT_TRUE(result.empty());
    }
    // Does not exist in database (id is 0)
    {
        RuleConstantCondition c{3, false};
        cs.RemoveRuleCondition(c, user);
    }
    // Children
    {
        const uint64_t lId = 1;
        const uint64_t cId = 3;
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = cId, ruleConditions.conditionType = 1));
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = lId, ruleConditions.conditionType = 2));
        auto l = std::make_unique<RuleConstantCondition>(2, false);
        l->SetId(lId);
        auto r = std::make_unique<RuleConstantCondition>(2, false);
        // r has id 0, not deleted
        RuleCompareCondition c{1, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(cId);
        cs.RemoveRuleCondition(c, user);
        auto resultC
            = db(select(ruleConditions.conditionId).from(ruleConditions).where(ruleConditions.conditionId == lId));
        auto resultL
            = db(select(ruleConditions.conditionId).from(ruleConditions).where(ruleConditions.conditionId == cId));
        EXPECT_TRUE(resultC.empty());
        EXPECT_TRUE(resultL.empty());
    }
    // Children with children
    {
        const uint64_t subId = 5;
        const uint64_t lId = 1;
        const uint64_t cId = 3;
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = subId, ruleConditions.conditionType = 2));
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = cId, ruleConditions.conditionType = 1));
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = lId, ruleConditions.conditionType = 2));
        std::unique_ptr<RuleConstantCondition> sub = std::make_unique<RuleConstantCondition>(2, false);
        sub->SetId(subId);
        std::unique_ptr<RuleCompareCondition> l
            = std::make_unique<RuleCompareCondition>(2, std::move(sub), nullptr, RuleCompareCondition::Operator::OR);
        l->SetId(lId);
        std::unique_ptr<RuleConstantCondition> r = std::make_unique<RuleConstantCondition>(2, true);
        // r has id 0, not deleted
        RuleCompareCondition c{1, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(cId);

        cs.RemoveRuleCondition(c, user);
        auto resultSub
            = db(select(ruleConditions.conditionId).from(ruleConditions).where(ruleConditions.conditionId == subId));
        auto resultC
            = db(select(ruleConditions.conditionId).from(ruleConditions).where(ruleConditions.conditionId == lId));
        auto resultL
            = db(select(ruleConditions.conditionId).from(ruleConditions).where(ruleConditions.conditionId == cId));
        EXPECT_TRUE(resultSub.empty());
        EXPECT_TRUE(resultC.empty());
        EXPECT_TRUE(resultL.empty());
    }
}

TEST_F(DBRuleConditionSerializeTest, InsertRuleCondition)
{
    using namespace ::testing;

    std::vector<uint8_t> data;
    // No childs, with id
    {
        const uint64_t cId = 3;
        const uint64_t cType = 2;
        RuleConstantCondition c{cType, false};
        c.SetId(cId);
        messages::RuleCondition msg = c.Serialize(false);
        data.resize(msg.data().ByteSizeLong());
        msg.data().SerializeToArray(data.data(), data.size());
        cs.InsertRuleCondition(c, user);
        auto result = db(select(ruleConditions.conditionId)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == cId && ruleConditions.conditionType == cType
                                 && ruleConditions.conditionData == data));
        EXPECT_FALSE(result.empty());
        db(remove_from(ruleConditions).unconditionally());
    }
    // No childs, without id
    {
        const uint64_t cType = 2;
        RuleConstantCondition c{cType, false};
        messages::RuleCondition msg = c.Serialize(false);
        data.resize(msg.data().ByteSizeLong());
        msg.data().SerializeToArray(data.data(), data.size());
        cs.InsertRuleCondition(c, user);
        auto result = db(select(ruleConditions.conditionId)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionType == cType && ruleConditions.conditionData == data));
        EXPECT_FALSE(result.empty());
        uint64_t insertedId = result.front().conditionId;
        EXPECT_NE(insertedId, 0);
        EXPECT_EQ(insertedId, c.GetId());
        db(remove_from(ruleConditions).unconditionally());
    }
    std::vector<uint8_t> dataL;
    std::vector<uint8_t> dataR;
    // Children
    {
        const uint64_t lId = 5;
        // cId cannot be 1, because r takes that id first
        const uint64_t cId = 3;
        const uint64_t cType = 3;
        // Using type to distinguish l and r, because r id is unknown
        const uint64_t lType = 2;
        const uint64_t rType = 26;
        auto l = std::make_unique<RuleConstantCondition>(lType, false);
        l->SetId(lId);
        messages::RuleCondition msgL = l->Serialize(false);
        dataL.resize(msgL.data().ByteSizeLong());
        msgL.data().SerializeToArray(dataL.data(), dataL.size());
        auto r = std::make_unique<RuleConstantCondition>(rType, false);
        // r has id 0
        messages::RuleCondition msgR = r->Serialize(false);
        dataR.resize(msgR.data().ByteSizeLong());
        msgR.data().SerializeToArray(dataR.data(), dataR.size());
        RuleCompareCondition c{cType, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(cId);
		messages::RuleCompareConditionData value;
		google::protobuf::Any msg = c.Serialize(false).data();
		ASSERT_TRUE(msg.UnpackTo(&value));
		// data can only be serialized later, because r id has to be known first
        cs.InsertRuleCondition(c, user);
        auto resultR = db(select(ruleConditions.conditionId)
                              .from(ruleConditions)
                              .where(ruleConditions.conditionType == rType && ruleConditions.conditionData == dataR));
        auto resultL = db(select(ruleConditions.conditionId)
                              .from(ruleConditions)
                              .where(ruleConditions.conditionId == lId && ruleConditions.conditionType == lType
                                  && ruleConditions.conditionData == dataL));
        EXPECT_FALSE(resultR.empty());
        EXPECT_FALSE(resultL.empty());
        // C select statement checks for changed r id
        uint64_t insertedRId = resultR.front().conditionId;

		// Serialize data with correct id
		value.set_right_id(insertedRId);
		msg.Clear();
		msg.PackFrom(value);
		data.resize(msg.ByteSizeLong());
		msg.SerializeToArray(data.data(), data.size());

        auto resultC = db(select(ruleConditions.conditionId)
                              .from(ruleConditions)
                              .where(ruleConditions.conditionId == cId && ruleConditions.conditionType == cType
                                  && ruleConditions.conditionData == data));
        EXPECT_FALSE(resultC.empty());
        EXPECT_EQ(insertedRId, c.GetChilds()[1]->GetId());
        db(remove_from(ruleConditions).unconditionally());
    }
    // Children with children
    {
        const uint64_t lId = 5;
        const uint64_t subId = 4;
        const uint64_t rId = 1;
        const uint64_t cId = 3;
        const uint64_t cType = 3;
        // Using type to distinguish l and r, because r id is unknown
        const uint64_t lType = 2;
        const uint64_t rType = 26;
        auto sub = std::make_unique<RuleConstantCondition>(lType, false);
        sub->SetId(subId);
        messages::RuleCondition msgS = sub->Serialize(false);
        std::vector<uint8_t> dataS;
        dataS.resize(msgS.data().ByteSizeLong());
        msgS.data().SerializeToArray(dataS.data(), dataS.size());
        auto l = std::make_unique<RuleConstantCondition>(lType, false);
        l->SetId(lId);
        messages::RuleCondition msgL = l->Serialize(false);
        dataL.resize(msgL.data().ByteSizeLong());
        msgL.data().SerializeToArray(dataL.data(), dataL.size());
        auto r = std::make_unique<RuleCompareCondition>(
            rType, std::move(sub), nullptr, RuleCompareCondition::Operator::OR);
        r->SetId(rId);
        messages::RuleCondition msgR = r->Serialize(false);
        dataR.resize(msgR.data().ByteSizeLong());
        msgR.data().SerializeToArray(dataR.data(), dataR.size());
        RuleCompareCondition c{cType, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(cId);
        messages::RuleCondition msg = c.Serialize(false);
        data.resize(msg.data().ByteSizeLong());
        msg.data().SerializeToArray(data.data(), data.size());
        cs.InsertRuleCondition(c, user);
        auto resultSub = db(select(ruleConditions.conditionId)
                                .from(ruleConditions)
                                .where(ruleConditions.conditionId == subId && ruleConditions.conditionType == lType
                                    && ruleConditions.conditionData == dataS));
        auto resultR = db(select(ruleConditions.conditionId)
                              .from(ruleConditions)
                              .where(ruleConditions.conditionId == rId && ruleConditions.conditionType == rType
                                  && ruleConditions.conditionData == dataR));
        auto resultL = db(select(ruleConditions.conditionId)
                              .from(ruleConditions)
                              .where(ruleConditions.conditionId == lId && ruleConditions.conditionType == lType
                                  && ruleConditions.conditionData == dataL));
        EXPECT_FALSE(resultR.empty());
        EXPECT_FALSE(resultL.empty());
        EXPECT_FALSE(resultSub.empty());
        // C select statement checks for changed r id
        auto resultC = db(select(ruleConditions.conditionId)
                              .from(ruleConditions)
                              .where(ruleConditions.conditionId == cId && ruleConditions.conditionType == cType
                                  && ruleConditions.conditionData == data));
        EXPECT_FALSE(resultC.empty());
        db(remove_from(ruleConditions).unconditionally());
    }
}

TEST_F(DBRuleConditionSerializeTest, AddRuleCondition)
{
    using namespace ::testing;

    const uint64_t cId = 2;
    const uint64_t cType = 3;
    db(insert_into(ruleConditions).set(ruleConditions.conditionId = cId, ruleConditions.conditionType = 2));
    RuleConstantCondition c{cType, false};
    c.SetId(cId);
    cs.AddRuleCondition(c, user);
    auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType)
                         .from(ruleConditions)
                         .where(ruleConditions.conditionId == cId));
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(cId, result.front().conditionId);
    EXPECT_EQ(cType, result.front().conditionType);
}

TEST_F(DBRuleConditionSerializeTest, GetRuleCondition)
{
    using namespace ::testing;
    try
    {
        // Success
        {
            const int64_t conditionType = 1;
            auto pMockC = std::make_unique<MockRuleCondition>(conditionType);
            MockRuleCondition& mockC = *pMockC;
            Res::ConditionRegistry().Register(
                RuleConditionInfo{[&](std::size_t) { return std::move(pMockC); }, "c"}, conditionType);

            const uint64_t conditionId = 5;
            db(insert_into(ruleConditions)
                    .set(ruleConditions.conditionId = conditionId, ruleConditions.conditionType = conditionType));
            EXPECT_CALL(mockC, Parse(Ref(cs), Ref(Res::ConditionRegistry()), _, _));
            auto p = cs.GetRuleCondition(conditionId, user);
            EXPECT_EQ(&mockC, p.get());

            db(remove_from(ruleConditions).unconditionally());
            Res::ConditionRegistry().RemoveAll();
        }
        // No results
        {
            const uint64_t conditionId = 5;
            EXPECT_THROW(cs.GetRuleCondition(conditionId, user), std::out_of_range);
        }
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}

TEST_F(DBRuleSerializeTest, AddRule)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x34562};
    // Fails because no condition
    {
        Rule r;
        EXPECT_THROW(rs.AddRule(r, user), std::invalid_argument);
    }
    // Condition with id 0
    {
        const uint64_t newRuleId = 1;
        const uint64_t newConditionId = 1;
        const uint64_t newActionId = 1;

        auto pC = std::make_unique<RuleConstantCondition>(0, false);
        Action a{0, "an", "ai", 0, {}, false};
        Rule r{0, "n", "i", 0, std::move(pC), a, true};
        rs.AddRule(r, user);

        // r was changed in call
        EXPECT_EQ(newConditionId, r.GetCondition().GetId());
        EXPECT_EQ(newActionId, r.GetEffect().GetId());
        EXPECT_EQ(newRuleId, r.GetId());

        std::vector<Action> actions = as.GetAllActions(Filter(), user);
        EXPECT_EQ(1, actions.size());
        a.SetId(newActionId);
        EXPECT_EQ(a, actions.at(0));

        RuleConditions::Ptr cond = rs.GetConditionSerialize().GetRuleCondition(r.GetCondition().GetId(), user);
        EXPECT_EQ(r.GetCondition().GetType(), cond->GetType());
        messages::RuleCondition c = r.GetCondition().Serialize(false);
        EXPECT_TRUE(google::protobuf::util::MessageDifferencer::ApproximatelyEquals(c, cond->Serialize(false)));

        auto ruleResult = db(select(rules.ruleId, rules.actionId, rules.conditionId, rules.ruleName, rules.ruleIconName,
            rules.ruleColor, rules.ruleEnabled)
                                 .from(rules)
                                 .unconditionally());
        auto& ruleFront = ruleResult.front();
        EXPECT_EQ(r.GetId(), ruleFront.ruleId);
        EXPECT_EQ(r.GetName(), ruleFront.ruleName.value());
        EXPECT_EQ(r.GetIcon(), ruleFront.ruleIconName.value());
        EXPECT_EQ(r.GetColor(), ruleFront.ruleColor);
        EXPECT_EQ(r.GetCondition().GetId(), ruleFront.conditionId);
        EXPECT_EQ(r.GetEffect().GetId(), ruleFront.actionId);

        ClearDatabase();
    }
    // Id already set
    {
        const uint64_t ruleId = 3;
        const uint64_t conditionId = 2;
        const uint64_t actionId = 4;

        auto pC = std::make_unique<RuleConstantCondition>(0, false);
        pC->SetId(conditionId);
        Action a{actionId, "an", "ai", 0, {}, false};
        Rule r{ruleId, "n", "i", 0, std::move(pC), a, true};

        rs.AddRule(r, user);
        // r should still have same ids
        EXPECT_EQ(conditionId, r.GetCondition().GetId());
        EXPECT_EQ(actionId, r.GetEffect().GetId());
        EXPECT_EQ(ruleId, r.GetId());

        std::vector<Action> actions = as.GetAllActions(Filter(), user);
        EXPECT_EQ(1, actions.size());
        EXPECT_EQ(a, actions.at(0));

        RuleConditions::Ptr cond = rs.GetConditionSerialize().GetRuleCondition(conditionId, user);
        EXPECT_EQ(r.GetCondition().GetType(), cond->GetType());
        EXPECT_TRUE(google::protobuf::util::MessageDifferencer::ApproximatelyEquals(
            r.GetCondition().Serialize(false), cond->Serialize(false)));

        auto ruleResult = db(select(rules.ruleId, rules.actionId, rules.conditionId, rules.ruleName, rules.ruleIconName,
            rules.ruleColor, rules.ruleEnabled)
                                 .from(rules)
                                 .unconditionally());
        auto& ruleFront = ruleResult.front();
        EXPECT_EQ(r.GetId(), ruleFront.ruleId);
        EXPECT_EQ(r.GetName(), ruleFront.ruleName.value());
        EXPECT_EQ(r.GetIcon(), ruleFront.ruleIconName.value());
        EXPECT_EQ(r.GetColor(), ruleFront.ruleColor);
        EXPECT_EQ(r.GetCondition().GetId(), ruleFront.conditionId);
        EXPECT_EQ(r.GetEffect().GetId(), ruleFront.actionId);

        ClearDatabase();
    }
}

TEST_F(DBRuleSerializeTest, RemoveRule)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x42634};
    // Rule not in database
    {
        const uint64_t ruleId = 3;
        auto c = std::make_unique<RuleConstantCondition>(2, false);
        c->SetId(2);
        Rule r{ruleId, "n", "i", 0, std::move(c), Action(1, "a", "b", 0, {}, false), true};

        rs.RemoveRule(r, user);
    }
    // Rule in database
    {
        const uint64_t ruleId = 3;
        const uint64_t conditionId = 2;
        db(insert_into(ruleConditions).set(ruleConditions.conditionId = conditionId, ruleConditions.conditionType = 1));
        db(insert_into(rules).set(rules.ruleId = ruleId, rules.conditionId = conditionId));
        auto c = std::make_unique<RuleConstantCondition>(2, false);
        c->SetId(conditionId);
        Rule r{ruleId, "n", "i", 0, std::move(c), Action(1, "a", "b", 0, {}, false), true};

        rs.RemoveRule(r, user);
        auto result = db(select(rules.ruleId).from(rules).where(rules.ruleId == ruleId));
        EXPECT_TRUE(result.empty());
    }
}

TEST_F(DBRuleSerializeTest, GetRule)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x1264};
    // No result
    {
        const uint64_t id = 2;
        absl::optional<Rule> result = rs.GetRule(id, user);
        EXPECT_FALSE(result.has_value());
    }
    // Result
    {
        const uint64_t id = 2;
        const char* name = "n";
        const char* icon = "i";
        const uint64_t color = 0x136443;
        const bool enabled = false;
        const uint64_t conditionId = 23;
        const uint64_t actionId = 5;
        const Action action{actionId, "an", "ai", 123, {}, false};

        as.AddAction(action, user);
        auto c = Res::ConditionRegistry().GetCondition(1);
        c->SetId(conditionId);
        rs.GetConditionSerialize().AddRuleCondition(*c, user);

        db(insert_into(rules).set(rules.ruleId = id, rules.ruleName = name, rules.ruleIconName = icon,
            rules.ruleColor = color, rules.ruleEnabled = enabled, rules.conditionId = conditionId,
            rules.actionId = actionId));

        absl::optional<Rule> result = rs.GetRule(id, user);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(id, result->GetId());
        EXPECT_EQ(name, result->GetName());
        EXPECT_EQ(icon, result->GetIcon());
        EXPECT_EQ(color, result->GetColor());
        EXPECT_EQ(enabled, result->IsEnabled());
        EXPECT_EQ(action, result->GetEffect());
        EXPECT_EQ(c->ToJson(), result->GetCondition().ToJson());
    }
}

TEST_F(DBRuleSerializeTest, GetAllRules)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x16364};
    // No result
    {
        std::vector<Rule> results = rs.GetAllRules(Filter(), user);
        EXPECT_TRUE(results.empty());
    }
    // One result
    {
        auto c = Res::ConditionRegistry().GetCondition(1);
        c->SetId(1);
        Rule r{2, "n", "i", 0x1346, std::move(c), Action(3, "", "", 0x52, {}), true};

        rs.AddRule(r, user);
        std::vector<Rule> results = rs.GetAllRules(Filter(), user);
        ASSERT_EQ(1, results.size());
        EXPECT_EQ(r, results.front());
    }
    // Multiple results
    {
        // condition id is same for all to be simpler
        std::vector<Rule> rules = {
            Rule{2, "n", "i", 2355, nullptr, ::Action{5, "an", "ai", 123, {}, false}},
            Rule{3, "n1", "i1", 2351, nullptr, ::Action{6, "an1", "ai1", 1234, {}, false}},
            Rule{4, "n2", "i2", 2352, nullptr, ::Action{7, "an2", "ai2", 1235, {}, false}},
            Rule{5, "n3", "i3", 2353, nullptr, ::Action{8, "an3", "ai3", 1236, {}, false}},
        };
        uint64_t conditionId = 1;
        for (auto& rule : rules)
        {
            auto c = Res::ConditionRegistry().GetCondition(1);
            c->SetId(conditionId);
            ++conditionId;
            rule.SetCondition(std::move(c));
            rs.AddRule(rule, user);
        }
        std::vector<Rule> results = rs.GetAllRules(Filter(), user);
        EXPECT_EQ(rules, results);
    }
}

TEST_F(DBRuleSerializeTest, RemoveRuleId)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x1523};

    // Rule not found
    {
        const uint64_t id = 2;
        rs.RemoveRule(id, user);
    }
    // Success
    {
        auto c = Res::ConditionRegistry().GetCondition(1);
        uint64_t conditionId = 3;
        c->SetId(conditionId);
        Rule r{1, "n", "i", 23463, std::move(c), Action{3, "an", "ai", 25, {}, false}};
        rs.AddRule(r, user);
        rs.RemoveRule(r, user);
        EXPECT_TRUE(rs.GetAllRules(Filter(), user).empty());
    }
}
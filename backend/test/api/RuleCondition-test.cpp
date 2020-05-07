#include <google/protobuf/wrappers.pb.h>
#include <gtest/gtest.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/update.h>

#include "../mocks/MockDeviceSerialize.h"
#include "../mocks/MockRuleCondition.h"
#include "../mocks/MockRuleSerialize.h"
#include "api/DeviceRegistry.h"
#include "api/Rule.h"
#include "api/rule_conditions.pb.h"
#include "database/DBRuleSerialize.h"
#include "database/RulesTable.h"
#include "events/Events.h"

using namespace ::RuleConditions;

TEST(RuleConstantCondition, Constructor)
{
    {
        RuleConstantCondition r {};
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleConstantCondition r;
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleConstantCondition r(1);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleConstantCondition r(2, false);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
}

TEST(RuleConstantCondition, SetId)
{
    RuleConstantCondition r;
    r.SetId(1);
    EXPECT_EQ(1, r.GetId());
    r.SetId(3);
    EXPECT_EQ(3, r.GetId());
}

TEST(RuleConstantCondition, IsSatisfied)
{
    {
        RuleConstantCondition r {};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleConstantCondition r {0, false};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleConstantCondition r {0, true};
        EXPECT_TRUE(r.IsSatisfied());
    }
}

TEST(RuleConstantCondition, IsSatisfiedEvent)
{
    // IsSatisfiedAfterEvent always returns the state
    {
        RuleConstantCondition r {};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleConstantCondition r {0, false};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleConstantCondition r {0, true};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleConstantCondition, ShouldExecuteOn)
{
    // Should execute for any event type
    {
        RuleConstantCondition r;
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::deviceChange));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::ruleChange));
    }
    {
        RuleConstantCondition r {0, false};
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::deviceChange));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::ruleChange));
    }
    {
        RuleConstantCondition r {0, true};
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::deviceChange));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::ruleChange));
    }
}

TEST(RuleConstantCondition, ToJson)
{
    {
        nlohmann::json json {{"id", 0}, {"type", 0}, {"state", false}};
        EXPECT_EQ(json, RuleConstantCondition().ToJson());
    }
    {
        nlohmann::json json {{"id", 0}, {"type", 2}, {"state", true}};
        EXPECT_EQ(json, RuleConstantCondition(2, true).ToJson());
    }
    {
        nlohmann::json json {{"id", 3}, {"type", 1}, {"state", false}};
        RuleConstantCondition r(1, false);
        r.SetId(3);
        EXPECT_EQ(json, r.ToJson());
    }
}

TEST(RuleConstantCondition, ParseJson)
{
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"state", true}};
        RuleConstantCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2},
            // Value not 0 or 1 (is true)
            {"state", 5}};
        RuleConstantCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {// Id field missing (defaulted to 0)
            {"type", 0}, {"state", false}};
        RuleConstantCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"state", true}};
        RuleConstantCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(json, r.ToJson());
    }
    {
        nlohmann::json json {{"id", 1},
            // Type field missing
            {"state", true}};
        RuleConstantCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {
            {"id", 1}, {"type", 1}
            // state field missing
        };
        RuleConstantCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"state", true}, {"v2", "ignored"}};
        RuleConstantCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
    }
}

TEST(RuleConstantCondition, ParseDb)
{
    using namespace ::testing;
    DBHandler dbHandler {":memory:"};
    auto& db = dbHandler.GetDatabase();
    db.execute(RuleConditionsTable::createStatement);
    RuleConditionsTable ruleConditions;
    auto t = sqlpp::start_transaction(db);
    UserHeldTransaction transaction(UserId(0x4326), t);
    google::protobuf::Any any;
    google::protobuf::BoolValue value;
    std::vector<uint8_t> data;
    {
        value.set_value(false);
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        const uint64_t id = 2;
        db(insert_into(ruleConditions)
                .set(ruleConditions.conditionId = id, ruleConditions.conditionType = 3,
                    ruleConditions.conditionData = data));

        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleConstantCondition r;
        r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_FALSE(r.IsSatisfied());
        db(remove_from(ruleConditions).where(ruleConditions.conditionId == id));
    }
    {
        value.set_value(true);
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        const uint64_t id = 2;
        db(insert_into(ruleConditions)
                .set(ruleConditions.conditionId = id, ruleConditions.conditionType = 3,
                    ruleConditions.conditionData = data));

        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleConstantCondition r;
        r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
        db(remove_from(ruleConditions).where(ruleConditions.conditionId == id));
    }
    // Invalid condition data
    {
        any.Clear();
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        const uint64_t id = 2;
        db(insert_into(ruleConditions)
                .set(ruleConditions.conditionId = id, ruleConditions.conditionType = 3,
                    ruleConditions.conditionData = data));

        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleConstantCondition r;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction),
            std::invalid_argument);
        db(remove_from(ruleConditions).where(ruleConditions.conditionId == id));
    }
    // Data is null
    {
        const uint64_t id = 2;
        db(insert_into(ruleConditions)
                .set(ruleConditions.conditionId = id, ruleConditions.conditionType = 3,
                    ruleConditions.conditionData = sqlpp::null));

        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleConstantCondition r;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction),
            std::invalid_argument);
        db(remove_from(ruleConditions).where(ruleConditions.conditionId == id));
    }
    t.commit();
}

TEST(RuleConstantCondition, Create)
{
    Registry reg;
    reg.Register(RuleConditionInfo {[](std::size_t i) { return std::make_unique<RuleConstantCondition>(i); }, "c"}, 2);
    {
        RuleConstantCondition r(2, true);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_FALSE(p->IsSatisfied());
    }
    {
        RuleConstantCondition r(2, false);
        r.SetId(2);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_FALSE(p->IsSatisfied());
    }
}

TEST(RuleConstantCondition, Clone)
{
    Registry reg;
    reg.Register(RuleConditionInfo {[](std::size_t i) { return std::make_unique<RuleConstantCondition>(i); }, "c"}, 2);
    {
        RuleConstantCondition r(2, true);
        r.SetId(3);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(3, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_TRUE(p->IsSatisfied());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
    {
        RuleConstantCondition r(2, false);
        r.SetId(1);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(1, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_FALSE(p->IsSatisfied());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
}

TEST(RuleCompareCondition, Constructor)
{
    {
        RuleCompareCondition r {};
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleCompareCondition r;
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleCompareCondition r(1);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleCompareCondition r(2, std::make_unique<RuleConstantCondition>(1, false),
            std::make_unique<RuleConstantCondition>(1, true), RuleCompareCondition::Operator::EQUAL);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_TRUE(r.HasChilds());
        EXPECT_EQ(2, r.GetChilds().size());
    }
    {
        RuleCompareCondition r(
            3, nullptr, std::make_unique<RuleConstantCondition>(1, true), RuleCompareCondition::Operator::OR);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_TRUE(r.HasChilds());
        EXPECT_EQ(1, r.GetChilds().size());
    }
    {
        RuleCompareCondition r(
            0, std::make_unique<RuleConstantCondition>(1, false), nullptr, RuleCompareCondition::Operator::AND);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_TRUE(r.HasChilds());
        EXPECT_EQ(1, r.GetChilds().size());
    }
}

TEST(RuleCompareCondition, SetId)
{
    RuleCompareCondition r;
    r.SetId(1);
    EXPECT_EQ(1, r.GetId());
    r.SetId(3);
    EXPECT_EQ(3, r.GetId());
}

TEST(RuleCompareCondition, IsSatisfiedAnd)
{
    {
        RuleCompareCondition r;
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::AND};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfied());
    }
}

TEST(RuleCompareCondition, IsSatisfiedOr)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfied());
    }
}

TEST(RuleCompareCondition, IsSatisfiedNand)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NAND};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NAND};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NAND};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfied());
    }
}

TEST(RuleCompareCondition, IsSatisfiedNor)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::NOR};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfied());
    }
}

TEST(RuleCompareCondition, IsSatisfiedEqual)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::EQUAL};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::EQUAL};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
}

TEST(RuleCompareCondition, IsSatisfiedNotEqual)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfied());
    }
}

TEST(RuleCompareCondition, IsSatisfiedEvent)
{
    // Verify that IsSatisfiedAfterEvent is called on children
    // logic is verified in IsSatisfiedEventXXX
    using namespace ::testing;
    {
        std::unique_ptr<MockRuleCondition> pM1 = std::make_unique<MockRuleCondition>(0);
        std::unique_ptr<MockRuleCondition> pM2 = std::make_unique<MockRuleCondition>(0);
        MockRuleCondition& m1 = *pM1;
        MockRuleCondition& m2 = *pM2;
        EXPECT_CALL(m1, ShouldExecuteOn(_)).WillOnce(Return(true));
        EXPECT_CALL(m2, ShouldExecuteOn(_)).WillOnce(Return(true));

        RuleCompareCondition r {1, std::move(pM1), std::move(pM2), RuleCompareCondition::Operator::AND};
        Events::ErrorEvent e = Events::ErrorEvent("a", "b");
        EXPECT_CALL(m1, IsSatisfiedAfterEvent(Ref(e))).WillOnce(Return(true));
        EXPECT_CALL(m2, IsSatisfiedAfterEvent(Ref(e))).WillOnce(Return(true));
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
    }
    {
        std::unique_ptr<MockRuleCondition> pM1 = std::make_unique<MockRuleCondition>(0);
        MockRuleCondition& m1 = *pM1;

        RuleCompareCondition r {1, std::move(pM1), nullptr, RuleCompareCondition::Operator::AND};
        Events::ErrorEvent e = Events::ErrorEvent("a", "b");
        // Sub conditions will only be called if both are set (except or + nor)
        EXPECT_CALL(m1, IsSatisfiedAfterEvent(Ref(e))).Times(0);
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
    }
    {
        std::unique_ptr<MockRuleCondition> pM1 = std::make_unique<MockRuleCondition>(0);
        MockRuleCondition& m1 = *pM1;
        EXPECT_CALL(m1, ShouldExecuteOn(_)).WillOnce(Return(true));

        RuleCompareCondition r {1, std::move(pM1), nullptr, RuleCompareCondition::Operator::NOR};
        Events::ErrorEvent e = Events::ErrorEvent("a", "b");
        EXPECT_CALL(m1, IsSatisfiedAfterEvent(Ref(e))).WillOnce(Return(false));
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
    }
}

TEST(RuleCompareCondition, IsSatisfiedEventAnd)
{
    {
        RuleCompareCondition r;
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::AND};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleCompareCondition, IsSatisfiedEventOr)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::OR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::OR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleCompareCondition, IsSatisfiedEventNand)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NAND};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NAND};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NAND};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::NAND};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleCompareCondition, IsSatisfiedEventNor)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::NOR};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::NOR};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleCompareCondition, IsSatisfiedEventEqual)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::EQUAL};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::EQUAL};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleCompareCondition, IsSatisfiedEventNotEqual)
{
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, false),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_TRUE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, std::make_unique<RuleConstantCondition>(0, true),
            std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, false), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, false), nullptr, RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, nullptr, std::make_unique<RuleConstantCondition>(0, true), RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
    {
        RuleCompareCondition r {
            1, std::make_unique<RuleConstantCondition>(0, true), nullptr, RuleCompareCondition::Operator::NOT_EQUAL};
        EXPECT_FALSE(r.IsSatisfiedAfterEvent(Events::ErrorEvent("a", "b")));
    }
}

TEST(RuleCompareCondition, ShouldExecuteOn)
{
    using namespace ::testing;
    {
        RuleCompareCondition r;
        EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::deviceChange));
    }
    {
        RuleCompareCondition r {1, nullptr, nullptr, RuleCompareCondition::Operator::AND};
        EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::deviceChange));
    }
    {
        std::unique_ptr<MockRuleCondition> pM1 = std::make_unique<MockRuleCondition>(0);
        std::unique_ptr<MockRuleCondition> pM2 = std::make_unique<MockRuleCondition>(0);
        MockRuleCondition& m1 = *pM1;
        MockRuleCondition& m2 = *pM2;

        RuleCompareCondition r {1, std::move(pM1), std::move(pM2), RuleCompareCondition::Operator::AND};
        {
            EventType e = EventTypes::error;
            EXPECT_CALL(m1, ShouldExecuteOn(e)).WillOnce(Return(true));
            // Short circuited
            // EXPECT_CALL(m2, ShouldExecuteOn(e)).WillOnce(Return(true));
            EXPECT_TRUE(r.ShouldExecuteOn(e));
        }
        {
            EventType e = EventTypes::deviceChange;
            EXPECT_CALL(m1, ShouldExecuteOn(e)).WillOnce(Return(false));
            EXPECT_CALL(m2, ShouldExecuteOn(e)).WillOnce(Return(true));
            EXPECT_TRUE(r.ShouldExecuteOn(e));
        }
    }
    {
        std::unique_ptr<MockRuleCondition> pM1 = std::make_unique<MockRuleCondition>(0);
        MockRuleCondition& m1 = *pM1;

        RuleCompareCondition r {1, std::move(pM1), nullptr, RuleCompareCondition::Operator::AND};
        EventType e = EventTypes::error;
        EXPECT_CALL(m1, ShouldExecuteOn(e)).WillOnce(Return(true));
        EXPECT_TRUE(r.ShouldExecuteOn(e));
        EXPECT_CALL(m1, ShouldExecuteOn(e)).WillOnce(Return(false));
        EXPECT_FALSE(r.ShouldExecuteOn(e));
    }
    {
        std::unique_ptr<MockRuleCondition> pM1 = std::make_unique<MockRuleCondition>(0);
        MockRuleCondition& m1 = *pM1;

        RuleCompareCondition r {1, nullptr, std::move(pM1), RuleCompareCondition::Operator::AND};
        EventType e = EventTypes::error;
        EXPECT_CALL(m1, ShouldExecuteOn(e)).WillOnce(Return(true));
        EXPECT_TRUE(r.ShouldExecuteOn(e));
        EXPECT_CALL(m1, ShouldExecuteOn(e)).WillOnce(Return(false));
        EXPECT_FALSE(r.ShouldExecuteOn(e));
    }
}

TEST(RuleCompareCondition, ToJson)
{
    {
        nlohmann::json json {{"id", 0}, {"type", 0}, {"compare", 0}};
        EXPECT_EQ(json, RuleCompareCondition().ToJson());
    }
    {
        RuleCompareCondition::Operator o = RuleCompareCondition::Operator::AND;
        nlohmann::json json {{"id", 0}, {"type", 2}, {"left", RuleConstantCondition(1, false).ToJson()},
            {"right", RuleConstantCondition(1, true).ToJson()}, {"compare", static_cast<int>(o)}};
        RuleCompareCondition r(
            2, std::make_unique<RuleConstantCondition>(1, false), std::make_unique<RuleConstantCondition>(1, true), o);
        EXPECT_EQ(json, r.ToJson());
    }
    {
        RuleCompareCondition::Operator o = RuleCompareCondition::Operator::OR;
        nlohmann::json json {{"id", 2}, {"type", 2}, {"left", RuleConstantCondition(1, false).ToJson()},
            {"right", RuleConstantCondition(1, true).ToJson()}, {"compare", static_cast<int>(o)}};
        RuleCompareCondition r(
            2, std::make_unique<RuleConstantCondition>(1, false), std::make_unique<RuleConstantCondition>(1, true), o);
        r.SetId(2);
        EXPECT_EQ(json, r.ToJson());
    }
}

TEST(RuleCompareCondition, ParseJson)
{
    Registry reg;
    reg.RegisterDefaultConditions();
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"left", RuleConstantCondition(0, false).ToJson()},
            {"right", RuleConstantCondition(0, true).ToJson()},
            {"compare", static_cast<int>(RuleCompareCondition::Operator::OR)}};
        RuleCompareCondition r;
        r.Parse(reg, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_EQ(2, r.GetChilds().size());
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        nlohmann::json json {// Id missing, defaulted to 0
            {"type", 1}, {"left", RuleConstantCondition(0, false).ToJson()},
            {"right", RuleConstantCondition(0, true).ToJson()},
            {"compare", static_cast<int>(RuleCompareCondition::Operator::OR)}};
        RuleCompareCondition r;
        r.Parse(reg, json);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 1},
            // left missing, set to nullptr
            {"right", RuleConstantCondition(0, true).ToJson()},
            {"compare", static_cast<int>(RuleCompareCondition::Operator::OR)}};
        RuleCompareCondition r;
        r.Parse(reg, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_EQ(1, r.GetChilds().size());
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 1}, {"left", RuleConstantCondition(0, false).ToJson()},
            // right missing, set to nullptr
            {"compare", static_cast<int>(RuleCompareCondition::Operator::OR)}};
        RuleCompareCondition r;
        r.Parse(reg, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_EQ(1, r.GetChilds().size());
        EXPECT_FALSE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 1}, {"left", "wrong type, set to nullptr"},
            {"right", RuleConstantCondition(0, false).ToJson()},
            {"compare", static_cast<int>(RuleCompareCondition::Operator::NOR)}};
        RuleCompareCondition r;
        r.Parse(reg, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_EQ(1, r.GetChilds().size());
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 1}, {"left", RuleConstantCondition(0, false).ToJson()},
            {"right", "wrong type, set to nullptr"},
            {"compare", static_cast<int>(RuleCompareCondition::Operator::NOR)}};
        RuleCompareCondition r;
        r.Parse(reg, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_EQ(1, r.GetChilds().size());
        EXPECT_TRUE(r.IsSatisfied());
    }
    {
        nlohmann::json json {{"id", 1},
            // type missing
            {"left", RuleConstantCondition(0, false).ToJson()}, {"right", RuleConstantCondition(0, true).ToJson()},
            {"compare", static_cast<int>(RuleCompareCondition::Operator::OR)}};
        RuleCompareCondition r;
        EXPECT_THROW(r.Parse(reg, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {
            {"id", 1}, {"type", 2}, {"left", RuleConstantCondition(0, false).ToJson()},
            {"right", RuleConstantCondition(0, true).ToJson()},
            // compare missing
        };
        RuleCompareCondition r;
        EXPECT_THROW(r.Parse(reg, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"left", RuleConstantCondition(0, false).ToJson()},
            {"right", RuleConstantCondition(0, true).ToJson()},
            // invalid compare
            {"compare", 0xFFFFFF}};
        RuleCompareCondition r;
        EXPECT_THROW(r.Parse(reg, json), std::out_of_range);
    }
}

TEST(RuleCompareCondition, ParseDb)
{
    using namespace ::testing;
    DBHandler dbHandler {":memory:"};
    auto& db = dbHandler.GetDatabase();
    db.execute(RuleConditionsTable::createStatement);
    RuleConditionsTable ruleConditions;
    const uint64_t id = 2;

    auto t = sqlpp::start_transaction(db);
    UserHeldTransaction transaction(UserId(0x4326), t);
    google::protobuf::Any any;
    messages::RuleCompareConditionData value;
    std::vector<uint8_t> data;
    // No conditions set
    {
        value.set_compare(
            static_cast<messages::RuleCompareConditionData::Operator>(RuleCompareCondition::Operator::OR));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(insert_into(ruleConditions)
                .set(ruleConditions.conditionId = id, ruleConditions.conditionType = 3,
                    ruleConditions.conditionData = data));

        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleCompareCondition r;
        r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_FALSE(r.IsSatisfied());
        EXPECT_FALSE(r.HasChilds());
    }
    // Compare negative
    {
        value.set_compare(static_cast<messages::RuleCompareConditionData::Operator>(-1));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));
        RuleCompareCondition r;
        EXPECT_THROW(
            r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction), std::out_of_range);
    }
    // Compare too big
    {
        value.set_compare(static_cast<messages::RuleCompareConditionData::Operator>(
            static_cast<int>(RuleCompareCondition::Operator::NOT_EQUAL) + 1));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleCompareCondition r;
        EXPECT_THROW(
            r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction), std::out_of_range);
    }
    // Left set
    {
        const std::size_t leftId = 53;
        value.set_compare(
            static_cast<messages::RuleCompareConditionData::Operator>(RuleCompareCondition::Operator::OR));
        value.set_left_id(leftId);
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleCompareCondition r;
        MockConditionSerialize ser;
        EXPECT_CALL(ser, GetRuleCondition(leftId, Ref(transaction)))
            .WillOnce(Return(ByMove(std::make_unique<RuleConstantCondition>(1, true))));
        r.Parse(ser, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
        EXPECT_TRUE(r.HasChilds());
    }
    // Right set
    {
        const std::size_t rightId = 53;
        value.set_compare(
            static_cast<messages::RuleCompareConditionData::Operator>(RuleCompareCondition::Operator::OR));
        value.set_right_id(rightId);
        value.clear_left_id();
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleCompareCondition r;
        MockConditionSerialize ser;
        EXPECT_CALL(ser, GetRuleCondition(rightId, Ref(transaction)))
            .WillOnce(Return(ByMove(std::make_unique<RuleConstantCondition>(1, true))));
        r.Parse(ser, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
        EXPECT_TRUE(r.HasChilds());
    }
    // Both set
    {
        const std::size_t rightId = 53;
        const std::size_t leftId = 54;
        value.set_compare(
            static_cast<messages::RuleCompareConditionData::Operator>(RuleCompareCondition::Operator::NOT_EQUAL));
        value.set_left_id(leftId);
        value.set_right_id(rightId);
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                             .from(ruleConditions)
                             .where(ruleConditions.conditionId == id));

        RuleCompareCondition r;
        MockConditionSerialize ser;
        EXPECT_CALL(ser, GetRuleCondition(rightId, Ref(transaction)))
            .WillOnce(Return(ByMove(std::make_unique<RuleConstantCondition>(1, true))));
        EXPECT_CALL(ser, GetRuleCondition(leftId, Ref(transaction)))
            .WillOnce(Return(ByMove(std::make_unique<RuleConstantCondition>(1, false))));
        r.Parse(ser, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_TRUE(r.IsSatisfied());
        EXPECT_TRUE(r.HasChilds());
    }
    t.commit();
}

TEST(RuleCompareCondition, Create)
{
    Registry reg;
    reg.Register(RuleConditionInfo {[](std::size_t i) { return std::make_unique<RuleCompareCondition>(i); }, "c"}, 2);
    {
        RuleCompareCondition r(
            2, std::make_unique<RuleConstantCondition>(), nullptr, RuleCompareCondition::Operator::NAND);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_FALSE(p->IsSatisfied());
        EXPECT_FALSE(p->HasChilds());
    }
    {
        RuleCompareCondition r(
            2, std::make_unique<RuleConstantCondition>(), nullptr, RuleCompareCondition::Operator::NAND);
        r.SetId(2);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_FALSE(p->IsSatisfied());
        EXPECT_FALSE(p->HasChilds());
    }
}

TEST(RuleCompareCondition, Clone)
{
    using namespace ::testing;
    Registry reg;
    reg.Register(RuleConditionInfo {[](std::size_t i) { return std::make_unique<RuleCompareCondition>(i); }, "c"}, 2);
    reg.Register(
        RuleConditionInfo {
            [&](std::size_t i) {
                std::unique_ptr<MockRuleCondition> m = std::make_unique<MockRuleCondition>(i);
                EXPECT_CALL(*m, Clone(Ref(reg))).Times(AnyNumber()).WillRepeatedly(Invoke([](const Registry& reg) {
                    return reg.GetCondition(1);
                }));
                EXPECT_CALL(*m, IsSatisfied()).WillRepeatedly(Return(false));
                EXPECT_CALL(*m, ToJsonImpl())
                    .Times(AnyNumber())
                    .WillRepeatedly(Return(JsonWrapper({{"type", i}, {"id", 0}, {"stuff", "x"}})));
                return m;
            },
            "c"},
        1);
    {
        RuleCompareCondition r(2, reg.GetCondition(1), nullptr, RuleCompareCondition::Operator::NAND);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_FALSE(p->IsSatisfied());
        EXPECT_TRUE(p->HasChilds());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
    {
        RuleCompareCondition r(2, reg.GetCondition(1), nullptr, RuleCompareCondition::Operator::NOR);
        r.SetId(2);

        RuleCondition::Ptr p = r.Clone(reg);
        EXPECT_EQ(2, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_TRUE(p->IsSatisfied());
        EXPECT_TRUE(p->HasChilds());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
    {
        RuleCompareCondition r(2, nullptr, reg.GetCondition(1), RuleCompareCondition::Operator::NOR);
        r.SetId(2);

        RuleCondition::Ptr p = r.Clone(reg);
        EXPECT_EQ(2, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_TRUE(p->IsSatisfied());
        EXPECT_TRUE(p->HasChilds());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
}

TEST(RuleCompareCondition, GetChilds)
{
    using namespace ::testing;
    {
        std::unique_ptr<RuleCondition> pM1 = std::make_unique<RuleConstantCondition>(1, false);
        std::unique_ptr<RuleCondition> pM2 = std::make_unique<RuleConstantCondition>(1, true);
        RuleCondition* m1 = pM1.get();
        RuleCondition* m2 = pM2.get();
        RuleCompareCondition r(2, std::move(pM1), std::move(pM2), RuleCompareCondition::Operator::EQUAL);
        EXPECT_TRUE(r.HasChilds());
        std::vector<RuleCondition*> childs = r.GetChilds();
        EXPECT_THAT(childs, UnorderedElementsAre(m1, m2));
    }

    {
        std::unique_ptr<RuleCondition> pM1 = std::make_unique<RuleConstantCondition>(1, true);
        RuleCondition* m1 = pM1.get();
        RuleCompareCondition r(3, nullptr, std::move(pM1), RuleCompareCondition::Operator::OR);
        EXPECT_TRUE(r.HasChilds());
        std::vector<RuleCondition*> childs = r.GetChilds();
        EXPECT_THAT(childs, UnorderedElementsAre(m1));
    }

    {
        std::unique_ptr<RuleCondition> pM1 = std::make_unique<RuleConstantCondition>(1, false);
        RuleCondition* m1 = pM1.get();
        RuleCompareCondition r(0, std::move(pM1), nullptr, RuleCompareCondition::Operator::AND);
        EXPECT_TRUE(r.HasChilds());
        std::vector<RuleCondition*> childs = r.GetChilds();
        EXPECT_THAT(childs, UnorderedElementsAre(m1));
    }

    // Const overloads
    {
        std::unique_ptr<RuleCondition> pM1 = std::make_unique<RuleConstantCondition>(1, false);
        std::unique_ptr<RuleCondition> pM2 = std::make_unique<RuleConstantCondition>(1, true);
        RuleCondition* m1 = pM1.get();
        RuleCondition* m2 = pM2.get();
        const RuleCompareCondition r(2, std::move(pM1), std::move(pM2), RuleCompareCondition::Operator::EQUAL);
        EXPECT_TRUE(r.HasChilds());
        std::vector<const RuleCondition*> childs = r.GetChilds();
        EXPECT_THAT(childs, UnorderedElementsAre(m1, m2));
    }

    {
        std::unique_ptr<RuleCondition> pM1 = std::make_unique<RuleConstantCondition>(1, true);
        RuleCondition* m1 = pM1.get();
        const RuleCompareCondition r(3, nullptr, std::move(pM1), RuleCompareCondition::Operator::OR);
        EXPECT_TRUE(r.HasChilds());
        std::vector<const RuleCondition*> childs = r.GetChilds();
        EXPECT_THAT(childs, UnorderedElementsAre(m1));
    }

    {
        std::unique_ptr<RuleCondition> pM1 = std::make_unique<RuleConstantCondition>(1, false);
        RuleCondition* m1 = pM1.get();
        const RuleCompareCondition r(0, std::move(pM1), nullptr, RuleCompareCondition::Operator::AND);
        EXPECT_TRUE(r.HasChilds());
        std::vector<const RuleCondition*> childs = r.GetChilds();
        EXPECT_THAT(childs, UnorderedElementsAre(m1));
    }
}

TEST(RuleTimeCondition, Constructor)
{
    {
        RuleTimeCondition r {};
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleTimeCondition r;
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleTimeCondition r(1);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleTimeCondition r(2, 3, 55, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
}

TEST(RuleTimeCondition, SetId)
{
    RuleTimeCondition r;
    r.SetId(1);
    EXPECT_EQ(1, r.GetId());
    r.SetId(3);
    EXPECT_EQ(3, r.GetId());
}

// TODO: TEST(RuleTimeCondition, IsSatisfied)
// TODO: TEST(RuleTimeCondition, IsSatisfiedEvent)

TEST(RuleTimeCondition, ShouldExecuteOn)
{
    // Should execute for any event type
    {
        RuleTimeCondition r;
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::deviceChange));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::ruleChange));
    }
    {
        RuleTimeCondition r {0, 4, 6, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::absolute};
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::deviceChange));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::ruleChange));
    }
    {
        RuleTimeCondition r {0, 3, 6, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::dayMonth};
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::error));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::deviceChange));
        EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::ruleChange));
    }
}

TEST(RuleTimeCondition, ToJson)
{
    {
        nlohmann::json json {{"id", 0}, {"type", 0}, {"time1", 0}, {"time2", 0}, {"compare", 0},
            {"timeType", static_cast<int>(RuleTimeCondition::Type::year)}};
        EXPECT_EQ(json, RuleTimeCondition().ToJson());
    }
    {
        nlohmann::json json {{"id", 0}, {"type", 2}, {"time1", 345}, {"time2", 724},
            {"compare", static_cast<int>(RuleTimeCondition::Operator::inRange)},
            {"timeType", static_cast<int>(RuleTimeCondition::Type::year)}};
        EXPECT_EQ(json,
            RuleTimeCondition(2, 345, 724, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::year)
                .ToJson());
    }
    {
        nlohmann::json json {{"id", 3}, {"type", 1}, {"time1", 345}, {"time2", 724},
            {"compare", static_cast<int>(RuleTimeCondition::Operator::inRange)},
            {"timeType", static_cast<int>(RuleTimeCondition::Type::year)}};
        RuleTimeCondition r(1, 345, 724, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::year);
        r.SetId(3);
        EXPECT_EQ(json, r.ToJson());
    }
}

TEST(RuleTimeCondition, ParseJson)
{
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", 0}, {"timeType", 0}};
        RuleTimeCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
    }
    {
        nlohmann::json json {// Id field missing (defaulted to 0)
            {"type", 0}, {"time1", 6}, {"time2", 8}, {"compare", 0}, {"timeType", 0}};
        RuleTimeCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", 0}, {"timeType", 0}};
        RuleTimeCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(json, r.ToJson());
    }
    {
        nlohmann::json json {{"id", 1},
            // Type field missing
            {"time1", 6}, {"time2", 8}, {"compare", 0}, {"timeType", 0}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 1},
            // time1 field missing
            {"time2", 8}, {"compare", 0}, {"timeType", 0}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"time1", 6},
            // time2 field missing
            {"compare", 0}, {"timeType", 0}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8},
            // compare field missing
            {"timeType", 0}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {
            {"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", 0},
            // timeType field missing
        };
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), nlohmann::json::exception);
    }
    {
        nlohmann::json json {
            {"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", 0}, {"timeType", 0}, {"add2", "ignored"}};
        RuleTimeCondition r;
        r.Parse(Registry {}, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
    }
    // Operator negative
    {
        nlohmann::json json {
            {"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", -1}, {"timeType", 0}, {"add2", "ignored"}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), std::out_of_range);
    }
    // Operator too big
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8},
            {"compare", static_cast<int>(RuleTimeCondition::Operator::inRange) + 1}, {"timeType", 0},
            {"add2", "ignored"}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), std::out_of_range);
    }
    // Type negative
    {
        nlohmann::json json {
            {"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", 0}, {"timeType", -1}, {"add2", "ignored"}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), std::out_of_range);
    }
    // Type too big
    {
        nlohmann::json json {{"id", 1}, {"type", 2}, {"time1", 6}, {"time2", 8}, {"compare", 0},
            {"timeType", static_cast<int>(RuleTimeCondition::Type::absolute) + 1}, {"add2", "ignored"}};
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(Registry {}, json), std::out_of_range);
    }
}

TEST(RuleTimeCondition, ParseDb)
{
    using namespace ::testing;
    DBHandler dbHandler {":memory:"};
    auto& db = dbHandler.GetDatabase();
    db.execute(RuleConditionsTable::createStatement);
    RuleConditionsTable ruleConditions;
    const uint64_t id = 2;

    auto t = sqlpp::start_transaction(db);
    UserHeldTransaction transaction(UserId(0x4326), t);
    auto selectStatement
        = select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
              .from(ruleConditions)
              .where(ruleConditions.conditionId == id);
    google::protobuf::Any any;
    messages::RuleTimeConditionData value;
    std::vector<uint8_t> data;
    {
        value.set_compare(static_cast<messages::RuleTimeConditionData::Operator>(RuleTimeCondition::Operator::greater));
        value.set_time_type(static_cast<messages::RuleTimeConditionData::Type>(RuleTimeCondition::Type::hourMinSec));
        value.set_time1(3000);
        value.set_time2(4000);
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(insert_into(ruleConditions)
                .set(ruleConditions.conditionId = id, ruleConditions.conditionType = 3,
                    ruleConditions.conditionData = data));

        auto result = db(selectStatement);

        RuleTimeCondition r;
        r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction);
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_EQ(nlohmann::json({{"id", 2}, {"type", 3}, {"time1", 3000}, {"time2", 4000},
                      {"compare", static_cast<int>(RuleTimeCondition::Operator::greater)},
                      {"timeType", static_cast<int>(RuleTimeCondition::Type::hourMinSec)}}),
            r.ToJson());
    }
    // Operator negative
    {
        value.set_compare(static_cast<messages::RuleTimeConditionData::Operator>(-1));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(selectStatement);
        RuleTimeCondition r;
        EXPECT_THROW(
            r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction), std::out_of_range);
    }
    // Operator too big
    {
        value.set_compare(static_cast<messages::RuleTimeConditionData::Operator>(
            static_cast<int>(RuleTimeCondition::Operator::inRange) + 1));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(selectStatement);
        RuleTimeCondition r;
        EXPECT_THROW(
            r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction), std::out_of_range);
    }
    // Type negative
    {
        value.set_compare(static_cast<messages::RuleTimeConditionData::Operator>(RuleTimeCondition::Operator::inRange));
        value.set_time_type(static_cast<messages::RuleTimeConditionData::Type>(-1));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(selectStatement);
        RuleTimeCondition r;
        EXPECT_THROW(
            r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction), std::out_of_range);
    }
    // Type too big
    {
        value.set_time_type(static_cast<messages::RuleTimeConditionData::Type>(
            static_cast<int>(RuleTimeCondition::Type::absolute) + 1));
        any.PackFrom(value);
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(selectStatement);
        RuleTimeCondition r;
        EXPECT_THROW(
            r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction), std::out_of_range);
    }
    // Invalid any
    {
        any.Clear();
        data.resize(any.ByteSizeLong());
        any.SerializeToArray(data.data(), data.size());
        db(update(ruleConditions).where(ruleConditions.conditionId == id).set(ruleConditions.conditionData = data));
        auto result = db(selectStatement);
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction),
            std::invalid_argument);
    }
    // Data is null
    {
        db(update(ruleConditions)
                .where(ruleConditions.conditionId == id)
                .set(ruleConditions.conditionData = sqlpp::null));
        auto result = db(selectStatement);
        RuleTimeCondition r;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize {dbHandler}, Registry {}, result.front(), transaction),
            std::invalid_argument);
    }
    t.commit();
}

TEST(RuleTimeCondition, Create)
{
    Registry reg;
    reg.Register(RuleConditionInfo {[](std::size_t i) { return std::make_unique<RuleTimeCondition>(i); }, "c"}, 2);
    {
        RuleTimeCondition r(2, 1, 2, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
    }
    {
        RuleTimeCondition r(2, 1, 2, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour);
        r.SetId(2);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
    }
}

TEST(RuleTimeCondition, Clone)
{
    Registry reg;
    reg.Register(RuleConditionInfo {[](std::size_t i) { return std::make_unique<RuleTimeCondition>(i); }, "c"}, 2);
    {
        RuleTimeCondition r(2, 1, 2, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour);
        r.SetId(3);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(3, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
    {
        RuleTimeCondition r(2, 1, 2, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour);
        r.SetId(1);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(1, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeHourMinSec)
{
    using std::chrono::system_clock;
    // Time is in the future (on this day)
    // Equals
    {
        system_clock::time_point t = system_clock::now();
        // Add 1 hour to ensure it is in the future
        t += std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::equals,
            RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();
        // Add 1 hour to ensure it is in the future
        t += std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::notEquals,
            RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();
        // Add 1 hour to ensure it is in the future
        t += std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::greater,
            RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();
        // Add 1 hour to ensure it is in the future
        t += std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {
            1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // Expected is on midnight next day
        system_clock::time_point expected
            = t - std::chrono::seconds(hour * 3600 + min * 60 + sec) + std::chrono::hours(24);
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (expected - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - expected))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // InRange
    {
        system_clock::time_point t = system_clock::now();
        // Add 1 hour to ensure it is in the future
        t += std::chrono::hours(1);
        time_t timeT0 = system_clock::to_time_t(t);
        std::tm* tm0 = std::localtime(&timeT0);

        const int hour0 = tm0->tm_hour;
        const int min0 = tm0->tm_min;
        const int sec0 = tm0->tm_sec;

        time_t timeT1 = system_clock::to_time_t(t + std::chrono::hours(1));
        std::tm* tm1 = std::localtime(&timeT1);
        const int hour1 = tm1->tm_hour;
        const int min1 = tm1->tm_min;
        const int sec1 = tm1->tm_sec;

        RuleTimeCondition r {1, hour0 * 3600 + min0 * 60 + sec0, hour1 * 3600 + min1 * 60 + sec1,
            RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is first time
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }

    // Time is in the past (on this day)
    // Equals
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::equals,
            RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::notEquals,
            RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::greater,
            RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;

        RuleTimeCondition r {
            1, hour * 3600 + min * 60 + sec, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // Expected is on midnight next day
        system_clock::time_point expected
            = t - std::chrono::seconds(hour * 3600 + min * 60 + sec) + std::chrono::hours(24);
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (expected - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - expected))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // InRange
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT0 = system_clock::to_time_t(t);
        std::tm* tm0 = std::localtime(&timeT0);

        const int hour0 = tm0->tm_hour;
        const int min0 = tm0->tm_min;
        const int sec0 = tm0->tm_sec;

        time_t timeT1 = system_clock::to_time_t(t + std::chrono::hours(1));
        std::tm* tm1 = std::localtime(&timeT1);
        const int hour1 = tm1->tm_hour;
        const int min1 = tm1->tm_min;
        const int sec1 = tm1->tm_sec;

        RuleTimeCondition r {1, hour0 * 3600 + min0 * 60 + sec0, hour1 * 3600 + min1 * 60 + sec1,
            RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::hourMinSec};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is first time on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeHour)
{
    using std::chrono::system_clock;
    // Time is in the future (on this day)
    // Equals
    {
        system_clock::time_point t = system_clock::now();
        // Add 2 hours to ensure it is in the future
        t += std::chrono::hours(2);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();
        // Add 2 hours to ensure it is in the future
        t += std::chrono::hours(2);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();
        // Add 2 hours to ensure it is in the future
        t += std::chrono::hours(2);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();
        // Add 2 hours to ensure it is in the future
        const int hourOffset = 2;
        t += std::chrono::hours(hourOffset);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // Expected is on next midnight
        system_clock::time_point expected;
        if (hour < hourOffset)
        {
            // Next midnight is on next day
            expected = t - std::chrono::hours(hour);
        }
        else
        {
            // Next midnight is on day after
            expected = t + std::chrono::hours(24 - hour);
        }
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (expected - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(expected - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - expected))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - expected).count() << "ms difference";
    }
    // InRange
    {
        system_clock::time_point t = system_clock::now();
        // Add 2 hours to ensure it is in the future
        t += std::chrono::hours(2);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, hour + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is first time
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }

    // Time is in the past (on this day)
    // Equals
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // Expected is on midnight next day
        system_clock::time_point expected = t + std::chrono::hours(24 - hour);
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (expected - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(expected - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - expected))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - expected).count() << "ms difference";
    }
    // InRange
    {
        system_clock::time_point t = system_clock::now();
        // Subtract 1 hour to ensure it is in the past
        t -= std::chrono::hours(1);
        time_t timeT = system_clock::to_time_t(t);
        std::tm* tm = std::localtime(&timeT);

        const int hour = tm->tm_hour;
        const int min = tm->tm_min;
        const int sec = tm->tm_sec;
        // Set t to beginning of hour
        t -= std::chrono::seconds(min * 60 + sec);

        RuleTimeCondition r {1, hour, hour + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::hour};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance, result is first time on next day
        EXPECT_GT(std::chrono::seconds(1), ((t + std::chrono::hours(24)) - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - (t + std::chrono::hours(24))))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeDayWeek)
{
    using std::chrono::system_clock;
    // Day is in the future
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextDay = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_wday, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextDay - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextDay - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextDay))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextDay).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextDay = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_wday, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextDay - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextDay - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextDay))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextDay).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextDay = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_wday, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextDay - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextDay - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextDay))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextDay).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        const int day = tm.tm_wday;

        tm.tm_mday += 7 - tm.tm_wday;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_wday, tm.tm_wday + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }

    // Day is in the past
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_wday;

        tm.tm_mday += 7;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_wday;

        tm.tm_mday += 7;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_wday;

        tm.tm_mday += 7 - tm.tm_wday;
        // If day was 6 (Saturday), another week has to be skipped because Sunday at 0:00 was in the past
        if (day == 6)
        {
            tm.tm_mday += 7;
        }

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_wday;

        tm.tm_mday += 7;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::dayWeek};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeDayMonth)
{
    using std::chrono::system_clock;
    // Day is in the future
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextDay = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mday, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextDay - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextDay - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextDay))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextDay).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextDay = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_mday, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextDay - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextDay - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextDay))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextDay).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextDay = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mday, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextDay - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextDay - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextDay))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextDay).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        const int day = tm.tm_mday;

        tm.tm_mon += 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday += 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_mday, tm.tm_mday + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }

    // Day is in the past
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_mday;

        tm.tm_mon += 1;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_mday;

        tm.tm_mon += 1;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_mday;

        tm.tm_mon += 1;
        tm.tm_mday = 1;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mday -= 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        // Adjust month if day was first
        std::mktime(&tm);

        const int day = tm.tm_mday;

        tm.tm_mon += 1;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, day, 0, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::dayMonth};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeMonth)
{
    using std::chrono::system_clock;
    // Month is in the future
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mon += 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mon, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mon += 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mon, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mon += 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mon, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        const int mon = tm.tm_mon + 1;
        tm.tm_year += 1;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, mon, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_mon += 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextMonth = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_mon, tm.tm_mon + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextMonth - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextMonth - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextMonth))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextMonth).count() << "ms difference";
    }

    // Month is in the past
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon -= 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mon, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon -= 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mon, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon -= 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_mon, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        const int mon = tm.tm_mon - 1;
        tm.tm_year += 1;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, mon, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon -= 1;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_mon, tm.tm_mon + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::month};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeYear)
{
    using std::chrono::system_clock;
    // Year is in the future
    // Equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_year, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::year};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_year, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::year};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {1, tm.tm_year, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::year};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        RuleTimeCondition r {1, tm.tm_year + 1, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::year};
        system_clock::time_point result = r.GetNextExecutionTime();
        // Less always returns 0
        EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        tm.tm_year += 1;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;

        const auto nextYear = system_clock::from_time_t(std::mktime(&tm));

        RuleTimeCondition r {
            1, tm.tm_year, tm.tm_year + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::year};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (nextYear - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(nextYear - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - nextYear))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - nextYear).count() << "ms difference";
    }

    // Year is in the past
    {
        system_clock::time_point t = system_clock::now();

        time_t timeT = system_clock::to_time_t(t);
        std::tm tm = *std::localtime(&timeT);

        // Equals
        {
            RuleTimeCondition r {
                1, tm.tm_year - 1, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::year};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past year always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // Not equals
        {
            RuleTimeCondition r {
                1, tm.tm_year - 1, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::year};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past year always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // Greater
        {
            RuleTimeCondition r {
                1, tm.tm_year - 1, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::year};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past year always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // Less
        {
            RuleTimeCondition r {
                1, tm.tm_year - 1, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::year};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past year always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // InRange
        {
            RuleTimeCondition r {
                1, tm.tm_year - 1, tm.tm_year + 1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::year};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past year always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
    }
}

TEST(RuleTimeCondition, GetNextExecutionTimeAbsolute)
{
    using std::chrono::system_clock;
    // Time is in the future
    // Equals
    {
        system_clock::time_point t = system_clock::now();
        t += std::chrono::seconds(1);

        time_t timeT = system_clock::to_time_t(t);

        RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::absolute};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Not equals
    {
        system_clock::time_point t = system_clock::now();
        t += std::chrono::seconds(1);

        time_t timeT = system_clock::to_time_t(t);

        RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::absolute};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Greater
    {
        system_clock::time_point t = system_clock::now();
        t += std::chrono::seconds(1);

        time_t timeT = system_clock::to_time_t(t);

        RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::absolute};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }
    // Less
    {
        system_clock::time_point t = system_clock::now();
        t += std::chrono::seconds(1);

        time_t timeT = system_clock::to_time_t(t);

        RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::absolute};
        system_clock::time_point result = r.GetNextExecutionTime();
        EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
    }
    // In Range
    {
        system_clock::time_point t = system_clock::now();
        t += std::chrono::seconds(1);

        time_t timeT = system_clock::to_time_t(t);
        time_t timeT1 = system_clock::to_time_t(t + std::chrono::seconds(1));

        RuleTimeCondition r {1, timeT, timeT1, RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::absolute};
        system_clock::time_point result = r.GetNextExecutionTime();
        // 1 second tolerance
        EXPECT_GT(std::chrono::seconds(1), (t - result))
            << std::chrono::duration_cast<std::chrono::milliseconds>(t - result).count() << "ms difference";
        EXPECT_GT(std::chrono::seconds(1), (result - t))
            << std::chrono::duration_cast<std::chrono::milliseconds>(result - t).count() << "ms difference";
    }

    // Time is in the past
    {
        system_clock::time_point t = system_clock::now();
        t -= std::chrono::seconds(1);

        time_t timeT = system_clock::to_time_t(t);

        // Equals
        {
            RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::equals, RuleTimeCondition::Type::absolute};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past time always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // Not equals
        {
            RuleTimeCondition r {
                1, timeT, 0, RuleTimeCondition::Operator::notEquals, RuleTimeCondition::Type::absolute};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past time always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // Greater
        {
            RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::greater, RuleTimeCondition::Type::absolute};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past time always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // Less
        {
            RuleTimeCondition r {1, timeT, 0, RuleTimeCondition::Operator::less, RuleTimeCondition::Type::absolute};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past time always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
        // InRange
        {
            RuleTimeCondition r {1, timeT, system_clock::to_time_t(t + std::chrono::seconds(1)),
                RuleTimeCondition::Operator::inRange, RuleTimeCondition::Type::absolute};
            system_clock::time_point result = r.GetNextExecutionTime();
            // Past time always returns 0
            EXPECT_EQ(system_clock::time_point(std::chrono::seconds(0)), result);
        }
    }
}

TEST(RuleDeviceCondition, Constructor)
{
    {
        RuleDeviceCondition r {};
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleDeviceCondition r;
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        RuleDeviceCondition r(1);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(1, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
    {
        DBHandler db {":memory:"};
        MockDeviceSerialize deviceSer;
        EventEmitter<Events::DeviceChangeEvent> events;
        EventEmitter<Events::DevicePropertyChangeEvent> pEvents;
        DeviceRegistry deviceReg(deviceSer, events, pEvents);
        RuleDeviceCondition r(2, deviceReg, DeviceId {0}, "test", 4, 5, RuleDeviceCondition::Operator::NOT_EQUALS);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(2, r.GetType());
        EXPECT_FALSE(r.HasChilds());
        EXPECT_TRUE(r.GetChilds().empty());
    }
}

TEST(RuleDeviceCondition, SetId)
{
    RuleDeviceCondition r;
    r.SetId(1);
    EXPECT_EQ(1, r.GetId());
    r.SetId(3);
    EXPECT_EQ(3, r.GetId());
}

#if 0
TEST(RuleDeviceCondition, IsSatisfiedEquals)
{
	using namespace ::testing;
	Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
	Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

	MockDBHandler dbHandler;
	dbHandler.UseDefaults();
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{};
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3 };
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 2, 3, 4, RuleDeviceCondition::Operator::EQUALS };
		// Fails because node not found
		EXPECT_FALSE(r.IsSatisfied());
		Mock::VerifyAndClearExpectations(&dbHandler);
	}
	{
		RuleSensorCondition r{ 3, deviceRegistry, 1, 0, 3, 4, RuleSensorCondition::Operator::EQUALS };
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", {}, {}, "s", NodePath(1, 1)));
		// Fails because sensor not found
		EXPECT_FALSE(r.IsSatisfied());
	}
	// Correct value
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
	// Incorrect value
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	// With variation
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "1";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	// Failed conversion to int
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "abdcer";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
}

TEST(RuleDeviceCondition, IsSatisfiedNotEquals)
{
	MockDBHandler dbHandler;
	dbHandler.UseDefaults();
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::NOT_EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::NOT_EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
	// With variation
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::NOT_EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "1";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
}

TEST(RuleDeviceCondition, IsSatisfiedGreater)
{
	MockDBHandler dbHandler;
	dbHandler.UseDefaults();
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
	// Variation is ignored for greater
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "1";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
}

TEST(RuleDeviceCondition, IsSatisfiedLess)
{
	MockDBHandler dbHandler;
	dbHandler.UseDefaults();
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	// Variation is ignored for less
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "1";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
}

TEST(RuleDeviceCondition, IsSatisfiedRange)
{
	using namespace ::testing;
	Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
	Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

	MockDBHandler dbHandler;
	dbHandler.UseDefaults();
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 4, RuleDeviceCondition::Operator::IN_RANGE };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 5, RuleDeviceCondition::Operator::IN_RANGE };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "6";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());
	}
	// Wrap around (second less than first)
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 4, 1, RuleDeviceCondition::Operator::IN_RANGE };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };

		s.m_state = "0";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "1";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "2";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "3";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_FALSE(r.IsSatisfied());

		s.m_state = "4";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "5";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());

		s.m_state = "6";
		ExpectGetNode(dbHandler, NodeData(1, "n", "l", { s }, {}, "s", NodePath(1, 1)));
		EXPECT_TRUE(r.IsSatisfied());
	}
}

TEST(RuleDeviceCondition, ShouldExecuteOn)
{
	// Should execute for sensor change
	{
		RuleDeviceCondition r;
		EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::sensorChange));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::error));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::nodeMessage));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::nodeChange));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::ruleChange));
	}
	{
		RuleDeviceCondition r{ 1 };
		EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::sensorChange));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::error));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::nodeMessage));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::nodeChange));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::ruleChange));
	}
	{
		DBHandler dbHandler{ "test" };
		DeviceRegistry deviceRegistry;
		RuleDeviceCondition r{ 0, deviceRegistry, 1, 2, 3, 4, RuleDeviceCondition::Operator::EQUALS };
		EXPECT_TRUE(r.ShouldExecuteOn(EventTypes::sensorChange));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::nodeMessage));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::nodeChange));
		EXPECT_FALSE(r.ShouldExecuteOn(EventTypes::ruleChange));
	}
}

TEST(RuleDeviceCondition, IsSatisfiedEvent)
{
	DBHandler dbHandler{ "test" };
	DeviceRegistry deviceRegistry;
	// Wrong event type
	{
		RuleDeviceCondition r{ 1 };
		EXPECT_THROW(r.IsSatisfiedAfterEvent(Events::ErrorEvent("test", "test")), std::logic_error);
	}
	Sensor s{ 1, 0, "n", "l", 1, 2, 1 };
	s.m_state = "1";
	// Wrong node
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 2, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, s }, { 1, 0, s }, Events::SensorFields::ALL);
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	// Wrong sensor
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 1, s }, { 1, 1, s }, Events::SensorFields::ALL);
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	// Should not execute for ADD or REMOVE
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, Sensor::Deleted() }, { 1, 0, s }, Events::SensorFields::ADD);
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
		Events::DeviceChangeEvent e2({ 1, 0, s }, { 1, 0, Sensor::Deleted() }, Events::SensorFields::REMOVE);
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	// Correct sensor (ALL change)
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, s }, { 1, 0, s }, Events::SensorFields::ALL);
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Correct sensor (LOCATION change)
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, s }, { 1, 0, s }, Events::SensorFields::LOCATION);
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Correct sensor (NAME change)
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, s }, { 1, 0, s }, Events::SensorFields::NAME);
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Correct sensor (STATE change)
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, s }, { 1, 0, s }, Events::SensorFields::STATE);
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Correct sensor (TYPE_PIN_LISTENER change)
	{
		RuleDeviceCondition r{ 1, deviceRegistry, 1, 0, 1, 0, RuleDeviceCondition::Operator::EQUALS };
		Events::DeviceChangeEvent e({ 1, 0, s }, { 1, 0, s }, Events::SensorFields::TYPE_PIN_LISTENER);
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Failed conversion to int
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::EQUALS };
		s.m_state = "abdcer";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
}

TEST(RuleDeviceCondition, IsSatisfiedEventEquals)
{
	using namespace ::testing;
	Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
	Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

	DBHandler dbHandler{ "test" };
	DeviceRegistry deviceRegistry;
	// Correct value
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 1, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Incorrect value
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 1, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	// With variation
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::EQUALS };
		Sensor s{ 1, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "3";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "1";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
}

TEST(RuleDeviceCondition, IsSatisfiedEventNotEquals)
{
	DBHandler dbHandler{ "test" };
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::NOT_EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::NOT_EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// With variation
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::NOT_EQUALS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "3";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "1";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
}

TEST(RuleDeviceCondition, IsSatisfiedEventGreater)
{
	DBHandler dbHandler{ "test" };
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "4";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	// Variation is ignored for greater
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::GREATER };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "3";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "1";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
}

TEST(RuleDeviceCondition, IsSatisfiedEventLess)
{
	DBHandler dbHandler{ "test" };
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 0, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "4";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	// Variation is ignored for less
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 1, RuleDeviceCondition::Operator::LESS };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "2";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "3";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "1";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
}

TEST(RuleDeviceCondition, IsSatisfiedEventRange)
{
	using namespace ::testing;
	Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
	Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

	DBHandler dbHandler{ "test" };
	DeviceRegistry deviceRegistry;
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 4, RuleDeviceCondition::Operator::IN_RANGE };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "2";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 3, 5, RuleDeviceCondition::Operator::IN_RANGE };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };
		s.m_state = "3";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "2";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "6";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));
	}
	// Wrap around (second less than first)
	{
		RuleDeviceCondition r{ 3, deviceRegistry, 1, 0, 4, 1, RuleDeviceCondition::Operator::IN_RANGE };
		Sensor s{ 3, 0, "n", "l", 2, 3, 1 };

		s.m_state = "0";
		Events::DeviceChangeEvent e{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "1";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "2";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "3";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_FALSE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "4";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "5";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));

		s.m_state = "6";
		e = Events::DeviceChangeEvent{ {1, 0, s}, {1, 0, s}, Events::SensorFields::ALL };
		EXPECT_TRUE(r.IsSatisfiedAfterEvent(e));
	}
}

TEST(RuleDeviceCondition, ToJson)
{
	{
		nlohmann::json json{ {"id", 0}, {"type", 0}, {"v1", 0}, {"v2", 0}, {"cmp", 0}, {"add1", -1}, {"add2", -1} };
		EXPECT_EQ(json, RuleDeviceCondition().ToJson());
	}
	{
		nlohmann::json json{ {"id", 0}, {"type", 2}, {"v1", 0}, {"v2", 0}, {"cmp", 0}, {"add1", -1}, {"add2", -1} };
		EXPECT_EQ(json, RuleDeviceCondition(2).ToJson());
	}
	{
		nlohmann::json json{ {"id", 3}, {"type", 1}, {"v1", 0}, {"v2", 0}, {"cmp", 0}, {"add1", -1}, {"add2", -1} };
		RuleDeviceCondition r(1);
		r.SetId(3);
		EXPECT_EQ(json, r.ToJson());
	}
	{
		nlohmann::json json{ {"id", 3}, {"type", 1}, {"v1", 4}, {"v2", 5},
			{"cmp", static_cast<int>(RuleDeviceCondition::Operator::GREATER)}, {"add1", 2}, {"add2", 3} };
		DBHandler dbHandler{ "test" };
		DeviceRegistry deviceRegistry;
		RuleDeviceCondition r(1, deviceRegistry, 2, 3, 4, 5, RuleDeviceCondition::Operator::GREATER);
		r.SetId(3);
		EXPECT_EQ(json, r.ToJson());
	}
}



TEST(RuleDeviceCondition, ParseJson)
{
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8}, {"cmp", 0}, {"add1", 0}, {"add2", 0}};
        RuleDeviceCondition r;
        r.Parse( Registry{}, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
    }
    {
        nlohmann::json json{// Id field missing (defaulted to 0)
            {"type", 0}, {"v1", 6}, {"v2", 8}, {"cmp", 0}, {"add1", 0}, {"add2", 0}};
        RuleDeviceCondition r;
        r.Parse(Registry{}, json);
        EXPECT_EQ(0, r.GetId());
        EXPECT_EQ(0, r.GetType());
    }
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8},
            {"cmp", static_cast<int>(RuleDeviceCondition::Operator::LESS)}, {"add1", 23}, {"add2", 10}};
        RuleDeviceCondition r;
        r.Parse( Registry{}, json);
        EXPECT_EQ(json, r.ToJson());
    }
    {
        nlohmann::json json{{"id", 1},
            // Type field missing
            {"v1", 6}, {"v2", 8}, {"cmp", 0}, {"add1", 0}, {"add2", 0}};
        RuleDeviceCondition r;
        EXPECT_THROW(r.Parse( Registry{}, json), std::out_of_range);
    }
    {
        nlohmann::json json{{"id", 1}, {"type", 1},
            // v1 field missing
            {"v2", 8}, {"cmp", 0}, {"add1", 0}, {"add2", 0}};
        RuleDeviceCondition r;
        EXPECT_THROW(r.Parse( Registry{}, json), std::out_of_range);
    }
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6},
            // v2 field missing
            {"cmp", 0}, {"add1", 0}, {"add2", 0}};
        RuleDeviceCondition r;
        EXPECT_THROW(r.Parse( Registry{}, json), std::out_of_range);
    }
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8},
            // cmp field missing
            {"add1", 0}, {"add2", 0}};
        RuleDeviceCondition r;
        EXPECT_THROW(r.Parse( Registry{}, json), std::out_of_range);
    }
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8}, {"cmp", 0},
            // add1 field missing
            {"add2", 0}};
        RuleDeviceCondition r;
        EXPECT_THROW(r.Parse( Registry{}, json), std::out_of_range);
    }
    {
        nlohmann::json json{
            {"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8}, {"cmp", 0}, {"add1", 0}
            // add2 field missing
        };
        RuleDeviceCondition r;
        EXPECT_THROW(r.Parse( Registry{}, json), std::out_of_range);
    }
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8},
            {"cmp", static_cast<int>(RuleDeviceCondition::Operator::IN_RANGE)}, {"add1", 3}, {"add2", 0}};
        RuleDeviceCondition r;
        r.Parse( Registry{}, json);
        EXPECT_EQ(1, r.GetId());
        EXPECT_EQ(2, r.GetType());
        Sensor s{3, 0, "A", "b", 0, 0, 0};
        s.m_state = "6";
        EXPECT_TRUE(
            r.IsSatisfiedAfterEvent(Events::DeviceChangeEvent({3, 0, s}, {3, 0, s}, Events::SensorFields::ALL)));
    }
    // Operator negative
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8}, {"cmp", -1}, {"add1", 3}, {"add2", 0}};
        RuleDeviceCondition r;
        MockDBHandler db;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize{db}, Registry{}, json), std::out_of_range);
    }
    // Operator too big
    {
        nlohmann::json json{{"id", 1}, {"type", 2}, {"v1", 6}, {"v2", 8},
            {"cmp", static_cast<int>(RuleDeviceCondition::Operator::IN_RANGE) + 1}, {"add1", 3}, {"add2", 0}};
        RuleDeviceCondition r;
        MockDBHandler db;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize{db}, Registry{}, json), std::out_of_range);
    }
}

TEST(RuleDeviceCondition, ParseDb)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> pSt = std::make_shared<MockStatement>("test statement;");

    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(3));
        EXPECT_CALL(*pSt, GetInt(2)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt(3)).WillOnce(Return(0));
        EXPECT_CALL(*pSt, GetInt(4)).WillOnce(Return(static_cast<int>(RuleDeviceCondition::Operator::EQUALS)));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(4));
        EXPECT_CALL(*pSt, GetInt(6)).WillOnce(Return(0));

        RuleDeviceCondition r;
        MockDBHandler db;
        r.Parse(DBRuleConditionSerialize{db}, Registry{}, DBResult{pSt});
        EXPECT_EQ(2, r.GetId());
        EXPECT_EQ(3, r.GetType());
        EXPECT_EQ(nlohmann::json({{"id", 2}, {"type", 3}, {"v1", 1}, {"v2", 0},
                      {"cmp", static_cast<int>(RuleDeviceCondition::Operator::EQUALS)}, {"add1", 4}, {"add2", 0}}),
            r.ToJson());
    }
    // Operator negative
    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(3));
        EXPECT_CALL(*pSt, GetInt(2)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt(3)).WillOnce(Return(0));
        EXPECT_CALL(*pSt, GetInt(4)).WillOnce(Return(-1));

        RuleDeviceCondition r;
        MockDBHandler db;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize{db}, Registry{}, DBResult{pSt}), std::out_of_range);
    }
    // Operator too big
    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(3));
        EXPECT_CALL(*pSt, GetInt(2)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt(3)).WillOnce(Return(0));
        EXPECT_CALL(*pSt, GetInt(4)).WillOnce(Return(static_cast<int>(RuleDeviceCondition::Operator::IN_RANGE) + 1));

        RuleDeviceCondition r;
        MockDBHandler db;
        EXPECT_THROW(r.Parse(DBRuleConditionSerialize{db}, Registry{}, DBResult{pSt}), std::out_of_range);
    }
}

TEST(RuleDeviceCondition, GetFields)
{
    using namespace ::testing;
    {
        RuleDeviceCondition r;
        EXPECT_THAT(r.GetFields(), ElementsAre(0, 0, 0, -1, -1));
    }
    {
        DBHandler dbHandler{"test"};
        DeviceRegistry deviceRegistry;
        RuleDeviceCondition r{2, deviceRegistry, 4, 1, 10, 0, RuleDeviceCondition::Operator::GREATER};
        EXPECT_THAT(
            r.GetFields(), ElementsAre(10, 0, static_cast<int64_t>(RuleDeviceCondition::Operator::GREATER), 4, 1));
    }
}

TEST(RuleDeviceCondition, Create)
{
    Registry reg;
    reg.Register(RuleConditionInfo{[](std::size_t i) { return std::make_unique<RuleDeviceCondition>(i); }, "c"}, 2);
    DBHandler dbHandler{"test"};
    DeviceRegistry deviceRegistry;
    {
        RuleDeviceCondition r(2, deviceRegistry, 1, 2, 3, 0, RuleDeviceCondition::Operator::EQUALS);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
    }
    {
        RuleDeviceCondition r(2, deviceRegistry, 1, 2, 3, 0, RuleDeviceCondition::Operator::EQUALS);
        r.SetId(2);

        RuleCondition::Ptr p = r.Create(reg);

        EXPECT_EQ(0, p->GetId());
        EXPECT_EQ(2, p->GetType());
    }
}

TEST(RuleDeviceCondition, Clone)
{
    Registry reg;
    reg.Register(RuleConditionInfo{[](std::size_t i) { return std::make_unique<RuleDeviceCondition>(i); }, "c"}, 2);
    DBHandler dbHandler{"test"};
    DeviceRegistry deviceRegistry;
    {
        RuleDeviceCondition r(2, deviceRegistry, 1, 2, 3, 0, RuleDeviceCondition::Operator::EQUALS);
        r.SetId(3);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(3, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
    {
        RuleDeviceCondition r(2, deviceRegistry, 1, 2, 3, 0, RuleDeviceCondition::Operator::EQUALS);
        r.SetId(1);

        RuleCondition::Ptr p = r.Clone(reg);

        EXPECT_EQ(1, p->GetId());
        EXPECT_EQ(2, p->GetType());
        EXPECT_EQ(r.ToJson(), p->ToJson());
    }
}
#endif
#include "database/DBRuleSerialize.h"

#include <gtest/gtest.h>

#include "../api/ActionSerializeUtils.h"
#include "../mocks/MockDBHandler.h"
#include "../mocks/MockRuleCondition.h"
#include "database/DBActionSerialize.h"

using namespace ::RuleConditions;

TEST(DBRuleConditionSerialize, RemoveRuleCondition)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBRuleConditionSerialize cs{dbHandler};

    // No childs
    {

        const char* deleteStatement = "DELETE FROM rule_conditions WHERE condition_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(deletePair.first));
            dbHandler.ExpectSavepointRelease("RemoveRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(3);
        {
            InSequence s;
            EXPECT_CALL(stD, BindInt64(1, 3));
            EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
        }

        cs.RemoveRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Delete failed
    {

        const char* deleteStatement = "DELETE FROM rule_conditions WHERE condition_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(deletePair.first));
            dbHandler.ExpectSavepointRollback("RemoveRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(3);
        {
            InSequence s;
            EXPECT_CALL(stD, BindInt64(1, 3));
            EXPECT_CALL(stD, Step()).WillOnce(Return(MockDatabase::error));
        }
        EXPECT_CALL(stD, GetError()).WillRepeatedly(Return("test"));

        EXPECT_THROW(cs.RemoveRuleCondition(c), std::runtime_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Does not exist in database (id is 0)
    {

        const char* deleteStatement = "DELETE FROM rule_conditions WHERE condition_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(deletePair.first));
            dbHandler.ExpectSavepointRelease("RemoveRuleCondition");
        }
        RuleConstantCondition c{2, false};
        EXPECT_CALL(stD, Step()).Times(0);

        cs.RemoveRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Children
    {

        const char* deleteStatement = "DELETE FROM rule_conditions WHERE condition_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(deletePair.first));
            dbHandler.ExpectSavepointRelease("RemoveRuleCondition");
        }
        std::unique_ptr<RuleConstantCondition> l = std::make_unique<RuleConstantCondition>(2, false);
        l->SetId(1);
        std::unique_ptr<RuleConstantCondition> r = std::make_unique<RuleConstantCondition>(2, true);
        // r has id 0, not deleted
        RuleCompareCondition c{1, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(3);
        {
            ExpectationSet cDone;
            {
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, 3));
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
                cDone += EXPECT_CALL(stD, Reset());
            }
            {
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, 1)).After(cDone);
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
                EXPECT_CALL(stD, Reset());
            }
        }

        cs.RemoveRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Children with children
    {

        const char* deleteStatement = "DELETE FROM rule_conditions WHERE condition_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(deletePair.first));
            dbHandler.ExpectSavepointRelease("RemoveRuleCondition");
        }
        std::unique_ptr<RuleConstantCondition> sub = std::make_unique<RuleConstantCondition>(2, false);
        sub->SetId(5);
        std::unique_ptr<RuleCompareCondition> l
            = std::make_unique<RuleCompareCondition>(2, std::move(sub), nullptr, RuleCompareCondition::Operator::OR);
        l->SetId(1);
        std::unique_ptr<RuleConstantCondition> r = std::make_unique<RuleConstantCondition>(2, true);
        // r has id 0, not deleted
        RuleCompareCondition c{1, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(3);
        {
            ExpectationSet cDone;
            // Parent
            {
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, 3));
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
                cDone += EXPECT_CALL(stD, Reset());
            }
            ExpectationSet lDone;
            // l
            {
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, 1)).After(cDone);
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
                lDone += EXPECT_CALL(stD, Reset());
            }
            // sub
            {
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, 5)).After(lDone);
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
                EXPECT_CALL(stD, Reset());
            }
        }

        cs.RemoveRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(DBRuleConditionSerialize, InsertRuleCondition)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBRuleConditionSerialize cs{dbHandler};

    // No childs, with id
    {

        const char* insertStatement
            = "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
              "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const char* insertNoIdStatement
            = "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        auto insertNoIdPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stNoIdI = insertNoIdPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(insertPair.first));
            EXPECT_CALL(dbHandler, GetStatement(insertNoIdStatement)).WillOnce(Return(insertNoIdPair.first));
            dbHandler.ExpectSavepointRelease("InsertRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(3);
        std::array<int64_t, 5> values = c.GetFields();
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stI, BindInt64(1, c.GetId())).RetiresOnSaturation();
            bound += EXPECT_CALL(stI, BindInt64(2, c.GetType())).RetiresOnSaturation();
            for (std::size_t i = 0; i < 5; ++i)
            {
                bound += EXPECT_CALL(stI, BindInt64(i + 3, values[i])).RetiresOnSaturation();
            }
            {
                InSequence s;
                EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done));
                EXPECT_CALL(stI, Reset());
            }
        }

        EXPECT_CALL(stNoIdI, Step()).Times(0);

        cs.InsertRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Insert failed
    {

        const char* insertStatement
            = "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
              "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const char* insertNoIdStatement
            = "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        auto insertNoIdPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stNoIdI = insertNoIdPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(insertPair.first));
            EXPECT_CALL(dbHandler, GetStatement(insertNoIdStatement)).WillOnce(Return(insertNoIdPair.first));
            dbHandler.ExpectSavepointRollback("InsertRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(3);
        std::array<int64_t, 5> values = c.GetFields();
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stI, BindInt64(1, c.GetId())).RetiresOnSaturation();
            bound += EXPECT_CALL(stI, BindInt64(2, c.GetType())).RetiresOnSaturation();
            for (std::size_t i = 0; i < 5; ++i)
            {
                bound += EXPECT_CALL(stI, BindInt64(i + 3, values[i])).RetiresOnSaturation();
            }
            {
                InSequence s;
                EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockDatabase::error));
                // No reset after error
            }
            EXPECT_CALL(stI, GetError()).WillRepeatedly(Return("test"));
        }

        EXPECT_CALL(stNoIdI, Step()).Times(0);

        EXPECT_THROW(cs.InsertRuleCondition(c), std::runtime_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // No childs, without id
    {

        const char* insertStatement
            = "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
              "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const char* insertNoIdStatement
            = "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        auto insertNoIdPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stNoIdI = insertNoIdPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(insertPair.first));
            EXPECT_CALL(dbHandler, GetStatement(insertNoIdStatement)).WillOnce(Return(insertNoIdPair.first));
            dbHandler.ExpectSavepointRelease("InsertRuleCondition");
        }
        RuleConstantCondition c{2, false};
        std::array<int64_t, 5> values = c.GetFields();
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stNoIdI, BindInt64(1, c.GetType())).RetiresOnSaturation();
            for (std::size_t i = 0; i < 5; ++i)
            {
                bound += EXPECT_CALL(stNoIdI, BindInt64(i + 2, values[i])).RetiresOnSaturation();
            }
            {
                InSequence s;
                EXPECT_CALL(stNoIdI, Step()).After(bound).WillOnce(Return(MockStatement::done));
                EXPECT_CALL(stNoIdI, Reset());
                EXPECT_CALL(dbHandler, GetLastInsertRowid()).WillOnce(Return(4));
            }
        }

        EXPECT_CALL(stI, Step()).Times(0);

        cs.InsertRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Children
    {
        const char* insertStatement
            = "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
              "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const char* insertNoIdStatement
            = "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        auto insertNoIdPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stNoIdI = insertNoIdPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(insertPair.first));
            EXPECT_CALL(dbHandler, GetStatement(insertNoIdStatement)).WillOnce(Return(insertNoIdPair.first));
            dbHandler.ExpectSavepointRelease("InsertRuleCondition");
        }
        const int64_t lId = 5;
        const int64_t type = 2;
        std::unique_ptr<RuleConstantCondition> l = std::make_unique<RuleConstantCondition>(type, false);
        l->SetId(lId);
        std::array<int64_t, 5> valuesL = l->GetFields();
        std::unique_ptr<RuleConstantCondition> r = std::make_unique<RuleConstantCondition>(type, true);
        // r has id 0
        const int64_t futureRId = 4;
        std::array<int64_t, 5> valuesR = r->GetFields();
        RuleCompareCondition c{1, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(3);
        std::array<int64_t, 5> valuesC = c.GetFields();
        // Id of r whill be changed in insert
        valuesC[1] = futureRId;
        {
            // Children are added BEFORE parents
            ExpectationSet childrenDone;
            // L
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stI, BindInt64(1, lId)).RetiresOnSaturation();
                bound += EXPECT_CALL(stI, BindInt64(2, type)).RetiresOnSaturation();
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stI, BindInt64(i + 3, valuesL[i])).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done));
                    childrenDone += EXPECT_CALL(stI, Reset());
                }
            }
            // R
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stNoIdI, BindInt64(1, type)).RetiresOnSaturation();
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stNoIdI, BindInt64(i + 2, valuesR[i])).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stNoIdI, Step()).After(bound).WillOnce(Return(MockStatement::done));
                    EXPECT_CALL(stNoIdI, Reset());
                    childrenDone += EXPECT_CALL(dbHandler, GetLastInsertRowid()).WillOnce(Return(futureRId));
                }
            }
            // C
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stI, BindInt64(1, c.GetId())).After(childrenDone);
                bound += EXPECT_CALL(stI, BindInt64(2, c.GetType())).After(childrenDone);
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stI, BindInt64(i + 3, valuesC[i])).After(childrenDone).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done));
                    EXPECT_CALL(stI, Reset());
                }
            }
        }

        cs.InsertRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Children with children
    {

        const char* insertStatement
            = "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
              "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const char* insertNoIdStatement
            = "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        auto insertNoIdPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stNoIdI = insertNoIdPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(insertPair.first));
            EXPECT_CALL(dbHandler, GetStatement(insertNoIdStatement)).WillOnce(Return(insertNoIdPair.first));
            dbHandler.ExpectSavepointRelease("InsertRuleCondition");
        }
        const int64_t type = 2;
        const int64_t subId = 5;
        std::unique_ptr<RuleConstantCondition> sub = std::make_unique<RuleConstantCondition>(type, false);
        sub->SetId(subId);
        std::array<int64_t, 5> valuesSub = sub->GetFields();
        const int64_t lId = 2;
        std::unique_ptr<RuleConstantCondition> l = std::make_unique<RuleConstantCondition>(type, true);
        l->SetId(lId);
        std::array<int64_t, 5> valuesL = l->GetFields();
        const int64_t rId = 1;
        std::unique_ptr<RuleCompareCondition> r
            = std::make_unique<RuleCompareCondition>(1, std::move(sub), nullptr, RuleCompareCondition::Operator::OR);
        r->SetId(rId);
        std::array<int64_t, 5> valuesR = r->GetFields();
        RuleCompareCondition c{1, std::move(l), std::move(r), RuleCompareCondition::Operator::OR};
        c.SetId(3);
        std::array<int64_t, 5> valuesC = c.GetFields();
        {
            // If this test fails, maybe this order got messed up
            // It might be better to just use InSequence everywhere instead of ExpectationSets,
            // because some binds might be attributed to the wrong one

            // Children are added BEFORE parents
            ExpectationSet childrenDone;
            // L
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stI, BindInt64(1, lId)).RetiresOnSaturation();
                bound += EXPECT_CALL(stI, BindInt64(2, type)).RetiresOnSaturation();
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stI, BindInt64(i + 3, valuesL[i])).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                    childrenDone += EXPECT_CALL(stI, Reset()).RetiresOnSaturation();
                }
            }
            ExpectationSet subDone;
            // Sub is child of r, inserted after l and before r
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stI, BindInt64(1, subId)).RetiresOnSaturation();
                bound += EXPECT_CALL(stI, BindInt64(2, type)).RetiresOnSaturation();
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stI, BindInt64(i + 3, valuesSub[i])).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                    subDone += EXPECT_CALL(stI, Reset()).RetiresOnSaturation();
                }
            }
            // R
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stI, BindInt64(1, rId)).After(subDone).RetiresOnSaturation();
                bound += EXPECT_CALL(stI, BindInt64(2, 1)).After(subDone).RetiresOnSaturation();
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stI, BindInt64(i + 3, valuesR[i])).After(subDone).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                    childrenDone += EXPECT_CALL(stI, Reset()).RetiresOnSaturation();
                }
            }
            // C
            {
                ExpectationSet bound;
                bound += EXPECT_CALL(stI, BindInt64(1, c.GetId())).After(childrenDone);
                bound += EXPECT_CALL(stI, BindInt64(2, c.GetType())).After(childrenDone);
                for (std::size_t i = 0; i < 5; ++i)
                {
                    bound += EXPECT_CALL(stI, BindInt64(i + 3, valuesC[i])).After(childrenDone).RetiresOnSaturation();
                }
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                    EXPECT_CALL(stI, Reset()).RetiresOnSaturation();
                }
            }
        }

        EXPECT_CALL(stNoIdI, Step()).Times(0);

        cs.InsertRuleCondition(c);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(DBRuleConditionSerialize, AddRuleCondition)
{
    using namespace ::testing;

    MockDBHandler db;
    db.UseDefaults();

    DBRuleConditionSerialize cs{db};
    // Success
    {
        EXPECT_CALL(db.db, ExecuteStatement(_)).Times(AnyNumber());
        // Just uses one remove and one insert
        {
            InSequence s;
            EXPECT_CALL(db, GetSavepoint("AddRuleCondition"));
            EXPECT_CALL(db, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(db, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"));
            db.ExpectSavepointRelease("RemoveRuleCondition");
            EXPECT_CALL(db, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(db,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
                    "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            EXPECT_CALL(db,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
                    "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);"));
            db.ExpectSavepointRelease("InsertRuleCondition");
            db.ExpectSavepointRelease("AddRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(2);
        cs.AddRuleCondition(c);
    }
    // Fail in delete
    {
        EXPECT_CALL(db.db, ExecuteStatement(_)).Times(AnyNumber());
        // Just uses one remove and one insert
        {
            InSequence s;
            EXPECT_CALL(db, GetSavepoint("AddRuleCondition"));
            EXPECT_CALL(db, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(db, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"))
                .WillOnce(Throw(std::runtime_error("test")));
            db.ExpectSavepointRollback("RemoveRuleCondition");
            db.ExpectSavepointRollback("AddRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(2);
        EXPECT_THROW(cs.AddRuleCondition(c), std::runtime_error);
    }
    // Fails in insert
    {
        EXPECT_CALL(db.db, ExecuteStatement(_)).Times(AnyNumber());
        // Just uses one remove and one insert
        {
            InSequence s;
            EXPECT_CALL(db, GetSavepoint("AddRuleCondition"));
            EXPECT_CALL(db, GetSavepoint("RemoveRuleCondition"));
            EXPECT_CALL(db, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"));
            db.ExpectSavepointRelease("RemoveRuleCondition");
            EXPECT_CALL(db, GetSavepoint("InsertRuleCondition"));
            EXPECT_CALL(db,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
                    "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            EXPECT_CALL(db,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
                    "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);"))
                .WillOnce(Throw(std::runtime_error("test")));
            db.ExpectSavepointRollback("InsertRuleCondition");
            db.ExpectSavepointRollback("AddRuleCondition");
        }
        RuleConstantCondition c{2, false};
        c.SetId(2);
        EXPECT_THROW(cs.AddRuleCondition(c), std::runtime_error);
    }
}

TEST(DBRuleConditionSerialize, GetRuleCondition)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBRuleConditionSerialize cs{dbHandler};
    try
    {
        // Success
        {
            const int64_t conditionType = 1;
            auto pMockC = std::make_unique<MockRuleCondition>(conditionType);
            MockRuleCondition& mockC = *pMockC;
            Res::ConditionRegistry().Register(
                RuleConditionInfo{[&](std::size_t) { return std::move(pMockC); }, "c"}, conditionType);

            const int64_t conditionId = 5;
            const char* selectStatement
                = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
                  "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
            auto selectPair = dbHandler.GetMockedStatement(selectStatement);
            MockStatement& st = selectPair.second;
            {
                InSequence s;
                EXPECT_CALL(dbHandler, GetROSavepoint("GetRuleCondition"));
                EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(selectPair.first)));
                dbHandler.ExpectROSavepointRelease("GetRuleCondition");
            }
            ExpectationSet step;
            {
                InSequence s;
                EXPECT_CALL(st, BindInt64(1, conditionId));
                step += EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row));
            }
            {
                EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
                EXPECT_CALL(st, GetInt64(1)).After(step).WillOnce(Return(conditionType));
                EXPECT_CALL(mockC, Parse(Ref(cs), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_))).After(step);
            }

            auto p = cs.GetRuleCondition(conditionId);
            EXPECT_EQ(&mockC, p.get());

            Mock::VerifyAndClearExpectations(&dbHandler);
            Res::ConditionRegistry().RemoveAll();
        }
        // No results
        {
            const int64_t conditionId = 5;
            const char* selectStatement
                = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
                  "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
            auto selectPair = dbHandler.GetMockedStatement(selectStatement);
            MockStatement& st = selectPair.second;
            {
                InSequence s;
                EXPECT_CALL(dbHandler, GetROSavepoint("GetRuleCondition"));
                EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(selectPair.first)));
                dbHandler.ExpectROSavepointRollback("GetRuleCondition");
            }
            ExpectationSet step;
            {
                InSequence s;
                EXPECT_CALL(st, BindInt64(1, conditionId));
                step += EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
            }

            EXPECT_THROW(cs.GetRuleCondition(conditionId), std::out_of_range);

            Mock::VerifyAndClearExpectations(&dbHandler);
        }
        // Fail
        {
            const int64_t conditionId = 5;
            const char* selectStatement
                = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
                  "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
            auto selectPair = dbHandler.GetMockedStatement(selectStatement);
            MockStatement& st = selectPair.second;
            {
                InSequence s;
                EXPECT_CALL(dbHandler, GetROSavepoint("GetRuleCondition"));
                EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(selectPair.first)));
                dbHandler.ExpectROSavepointRollback("GetRuleCondition");
            }
            {
                InSequence s;
                EXPECT_CALL(st, BindInt64(1, conditionId));
                EXPECT_CALL(st, Step()).WillOnce(Return(MockDatabase::error));
            }
            EXPECT_CALL(st, GetError()).WillRepeatedly(Return("test"));

            EXPECT_THROW(cs.GetRuleCondition(conditionId), std::runtime_error);

            Mock::VerifyAndClearExpectations(&dbHandler);
        }
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}

TEST(DBRuleSerialize, AddRuleOnly)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};
    // Rule did not exist (id 0)
    {
        const int64_t newId = 3;
        const char* insertStatement = "INSERT INTO rules (rule_name, rule_icon_name, rule_color, condition_id, "
                                      "action_id, rule_enabled) VALUES (?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRuleOnly"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(insertPair.first)));
            dbHandler.ExpectSavepointRelease("AddRuleOnly");
        }
        const unsigned int color = 0x01792346;
        const int64_t conditionId = 3;
        const int64_t actionId = 1;
        auto condition = std::make_unique<RuleConstantCondition>(1, false);
        condition->SetId(conditionId);
        ::Action a;
        a.SetId(actionId);
        Rule r{0, "name", "icon", color, std::move(condition), a, false};
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stI, BindString(1, "name"));
            bound += EXPECT_CALL(stI, BindString(2, "icon"));
            bound += EXPECT_CALL(stI, BindInt64(3, color));
            bound += EXPECT_CALL(stI, BindInt64(4, conditionId));
            bound += EXPECT_CALL(stI, BindInt64(5, actionId));
            bound += EXPECT_CALL(stI, BindInt(6, false));
            {
                InSequence s;
                EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockStatement::done));
                EXPECT_CALL(dbHandler, GetLastInsertRowid()).WillOnce(Return(newId));
            }
        }
        EXPECT_EQ(newId, rs.AddRuleOnly(r));

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Rule existed (id not 0)
    {
        const char* updateStatement = "UPDATE rules SET rule_name=?1, rule_icon_name=?2, rule_color=?3, "
                                      "condition_id=?4, action_id=?5, rule_enabled=?6 WHERE rule_id=?7;";
        auto updatePair = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = updatePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRuleOnly"));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(updatePair.first)));
            dbHandler.ExpectSavepointRelease("AddRuleOnly");
        }
        const unsigned int color = 0x01792346;
        const size_t conditionId = 3;
        const size_t actionId = 1;
        const size_t ruleId = 4;
        auto condition = std::make_unique<RuleConstantCondition>(1, false);
        condition->SetId(conditionId);
        ::Action a;
        a.SetId(actionId);
        Rule r{ruleId, "name", "icon", color, std::move(condition), a, false};
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stU, BindString(1, "name"));
            bound += EXPECT_CALL(stU, BindString(2, "icon"));
            bound += EXPECT_CALL(stU, BindInt64(3, color));
            bound += EXPECT_CALL(stU, BindInt64(4, conditionId));
            bound += EXPECT_CALL(stU, BindInt64(5, actionId));
            bound += EXPECT_CALL(stU, BindInt(6, false));
            bound += EXPECT_CALL(stU, BindInt64(7, ruleId));
            EXPECT_CALL(stU, Step()).After(bound).WillOnce(Return(MockStatement::done));
        }
        EXPECT_EQ(ruleId, rs.AddRuleOnly(r));

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Insert failed
    {
        const char* insertStatement = "INSERT INTO rules (rule_name, rule_icon_name, rule_color, condition_id, "
                                      "action_id, rule_enabled) VALUES (?1,?2,?3,?4,?5,?6);";
        auto insertPair = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = insertPair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRuleOnly"));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(insertPair.first)));
            dbHandler.ExpectSavepointRollback("AddRuleOnly");
        }
        const unsigned int color = 0x01792346;
        const int64_t conditionId = 3;
        const int64_t actionId = 1;
        auto condition = std::make_unique<RuleConstantCondition>(1, false);
        condition->SetId(conditionId);
        ::Action a;
        a.SetId(actionId);
        Rule r{0, "name", "icon", color, std::move(condition), a, false};
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stI, BindString(1, "name"));
            bound += EXPECT_CALL(stI, BindString(2, "icon"));
            bound += EXPECT_CALL(stI, BindInt64(3, color));
            bound += EXPECT_CALL(stI, BindInt64(4, conditionId));
            bound += EXPECT_CALL(stI, BindInt64(5, actionId));
            bound += EXPECT_CALL(stI, BindInt(6, false));
            EXPECT_CALL(stI, Step()).After(bound).WillOnce(Return(MockDatabase::error));
            EXPECT_CALL(stI, GetError()).WillRepeatedly(Return("test"));
        }
        EXPECT_THROW(rs.AddRuleOnly(r), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Update failed
    {
        const char* updateStatement = "UPDATE rules SET rule_name=?1, rule_icon_name=?2, rule_color=?3, "
                                      "condition_id=?4, action_id=?5, rule_enabled=?6 WHERE rule_id=?7;";
        auto updatePair = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = updatePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRuleOnly"));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(updatePair.first)));
            dbHandler.ExpectSavepointRollback("AddRuleOnly");
        }
        const unsigned int color = 0x01792346;
        const size_t conditionId = 3;
        const size_t actionId = 1;
        const size_t ruleId = 4;
        auto condition = std::make_unique<RuleConstantCondition>(1, false);
        condition->SetId(conditionId);
        ::Action a;
        a.SetId(actionId);
        Rule r{ruleId, "name", "icon", color, std::move(condition), a, false};
        {
            ExpectationSet bound;
            bound += EXPECT_CALL(stU, BindString(1, "name"));
            bound += EXPECT_CALL(stU, BindString(2, "icon"));
            bound += EXPECT_CALL(stU, BindInt64(3, color));
            bound += EXPECT_CALL(stU, BindInt64(4, conditionId));
            bound += EXPECT_CALL(stU, BindInt64(5, actionId));
            bound += EXPECT_CALL(stU, BindInt(6, false));
            bound += EXPECT_CALL(stU, BindInt64(7, ruleId));
            EXPECT_CALL(stU, Step()).After(bound).WillOnce(Return(MockDatabase::error));
            EXPECT_CALL(stU, GetError()).WillRepeatedly(Return("test"));
        }
        EXPECT_THROW(rs.AddRuleOnly(r), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(DBRuleSerialize, AddRule)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};

    // Fails because no condition
    {
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRule"));
            dbHandler.ExpectSavepointRollback("AddRule");
        }
        Rule r;

        EXPECT_THROW(rs.AddRule(r), std::invalid_argument);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Condition with id 0
    {
        // Other stuff is called in sub functions, does not need to be tested here
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRule"));
            // Do not try to delete if condition did not exist
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;")).Times(0);
            // Action is inserted
            EXPECT_CALL(dbHandler,
                GetStatement("INSERT INTO actions (action_name, action_icon_name, action_color, action_visible) VALUES "
                             "(?1,?2,?3, ?4);"));
            // Rule condition is inserted
            EXPECT_CALL(dbHandler,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
                    "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            EXPECT_CALL(dbHandler,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
                    "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);"));
            // AddRuleOnly
            EXPECT_CALL(dbHandler,
                GetStatement("INSERT INTO rules (rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                             "rule_enabled) VALUES (?1,?2,?3,?4,?5,?6);"));
            dbHandler.ExpectSavepointRelease("AddRule");
        }
        const size_t newRuleId = 3;
        const size_t newConditionId = 2;
        const size_t newActionId = 4;
        // Newly inserted ids
        EXPECT_CALL(dbHandler, GetLastInsertRowid())
            .WillOnce(Return(newActionId))
            .WillOnce(Return(newConditionId))
            .WillOnce(Return(newRuleId));

        auto pC = std::make_unique<RuleConstantCondition>(1, false);
        ::Action a{0, "an", "ai", 0, {}, false};
        Rule r{0, "n", "i", 0, std::move(pC), a, true};

        rs.AddRule(r);
        // r was changed in call
        EXPECT_EQ(newConditionId, r.GetCondition().GetId());
        EXPECT_EQ(newActionId, r.GetEffect().GetId());
        EXPECT_EQ(newRuleId, r.GetId());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Id already set
    {
        // Other stuff is called in sub functions, does not need to be tested here
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRule"));
            // Delete old condition
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"));
            // Action is inserted
            EXPECT_CALL(dbHandler,
                GetStatement("UPDATE actions SET action_name=?1, action_icon_name=?2, action_color=?3, "
                             "action_visible=?4 WHERE action_id=?5;"));
            // Rule condition is inserted
            EXPECT_CALL(dbHandler,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
                    "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            EXPECT_CALL(dbHandler,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
                    "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);"));
            // AddRuleOnly
            EXPECT_CALL(dbHandler,
                GetStatement("UPDATE rules SET rule_name=?1, rule_icon_name=?2, rule_color=?3, condition_id=?4, "
                             "action_id=?5, rule_enabled=?6 WHERE rule_id=?7;"));
            dbHandler.ExpectSavepointRelease("AddRule");
        }
        const size_t ruleId = 3;
        const size_t conditionId = 2;
        const size_t actionId = 4;

        auto pC = std::make_unique<RuleConstantCondition>(1, false);
        pC->SetId(conditionId);
        ::Action a{actionId, "an", "ai", 0, {}, false};
        Rule r{ruleId, "n", "i", 0, std::move(pC), a, true};

        rs.AddRule(r);
        // r should still have same ids
        EXPECT_EQ(conditionId, r.GetCondition().GetId());
        EXPECT_EQ(actionId, r.GetEffect().GetId());
        EXPECT_EQ(ruleId, r.GetId());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Id already set, fails
    {
        // Other stuff is called in sub functions, does not need to be tested here
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddRule"));
            // Delete old condition
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"));
            // Action is inserted
            EXPECT_CALL(dbHandler,
                GetStatement("UPDATE actions SET action_name=?1, action_icon_name=?2, action_color=?3, "
                             "action_visible=?4 WHERE action_id=?5;"));
            // Rule condition is inserted
            EXPECT_CALL(dbHandler,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_id, condition_type, condition_val1, condition_val2, "
                    "condition_operator, condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            EXPECT_CALL(dbHandler,
                GetStatement(
                    "INSERT INTO rule_conditions(condition_type, condition_val1, condition_val2, condition_operator, "
                    "condition_additional1, condition_additional2) VALUES(?1,?2,?3,?4,?5,?6);"));
            // AddRuleOnly
            EXPECT_CALL(dbHandler,
                GetStatement("UPDATE rules SET rule_name=?1, rule_icon_name=?2, rule_color=?3, condition_id=?4, "
                             "action_id=?5, rule_enabled=?6 WHERE rule_id=?7;"))
                .WillOnce(Throw(std::runtime_error("test")));
            dbHandler.ExpectSavepointRollback("AddRule");
        }
        const size_t ruleId = 3;
        const size_t conditionId = 2;
        const size_t actionId = 4;

        auto pC = std::make_unique<RuleConstantCondition>(1, false);
        pC->SetId(conditionId);
        ::Action a{actionId, "an", "ai", 0, {}, false};
        Rule r{ruleId, "n", "i", 0, std::move(pC), a, true};

        EXPECT_THROW(rs.AddRule(r), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(DBRuleSerialize, RemoveRule)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};

    // Success
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());

        const char* deleteStatement = "DELETE FROM rules WHERE rule_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(deletePair.first)));
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"));
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM actions WHERE action_id=?1;"));
            dbHandler.ExpectSavepointRelease("RemoveRule");
        }
        const size_t ruleId = 3;
        {
            InSequence s;
            EXPECT_CALL(stD, BindInt64(1, ruleId)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
        }

        auto c = std::make_unique<RuleConstantCondition>(2, false);
        c->SetId(2);
        Rule r{ruleId, "n", "i", 0, std::move(c), ::Action(1, "a", "b", 0, {}, false), true};

        rs.RemoveRule(r);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());

        const char* deleteStatement = "DELETE FROM rules WHERE rule_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(deletePair.first)));
            dbHandler.ExpectSavepointRollback("RemoveRule");
        }
        const size_t ruleId = 3;
        {
            InSequence s;
            EXPECT_CALL(stD, BindInt64(1, ruleId)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(stD, Step()).WillOnce(Return(MockDatabase::error));
        }
        EXPECT_CALL(stD, GetError()).WillRepeatedly(Return("test"));

        auto c = std::make_unique<RuleConstantCondition>(2, false);
        c->SetId(2);
        Rule r{ruleId, "n", "i", 0, std::move(c), ::Action(1, "a", "b", 0, {}, false), true};

        EXPECT_THROW(rs.RemoveRule(r), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(DBRuleSerialize, GetRulesFromQuery)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    auto st = std::make_shared<MockStatement>("test;");
    DBResult r{st};

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};

    // No results
    {
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::done));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        std::vector<Rule> results = rs.GetRulesFromQuery(r);
        EXPECT_TRUE(results.empty());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // One result
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    return p;
                },
                "c"},
            conditionType);

        const size_t id = 2;
        const char* name = "n";
        const char* icon = "i";
        const unsigned int color = 2355;
        const bool enabled = false;
        const size_t conditionId = 23;
        const size_t actionId = 5;
        const ::Action action{actionId, "an", "ai", 123, {}, false};

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        EXPECT_CALL(*st, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::done));
        ExpectationSet step = EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(*st, GetInt64(0)).After(step).WillOnce(Return(id));
        EXPECT_CALL(*st, GetString(1)).After(step).WillOnce(Return(name));
        EXPECT_CALL(*st, GetString(2)).After(step).WillOnce(Return(icon));
        EXPECT_CALL(*st, GetInt64(3)).After(step).WillOnce(Return(color));
        EXPECT_CALL(*st, GetInt64(4)).After(step).WillOnce(Return(conditionId));
        EXPECT_CALL(*st, GetInt64(5)).After(step).WillOnce(Return(actionId));
        EXPECT_CALL(*st, GetInt(6)).After(step).WillOnce(Return(enabled));

        ExpectGetAction(dbHandler, action);

        const char* conditionSelectStatement
            = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
        auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
        MockStatement& stC = stCPair.second;

        EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement)).WillOnce(Return(ByMove(stCPair.first)));
        {
            ExpectationSet bound = EXPECT_CALL(stC, BindInt64(1, conditionId));
            ExpectationSet step = EXPECT_CALL(stC, Step()).After(bound).WillOnce(Return(MockStatement::row));
            EXPECT_CALL(stC, GetColumnCount()).WillRepeatedly(Return(7));
            EXPECT_CALL(stC, GetInt64(0)).After(bound).WillOnce(Return(conditionId));
            EXPECT_CALL(stC, GetInt64(1)).After(bound).WillRepeatedly(Return(conditionType));
        }

        std::vector<Rule> results = rs.GetRulesFromQuery(r);
        ASSERT_EQ(1, results.size());
        const Rule& r = results.front();
        EXPECT_EQ(id, r.GetId());
        EXPECT_EQ(name, r.GetName());
        EXPECT_EQ(icon, r.GetIcon());
        EXPECT_EQ(color, r.GetColor());
        EXPECT_EQ(enabled, r.IsEnabled());
        EXPECT_EQ(conditionId, r.GetCondition().GetId());

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
        Mock::VerifyAndClearExpectations(&st);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    // Multiple results
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    return p;
                },
                "c"},
            conditionType);

        // condition id is same for all to be simpler
        const int64_t conditionId = 12;
        std::vector<Rule> rules = {
            Rule{2, "n", "i", 2355, nullptr, ::Action{5, "an", "ai", 123, {}, false}},
            Rule{3, "n1", "i1", 2351, nullptr, ::Action{6, "an1", "ai1", 1234, {}, false}},
            Rule{4, "n2", "i2", 2352, nullptr, ::Action{7, "an2", "ai2", 1235, {}, false}},
            Rule{5, "n3", "i3", 2353, nullptr, ::Action{8, "an3", "ai3", 1236, {}, false}},
        };
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::done));
        EXPECT_CALL(*st, GetColumnCount()).WillRepeatedly(Return(7));
        // Rules in reverse order
        for (std::size_t i = rules.size(); i > 0; --i)
        {
            const Rule& r = rules[i - 1];
            // Dirty trick to make gmock expect GetAction for each individual rule
            ExpectationSet step = EXPECT_CALL(*st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(*st, GetInt64(0)).After(step).WillOnce(Return(r.GetId())).RetiresOnSaturation();
            EXPECT_CALL(*st, GetString(1)).After(step).WillOnce(Return(r.GetName())).RetiresOnSaturation();
            EXPECT_CALL(*st, GetString(2)).After(step).WillOnce(Return(r.GetIcon())).RetiresOnSaturation();
            EXPECT_CALL(*st, GetInt64(3)).After(step).WillOnce(Return(r.GetColor())).RetiresOnSaturation();
            EXPECT_CALL(*st, GetInt64(4)).After(step).WillOnce(Return(conditionId)).RetiresOnSaturation();
            EXPECT_CALL(*st, GetInt64(5)).After(step).WillOnce(Return(r.GetEffect().GetId())).RetiresOnSaturation();
            EXPECT_CALL(*st, GetInt(6)).After(step).WillOnce(Return(r.IsEnabled())).RetiresOnSaturation();

            const char* conditionSelectStatement
                = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
                  "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
            auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
            MockStatement& stC = stCPair.second;

            ExpectGetAction(dbHandler, r.GetEffect());

            EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement))
                .After(step)
                .WillOnce(Return(ByMove(stCPair.first)))
                .RetiresOnSaturation();
            {
                ExpectationSet bound = EXPECT_CALL(stC, BindInt64(1, conditionId)).After(step).RetiresOnSaturation();
                ExpectationSet stepC = EXPECT_CALL(stC, Step())
                                           .After(bound)
                                           .After(step)
                                           .WillOnce(Return(MockStatement::row))
                                           .RetiresOnSaturation();
                EXPECT_CALL(stC, GetColumnCount()).After(stepC).WillRepeatedly(Return(7)).RetiresOnSaturation();
                EXPECT_CALL(stC, GetInt64(0)).After(stepC).WillOnce(Return(conditionId)).RetiresOnSaturation();
                EXPECT_CALL(stC, GetInt64(1)).After(stepC).WillRepeatedly(Return(conditionType)).RetiresOnSaturation();
            }
        }

        std::vector<Rule> results = rs.GetRulesFromQuery(r);
        ASSERT_EQ(rules.size(), results.size());
        for (std::size_t i = 0; i < rules.size(); ++i)
        {
            const Rule& r = results[i];
            const Rule& expected = rules[i];
            EXPECT_EQ(expected.GetId(), r.GetId());
            EXPECT_EQ(expected.GetName(), r.GetName());
            EXPECT_EQ(expected.GetIcon(), r.GetIcon());
            EXPECT_EQ(expected.GetColor(), r.GetColor());
            EXPECT_EQ(expected.IsEnabled(), r.IsEnabled());
            EXPECT_EQ(conditionId, r.GetCondition().GetId());
        }
        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
        Mock::VerifyAndClearExpectations(&st);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}

TEST(DBRuleSerialize, GetRule)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};

    // No result
    {
        const int64_t id = 2;

        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules WHERE rule_id=?1;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(st, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        EXPECT_THROW(rs.GetRule(id), std::out_of_range);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // One result
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .Times(AnyNumber())
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    EXPECT_CALL(*p, Clone(Ref(Res::ConditionRegistry())))
                        .Times(AnyNumber())
                        .WillRepeatedly(
                            Invoke([ptr = p.get()](const auto& reg) { return ptr->RuleCondition::Clone(reg); }));
                    return p;
                },
                "c"},
            conditionType);

        const size_t id = 2;
        const char* name = "n";
        const char* icon = "i";
        const int64_t color = 2355;
        const bool enabled = false;
        const size_t conditionId = 23;
        const size_t actionId = 5;
        const ::Action action{actionId, "an", "ai", 123, {}, false};

        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules WHERE rule_id=?1;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        ExpectationSet bound = EXPECT_CALL(st, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        ExpectationSet step
            = EXPECT_CALL(st, Step()).After(bound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt64(0)).After(step).WillOnce(Return(id));
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(name));
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(icon));
        EXPECT_CALL(st, GetInt64(3)).After(step).WillOnce(Return(color));
        EXPECT_CALL(st, GetInt64(4)).After(step).WillOnce(Return(conditionId));
        EXPECT_CALL(st, GetInt64(5)).After(step).WillOnce(Return(actionId));
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(enabled));

        ExpectGetAction(dbHandler, action);

        const char* conditionSelectStatement
            = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
        auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
        MockStatement& stC = stCPair.second;

        EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement)).WillOnce(Return(ByMove(stCPair.first)));
        {
            ExpectationSet boundC = EXPECT_CALL(stC, BindInt64(1, conditionId));
            ExpectationSet step = EXPECT_CALL(stC, Step()).After(boundC).WillOnce(Return(MockStatement::row));
            EXPECT_CALL(stC, GetColumnCount()).WillRepeatedly(Return(7));
            EXPECT_CALL(stC, GetInt64(0)).After(boundC).WillOnce(Return(conditionId));
            EXPECT_CALL(stC, GetInt64(1)).After(boundC).WillRepeatedly(Return(conditionType));
        }

        Rule r = rs.GetRule(id);
        EXPECT_EQ(id, r.GetId());
        EXPECT_EQ(name, r.GetName());
        EXPECT_EQ(icon, r.GetIcon());
        EXPECT_EQ(color, r.GetColor());
        EXPECT_EQ(enabled, r.IsEnabled());
        EXPECT_EQ(conditionId, r.GetCondition().GetId());

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}

TEST(DBRuleSerialize, GetAllRules)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};

    // No result
    {
        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        std::vector<Rule> results = rs.GetAllRules();
        EXPECT_TRUE(results.empty());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // One result
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .Times(AnyNumber())
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    EXPECT_CALL(*p, Clone(Ref(Res::ConditionRegistry())))
                        .Times(AnyNumber())
                        .WillRepeatedly(
                            Invoke([ptr = p.get()](const auto& reg) { return ptr->RuleCondition::Clone(reg); }));
                    return p;
                },
                "c"},
            conditionType);

        const size_t id = 2;
        const char* name = "n";
        const char* icon = "i";
        const int64_t color = 2355;
        const bool enabled = false;
        const size_t conditionId = 23;
        const size_t actionId = 5;
        const ::Action action{actionId, "an", "ai", 123, {}, false};

        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        ExpectationSet step = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt64(0)).After(step).WillOnce(Return(id));
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(name));
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(icon));
        EXPECT_CALL(st, GetInt64(3)).After(step).WillOnce(Return(color));
        EXPECT_CALL(st, GetInt64(4)).After(step).WillOnce(Return(conditionId));
        EXPECT_CALL(st, GetInt64(5)).After(step).WillOnce(Return(actionId));
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(enabled));

        ExpectGetAction(dbHandler, action);

        const char* conditionSelectStatement
            = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
        auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
        MockStatement& stC = stCPair.second;

        EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement)).WillOnce(Return(ByMove(stCPair.first)));
        {
            ExpectationSet boundC = EXPECT_CALL(stC, BindInt64(1, conditionId));
            ExpectationSet step = EXPECT_CALL(stC, Step()).After(boundC).WillOnce(Return(MockStatement::row));
            EXPECT_CALL(stC, GetColumnCount()).WillRepeatedly(Return(7));
            EXPECT_CALL(stC, GetInt64(0)).After(boundC).WillOnce(Return(conditionId));
            EXPECT_CALL(stC, GetInt64(1)).After(boundC).WillRepeatedly(Return(conditionType));
        }

        std::vector<Rule> results = rs.GetAllRules();
        ASSERT_EQ(1, results.size());
        const Rule& r = results.front();
        EXPECT_EQ(id, r.GetId());
        EXPECT_EQ(name, r.GetName());
        EXPECT_EQ(icon, r.GetIcon());
        EXPECT_EQ(color, r.GetColor());
        EXPECT_EQ(enabled, r.IsEnabled());
        EXPECT_EQ(conditionId, r.GetCondition().GetId());

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    // Multiple results
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    return p;
                },
                "c"},
            conditionType);

        // condition id is same for all to be simpler
        const int64_t conditionId = 12;
        std::vector<Rule> rules = {
            Rule{2, "n", "i", 2355, nullptr, ::Action{5, "an", "ai", 123, {}, false}},
            Rule{3, "n1", "i1", 2351, nullptr, ::Action{6, "an1", "ai1", 1234, {}, false}},
            Rule{4, "n2", "i2", 2352, nullptr, ::Action{7, "an2", "ai2", 1235, {}, false}},
            Rule{5, "n3", "i3", 2353, nullptr, ::Action{8, "an3", "ai3", 1236, {}, false}},
        };
        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
        }
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
        // Rules in reverse order
        for (std::size_t i = rules.size(); i > 0; --i)
        {
            const Rule& r = rules[i - 1];
            // Dirty trick to make gmock expect GetAction for each individual rule
            ExpectationSet step = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt64(0)).After(step).WillOnce(Return(r.GetId())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(r.GetName())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(r.GetIcon())).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt64(3)).After(step).WillOnce(Return(r.GetColor())).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt64(4)).After(step).WillOnce(Return(conditionId)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt64(5)).After(step).WillOnce(Return(r.GetEffect().GetId())).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(r.IsEnabled())).RetiresOnSaturation();

            const char* conditionSelectStatement
                = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
                  "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
            auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
            MockStatement& stC = stCPair.second;

            ExpectGetAction(dbHandler, r.GetEffect());

            EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement))
                .After(step)
                .WillOnce(Return(ByMove(stCPair.first)))
                .RetiresOnSaturation();
            {
                ExpectationSet bound = EXPECT_CALL(stC, BindInt64(1, conditionId)).After(step).RetiresOnSaturation();
                ExpectationSet stepC = EXPECT_CALL(stC, Step())
                                           .After(bound)
                                           .After(step)
                                           .WillOnce(Return(MockStatement::row))
                                           .RetiresOnSaturation();
                EXPECT_CALL(stC, GetColumnCount()).After(stepC).WillRepeatedly(Return(7)).RetiresOnSaturation();
                EXPECT_CALL(stC, GetInt64(0)).After(stepC).WillOnce(Return(conditionId)).RetiresOnSaturation();
                EXPECT_CALL(stC, GetInt64(1)).After(stepC).WillRepeatedly(Return(conditionType)).RetiresOnSaturation();
            }
        }

        std::vector<Rule> results = rs.GetAllRules();
        ASSERT_EQ(rules.size(), results.size());
        for (std::size_t i = 0; i < rules.size(); ++i)
        {
            const Rule& r = results[i];
            const Rule& expected = rules[i];
            EXPECT_EQ(expected.GetId(), r.GetId());
            EXPECT_EQ(expected.GetName(), r.GetName());
            EXPECT_EQ(expected.GetIcon(), r.GetIcon());
            EXPECT_EQ(expected.GetColor(), r.GetColor());
            EXPECT_EQ(expected.IsEnabled(), r.IsEnabled());
            EXPECT_EQ(conditionId, r.GetCondition().GetId());
        }
        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}

TEST(DBRuleSerialize, RemoveRuleId)
{
    using namespace ::testing;
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    // TODO: Mock DBActionSerialize and simplify the tests
    DBActionSerialize as{dbHandler};
    DBRuleSerialize rs{dbHandler, as};

    // Rule not found
    {
        const int64_t id = 2;

        // Get rule fails
        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules WHERE rule_id=?1;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule_id"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(st, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
            dbHandler.ExpectSavepointRollback("RemoveRule_id");
        }
        EXPECT_THROW(rs.RemoveRule(id), std::out_of_range);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Success
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .Times(AnyNumber())
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    EXPECT_CALL(*p, Clone(Ref(Res::ConditionRegistry())))
                        .Times(AnyNumber())
                        .WillRepeatedly(
                            Invoke([ptr = p.get()](const auto& reg) { return ptr->RuleCondition::Clone(reg); }));
                    EXPECT_CALL(*p, HasChilds()).Times(AnyNumber()).WillRepeatedly(Return(false));
                    return p;
                },
                "c"},
            conditionType);

        // Get rule
        const size_t id = 2;
        const char* name = "n";
        const char* icon = "i";
        const unsigned int color = 2355;
        const bool enabled = false;
        const size_t conditionId = 23;
        const size_t actionId = 5;
        const ::Action action{actionId, "an", "ai", 123, {}, false};

        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules WHERE rule_id=?1;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule_id"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
            dbHandler.ExpectSavepointRelease("RemoveRule_id");
        }
        ExpectationSet bound = EXPECT_CALL(st, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        ExpectationSet step
            = EXPECT_CALL(st, Step()).After(bound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt64(0)).After(step).WillOnce(Return(id));
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(name));
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(icon));
        EXPECT_CALL(st, GetInt64(3)).After(step).WillOnce(Return(color));
        EXPECT_CALL(st, GetInt64(4)).After(step).WillOnce(Return(conditionId));
        EXPECT_CALL(st, GetInt64(5)).After(step).WillOnce(Return(actionId));
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(enabled));

        ExpectGetAction(dbHandler, action);

        const char* conditionSelectStatement
            = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
        auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
        MockStatement& stC = stCPair.second;

        EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement)).WillOnce(Return(ByMove(stCPair.first)));
        {
            ExpectationSet boundC = EXPECT_CALL(stC, BindInt64(1, conditionId));
            ExpectationSet step = EXPECT_CALL(stC, Step()).After(boundC).WillOnce(Return(MockStatement::row));
            EXPECT_CALL(stC, GetColumnCount()).WillRepeatedly(Return(7));
            EXPECT_CALL(stC, GetInt64(0)).After(boundC).WillOnce(Return(conditionId));
            EXPECT_CALL(stC, GetInt64(1)).After(boundC).WillRepeatedly(Return(conditionType));
        }

        // Delete rule
        const char* deleteStatement = "DELETE FROM rules WHERE rule_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(deletePair.first)));
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM rule_conditions WHERE condition_id=?1;"));
            EXPECT_CALL(dbHandler, GetStatement("DELETE FROM actions WHERE action_id=?1;"));
            dbHandler.ExpectSavepointRelease("RemoveRule");
        }
        {
            InSequence s;
            EXPECT_CALL(stD, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done));
        }

        rs.RemoveRule(id);

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
    // Delete fails
    try
    {
        // Other savepoints
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

        // Register condition type
        const int64_t conditionType = 1;
        Res::ConditionRegistry().Register(
            RuleConditionInfo{
                [&](std::size_t i) {
                    auto p = std::make_unique<MockRuleCondition>(i);
                    EXPECT_CALL(
                        *p, Parse(Ref(rs.GetConditionSerialize()), Ref(Res::ConditionRegistry()), Matcher<DBResult>(_)))
                        .Times(AnyNumber())
                        .WillOnce(Invoke([ptr = p.get()](const auto&, const auto&, DBResult r) {
                            ptr->SetId(r.GetColumnInt64(0));
                        }));
                    EXPECT_CALL(*p, Clone(Ref(Res::ConditionRegistry())))
                        .Times(AnyNumber())
                        .WillRepeatedly(
                            Invoke([ptr = p.get()](const auto& reg) { return ptr->RuleCondition::Clone(reg); }));
                    return p;
                },
                "c"},
            conditionType);

        // Get rule
        const size_t id = 2;
        const char* name = "n";
        const char* icon = "i";
        const unsigned int color = 2355;
        const bool enabled = false;
        const size_t conditionId = 23;
        const size_t actionId = 5;
        const ::Action action{actionId, "an", "ai", 123, {}, false};

        const char* selectStatement = "SELECT rule_id, rule_name, rule_icon_name, rule_color, condition_id, action_id, "
                                      "rule_enabled FROM rules WHERE rule_id=?1;";
        auto stPair = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = stPair.second;

        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule_id"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(stPair.first)));
            EXPECT_CALL(dbHandler, GetROSavepoint("GetRulesFromQuery"));
            dbHandler.ExpectROSavepointRelease("GetRulesFromQuery");
            dbHandler.ExpectSavepointRollback("RemoveRule_id");
        }
        ExpectationSet bound = EXPECT_CALL(st, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        ExpectationSet step
            = EXPECT_CALL(st, Step()).After(bound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt64(0)).After(step).WillOnce(Return(id));
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(name));
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(icon));
        EXPECT_CALL(st, GetInt64(3)).After(step).WillOnce(Return(color));
        EXPECT_CALL(st, GetInt64(4)).After(step).WillOnce(Return(conditionId));
        EXPECT_CALL(st, GetInt64(5)).After(step).WillOnce(Return(actionId));
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(enabled));

        ExpectGetAction(dbHandler, action);

        const char* conditionSelectStatement
            = "SELECT condition_id, condition_type, condition_val1, condition_val2, condition_operator, "
              "condition_additional1, condition_additional2 FROM rule_conditions WHERE condition_id=?1;";
        auto stCPair = dbHandler.GetMockedStatement(conditionSelectStatement);
        MockStatement& stC = stCPair.second;

        EXPECT_CALL(dbHandler, GetROStatement(conditionSelectStatement)).WillOnce(Return(ByMove(stCPair.first)));
        {
            ExpectationSet boundC = EXPECT_CALL(stC, BindInt64(1, conditionId));
            ExpectationSet step = EXPECT_CALL(stC, Step()).After(boundC).WillOnce(Return(MockStatement::row));
            EXPECT_CALL(stC, GetColumnCount()).WillRepeatedly(Return(7));
            EXPECT_CALL(stC, GetInt64(0)).After(boundC).WillOnce(Return(conditionId));
            EXPECT_CALL(stC, GetInt64(1)).After(boundC).WillRepeatedly(Return(conditionType));
        }

        // Delete rule
        const char* deleteStatement = "DELETE FROM rules WHERE rule_id=?1;";
        auto deletePair = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = deletePair.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("RemoveRule"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(deletePair.first)));
            dbHandler.ExpectSavepointRollback("RemoveRule");
        }
        {
            InSequence s;
            EXPECT_CALL(stD, BindInt64(1, id)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(stD, Step()).WillOnce(Return(MockDatabase::error));
        }
        EXPECT_CALL(stD, GetError()).WillRepeatedly(Return("test"));

        EXPECT_THROW(rs.RemoveRule(id), std::runtime_error);

        Res::ConditionRegistry().RemoveAll();
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    catch (...)
    {
        Res::ConditionRegistry().RemoveAll();
        throw;
    }
}
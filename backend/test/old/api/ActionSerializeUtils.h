#ifndef ACTION_SERIALIZE_UTILS_H
#define ACTION_SERIALIZE_UTILS_H

#include "../mocks/MockDBHandler.h"
#include "api/Action.h"

inline void ExpectSubActionInsert(MockDBHandler& db, int64_t actionId, const std::vector<SubAction>& subActions)
{
    using namespace ::testing;

    const char* insertStatement = "INSERT INTO sub_actions (action_id, action_type, actor_node, actor_id, val, "
                                  "timeout, transition) VALUES (?1,?2,?3,?4,?5,?6,?7);";
    auto pStI = db.GetMockedStatement(insertStatement);
    MockStatement& stI = pStI.second;
    EXPECT_CALL(db, GetStatement(insertStatement)).WillOnce(Return(ByMove(pStI.first)));

    for (std::size_t i = subActions.size(); i > 0; --i)
    {
        std::array<DBValue, 6> values = subActions[i - 1].ToDBValues();
        ExpectationSet insertBound;
        insertBound
            += EXPECT_CALL(stI, BindInt64(1, actionId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
        for (std::size_t j = 0; j < 6; ++j)
        {
            if (values[j].IsInt64())
            {
                insertBound += EXPECT_CALL(stI, BindInt64(j + 2, values[j].GetInt64()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
            }
            else if (values[j].IsInt())
            {
                insertBound += EXPECT_CALL(stI, BindInt(j + 2, values[j].GetInt()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
            }
            else if (values[j].IsString())
            {
                insertBound += EXPECT_CALL(stI, BindString(j + 2, values[j].GetStr()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
            }
        }
        {
            InSequence s;
            EXPECT_CALL(stI, Step()).After(insertBound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
            EXPECT_CALL(stI, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
        }
    }
}

inline void ExpectGetActions(MockDBHandler& db, MockStatement& st, const std::vector<Action>& actions)
{
    using namespace ::testing;
    using ::Action;

    EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();

    // Actions in reverse order
    for (std::size_t i = actions.size(); i > 0; --i)
    {
        const Action& a = actions[i - 1];
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(5));
        ExpectationSet step = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt64(0)).After(step).WillOnce(Return(a.GetId())).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(a.GetName())).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(a.GetIcon())).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(3)).After(step).WillOnce(Return(a.GetColor())).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(4)).After(step).WillOnce(Return(a.GetVisibility())).RetiresOnSaturation();
    }

    const char* subActionSelectStatement = "SELECT action_type, actor_node, actor_id, val, timeout, transition FROM "
                                           "sub_actions WHERE action_id = ?1 ORDER BY sub_action_id;";
    auto pStA = db.GetMockedStatement(subActionSelectStatement);
    MockStatement& stA = pStA.second;

    EXPECT_CALL(db, GetROStatement(subActionSelectStatement))
        .WillOnce(Return(ByMove(pStA.first)))
        .RetiresOnSaturation();

    // Sub action expectations
    for (const Action& action : actions)
    {
        ExpectationSet bound = EXPECT_CALL(stA, BindInt64(1, action.GetId())).RetiresOnSaturation();
        EXPECT_CALL(stA, Step()).After(bound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
        EXPECT_CALL(stA, GetColumnCount()).After(bound).WillRepeatedly(Return(6)).RetiresOnSaturation();
        for (const SubAction& a : action.GetSubActions())
        {
            ExpectationSet stepExp
                = EXPECT_CALL(stA, Step()).After(bound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            std::array<DBValue, 6> values = a.ToDBValues();
            for (std::size_t j = 0; j < 6; ++j)
            {
                if (values[j].IsInt64())
                {
                    EXPECT_CALL(stA, GetInt64(j))
                        .After(bound)
                        .WillRepeatedly(Return(values[j].GetInt64()))
                        .RetiresOnSaturation();
                }
                else if (values[j].IsInt())
                {
                    EXPECT_CALL(stA, GetInt(j))
                        .After(bound)
                        .WillRepeatedly(Return(values[j].GetInt()))
                        .RetiresOnSaturation();
                }
                else if (values[j].IsString())
                {
                    EXPECT_CALL(stA, GetString(j))
                        .After(bound)
                        .WillRepeatedly(Return(values[j].GetStr()))
                        .RetiresOnSaturation();
                }
            }
        }
        EXPECT_CALL(stA, Reset()).After(bound).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
    }
}

inline void ExpectGetAction(MockDBHandler& db, const Action& a)
{
    using namespace ::testing;
    const char* selectStatement = "SELECT action_id, action_name, action_icon_name, action_color, action_visible FROM "
                                  "actions WHERE action_id = ?1;";
    const std::size_t actionId = a.GetId();
    auto pSt = db.GetMockedStatement(selectStatement);
    MockStatement& st = pSt.second;

    EXPECT_CALL(db, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first))).RetiresOnSaturation();
    {
        InSequence s;
        EXPECT_CALL(db, GetROSavepoint("GetActionsFromQuery")).RetiresOnSaturation();
        db.ExpectROSavepointRelease("GetActionsFromQuery");
    }

    EXPECT_CALL(st, BindInt64(1, actionId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
    ExpectGetActions(db, st, {a});
}

#endif
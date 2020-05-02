#ifndef NODE_SERIALIZE_UTILS_H
#define NODE_SERIALIZE_UTILS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../mocks/MockDBHandler.h"
#include "standard-api/NodeData.h"

inline void ExpectGetActors(MockDBHandler& db, uint16_t nodeId, const std::vector<Actor>& actors)
{
    using namespace ::testing;

    const char* selectStatement = "SELECT actor_id, actor_name, actor_location, actor_state, actor_type, actor_pin "
                                  "FROM actors WHERE node_id=?1 ORDER BY actor_id;";
    ExpectationSet e;
    auto pSt = db.GetMockedStatement(selectStatement);
    MockStatement& st = pSt.second;
    {
        InSequence s;
        EXPECT_CALL(db, GetROSavepoint("GetActors")).RetiresOnSaturation();
        EXPECT_CALL(db, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first))).RetiresOnSaturation();
        e += EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
        db.ExpectROSavepointRelease("GetActors");
    }

    // Last step
    EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
    EXPECT_CALL(st, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(6));
    // Register in reverse order
    for (std::size_t i = actors.size(); i > 0; --i)
    {
        const Actor& a = actors[i - 1];
        if (!a.IsDeleted())
        {
            ExpectationSet stepExp
                = EXPECT_CALL(st, Step()).After(e).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(0)).After(stepExp).WillOnce(Return(i - 1)).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(1)).After(stepExp).WillOnce(Return(a.GetName())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(2)).After(stepExp).WillOnce(Return(a.GetLocation())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(3)).After(stepExp).WillOnce(Return(a.m_state)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(4)).After(stepExp).WillOnce(Return(a.m_type)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(5)).After(stepExp).WillOnce(Return(a.m_pin)).RetiresOnSaturation();
        }
    }
}

inline void ExpectGetSensors(MockDBHandler& db, uint16_t nodeId, const std::vector<Sensor>& sensors)
{
    using namespace ::testing;

    const char* selectStatement = "SELECT sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, "
                                  "sensor_pin, sensor_listener FROM sensors WHERE node_id=?1 ORDER BY sensor_id;";
    ExpectationSet e;
    auto pSt = db.GetMockedStatement(selectStatement);
    MockStatement& st = pSt.second;
    {
        InSequence s;
        EXPECT_CALL(db, GetROSavepoint("GetSensors")).RetiresOnSaturation();
        EXPECT_CALL(db, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first))).RetiresOnSaturation();
        e += EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
        db.ExpectROSavepointRelease("GetSensors");
    }

    // Last step
    EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
    EXPECT_CALL(st, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(7));
    // Register in reverse order
    for (std::size_t i = sensors.size(); i > 0; --i)
    {
        const Sensor& s = sensors[i - 1];
        if (!s.IsDeleted())
        {
            ExpectationSet stepExp
                = EXPECT_CALL(st, Step()).After(e).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(0)).After(stepExp).WillOnce(Return(i - 1)).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(1)).After(stepExp).WillOnce(Return(s.GetName())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(2)).After(stepExp).WillOnce(Return(s.GetLocation())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(3)).After(stepExp).WillOnce(Return(s.m_state)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(4)).After(stepExp).WillOnce(Return(s.m_type)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(5)).After(stepExp).WillOnce(Return(s.m_pin)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(6)).After(stepExp).WillOnce(Return(s.m_interval)).RetiresOnSaturation();
        }
    }
}

inline void ExpectGetNode(MockDBHandler& db, const NodeData& n)
{
    using namespace ::testing;
    {
        const char* selectStatement = "SELECT node_id, node_name, node_location, node_state, node_path, "
                                      "node_path_distance, node_type FROM nodes WHERE node_id=?1;";
        auto pSt = db.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;

        EXPECT_CALL(db, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first))).RetiresOnSaturation();

        ExpectationSet selectBound
            = EXPECT_CALL(st, BindInt(1, n.m_id)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
        // No node found
        EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
        ExpectationSet step
            = EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetColumnCount()).After(step).WillRepeatedly(Return(7)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(0)).After(step).WillOnce(Return(n.m_id)).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(n.m_name)).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(n.m_location)).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(3)).After(step).WillOnce(Return(n.m_state)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(4)).After(step).WillOnce(Return(n.m_path.GetPath())).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(5)).After(step).WillOnce(Return(n.m_path.GetDistance())).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(n.m_type)).RetiresOnSaturation();
    }
    ExpectGetActors(db, n.m_id, n.m_actors);
    ExpectGetSensors(db, n.m_id, n.m_sensors);
}

inline void ExpectGetNodePath(MockDBHandler& db, const NodeData& n)
{
    using namespace ::testing;

    {
        const char* selectStatement
            = "SELECT node_id, node_name, node_location, node_state, node_path, node_path_distance, node_type FROM "
              "nodes WHERE node_path=?1 AND node_path_distance=?2;";
        auto pSt = db.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;

        EXPECT_CALL(db, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first))).RetiresOnSaturation();

        ExpectationSet selectBound
            = EXPECT_CALL(st, BindInt(1, n.m_path.GetPath())).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
        selectBound += EXPECT_CALL(st, BindInt(2, n.m_path.GetDistance()))
                           .WillOnce(Return(MockDatabase::ok))
                           .RetiresOnSaturation();
        // No node found
        EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
        ExpectationSet step
            = EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
        EXPECT_CALL(st, GetColumnCount()).After(step).WillRepeatedly(Return(7)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(0)).After(step).WillOnce(Return(n.m_id)).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(n.m_name)).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(n.m_location)).RetiresOnSaturation();
        EXPECT_CALL(st, GetString(3)).After(step).WillOnce(Return(n.m_state)).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(4)).After(step).WillOnce(Return(n.m_path.GetPath())).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(5)).After(step).WillOnce(Return(n.m_path.GetDistance())).RetiresOnSaturation();
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(n.m_type)).RetiresOnSaturation();
    }
    ExpectGetActors(db, n.m_id, n.m_actors);
    ExpectGetSensors(db, n.m_id, n.m_sensors);
}

#endif
#include "database/DBNodeSerialize.h"

#include <gtest/gtest.h>

#include "../mocks/MockDBHandler.h"
#include "../standard-api/NodeSerializeUtils.h"
#include "standard-api/NodeManager.h"
#include "standard-api/communication/NodeCommands.h"

TEST(ActorSerialize, GetActors)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBActorSerialize actorSerialize{dbHandler};

    // success
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);
        std::vector<Actor> actors{Actor(3, 0, "n0", "l0", ::Types::LED, Pins::D2),
            Actor(3, 1, "n1", "l1", ::Types::LAMP, Pins::D11), Actor(3, 2, "n2", "l2", ::Types::SOCKET, Pins::A5)};
        actors[0].m_state = "a";
        actors[1].m_state = "b";
        actors[2].m_state = "c";
        const uint16_t nodeId = 3;

        ExpectGetActors(dbHandler, nodeId, actors);

        std::vector<Actor> result = actorSerialize.GetActors(nodeId);
        EXPECT_THAT(result, ContainerEq(actors));
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail savepoint
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        EXPECT_CALL(dbHandler, GetROSavepoint("GetActors")).WillOnce(Throw(std::runtime_error("testing")));
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);

        EXPECT_THROW(actorSerialize.GetActors(3), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // fail statement
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_id, actor_name, actor_location, actor_state, actor_type, actor_pin "
                                      "FROM actors WHERE node_id=?1 ORDER BY actor_id;";
        const uint16_t nodeId = 3;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetActors"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Throw(std::runtime_error("test")));
            dbHandler.ExpectROSavepointRollback("GetActors");
        }

        EXPECT_THROW(actorSerialize.GetActors(nodeId), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // fail step
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_id, actor_name, actor_location, actor_state, actor_type, actor_pin "
                                      "FROM actors WHERE node_id=?1 ORDER BY actor_id;";
        const uint16_t nodeId = 3;
        ExpectationSet e;
        auto pSt = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetActors"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));
            e += EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
            dbHandler.ExpectROSavepointRollback("GetActors");
        }

        // First step fails
        EXPECT_CALL(st, Step()).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(st, GetError()).WillRepeatedly(Return("testing"));

        EXPECT_THROW(actorSerialize.GetActors(nodeId), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(ActorSerialize, UpdateActor)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBActorSerialize actorSerialize{dbHandler};

    // Delete existing actor
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* deleteStatement = "DELETE FROM actors WHERE node_id=?1 AND actor_id=?2;";
        const uint16_t nodeId = 3;
        const uint8_t actorId = 1;
        auto pSt = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateActor"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(pSt.first)));
            dbHandler.ExpectSavepointRelease("UpdateActor");
        }
        ExpectationSet e;
        e += EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        e += EXPECT_CALL(st, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(st, Step()).After(e).WillOnce(Return(MockStatement::done));

        actorSerialize.UpdateActor(nodeId, actorId, Actor::Deleted());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Insert new actor
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_uid FROM actors WHERE node_id=?1 AND actor_id=?2;";
        const char* insertStatement = "INSERT INTO actors (node_id, actor_id, actor_name, actor_location, actor_state, "
                                      "actor_type, actor_pin) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const uint16_t nodeId = 3;
        const uint8_t actorId = 1;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateActor"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(pStI.first)));
            dbHandler.ExpectSavepointRelease("UpdateActor");
        }
        Actor a{nodeId, actorId, "abc", "def", 2, 3};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        selectBound += EXPECT_CALL(stS, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

        ExpectationSet insertBound;
        insertBound += EXPECT_CALL(stI, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(3, a.GetName())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(4, a.GetLocation())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(5, a.GetState())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(6, a.m_type)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(7, a.m_pin)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(stI, Step()).After(insertBound).WillOnce(Return(MockStatement::done));

        actorSerialize.UpdateActor(nodeId, actorId, a);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Update existing actor
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_uid FROM actors WHERE node_id=?1 AND actor_id=?2;";
        const char* updateStatement = "UPDATE actors SET actor_name=?3, actor_location=?4, actor_state=?5, "
                                      "actor_type=?6, actor_pin=?7 WHERE node_id=?1 AND actor_id=?2;";
        const uint16_t nodeId = 3;
        const uint8_t actorId = 1;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateActor"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRelease("UpdateActor");
        }
        Actor a{nodeId, actorId, "abc", "def", 2, 3};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        selectBound += EXPECT_CALL(stS, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        // Select returns row to indicate that an actor was found
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::row));

        ExpectationSet updateBound;
        updateBound += EXPECT_CALL(stU, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(3, a.GetName())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(4, a.GetLocation())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(5, a.GetState())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(6, a.m_type)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(7, a.m_pin)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockStatement::done));

        actorSerialize.UpdateActor(nodeId, actorId, a);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail update existing actor
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_uid FROM actors WHERE node_id=?1 AND actor_id=?2;";
        const char* updateStatement = "UPDATE actors SET actor_name=?3, actor_location=?4, actor_state=?5, "
                                      "actor_type=?6, actor_pin=?7 WHERE node_id=?1 AND actor_id=?2;";
        const uint16_t nodeId = 3;
        const uint8_t actorId = 1;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateActor"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRollback("UpdateActor");
        }
        Actor a{nodeId, actorId, "abc", "def", 2, 3};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        selectBound += EXPECT_CALL(stS, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        // Select returns row to indicate that an actor was found
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::row));

        ExpectationSet updateBound;
        updateBound += EXPECT_CALL(stU, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(2, actorId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(3, a.GetName())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(4, a.GetLocation())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(5, a.GetState())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(6, a.m_type)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(7, a.m_pin)).WillOnce(Return(MockDatabase::ok));
        // Update fails
        EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(stU, GetError()).WillRepeatedly(Return("testing"));

        EXPECT_THROW(actorSerialize.UpdateActor(nodeId, actorId, a), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(ActorSerialize, InsertActors)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBActorSerialize actorSerialize{dbHandler};

    // Insert into empty table
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_id,actor_uid FROM actors WHERE node_id=?1;";
        const char* updateStatement
            = "UPDATE actors SET actor_name=?1,actor_location=?2,actor_state=?3,actor_type=?4,actor_pin=?5 WHERE "
              "actor_uid=?6;";
        const char* insertStatement = "INSERT INTO actors (node_id, actor_id, actor_name, actor_location, actor_state, "
                                      "actor_type, actor_pin) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const uint16_t nodeId = 3;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertActors"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(pStI.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRelease("InsertActors");
        }
        std::vector<Actor> actors{Actor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2), Actor::Deleted(),
            Actor(nodeId, 1, "n1", "l1", ::Types::LAMP, Pins::D11),
            Actor(nodeId, 2, "n2", "l2", ::Types::SOCKET, Pins::A5)};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        // No actors from select
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

        // Nothing updated
        EXPECT_CALL(stU, Step()).Times(0);

        for (std::size_t i = actors.size(); i > 0; --i)
        {
            const Actor& a = actors[i - 1];
            if (a.IsDeleted())
            {
                EXPECT_CALL(stI, BindInt(2, i - 1)).Times(0);
            }
            else
            {
                ExpectationSet insertBound;
                insertBound
                    += EXPECT_CALL(stI, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(2, i - 1)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(3, a.GetName()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(4, a.GetLocation()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(5, a.GetState()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(6, a.m_type)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(7, a.m_pin)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step())
                        .After(insertBound)
                        .WillOnce(Return(MockStatement::done))
                        .RetiresOnSaturation();
                    EXPECT_CALL(stI, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                }
            }
        }

        actorSerialize.InsertActors(nodeId, actors);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Insert, update and delete in table with some existing
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_id,actor_uid FROM actors WHERE node_id=?1;";
        const char* updateStatement
            = "UPDATE actors SET actor_name=?1,actor_location=?2,actor_state=?3,actor_type=?4,actor_pin=?5 WHERE "
              "actor_uid=?6;";
        const char* insertStatement = "INSERT INTO actors (node_id, actor_id, actor_name, actor_location, actor_state, "
                                      "actor_type, actor_pin) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const char* deleteStatement = "DELETE FROM actors WHERE actor_uid=?1;";
        const uint16_t nodeId = 3;
        // Actors passed to the update statement
        std::vector<Actor> actorsUpdated{Actor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2), Actor::Deleted(),
            Actor(nodeId, 2, "n2", "l2", ::Types::LAMP, Pins::D11),
            Actor(nodeId, 3, "n3", "l3", ::Types::SOCKET, Pins::A5)};
        // Actors ids + uids which will be returned by the select statement
        std::vector<std::pair<uint8_t, int64_t>> actorsInDatabase{{0, 1},
            // Actor 1 should be deleted
            {1, 2}, {2, 3},
            // Actor 3 to be inserted
            // Actor 4 to be deleted
            {4, 4}};
        // Actor ids and Actors which should be newly inserted (in actorsUpdated, but not in database)
        std::vector<std::pair<uint8_t, Actor>> actorsToInsert{
            {3, Actor(nodeId, 3, "n3", "l3", ::Types::SOCKET, Pins::A5)}};
        // Actor uids and Actors which should be updated (in both actorsUpdated and database)
        std::vector<std::pair<int64_t, Actor>> actorsToUpdate{
            {1, Actor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2)},
            {3, Actor(nodeId, 2, "n2", "l2", ::Types::LAMP, Pins::D11)},
        };
        // Actor uids which should be deleted (not in actorsUpdated but in database)
        std::vector<int64_t> actorsToDelete{2, 4};

        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        auto pStD = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stD = pStD.second;
        {
            ExpectationSet savepoint = EXPECT_CALL(dbHandler, GetSavepoint("InsertActors"));
            ExpectationSet statements;
            statements += EXPECT_CALL(dbHandler, GetStatement(selectStatement))
                              .After(savepoint)
                              .WillOnce(Return(ByMove(pStS.first)));
            statements += EXPECT_CALL(dbHandler, GetStatement(insertStatement))
                              .After(savepoint)
                              .WillOnce(Return(ByMove(pStI.first)));
            statements += EXPECT_CALL(dbHandler, GetStatement(updateStatement))
                              .After(savepoint)
                              .WillOnce(Return(ByMove(pStU.first)));
            if (!actorsToDelete.empty())
            {
                statements += EXPECT_CALL(dbHandler, GetStatement(deleteStatement))
                                  .After(savepoint)
                                  .WillOnce(Return(ByMove(pStD.first)));
            }
            dbHandler.ExpectSavepointRelease("InsertActors");
        }

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        // Last step
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));
        EXPECT_CALL(stS, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(6));
        // Register in reverse order
        for (std::size_t i = actorsInDatabase.size(); i > 0; --i)
        {
            const std::pair<uint8_t, int64_t>& p = actorsInDatabase[i - 1];
            ExpectationSet stepExp = EXPECT_CALL(stS, Step())
                                         .After(selectBound)
                                         .WillOnce(Return(MockStatement::row))
                                         .RetiresOnSaturation();
            // actor_id
            EXPECT_CALL(stS, GetInt(0)).After(stepExp).WillOnce(Return(p.first)).RetiresOnSaturation();
            // actor_uid
            EXPECT_CALL(stS, GetInt64(1)).After(stepExp).WillOnce(Return(p.second)).RetiresOnSaturation();
        }

        // Expectations for updates
        if (actorsToUpdate.empty())
        {
            EXPECT_CALL(stU, Step()).Times(0);
        }
        for (std::size_t i = actorsToUpdate.size(); i > 0; --i)
        {
            const std::pair<int64_t, Actor>& a = actorsToUpdate[i - 1];
            ExpectationSet updateBound = EXPECT_CALL(stU, BindString(1, a.second.GetName()))
                                             .WillOnce(Return(MockDatabase::ok))
                                             .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindString(2, a.second.GetLocation()))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindString(3, a.second.GetState()))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindInt(4, a.second.m_type))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindInt(5, a.second.m_pin))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound
                += EXPECT_CALL(stU, BindInt64(6, a.first)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
            {
                InSequence s;
                EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                EXPECT_CALL(stU, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
            }
        }

        // Expectations for inserts
        if (actorsToInsert.empty())
        {
            EXPECT_CALL(stI, Step()).Times(0);
        }
        for (std::size_t i = actorsToInsert.size(); i > 0; --i)
        {
            const std::pair<uint8_t, Actor>& p = actorsToInsert[i - 1];
            if (p.second.IsDeleted())
            {
                EXPECT_CALL(stI, BindInt(2, i - 1)).Times(0);
            }
            else
            {
                ExpectationSet insertBound;
                insertBound
                    += EXPECT_CALL(stI, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(2, p.first)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(3, p.second.GetName()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(4, p.second.GetLocation()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(5, p.second.GetState()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindInt(6, p.second.m_type))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindInt(7, p.second.m_pin))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step())
                        .After(insertBound)
                        .WillOnce(Return(MockStatement::done))
                        .RetiresOnSaturation();
                    EXPECT_CALL(stI, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                }
            }
        }

        // Expectations for delete
        if (!actorsToDelete.empty())
        {
            for (std::size_t i = actorsToDelete.size(); i > 0; --i)
            {
                const int64_t& uid = actorsToDelete[i - 1];
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, uid)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                EXPECT_CALL(stD, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
            }
        }

        // Enable deleteAdditional
        actorSerialize.InsertActors(nodeId, actorsUpdated, true);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Exception in prepare (verify that savepoint is rolled back)
    {
        // UpdateActor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT actor_id,actor_uid FROM actors WHERE node_id=?1;";
        const uint16_t nodeId = 3;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertActors"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Throw(std::runtime_error("testing")));
            dbHandler.ExpectSavepointRollback("InsertActors");
        }
        std::vector<Actor> actors{Actor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2), Actor::Deleted(),
            Actor(nodeId, 1, "n1", "l1", ::Types::LAMP, Pins::D11),
            Actor(nodeId, 2, "n2", "l2", ::Types::SOCKET, Pins::A5)};

        EXPECT_THROW(actorSerialize.InsertActors(nodeId, actors), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(SensorSerialize, GetSensors)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBSensorSerialize sensorSerialize{dbHandler};

    // success
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);
        std::vector<Sensor> sensors{Sensor(3, 0, "n0", "l0", ::Types::LED, Pins::D2, 5),
            Sensor(3, 1, "n1", "l1", ::Types::LAMP, Pins::D11, 0),
            Sensor(3, 2, "n2", "l2", ::Types::SOCKET, Pins::A5, 3)};
        sensors[0].m_state = "a";
        sensors[1].m_state = "b";
        sensors[2].m_state = "c";
        const uint16_t nodeId = 3;
        ExpectGetSensors(dbHandler, nodeId, sensors);

        std::vector<Sensor> result = sensorSerialize.GetSensors(nodeId);
        EXPECT_THAT(result, ContainerEq(sensors));
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail savepoint
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        EXPECT_CALL(dbHandler, GetROSavepoint("GetSensors")).WillOnce(Throw(std::runtime_error("test")));
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);

        EXPECT_THROW(sensorSerialize.GetSensors(3), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // fail statement
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, "
                                      "sensor_pin, sensor_listener FROM sensors WHERE node_id=?1 ORDER BY sensor_id;";
        const uint16_t nodeId = 3;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetSensors"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Throw(std::runtime_error("test")));
            dbHandler.ExpectROSavepointRollback("GetSensors");
        }

        EXPECT_THROW(sensorSerialize.GetSensors(nodeId), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // fail step
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, "
                                      "sensor_pin, sensor_listener FROM sensors WHERE node_id=?1 ORDER BY sensor_id;";
        const uint16_t nodeId = 3;
        ExpectationSet e;
        auto pSt = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetSensors"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));
            e += EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
            dbHandler.ExpectROSavepointRollback("GetSensors");
        }

        // First step fails
        EXPECT_CALL(st, Step()).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(st, GetError()).WillRepeatedly(Return("testing"));

        EXPECT_THROW(sensorSerialize.GetSensors(nodeId), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(SensorSerialize, UpdateSensor)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBSensorSerialize sensorSerialize{dbHandler};

    // Delete existing sensor
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* deleteStatement = "DELETE FROM sensors WHERE node_id=?1 AND sensor_id=?2;";
        const uint16_t nodeId = 3;
        const uint8_t sensorId = 1;
        auto pSt = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateSensor"));
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(pSt.first)));
            dbHandler.ExpectSavepointRelease("UpdateSensor");
        }
        ExpectationSet e;
        e += EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        e += EXPECT_CALL(st, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(st, Step()).After(e).WillOnce(Return(MockStatement::done));

        sensorSerialize.UpdateSensor(nodeId, sensorId, Sensor::Deleted());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Insert new sensor
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_uid FROM sensors WHERE node_id=?1 AND sensor_id=?2;";
        const char* insertStatement
            = "INSERT INTO sensors (node_id, sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, "
              "sensor_pin, sensor_listener) VALUES(?1,?2,?3,?4,?5,?6,?7,?8);";
        const uint16_t nodeId = 3;
        const uint8_t sensorId = 1;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateSensor"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(pStI.first)));
            dbHandler.ExpectSavepointRelease("UpdateSensor");
        }
        Sensor s{nodeId, sensorId, "abc", "def", 2, 3, 5};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        selectBound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

        ExpectationSet insertBound;
        insertBound += EXPECT_CALL(stI, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(3, s.GetName())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(4, s.GetLocation())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(5, s.GetState())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(6, s.m_type)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(7, s.m_pin)).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(8, s.m_interval)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(stI, Step()).After(insertBound).WillOnce(Return(MockStatement::done));

        sensorSerialize.UpdateSensor(nodeId, sensorId, s);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Update existing sensor
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_uid FROM sensors WHERE node_id=?1 AND sensor_id=?2;";
        const char* updateStatement
            = "UPDATE sensors SET sensor_name=?3, sensor_location=?4, sensor_state=?5, sensor_type=?6, sensor_pin=?7, "
              "sensor_listener=?8 WHERE node_id=?1 AND sensor_id=?2;";
        const uint16_t nodeId = 3;
        const uint8_t sensorId = 1;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateSensor"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRelease("UpdateSensor");
        }
        Sensor s{nodeId, sensorId, "abc", "def", 2, 3, 5};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        selectBound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        // Select returns row to indicate that an sensor was found
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::row));

        ExpectationSet updateBound;
        updateBound += EXPECT_CALL(stU, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(3, s.GetName())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(4, s.GetLocation())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(5, s.GetState())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(6, s.m_type)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(7, s.m_pin)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(8, s.m_interval)).WillOnce(Return(MockDatabase::ok));
        EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockStatement::done));

        sensorSerialize.UpdateSensor(nodeId, sensorId, s);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail update existing sensor
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_uid FROM sensors WHERE node_id=?1 AND sensor_id=?2;";
        const char* updateStatement
            = "UPDATE sensors SET sensor_name=?3, sensor_location=?4, sensor_state=?5, sensor_type=?6, sensor_pin=?7, "
              "sensor_listener=?8 WHERE node_id=?1 AND sensor_id=?2;";
        const uint16_t nodeId = 3;
        const uint8_t sensorId = 1;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("UpdateSensor"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRollback("UpdateSensor");
        }
        Sensor s{nodeId, sensorId, "abc", "def", 2, 3, 5};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        selectBound += EXPECT_CALL(stS, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        // Select returns row to indicate that an sensor was found
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::row));

        ExpectationSet updateBound;
        updateBound += EXPECT_CALL(stU, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(2, sensorId)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(3, s.GetName())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(4, s.GetLocation())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(5, s.GetState())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(6, s.m_type)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(7, s.m_pin)).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(8, s.m_interval)).WillOnce(Return(MockDatabase::ok));
        // Update fails
        EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(stU, GetError()).WillRepeatedly(Return("testing"));

        EXPECT_THROW(sensorSerialize.UpdateSensor(nodeId, sensorId, s), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(SensorSerialize, InsertSensors)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBSensorSerialize sensorSerialize{dbHandler};

    // Insert into empty table
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_id,sensor_uid FROM sensors WHERE node_id=?1;";
        const char* updateStatement = "UPDATE sensors SET "
                                      "sensor_name=?1,sensor_location=?2,sensor_state=?3,sensor_type=?4,sensor_pin=?5,"
                                      "sensor_listener=?6 WHERE sensor_uid=?7;";
        const char* insertStatement
            = "INSERT INTO sensors (node_id, sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, "
              "sensor_pin, sensor_listener) VALUES(?1,?2,?3,?4,?5,?6,?7,?8);";
        const uint16_t nodeId = 3;
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertSensors"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(pStI.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRelease("InsertSensors");
        }
        std::vector<Sensor> sensors{Sensor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2, 2), Sensor::Deleted(),
            Sensor(nodeId, 1, "n1", "l1", ::Types::LAMP, Pins::D11, 3),
            Sensor(nodeId, 2, "n2", "l2", ::Types::SOCKET, Pins::A5, 0)};

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        // No sensors from select
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

        // Nothing updated
        EXPECT_CALL(stU, Step()).Times(0);

        for (std::size_t i = sensors.size(); i > 0; --i)
        {
            const Sensor& s = sensors[i - 1];
            if (s.IsDeleted())
            {
                EXPECT_CALL(stI, BindInt(2, i - 1)).Times(0);
            }
            else
            {
                ExpectationSet insertBound;
                insertBound
                    += EXPECT_CALL(stI, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(2, i - 1)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(3, s.GetName()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(4, s.GetLocation()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(5, s.GetState()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(6, s.m_type)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(7, s.m_pin)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindInt(8, s.m_interval))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step())
                        .After(insertBound)
                        .WillOnce(Return(MockStatement::done))
                        .RetiresOnSaturation();
                    EXPECT_CALL(stI, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                }
            }
        }

        sensorSerialize.InsertSensors(nodeId, sensors);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Insert, update and delete in table with some existing
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_id,sensor_uid FROM sensors WHERE node_id=?1;";
        const char* updateStatement = "UPDATE sensors SET "
                                      "sensor_name=?1,sensor_location=?2,sensor_state=?3,sensor_type=?4,sensor_pin=?5,"
                                      "sensor_listener=?6 WHERE sensor_uid=?7;";
        const char* insertStatement
            = "INSERT INTO sensors (node_id, sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, "
              "sensor_pin, sensor_listener) VALUES(?1,?2,?3,?4,?5,?6,?7,?8);";
        const char* deleteStatement = "DELETE FROM sensors WHERE sensor_uid=?1;";
        const uint16_t nodeId = 3;
        // Sensors passed to the update statement
        std::vector<Sensor> sensorsUpdated{Sensor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2, 2), Sensor::Deleted(),
            Sensor(nodeId, 2, "n2", "l2", ::Types::LAMP, Pins::D11, 3),
            Sensor(nodeId, 3, "n3", "l3", ::Types::SOCKET, Pins::A5, 0)};
        // Sensors ids + uids which will be returned by the select statement
        std::vector<std::pair<uint8_t, int64_t>> sensorsInDatabase{{0, 1},
            // Sensor 1 should be deleted
            {1, 2}, {2, 3},
            // Sensor 3 to be inserted
            // Sensor 4 to be deleted
            {4, 4}};
        // Sensor ids and Sensors which should be newly inserted (in sensorsUpdated, but not in database)
        std::vector<std::pair<uint8_t, Sensor>> sensorsToInsert{
            {3, Sensor(nodeId, 3, "n3", "l3", ::Types::SOCKET, Pins::A5, 0)}};
        // Sensor uids and Sensors which should be updated (in both sensorsUpdated and database)
        std::vector<std::pair<int64_t, Sensor>> sensorsToUpdate{
            {1, Sensor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2, 2)},
            {3, Sensor(nodeId, 2, "n2", "l2", ::Types::LAMP, Pins::D11, 3)},
        };
        // Sensor uids which should be deleted (not in sensorsUpdated but in database)
        std::vector<int64_t> sensorsToDelete{2, 4};

        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        auto pStD = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& stD = pStD.second;
        {
            ExpectationSet savepoint = EXPECT_CALL(dbHandler, GetSavepoint("InsertSensors"));
            ExpectationSet statements;
            statements += EXPECT_CALL(dbHandler, GetStatement(selectStatement))
                              .After(savepoint)
                              .WillOnce(Return(ByMove(pStS.first)));
            statements += EXPECT_CALL(dbHandler, GetStatement(insertStatement))
                              .After(savepoint)
                              .WillOnce(Return(ByMove(pStI.first)));
            statements += EXPECT_CALL(dbHandler, GetStatement(updateStatement))
                              .After(savepoint)
                              .WillOnce(Return(ByMove(pStU.first)));
            if (!sensorsToDelete.empty())
            {
                statements
                    += EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(pStD.first)));
            }
            EXPECT_CALL(dbHandler.db, ExecuteStatement("RELEASE SAVEPOINT InsertSensors;")).After(statements);
        }

        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
        // Last step
        EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));
        EXPECT_CALL(stS, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(2));
        // Register in reverse order
        for (std::size_t i = sensorsInDatabase.size(); i > 0; --i)
        {
            const std::pair<uint8_t, int64_t>& p = sensorsInDatabase[i - 1];
            ExpectationSet stepExp = EXPECT_CALL(stS, Step())
                                         .After(selectBound)
                                         .WillOnce(Return(MockStatement::row))
                                         .RetiresOnSaturation();
            // sensor_id
            EXPECT_CALL(stS, GetInt(0)).After(stepExp).WillOnce(Return(p.first)).RetiresOnSaturation();
            // sensor_uid
            EXPECT_CALL(stS, GetInt64(1)).After(stepExp).WillOnce(Return(p.second)).RetiresOnSaturation();
        }

        // Expectations for updates
        if (sensorsToUpdate.empty())
        {
            EXPECT_CALL(stU, Step()).Times(0);
        }
        for (std::size_t i = sensorsToUpdate.size(); i > 0; --i)
        {
            const std::pair<int64_t, Sensor>& s = sensorsToUpdate[i - 1];
            ExpectationSet updateBound = EXPECT_CALL(stU, BindString(1, s.second.GetName()))
                                             .WillOnce(Return(MockDatabase::ok))
                                             .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindString(2, s.second.GetLocation()))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindString(3, s.second.GetState()))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindInt(4, s.second.m_type))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindInt(5, s.second.m_pin))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound += EXPECT_CALL(stU, BindInt(6, s.second.m_interval))
                               .WillOnce(Return(MockDatabase::ok))
                               .RetiresOnSaturation();
            updateBound
                += EXPECT_CALL(stU, BindInt64(7, s.first)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
            {
                InSequence s;
                EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                EXPECT_CALL(stU, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
            }
        }

        // Expectations for inserts
        if (sensorsToInsert.empty())
        {
            EXPECT_CALL(stI, Step()).Times(0);
        }
        for (std::size_t i = sensorsToInsert.size(); i > 0; --i)
        {
            const std::pair<uint8_t, Sensor>& p = sensorsToInsert[i - 1];
            if (p.second.IsDeleted())
            {
                EXPECT_CALL(stI, BindInt(2, i - 1)).Times(0);
            }
            else
            {
                ExpectationSet insertBound;
                insertBound
                    += EXPECT_CALL(stI, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound
                    += EXPECT_CALL(stI, BindInt(2, p.first)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(3, p.second.GetName()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(4, p.second.GetLocation()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindString(5, p.second.GetState()))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindInt(6, p.second.m_type))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindInt(7, p.second.m_pin))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                insertBound += EXPECT_CALL(stI, BindInt(8, p.second.m_interval))
                                   .WillOnce(Return(MockDatabase::ok))
                                   .RetiresOnSaturation();
                {
                    InSequence s;
                    EXPECT_CALL(stI, Step())
                        .After(insertBound)
                        .WillOnce(Return(MockStatement::done))
                        .RetiresOnSaturation();
                    EXPECT_CALL(stI, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                }
            }
        }

        // Expectations for delete
        if (!sensorsToDelete.empty())
        {
            for (std::size_t i = sensorsToDelete.size(); i > 0; --i)
            {
                const int64_t& uid = sensorsToDelete[i - 1];
                InSequence s;
                EXPECT_CALL(stD, BindInt64(1, uid)).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
                EXPECT_CALL(stD, Step()).WillOnce(Return(MockStatement::done)).RetiresOnSaturation();
                EXPECT_CALL(stD, Reset()).WillOnce(Return(MockDatabase::ok)).RetiresOnSaturation();
            }
        }

        // Enable deleteAdditional
        sensorSerialize.InsertSensors(nodeId, sensorsUpdated, true);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Exception in prepare (verify that savepoint is rolled back)
    {
        // UpdateSensor should only use the read write database
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT sensor_id,sensor_uid FROM sensors WHERE node_id=?1;";
        const uint16_t nodeId = 3;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("InsertSensors"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Throw(std::runtime_error("test")));
            dbHandler.ExpectSavepointRollback("InsertSensors");
        }
        std::vector<Sensor> sensors{Sensor(nodeId, 0, "n0", "l0", ::Types::LED, Pins::D2, 3), Sensor::Deleted(),
            Sensor(nodeId, 1, "n1", "l1", ::Types::LAMP, Pins::D11, 2),
            Sensor(nodeId, 2, "n2", "l2", ::Types::SOCKET, Pins::A5, 0)};

        EXPECT_THROW(sensorSerialize.InsertSensors(nodeId, sensors), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeSerialize, GetFreeNodeId)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    // Consecutive nodes
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT node_id FROM nodes ORDER BY node_id ASC;";
        const uint16_t nodeCount = 4;
        auto pSt = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetFreeNodeId"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));
            dbHandler.ExpectROSavepointRelease("GetFreeNodeId");
        }
        // Last step
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        EXPECT_CALL(st, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(1));
        // Register in reverse order
        for (std::size_t i = nodeCount; i > 0; --i)
        {
            ExpectationSet stepExp = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(0)).After(stepExp).WillOnce(Return(i - 1)).RetiresOnSaturation();
        }
        EXPECT_EQ(nodeCount, nodeSerialize.GetFreeNodeId());
        Mock::VerifyAndClearExpectations(&dbHandler);
    }

    // Found hole in node ids
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT node_id FROM nodes ORDER BY node_id ASC;";
        const std::vector<uint16_t> nodes{1, 2, 4, 5};
        auto pSt = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetFreeNodeId"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));
            dbHandler.ExpectROSavepointRelease("GetFreeNodeId");
        }
        EXPECT_CALL(st, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(1));
        // Register in reverse order
        for (std::size_t i = 3; i > 0; --i)
        {
            ExpectationSet stepExp = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(0)).After(stepExp).WillOnce(Return(nodes[i - 1])).RetiresOnSaturation();
        }
        EXPECT_EQ(3, nodeSerialize.GetFreeNodeId());
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Error
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT node_id FROM nodes ORDER BY node_id ASC;";
        const std::vector<uint16_t> nodes{1, 2, 4, 5};
        auto pSt = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetROSavepoint("GetFreeNodeId"));
            EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));
            dbHandler.ExpectROSavepointRollback("GetFreeNodeId");
        }
        EXPECT_CALL(st, GetColumnCount()).Times(AnyNumber()).WillRepeatedly(Return(1));

        // Select fails
        EXPECT_CALL(st, Step()).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(st, GetError()).WillRepeatedly(Return("testing"));

        EXPECT_THROW(nodeSerialize.GetFreeNodeId(), std::runtime_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeSerialize, AddNodeOnly)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    // Node does not exist, insert
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT node_id FROM nodes WHERE node_id=?1;";
        const char* insertStatement = "INSERT INTO nodes (node_id, node_name, node_location, node_state, node_path, "
                                      "node_path_distance, node_type) VALUES(?1,?2,?3,?4,?5,?6,?7);";
        const NodeData node{2, "n", "l", {}, {}, "s", NodePath(2, 3), 1};
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStI = dbHandler.GetMockedStatement(insertStatement);
        MockStatement& stI = pStI.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddNodeOnly"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(insertStatement)).WillOnce(Return(ByMove(pStI.first)));
            dbHandler.ExpectSavepointRelease("AddNodeOnly");
        }
        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, node.m_id)).WillOnce(Return(MockDatabase::ok));
        // No node found
        ExpectationSet selectDone = EXPECT_CALL(stS, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

        ExpectationSet insertBound;
        insertBound += EXPECT_CALL(stI, BindInt(1, node.GetId())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(2, node.GetName())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(3, node.GetLocation())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindString(4, node.GetState())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(5, node.m_path.GetPath())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(6, node.m_path.GetDistance())).WillOnce(Return(MockDatabase::ok));
        insertBound += EXPECT_CALL(stI, BindInt(7, node.m_type)).WillOnce(Return(MockDatabase::ok));

        EXPECT_CALL(stI, Step()).After(insertBound).WillOnce(Return(MockStatement::done));

        nodeSerialize.AddNodeOnly(node);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Node does exists, update
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT node_id FROM nodes WHERE node_id=?1;";
        const char* updateStatement = "UPDATE nodes SET node_name=?2, node_location=?3, node_state=?4, node_path=?5, "
                                      "node_path_distance=?6, node_type=?7 WHERE node_id=?1;";
        const NodeData node{2, "n", "l", {}, {}, "s", NodePath(2, 3), 1};
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddNodeOnly"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRelease("AddNodeOnly");
        }
        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, node.m_id)).WillOnce(Return(MockDatabase::ok));
        // No node found
        ExpectationSet selectDone = EXPECT_CALL(stS, Step())
                                        .After(selectBound)
                                        .WillOnce(Return(MockStatement::row))
                                        .WillOnce(Return(MockStatement::done));

        ExpectationSet updateBound;
        updateBound += EXPECT_CALL(stU, BindInt(1, node.GetId())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(2, node.GetName())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(3, node.GetLocation())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(4, node.GetState())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(5, node.m_path.GetPath())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(6, node.m_path.GetDistance())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(7, node.m_type)).WillOnce(Return(MockDatabase::ok));

        EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockStatement::done));

        nodeSerialize.AddNodeOnly(node);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Node does exists, update fails
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);

        const char* selectStatement = "SELECT node_id FROM nodes WHERE node_id=?1;";
        const char* updateStatement = "UPDATE nodes SET node_name=?2, node_location=?3, node_state=?4, node_path=?5, "
                                      "node_path_distance=?6, node_type=?7 WHERE node_id=?1;";
        const NodeData node{2, "n", "l", {}, {}, "s", NodePath(2, 3), 1};
        auto pStS = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& stS = pStS.second;
        auto pStU = dbHandler.GetMockedStatement(updateStatement);
        MockStatement& stU = pStU.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddNodeOnly"));
            EXPECT_CALL(dbHandler, GetStatement(selectStatement)).WillOnce(Return(ByMove(pStS.first)));
            EXPECT_CALL(dbHandler, GetStatement(updateStatement)).WillOnce(Return(ByMove(pStU.first)));
            dbHandler.ExpectSavepointRollback("AddNodeOnly");
        }
        ExpectationSet selectBound;
        selectBound += EXPECT_CALL(stS, BindInt(1, node.m_id)).WillOnce(Return(MockDatabase::ok));
        // No node found
        ExpectationSet selectDone = EXPECT_CALL(stS, Step())
                                        .After(selectBound)
                                        .WillOnce(Return(MockStatement::row))
                                        .WillOnce(Return(MockStatement::done));

        ExpectationSet updateBound;
        updateBound += EXPECT_CALL(stU, BindInt(1, node.GetId())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(2, node.GetName())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(3, node.GetLocation())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindString(4, node.GetState())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(5, node.m_path.GetPath())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(6, node.m_path.GetDistance())).WillOnce(Return(MockDatabase::ok));
        updateBound += EXPECT_CALL(stU, BindInt(7, node.m_type)).WillOnce(Return(MockDatabase::ok));

        EXPECT_CALL(stU, Step()).After(updateBound).WillOnce(Return(MockDatabase::error));
        EXPECT_CALL(stU, GetError()).WillRepeatedly(Return("testing"));

        EXPECT_THROW(nodeSerialize.AddNodeOnly(node), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeSerialize, AddNode)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    // Success
    {
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);

        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        // For other savepoint releases
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());

        ExpectationSet e;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddNode"));
            // Just verify that the correct statements are used, the rest is tested in AddNodeOnly, InsertSensors and
            // InsertActors
            e += EXPECT_CALL(dbHandler,
                GetStatement("INSERT INTO nodes (node_id, node_name, node_location, node_state, node_path, "
                             "node_path_distance, node_type) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            dbHandler.ExpectSavepointRelease("AddNode");
        }

        EXPECT_CALL(dbHandler,
            GetStatement("INSERT INTO sensors (node_id, sensor_id, sensor_name, sensor_location, sensor_state, "
                         "sensor_type, sensor_pin, sensor_listener) VALUES(?1,?2,?3,?4,?5,?6,?7,?8);"))
            .After(e);
        EXPECT_CALL(dbHandler,
            GetStatement("INSERT INTO actors (node_id, actor_id, actor_name, actor_location, actor_state, actor_type, "
                         "actor_pin) VALUES(?1,?2,?3,?4,?5,?6,?7);"))
            .After(e);

        nodeSerialize.AddNode(NodeData{});

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail
    {
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);

        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        // For other savepoint releases
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());

        ExpectationSet e;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetSavepoint("AddNode"));
            // Just verify that the correct statements are used, the rest is tested in AddNodeOnly, InsertSensors and
            // InsertActors
            e += EXPECT_CALL(dbHandler,
                GetStatement("INSERT INTO nodes (node_id, node_name, node_location, node_state, node_path, "
                             "node_path_distance, node_type) VALUES(?1,?2,?3,?4,?5,?6,?7);"));
            dbHandler.ExpectSavepointRollback("AddNode");
        }

        // Insert fails
        EXPECT_CALL(dbHandler,
            GetStatement("INSERT INTO sensors (node_id, sensor_id, sensor_name, sensor_location, sensor_state, "
                         "sensor_type, sensor_pin, sensor_listener) VALUES(?1,?2,?3,?4,?5,?6,?7,?8);"))
            .After(e)
            .WillOnce(Throw(std::runtime_error("test")));

        EXPECT_THROW(nodeSerialize.AddNode(NodeData{}), std::runtime_error);

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeSerialize, RemoveNode)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    // Success
    {
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);

        const uint16_t nodeId = 3;
        const char* deleteStatement = "DELETE FROM nodes WHERE node_id=?1;";
        auto pSt = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(pSt.first)));
            EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        }
        nodeSerialize.RemoveNode(nodeId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Fail
    {
        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(0);
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(0);

        const uint16_t nodeId = 3;
        const char* deleteStatement = "DELETE FROM nodes WHERE node_id=?1;";
        auto pSt = dbHandler.GetMockedStatement(deleteStatement);
        MockStatement& st = pSt.second;
        {
            InSequence s;
            EXPECT_CALL(dbHandler, GetStatement(deleteStatement)).WillOnce(Return(ByMove(pSt.first)));
            EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));
            EXPECT_CALL(st, Step()).WillOnce(Return(MockDatabase::error));
        }
        EXPECT_CALL(st, GetError()).WillRepeatedly(Return("testing"));
        EXPECT_THROW(nodeSerialize.RemoveNode(nodeId), std::runtime_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeSerialize, GetNodesFromQuery)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();
    DBNodeSerialize nodeSerialize{dbHandler};
    NodeManager nodeManager{};

    std::shared_ptr<MockStatement> pSt = std::make_shared<MockStatement>("test;");
    MockStatement& st = *pSt;

    DBResult res{pSt};

    // Only need to get Sensors and Actors from ro db
    EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
    EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

    EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
    // For other savepoint releases
    EXPECT_CALL(dbHandler.roDb, ExecuteStatement(_)).Times(AnyNumber());

    // No results
    {
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        ::Range<NodeData> r = nodeSerialize.GetNodesFromQuery(res);
        EXPECT_EQ(r.begin(), r.end());
        Mock::VerifyAndClearExpectations(&st);
        // Dereference invalid iterator
        EXPECT_THROW(*r.begin(), std::logic_error);
    }
    // Multiple results
    {
        std::vector<NodeData> nodes{{1, "n1", "l1", {}, {}, "s1", NodePath(1, 1), 0},
            {2, "n2", "l2", {}, {}, "s2", NodePath(3, 1), 2}, {3, "n3", "l3", {}, {}, "s3", NodePath(2, 2), 1}};

        // Queries are evaluated lazily!
        // Last step
        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        // Nodes in reverse order
        for (std::size_t i = nodes.size(); i > 0; --i)
        {
            const NodeData& n = nodes[i - 1];
            EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
            ExpectationSet step = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row));
            EXPECT_CALL(st, GetInt(0)).After(step).WillOnce(Return(n.GetId())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(n.GetName())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(n.GetLocation())).RetiresOnSaturation();
            EXPECT_CALL(st, GetString(3)).After(step).WillOnce(Return(n.GetState())).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(4)).After(step).WillOnce(Return(n.m_path.GetPath())).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(5)).After(step).WillOnce(Return(n.m_path.GetDistance())).RetiresOnSaturation();
            EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(n.m_type)).RetiresOnSaturation();

            EXPECT_CALL(dbHandler,
                GetROStatement("SELECT actor_id, actor_name, actor_location, actor_state, actor_type, actor_pin FROM "
                               "actors WHERE node_id=?1 ORDER BY actor_id;"))
                .RetiresOnSaturation();
            EXPECT_CALL(dbHandler,
                GetROStatement("SELECT sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, sensor_pin, "
                               "sensor_listener FROM sensors WHERE node_id=?1 ORDER BY sensor_id;"))
                .RetiresOnSaturation();
        }

        ::Range<NodeData> r = nodeSerialize.GetNodesFromQuery(res);
        auto it = r.begin();
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            EXPECT_EQ(nodes[i], *it);
            ASSERT_NE(it, r.end());

            ++it;
        }
        EXPECT_EQ(it, r.end());
        Mock::VerifyAndClearExpectations(&st);
    }
    // Result with sensor and actor
    {
        NodeData n{1, "n1", "l1", {Sensor{1, 0, "ns0", "ls0", ::Types::PUSHBUTTON, ::Pins::D3, 3}},
            {Actor{1, 0, "na0", "nl0", ::Types::LAMP, ::Pins::D4}}, "s1", NodePath(1, 1), 0};

        EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));
        ExpectationSet step = EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::row));
        EXPECT_CALL(st, GetColumnCount()).WillRepeatedly(Return(7));
        EXPECT_CALL(st, GetInt(0)).After(step).WillOnce(Return(n.GetId()));
        EXPECT_CALL(st, GetString(1)).After(step).WillOnce(Return(n.GetName()));
        EXPECT_CALL(st, GetString(2)).After(step).WillOnce(Return(n.GetLocation()));
        EXPECT_CALL(st, GetString(3)).After(step).WillOnce(Return(n.GetState()));
        EXPECT_CALL(st, GetInt(4)).After(step).WillOnce(Return(n.m_path.GetPath()));
        EXPECT_CALL(st, GetInt(5)).After(step).WillOnce(Return(n.m_path.GetDistance()));
        EXPECT_CALL(st, GetInt(6)).After(step).WillOnce(Return(n.m_type));

        const char* actorSelectStatement = "SELECT actor_id, actor_name, actor_location, actor_state, actor_type, "
                                           "actor_pin FROM actors WHERE node_id=?1 ORDER BY actor_id;";
        const char* sensorSelectStatement
            = "SELECT sensor_id, sensor_name, sensor_location, sensor_state, sensor_type, sensor_pin, sensor_listener "
              "FROM sensors WHERE node_id=?1 ORDER BY sensor_id;";
        auto pStA = dbHandler.GetMockedStatement(actorSelectStatement);
        MockStatement& stA = pStA.second;
        auto pStS = dbHandler.GetMockedStatement(sensorSelectStatement);
        MockStatement& stS = pStS.second;

        EXPECT_CALL(dbHandler, GetROStatement(actorSelectStatement)).WillOnce(Return(ByMove(pStA.first)));
        EXPECT_CALL(dbHandler, GetROStatement(sensorSelectStatement)).WillOnce(Return(ByMove(pStS.first)));

        // Actor expectations
        {
            ExpectationSet bound = EXPECT_CALL(stA, BindInt(1, n.m_id));
            EXPECT_CALL(stA, Step()).WillOnce(Return(MockStatement::done));
            EXPECT_CALL(stA, GetColumnCount()).WillRepeatedly(Return(6));
            ExpectationSet stepExp
                = EXPECT_CALL(stA, Step()).After(bound).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            const Actor& a = n.m_actors[0];
            EXPECT_CALL(stA, GetInt(0)).After(stepExp).WillOnce(Return(0)).RetiresOnSaturation();
            EXPECT_CALL(stA, GetString(1)).After(stepExp).WillOnce(Return(a.GetName())).RetiresOnSaturation();
            EXPECT_CALL(stA, GetString(2)).After(stepExp).WillOnce(Return(a.GetLocation())).RetiresOnSaturation();
            EXPECT_CALL(stA, GetString(3)).After(stepExp).WillOnce(Return(a.m_state)).RetiresOnSaturation();
            EXPECT_CALL(stA, GetInt(4)).After(stepExp).WillOnce(Return(a.m_type)).RetiresOnSaturation();
            EXPECT_CALL(stA, GetInt(5)).After(stepExp).WillOnce(Return(a.m_pin)).RetiresOnSaturation();
        }
        // Sensor expectations
        {
            const Sensor& s = n.m_sensors[0];
            ExpectationSet bound = EXPECT_CALL(stS, BindInt(1, n.m_id));
            EXPECT_CALL(stS, Step()).After(bound).WillOnce(Return(MockStatement::done));
            EXPECT_CALL(stS, GetColumnCount()).WillRepeatedly(Return(7));
            ExpectationSet stepExp
                = EXPECT_CALL(stS, Step()).WillOnce(Return(MockStatement::row)).RetiresOnSaturation();
            EXPECT_CALL(stS, GetInt(0)).After(stepExp).WillOnce(Return(0)).RetiresOnSaturation();
            EXPECT_CALL(stS, GetString(1)).After(stepExp).WillOnce(Return(s.GetName())).RetiresOnSaturation();
            EXPECT_CALL(stS, GetString(2)).After(stepExp).WillOnce(Return(s.GetLocation())).RetiresOnSaturation();
            EXPECT_CALL(stS, GetString(3)).After(stepExp).WillOnce(Return(s.m_state)).RetiresOnSaturation();
            EXPECT_CALL(stS, GetInt(4)).After(stepExp).WillOnce(Return(s.m_type)).RetiresOnSaturation();
            EXPECT_CALL(stS, GetInt(5)).After(stepExp).WillOnce(Return(s.m_pin)).RetiresOnSaturation();
            EXPECT_CALL(stS, GetInt(6)).After(stepExp).WillOnce(Return(s.m_interval)).RetiresOnSaturation();
        }

        ::Range<NodeData> r = nodeSerialize.GetNodesFromQuery(res);
        auto it = r.begin();
        EXPECT_EQ(n, *it);
        EXPECT_EQ(n.m_sensors, it->m_sensors);
        EXPECT_EQ(n.m_actors, it->m_actors);
        ASSERT_NE(it, r.end());
        ++it;
        EXPECT_EQ(it, r.end());

        Mock::VerifyAndClearExpectations(&st);
    }
}

TEST(NodeSerialize, GetNodeById)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
    EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

    // No node found
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        // Just verify that the correct statements are used, the rest is tested in GetNodesFromQuery
        const char* selectStatement = "SELECT node_id, node_name, node_location, node_state, node_path, "
                                      "node_path_distance, node_type FROM nodes WHERE node_id=?1;";
        const uint16_t nodeId = 1;
        auto pSt = dbHandler.GetMockedStatement(selectStatement);
        MockStatement& st = pSt.second;

        EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));

        ExpectationSet selectBound = EXPECT_CALL(st, BindInt(1, nodeId)).WillOnce(Return(MockDatabase::ok));

        EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

        NodeData result = nodeSerialize.GetNodeById(nodeId);
        EXPECT_EQ(NodeData::EmptyNode(), result);
        EXPECT_TRUE(result.IsEmpty());

        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Node found
    {
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

        const uint16_t nodeId = 1;
        NodeData n{nodeId, "name", "location", {Sensor()}, {Actor::Deleted(), Actor()}, "state", NodePath(2, 1), 0};
        ExpectGetNode(dbHandler, n);
        NodeData result = nodeSerialize.GetNodeById(nodeId);
        EXPECT_EQ(n, result);
    }
}

TEST(NodeSerialize, GetNodesByName)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
    EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

    EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
    EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());

    // Just verify that the correct statements are used, the rest is tested in GetNodesFromQuery
    const char* selectStatement = "SELECT node_id, node_name, node_location, node_state, node_path, "
                                  "node_path_distance, node_type FROM nodes WHERE node_name=?1;";
    const std::string name = "test node";
    auto pSt = dbHandler.GetMockedStatement(selectStatement);
    MockStatement& st = pSt.second;

    EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));

    ExpectationSet selectBound = EXPECT_CALL(st, BindString(1, name)).WillOnce(Return(MockDatabase::ok));
    // No node found
    EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

    ::Range<NodeData> result = nodeSerialize.GetNodesByName(name);
    EXPECT_EQ(result.begin(), result.end());
}

TEST(NodeSerialize, GetNodeByPath)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
    EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

    EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
    EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());

    // Just verify that the correct statements are used, the rest is tested in GetNodesFromQuery
    const char* selectStatement
        = "SELECT node_id, node_name, node_location, node_state, node_path, node_path_distance, node_type FROM nodes "
          "WHERE node_path=?1 AND node_path_distance=?2;";
    const NodePath path{3, 1};
    auto pSt = dbHandler.GetMockedStatement(selectStatement);
    MockStatement& st = pSt.second;

    EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));

    ExpectationSet selectBound = EXPECT_CALL(st, BindInt(1, path.GetPath())).WillOnce(Return(MockDatabase::ok));
    selectBound += EXPECT_CALL(st, BindInt(2, path.GetDistance())).WillOnce(Return(MockDatabase::ok));
    // No node found
    EXPECT_CALL(st, Step()).After(selectBound).WillOnce(Return(MockStatement::done));

    NodeData result = nodeSerialize.GetNodeByPath(path);
    EXPECT_EQ(NodeData::EmptyNode(), result);
    EXPECT_TRUE(result.IsEmpty());
}

TEST(NodeSerialize, GetAllNodes)
{
    using namespace ::testing;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize nodeSerialize{dbHandler};

    EXPECT_CALL(dbHandler, GetStatement(_)).Times(0);
    EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(0);

    EXPECT_CALL(dbHandler, GetROSavepoint(_)).Times(AnyNumber());
    EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());

    // Just verify that the correct statements are used, the rest is tested in GetNodesFromQuery
    const char* selectStatement
        = "SELECT node_id, node_name, node_location, node_state, node_path, node_path_distance, node_type FROM nodes;";
    const NodePath path{3, 1};
    auto pSt = dbHandler.GetMockedStatement(selectStatement);
    MockStatement& st = pSt.second;

    EXPECT_CALL(dbHandler, GetROStatement(selectStatement)).WillOnce(Return(ByMove(pSt.first)));

    // No node found
    EXPECT_CALL(st, Step()).WillOnce(Return(MockStatement::done));

    ::Range<NodeData> result = nodeSerialize.GetAllNodes();
    EXPECT_EQ(result.begin(), result.end());
}
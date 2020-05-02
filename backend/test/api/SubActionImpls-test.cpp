#include <gtest/gtest.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/update.h>

#include "api/SubActionImpls.h"
#include "database/ActionsTable.h"

using namespace ::SubActionImpls;

TEST(NotificationImpl, ToJSON)
{
    {
        nlohmann::json json{{"type", 0}, {"category", 0}, {"message", ""}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, Notification().ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"category", 0}, {"message", ""}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, Notification(3).ToJSON());
    }
    {
        nlohmann::json json{
            {"type", 3}, {"category", 4}, {"message", "hello there"}, {"timeout", 0}, {"transition", true}};
        EXPECT_EQ(json, Notification::Create(3, 4, "hello there", std::chrono::seconds(0), true).ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"category", 4}, {"message", "hello there"},
            {"timeout", std::chrono::milliseconds(1000).count()}, {"transition", true}};
        EXPECT_EQ(json, Notification::Create(3, 4, "hello there", std::chrono::seconds(1), true).ToJSON());
    }
}

TEST(NotificationImpl, ParseJSON)
{
    {
        nlohmann::json json{{"type", 0}, {"category", 0}, {"message", ""}, {"timeout", 0}, {"transition", false}};
        Notification n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{
            {"type", 3}, {"category", 25}, {"message", "abcd"}, {"timeout", 10000}, {"transition", true}};
        Notification n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"category", 25}, {"actor", "ignored"}, {"message", "abcd"},
            {"timeout", 10000}, {"transition", true}};
        Notification n;
        n.Parse(json);
    }
    {
        nlohmann::json json{// type missing
            {"category", 25}, {"message", "abcd"}, {"timeout", 10000}, {"transition", true}};
        Notification n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3},
            // node missing
            {"message", "abcd"}, {"timeout", 10000}, {"transition", true}};
        Notification n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"category", 25},
            // value missing
            {"timeout", 10000}, {"transition", true}};
        Notification n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"category", 25}, {"message", "abcd"},
            // timeout missing
            {"transition", true}};
        Notification n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{
            {"type", 3}, {"category", 25}, {"message", "abcd"}, {"timeout", 10000}
            // transition missing
        };
        Notification n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
}

/*TEST(NotificationImpl, ParseDB)
{
    using namespace ::testing;
    DBHandler dbHandler{":memory:"};
    auto& db = dbHandler.GetDatabase();
    db.execute(SubActionsTable::createStatement);
    SubActionsTable subActions;
    auto selectStatement = select(subActions.actionType, subActions.actorNode, subActions.actorId, subActions.val,
        subActions.timeout, subActions.transition)
                               .from(subActions)
                               .unconditionally();
    {
        db(insert_into(subActions)
                .set(subActions.actionType = 2, subActions.actorNode = 3, subActions.actorId = "hello",
                    subActions.timeout = 1000, subActions.transition = false));
        Notification n;
        n.Parse(db, db(selectStatement).front());
        EXPECT_EQ(nlohmann::json(
                      {{"type", 2}, {"category", 3}, {"message", "hello"}, {"timeout", 1000}, {"transition", false}}),
            n.ToJSON());
    }
    {
        db(update(subActions)
                .set(subActions.actionType = 1, subActions.actorNode = 2, subActions.transition = true)
                .unconditionally());
        Notification n;
        n.Parse(db, db(selectStatement).front());
        EXPECT_EQ(nlohmann::json(
                      {{"type", 1}, {"category", 2}, {"message", "hello"}, {"timeout", 1000}, {"transition", true}}),
            n.ToJSON());
    }
}*/

// TODO: TEST(NotificationImpl, Execute)

TEST(DeviceSetImpl, ToJSON)
{
    {
        nlohmann::json json{
            {"type", 0}, {"deviceId", 0}, {"property", ""}, {"value", nlohmann::json()}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, DeviceSet().ToJSON());
    }
    {
        nlohmann::json json{
            {"type", 3}, {"deviceId", 0}, {"property", ""}, {"value", nlohmann::json()}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, DeviceSet(3).ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 4}, {"property", "property.test"}, {"value", "10"},
            {"timeout", 0}, {"transition", true}};
        EXPECT_EQ(
            json, DeviceSet::Create(3, DeviceId{4}, "property.test", "10", std::chrono::seconds(0), true).ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 4}, {"property", "property.test2"}, {"value", "10"},
            {"timeout", std::chrono::milliseconds(1000).count()}, {"transition", true}};
        EXPECT_EQ(
            json, DeviceSet::Create(3, DeviceId{4}, "property.test2", "10", std::chrono::seconds(1), true).ToJSON());
    }
}

TEST(DeviceSetImpl, ParseJSON)
{
    {
        nlohmann::json json{
            {"type", 0}, {"deviceId", 0}, {"property", ""}, {"value", "0"}, {"timeout", 0}, {"transition", false}};
        DeviceSet n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{
            {"type", 3}, {"deviceId", 25}, {"property", "10"}, {"value", "23"}, {"timeout", 10000}, {"transition", true}};
        DeviceSet n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{// type missing
            {"deviceId", 25}, {"property", "10"}, {"value", "23"}, {"timeout", 10000}, {"transition", true}};
        DeviceSet n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3},
            // node missing
            {"property", "10"}, {"value", "23"}, {"timeout", 10000}, {"transition", true}};
        DeviceSet n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 25},
            // actor missing
            {"value", "23"}, {"timeout", 10000}, {"transition", true}};
        DeviceSet n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 25}, {"property", "10"},
            // value missing
            {"timeout", 10000}, {"transition", true}};
        DeviceSet n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 25}, {"property", 10}, {"value", "23"},
            // timeout missing
            {"transition", true}};
        DeviceSet n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{
            {"type", 3}, {"deviceId", 25}, {"property", 10}, {"value", "23"}, {"timeout", 10000}
            // transition missing
        };
        DeviceSet n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
}

/*TEST(DeviceSetImpl, ParseDB)
{
    using namespace ::testing;
    DBHandler dbHandler{":memory:"};
    auto& db = dbHandler.GetDatabase();
    db.execute(SubActionsTable::createStatement);
    SubActionsTable subActions;
    auto selectStatement = select(subActions.actionType, subActions.actorNode, subActions.actorId, subActions.val,
        subActions.timeout, subActions.transition)
                               .from(subActions)
                               .unconditionally();

    {
        db(insert_into(subActions)
                .set(subActions.actionType = 2, subActions.actorNode = 3, subActions.actorId = 2, subActions.val = "3",
                    subActions.timeout = 1000, subActions.transition = false));
        DeviceSet n;
        n.Parse(db, db(selectStatement).front());
        EXPECT_EQ(nlohmann::json({{"type", 2}, {"deviceId", 3}, {"property", 2}, {"value", "3"}, {"timeout", 1000},
                      {"transition", false}}),
            n.ToJSON());
    }
    {
        db(update(subActions)
                .set(subActions.actionType = 1, subActions.actorNode = 2, subActions.actorId = 0,
                    subActions.transition = true)
                .unconditionally());

        DeviceSet n;
        n.Parse(db, db(selectStatement).front());
        EXPECT_EQ(nlohmann::json({{"type", 1}, {"deviceId", 2}, {"property", 0}, {"value", "3"}, {"timeout", 1000},
                      {"transition", true}}),
            n.ToJSON());
    }
}*/

// TODO: TEST(DeviceSetImpl, Execute)

TEST(DeviceToggleImpl, ToJSON)
{
    {
        nlohmann::json json{{"type", 0}, {"deviceId", 0}, {"property", ""}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, DeviceToggle().ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 0}, {"property", ""}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, DeviceToggle(3).ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 4}, {"property", "p"}, {"timeout", 0}, {"transition", true}};
		EXPECT_EQ(json, DeviceToggle::Create(3, DeviceId{ 4 }, "p", std::chrono::seconds(0), true).ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 4}, {"property", "p"},
            {"timeout", std::chrono::milliseconds(1000).count()}, {"transition", true}};
		EXPECT_EQ(json, DeviceToggle::Create(3, DeviceId{ 4 }, "p", std::chrono::seconds(1), true).ToJSON());
    }
}

TEST(DeviceToggleImpl, ParseJSON)
{
    {
        nlohmann::json json{{"type", 0}, {"deviceId", 0}, {"property", ""}, {"timeout", 0}, {"transition", false}};
        DeviceToggle n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 25}, {"property", "a"}, {"timeout", 10000}, {"transition", true}};
        DeviceToggle n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{
            {"type", 3}, {"deviceId", 25}, {"property", "a"}, {"value", "ignored"}, {"timeout", 10000}, {"transition", true}};
        DeviceToggle n;
        n.Parse(json);
    }
    {
        nlohmann::json json{// type missing
            {"deviceId", 25}, {"property", "a"}, {"timeout", 10000}, {"transition", true}};
        DeviceToggle n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3},
            // node missing
            {"property", "a"}, {"timeout", 10000}, {"transition", true}};
        DeviceToggle n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"deviceId", 25},
            // actor missing
            {"timeout", 10000}, {"transition", true}};
        DeviceToggle n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }

    {
        nlohmann::json json{{"type", 3}, {"deviceId", 25}, {"property", "b"},
            // timeout missing
            {"transition", true}};
        DeviceToggle n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{
            {"type", 3}, {"deviceId", 25}, {"property", "a"}, {"timeout", 10000}
            // transition missing
        };
        DeviceToggle n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
}

/*TEST(DeviceToggleImpl, ParseDB)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> pSt = std::make_shared<MockStatement>("test statement;");

    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(6));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt(1)).WillOnce(Return(3));
        EXPECT_CALL(*pSt, GetInt(2)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(4)).WillOnce(Return(1000));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(0));
        DeviceToggle n;
        n.Parse(DBHandler("test"), DBResult{pSt});
        EXPECT_EQ(nlohmann::json({{"type", 2}, {"deviceId", 3}, {"property", 2}, {"timeout", 1000}, {"transition", false}}),
            n.ToJSON());
    }
    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(6));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt(1)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt(2)).WillOnce(Return(0));
        EXPECT_CALL(*pSt, GetInt64(4)).WillOnce(Return(1000));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(1));
        DeviceToggle n;
        n.Parse(DBHandler("test"), DBResult{pSt});
        EXPECT_EQ(nlohmann::json({{"type", 1}, {"deviceId", 2}, {"property", 0}, {"timeout", 1000}, {"transition", true}}),
            n.ToJSON());
    }
    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(6));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt(1)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt(2)).WillOnce(Return(0));
        EXPECT_CALL(*pSt, GetInt64(4)).WillOnce(Return(1000));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(3));
        DeviceToggle n;
        n.Parse(DBHandler("test"), DBResult{pSt});
        EXPECT_EQ(nlohmann::json({{"type", 1}, {"deviceId", 2}, {"property", 0}, {"timeout", 1000}, {"transition", true}}),
            n.ToJSON());
    }
}*/

// TODO: TEST(DeviceToggleImpl, Execute)

TEST(RecursiveActionImpl, ToJSON)
{
    {
        nlohmann::json json{{"type", 0}, {"actionId", 0}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, RecursiveAction().ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"actionId", 0}, {"timeout", 0}, {"transition", false}};
        EXPECT_EQ(json, RecursiveAction(3).ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"actionId", 4}, {"timeout", 0}, {"transition", true}};
        EXPECT_EQ(json, RecursiveAction::Create(3, 4, std::chrono::seconds(0), true).ToJSON());
    }
    {
        nlohmann::json json{
            {"type", 3}, {"actionId", 4}, {"timeout", std::chrono::milliseconds(1000).count()}, {"transition", true}};
        EXPECT_EQ(json, RecursiveAction::Create(3, 4, std::chrono::seconds(1), true).ToJSON());
    }
}

TEST(RecursiveActionImpl, ParseJSON)
{
    {
        nlohmann::json json{{"type", 0}, {"actionId", 0}, {"timeout", 0}, {"transition", false}};
        RecursiveAction n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"actionId", 25}, {"timeout", 10000}, {"transition", true}};
        RecursiveAction n;
        n.Parse(json);
        EXPECT_EQ(json, n.ToJSON());
    }
    {
        nlohmann::json json{{"type", 3}, {"actionId", 25}, {"actor", "ignored"}, {"value", "ignored"}, {"timeout", 10000},
            {"transition", true}};
        RecursiveAction n;
        n.Parse(json);
    }
    {
        nlohmann::json json{// type missing
            {"actionId", 25}, {"timeout", 10000}, {"transition", true}};
        RecursiveAction n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3},
            // node missing
            {"timeout", 10000}, {"transition", true}};
        RecursiveAction n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{{"type", 3}, {"actionId", 25},
            // timeout missing
            {"transition", true}};
        RecursiveAction n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
    {
        nlohmann::json json{
            {"type", 3}, {"actionId", 25}, {"timeout", 10000}
            // transition missing
        };
        RecursiveAction n;
        EXPECT_THROW(n.Parse(json), nlohmann::json::exception);
    }
}

/*TEST(RecursiveActionImpl, ParseDB)
{
    using namespace ::testing;
    std::shared_ptr<MockStatement> pSt = std::make_shared<MockStatement>("test statement;");

    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(6));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(3));
        EXPECT_CALL(*pSt, GetInt64(4)).WillOnce(Return(1000));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(0));
        RecursiveAction n;
        n.Parse(DBHandler("test"), DBResult{pSt});
        EXPECT_EQ(nlohmann::json({{"type", 2}, {"node", 3}, {"timeout", 1000}, {"transition", false}}), n.ToJSON());
    }
    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(6));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(4)).WillOnce(Return(1000));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(1));
        RecursiveAction n;
        n.Parse(DBHandler("test"), DBResult{pSt});
        EXPECT_EQ(nlohmann::json({{"type", 1}, {"node", 2}, {"timeout", 1000}, {"transition", true}}), n.ToJSON());
    }
    {
        EXPECT_CALL(*pSt, GetColumnCount()).WillRepeatedly(Return(6));
        EXPECT_CALL(*pSt, GetInt64(0)).WillOnce(Return(1));
        EXPECT_CALL(*pSt, GetInt64(1)).WillOnce(Return(2));
        EXPECT_CALL(*pSt, GetInt64(4)).WillOnce(Return(1000));
        EXPECT_CALL(*pSt, GetInt(5)).WillOnce(Return(3));
        RecursiveAction n;
        n.Parse(DBHandler("test"), DBResult{pSt});
        EXPECT_EQ(nlohmann::json({{"type", 1}, {"node", 2}, {"timeout", 1000}, {"transition", true}}), n.ToJSON());
    }
}*/

// TODO: TEST(RecursiveActionImpl, Execute)

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>

#include "api/SubActionImpls.h"
#include "database/ActionsTable.h"
#include "database/DBActionSerialize.h"

class DBActionSerializeTest : public ::testing::Test
{
public:
    DBActionSerializeTest() : dbHandler{":memory:"}, db(dbHandler.GetDatabase()), as(dbHandler)
    {
        db.execute(ActionsTable::createStatement);
        db.execute(SubActionsTable::createStatement);

        Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
        Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
        Res::ActionRegistry().RegisterDefaultSubActions();
    }
    ~DBActionSerializeTest() { Res::ActionRegistry().RemoveAll(); }

    SubActionsTable subActions;
    ActionsTable actions;
    DBHandler dbHandler;
    DBHandler::DatabaseConnection& db;
    DBActionSerialize as;
};

TEST_F(DBActionSerializeTest, AddActionOnly)
{
    using namespace ::testing;
    using ::Action;
    UserId user{0x26346};
    // Action does not exist, insert (id 0)
    {
        Action action{0, "name", "icon", 0xFFE, {}, true};
        const uint64_t freeId = 1;
        EXPECT_EQ(freeId, as.AddActionOnly(action, user));
        action.SetId(freeId);

        Action saved = as.GetAction(freeId, user).value();
        EXPECT_EQ(saved, action) << "saved: " << saved.ToJson().dump();
        db(remove_from(actions).where(actions.actionId == freeId));
    }
    // Action does not exist, insert (id != 0)
    {
        const uint64_t actionId = 3;
        Action action{actionId, "name", "icon", 0xFFE, {}, true};
        EXPECT_EQ(actionId, as.AddActionOnly(action, user));

        Action saved = as.GetAction(actionId, user).value();
        EXPECT_EQ(saved, action) << "saved: " << saved.ToJson().dump();
        db(remove_from(actions).where(actions.actionId == actionId));
    }
    // Action exists, update
    {
        const uint64_t id = 2;
        const Action action{id, "n", "i", 0xEE, {}, false};
        db(insert_into(actions).set(actions.actionId = id, actions.actionName = "old", actions.actionIconName = "oldI",
            actions.actionColor = 0x00, actions.actionVisible = true));
        EXPECT_EQ(id, as.AddActionOnly(action, user));

        Action saved = as.GetAction(id, user).value();
        EXPECT_EQ(saved, action) << "saved: " << saved.ToJson().dump();
        db(remove_from(actions).where(actions.actionId == id));
    }
}

TEST_F(DBActionSerializeTest, AddAction)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x236};

    // Success (not existed before)
    {
        Action a{0, "a", "b", 0xFF,
            {SubAction(SubActionImpls::Notification(2)), SubAction(SubActionImpls::DeviceSet(0))}, true};
        const uint64_t actionId = 1;

        EXPECT_EQ(actionId, as.AddAction(a, user));

        a.SetId(actionId);

        Action saved = as.GetAction(actionId, user).value();
        EXPECT_EQ(saved, a) << "saved is " << saved.ToJson();
        db(remove_from(actions).where(actions.actionId == actionId));
    }
    // Success (existed before)
    {
        const uint64_t actionId = 3;
        Action a{actionId, "a", "b", 0xFE,
            {SubAction(SubActionImpls::Notification(2)), SubAction(SubActionImpls::DeviceSet(0))}};
        db(insert_into(actions).set(actions.actionId = actionId, actions.actionName = "old",
            actions.actionIconName = "oldI", actions.actionColor = 0x00, actions.actionVisible = true));
        const int64_t oldSubActionId
            = db(insert_into(subActions)
                     .set(subActions.actionId = actionId, subActions.actionType = 1, subActions.data = sqlpp::null));

        EXPECT_EQ(actionId, as.AddAction(a, user));

        Action saved = as.GetAction(actionId, user).value();
        EXPECT_EQ(saved, a) << "saved is " << saved.ToJson();
        EXPECT_TRUE(db(select(subActions.subActionId)
                           .from(subActions)
                           .where(subActions.subActionId == oldSubActionId && subActions.actionType == 4))
                        .empty());
        db(remove_from(actions).where(actions.actionId == actionId));
    }
}

TEST_F(DBActionSerializeTest, RemoveAction)
{
    using namespace ::testing;

    UserId user{0x2364};

    // Success
    {
        const uint64_t actionId = 3;
        db(insert_into(actions).set(actions.actionId = actionId, actions.actionName = "old",
            actions.actionIconName = "oldI", actions.actionColor = 0x00, actions.actionVisible = true));

        as.RemoveAction(actionId, user);
        EXPECT_TRUE(db(select(actions.actionId).from(actions).where(actions.actionId == actionId)).empty());
    }
    // Nothing to remove
    {
        as.RemoveAction(1, user);
    }
}

TEST_F(DBActionSerializeTest, GetAction)
{
    using namespace ::testing;
    using ::Action;

    UserId user{0x2364};

    // No action found
    {
        const uint64_t actionId = 4;
        absl::optional<Action> a = as.GetAction(actionId, user);
        EXPECT_FALSE(a.has_value());
    }
    // Action found
    {
        const uint64_t actionId = 4;
        const std::string name = "name";
        const std::string icon = "icon";
        const unsigned int color = 0x2634;
        const bool visible = true;

        db(insert_into(actions).set(actions.actionId = actionId, actions.actionName = name,
            actions.actionIconName = icon, actions.actionColor = color, actions.actionVisible = visible));
        absl::optional<Action> result = as.GetAction(actionId, user);
        EXPECT_TRUE(result.has_value());
        Action& action = result.value();
        EXPECT_EQ(actionId, action.GetId());
        EXPECT_EQ(name, action.GetName());
        EXPECT_EQ(icon, action.GetIcon());
        EXPECT_EQ(color, action.GetColor());
        EXPECT_EQ(visible, action.GetVisibility());
    }
}

TEST_F(DBActionSerializeTest, GetAllActions)
{
    using namespace ::testing;
    using ::Action;

    Filter filter;
    UserId user{0x64326};
    // No action found
    {
        std::vector<Action> actions = as.GetAllActions(filter, user);
        EXPECT_TRUE(actions.empty());
    }
    // Actions found
    {
        std::vector<Action> actions{{1, "name1", "icon1", 0xFFAAFF, {}, false},
            {3, "name3", "icon3", 0xAABBCCDD, {}, true}, {4, "name4", "icon4", 0x00FF00EE, {}, true}};
        for (Action& a : actions)
        {
            as.AddAction(a, user);
        }
        std::vector<Action> result = as.GetAllActions(filter, user);
        EXPECT_EQ(actions, result);
    }
}
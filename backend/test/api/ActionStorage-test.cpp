#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../mocks/MockActionSerialize.h"
#include "api/ActionStorage.h"

class ActionStorageTest : public ::testing::Test
{
public:
    ActionStorageTest() : storage(actionSer, eventEmitter) { eventEmitter.AddHandler(handler.AsStdFunction()); }

    MockActionSerialize actionSer;
    EventEmitter<Events::ActionChangeEvent> eventEmitter;
    ::testing::MockFunction<PostEventState(const Events::ActionChangeEvent&)> handler;
    ActionStorage storage;
};

TEST_F(ActionStorageTest, AddAction)
{
    using namespace ::testing;
    using Action = ::Action;
    // actionSer throws
    {
        Action action{892364, "tset", "icon", 189, {}};
        UserId user{829366};
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_CALL(actionSer, AddAction(action, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.AddAction(action, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&actionSer);
    // success
    {
        Action action{93986, "test", "icon", 125, {}};
        UserId user{892634};
        const uint64_t actionId = 2364;
        InSequence s;
        EXPECT_CALL(actionSer, AddAction(action, user)).WillOnce(Return(actionId));
        EXPECT_CALL(handler, Call(Truly([&](const Events::ActionChangeEvent& e) {
            return e.GetChanged().GetId() == actionId && e.GetChanged().GetName() == action.GetName()
                && e.GetChanged().GetIcon() == action.GetIcon() && e.GetChanged().GetColor() == action.GetColor()
                && e.GetChanged().GetVisibility() == action.GetVisibility()
                && e.GetChangedFields() == Events::ActionFields::ADD;
        })))
            .WillOnce(Return(PostEventState::handled));
        EXPECT_EQ(actionId, storage.AddAction(action, user));
    }
}

TEST_F(ActionStorageTest, UpdateAction)
{
    using namespace ::testing;
    using Action = ::Action;
    Action action{93986, "test", "icon", 125, {}};
    UserId user{82936};
    // actionSer throws
    {
        EXPECT_CALL(actionSer, AddAction(action, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_THROW(storage.UpdateAction(action, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&actionSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(actionSer, AddAction(action, user));
        EXPECT_CALL(handler, Call(Truly([&](const Events::ActionChangeEvent& e) {
            return e.GetChanged() == action && e.GetChangedFields() == Events::ActionFields::ALL;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.UpdateAction(action, user);
    }
}

TEST_F(ActionStorageTest, UpdateActionHeader)
{
    using namespace ::testing;
    using Action = ::Action;
    Action action{93986, "test", "icon", 125, {}};
    UserId user{82936};
    // actionSer throws
    {
        EXPECT_CALL(actionSer, AddActionOnly(action, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_THROW(storage.UpdateActionHeader(action, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&actionSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(actionSer, AddActionOnly(action, user));
        EXPECT_CALL(handler, Call(Truly([&](const Events::ActionChangeEvent& e) {
            return e.GetChanged() == action && e.GetChangedFields() == Events::ActionFields::NAME;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.UpdateActionHeader(action, user);
    }
}

TEST_F(ActionStorageTest, GetAction)
{
    using namespace ::testing;
    using Action = ::Action;
    const uint64_t actionId = 123589;
    Action action{actionId, "test", "icon", 125, {}};
    UserId user{82936};
    // actionSer throws
    {
        EXPECT_CALL(actionSer, GetAction(actionId, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.GetAction(actionId, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&actionSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(actionSer, GetAction(actionId, user)).WillOnce(Return(action));
        EXPECT_EQ(action, storage.GetAction(actionId, user));
    }
}

TEST_F(ActionStorageTest, GetAllActions)
{
    using namespace ::testing;
    using Action = ::Action;
    std::vector<Action> actions{{256, "test", "icon", 125, {}}, {234, "test2", "icon3", 123, {}}};
    UserId user{82936};
    // actionSer throws
    {
        EXPECT_CALL(actionSer, GetAllActions(_, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_THROW(storage.GetAllActions(Filter(), user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&actionSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(actionSer, GetAllActions(_, user)).WillOnce(Return(actions));
        EXPECT_EQ(actions, storage.GetAllActions(Filter(), user));
    }
}

TEST_F(ActionStorageTest, RemoveAction)
{
    using namespace ::testing;
    using Action = ::Action;
    const uint64_t actionId = 1643;
    UserId user{82936};
    // actionSer throws
    {
        EXPECT_CALL(actionSer, RemoveAction(actionId, user)).WillOnce(Throw(std::logic_error("test")));
        EXPECT_CALL(handler, Call(_)).Times(0);
        EXPECT_THROW(storage.RemoveAction(actionId, user), std::logic_error);
    }
    Mock::VerifyAndClearExpectations(&handler);
    Mock::VerifyAndClearExpectations(&actionSer);
    // success
    {
        InSequence s;
        EXPECT_CALL(actionSer, RemoveAction(actionId, user));
        EXPECT_CALL(handler, Call(Truly([&](const Events::ActionChangeEvent& e) {
            return e.GetOld().GetId() == actionId && e.GetChangedFields() == Events::ActionFields::REMOVE;
        })))
            .WillOnce(Return(PostEventState::handled));
        storage.RemoveAction(actionId, user);
    }
}

#include "standard-api/communication/MessageSequence.h"

#include <gtest/gtest.h>

#include "../mocks/MockNodeCommunication.h"
#include "api/Resources.h"
#include "standard-api/communication/MessageTypes.h"
#include "utility/Logger.h"

TEST(MessageSequence, DefaultConstructor)
{
    MessageSequence s;
    EXPECT_FALSE(s.HasNext());
}

TEST(MessageSequence, Constructor)
{
    {
        MessageSequence s({Message()}, 5);
        EXPECT_TRUE(s.HasNext());
        EXPECT_TRUE(s.HasTries());
        EXPECT_EQ(NodePath(), s.GetNextDestination());
    }
    {
        MessageSequence s(
            {Message(NodePath(3, 1), NodePath(), false, 0, NodeCommands::UNDEFINED), Message(), Message()});
        EXPECT_TRUE(s.HasNext());
        EXPECT_TRUE(s.HasTries());
        EXPECT_EQ(NodePath(3, 1), s.GetNextDestination());
    }
    {
        MessageSequence s(std::vector<Message>({}));
        EXPECT_FALSE(s.HasNext());
    }
    {
        MessageSequence s({Message()}, 0);
        EXPECT_TRUE(s.HasNext());
        EXPECT_FALSE(s.HasTries());
    }
}

TEST(MessageSequence, CopyConstructor)
{
    {
        MessageSequence s;
        MessageSequence s2{s};
        EXPECT_FALSE(s2.HasNext());
        MessageSequence s3 = s;
        EXPECT_FALSE(s3.HasNext());
    }
    {
        MessageSequence s({Message()});
        MessageSequence s2{s};
        EXPECT_TRUE(s2.HasNext());
        EXPECT_TRUE(s2.HasTries());
        EXPECT_EQ(NodePath(), s2.GetNextDestination());
        MessageSequence s3 = s;
        EXPECT_TRUE(s3.HasNext());
        EXPECT_TRUE(s3.HasTries());
        EXPECT_EQ(NodePath(), s3.GetNextDestination());
    }
}

TEST(MessageSequence, GetNextDestination)
{
    {
        MessageSequence s;
        EXPECT_THROW(s.GetNextDestination(), std::out_of_range);
    }
    // Other stuff already tested in constructor
}

TEST(MessageSequence, SendNext)
{
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    using namespace ::testing;

    MockNodeCommunication n{1};
    {
        MessageSequence s;
        EXPECT_CALL(n, SendMessageNow(_, _)).Times(0);
        EXPECT_TRUE(s.SendNext(n));
    }

    // msgs needs to be at least 3 long
    std::vector<::Message> msgs{Messages::AddSensorListener(NodePath(1, 1), NodePath(), 0, 15),
        Messages::Ack(NodePath(1, 1), NodePath()), Messages::Nak(NodePath(1, 1), NodePath())};
    // All successful
    {
        MessageSequence s{msgs, 5};
        for (std::size_t i = 0; i < msgs.size() - 1; ++i)
        {
            EXPECT_CALL(n, SendMessageNow(msgs[i], _)).WillOnce(Return(true));
            EXPECT_TRUE(s.SendNext(n));
            ASSERT_TRUE(s.HasNext());
        }
        EXPECT_CALL(n, SendMessageNow(msgs.back(), _)).WillOnce(Return(true));
        EXPECT_TRUE(s.SendNext(n));
        EXPECT_FALSE(s.HasNext());
        Mock::VerifyAndClearExpectations(&n);
    }
    // Fail at end, retry succeeds
    {
        MessageSequence s{msgs, 5};
        for (std::size_t i = 0; i < msgs.size() - 1; ++i)
        {
            EXPECT_CALL(n, SendMessageNow(msgs[i], _)).WillOnce(Return(true));
            EXPECT_TRUE(s.SendNext(n));
            ASSERT_TRUE(s.HasNext());
        }
        EXPECT_CALL(n, SendMessageNow(msgs.back(), _)).Times(4).WillRepeatedly(Return(false));
        for (std::size_t i = 0; i < 4; ++i)
        {
            EXPECT_FALSE(s.SendNext(n));
        }
        EXPECT_CALL(n, SendMessageNow(msgs.back(), _)).WillOnce(Return(true));
        EXPECT_TRUE(s.SendNext(n));
        EXPECT_FALSE(s.HasNext());
        Mock::VerifyAndClearExpectations(&n);
    }
    // Fail at end, all retries failed
    {
        MessageSequence s{msgs, 5};
        for (std::size_t i = 0; i < msgs.size() - 1; ++i)
        {
            EXPECT_CALL(n, SendMessageNow(msgs[i], _)).WillOnce(Return(true));
            EXPECT_TRUE(s.SendNext(n));
            ASSERT_TRUE(s.HasNext());
        }
        EXPECT_CALL(n, SendMessageNow(msgs.back(), _)).Times(5).WillRepeatedly(Return(false));
        for (std::size_t i = 0; i < 5; ++i)
        {
            EXPECT_FALSE(s.SendNext(n));
        }
        EXPECT_TRUE(s.HasNext());
        EXPECT_FALSE(s.HasTries());
        Mock::VerifyAndClearExpectations(&n);
    }
    // Fail at beginning and 2nd, success after retries
    {
        MessageSequence s{msgs, 5};
        EXPECT_CALL(n, SendMessageNow(msgs.front(), _)).Times(4).WillRepeatedly(Return(false));
        for (std::size_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(s.HasTries());
            EXPECT_FALSE(s.SendNext(n));
        }
        EXPECT_CALL(n, SendMessageNow(msgs.front(), _)).WillOnce(Return(true));
        EXPECT_TRUE(s.SendNext(n));

        // Retries reset after success
        EXPECT_CALL(n, SendMessageNow(msgs[1], _)).Times(4).WillRepeatedly(Return(false));
        for (std::size_t i = 0; i < 4; ++i)
        {
            EXPECT_FALSE(s.SendNext(n));
            EXPECT_TRUE(s.HasTries());
        }
        EXPECT_CALL(n, SendMessageNow(msgs[1], _)).WillOnce(Return(true));
        EXPECT_TRUE(s.SendNext(n));

        for (std::size_t i = 2; i < msgs.size() - 1; ++i)
        {
            EXPECT_CALL(n, SendMessageNow(msgs[i], _)).WillOnce(Return(true));
            EXPECT_TRUE(s.SendNext(n));
            ASSERT_TRUE(s.HasNext());
        }
        EXPECT_CALL(n, SendMessageNow(msgs.back(), _)).WillOnce(Return(true));
        EXPECT_TRUE(s.SendNext(n));
        EXPECT_FALSE(s.HasNext());
        Mock::VerifyAndClearExpectations(&n);
    }
    // Fail at beginning, fail after retries
    {
        MessageSequence s{msgs, 5};
        EXPECT_CALL(n, SendMessageNow(msgs.front(), _)).Times(5).WillRepeatedly(Return(false));
        for (std::size_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(s.HasTries());
            EXPECT_FALSE(s.SendNext(n));
        }
        EXPECT_FALSE(s.SendNext(n));
        EXPECT_TRUE(s.HasNext());
        EXPECT_FALSE(s.HasTries());
        Mock::VerifyAndClearExpectations(&n);
    }
}
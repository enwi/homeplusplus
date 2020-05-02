#include "standard-api/communication/Message.h"

#include <gtest/gtest.h>

TEST(Message, DefaultConstructor)
{
    Message m{};
    EXPECT_EQ(NodePath(), m.GetDestination());
    EXPECT_EQ(NodePath(), m.GetSender());
    EXPECT_FALSE(m.GetAck());
    EXPECT_EQ(0, m.GetPayload());
    EXPECT_EQ(NodeCommands::UNDEFINED, m.m_command);
}

TEST(Message, Constructor)
{
    {
        Message m(NodePath(0, 1), NodePath(1, 3), true, 4, NodeCommands::SYNC_DATA);
        EXPECT_EQ(NodePath(0, 1), m.GetDestination());
        EXPECT_EQ(NodePath(1, 3), m.GetSender());
        EXPECT_TRUE(m.GetAck());
        EXPECT_EQ(4, m.GetPayload());
        EXPECT_EQ(NodeCommands::SYNC_DATA, m.m_command);
    }
    {
        Message m{NodePath(3, 2), NodePath(4, 3), true, 0, NodeCommands::ACK};
        EXPECT_EQ(NodePath(3, 2), m.GetDestination());
        EXPECT_EQ(NodePath(4, 3), m.GetSender());
        EXPECT_TRUE(m.GetAck());
        EXPECT_EQ(0, m.GetPayload());
        EXPECT_EQ(NodeCommands::ACK, m.m_command);
    }
}

TEST(Message, Destination)
{
    Message m;
    m.SetDestination(NodePath(34, 5));
    EXPECT_EQ(NodePath(34, 5), m.GetDestination());
    m.SetDestination(NodePath(4, 2));
    EXPECT_EQ(NodePath(4, 2), m.GetDestination());
}

TEST(Message, Sender)
{
    Message m;
    m.SetSender(NodePath(34, 5));
    EXPECT_EQ(NodePath(34, 5), m.GetSender());
    m.SetSender(NodePath(4, 2));
    EXPECT_EQ(NodePath(4, 2), m.GetSender());
}

TEST(Message, Ack)
{
    Message m;
    m.SetAck(true);
    EXPECT_TRUE(m.GetAck());
    m.SetAck(false);
    EXPECT_FALSE(m.GetAck());
}

TEST(Message, Payload)
{
    Message m;
    m.SetPayload(3);
    EXPECT_EQ(3, m.GetPayload());
    m.SetPayload(0);
    EXPECT_EQ(0, m.GetPayload());
}

TEST(Message, SetData)
{
    Message m;
    m.SetData(0);
    for (const uint8_t& b : m.m_data)
    {
        EXPECT_EQ(0, b);
    }
    m.SetData(23);
    for (const uint8_t& b : m.m_data)
    {
        EXPECT_EQ(23, b);
    }

    uint8_t data[] = {0xFF, 0x02, 0x04, 0x53, 0x00};
    EXPECT_TRUE(m.SetData(data, sizeof(data)));
    for (uint8_t i = 0; i < sizeof(data); ++i)
    {
        EXPECT_EQ(data[i], m.m_data[i]);
    }

    uint8_t tooLongData[255]{};
    EXPECT_FALSE(m.SetData(tooLongData, sizeof(tooLongData)));
    // Verify that nothing was changed
    for (uint8_t i = 0; i < sizeof(data); ++i)
    {
        EXPECT_EQ(data[i], m.m_data[i]);
    }
}

TEST(Message, GetSize)
{
    Message m;
    // Prevent ODR-usage
    uint8_t header = Message::headerSize;
    EXPECT_EQ(header, m.GetSize());
    m.SetPayload(5);
    EXPECT_EQ(Message::headerSize + 5, m.GetSize());
    m.SetPayload(Message::maxPayload);
    // Prevent ODR-usage
    uint8_t len = Message::maxLength;
    EXPECT_EQ(len, m.GetSize());
}

TEST(Message, BVal)
{
    Message m;
    m.SetBVal(0, 2);
    EXPECT_EQ(2, m.GetBVal(0));
    // Check that payload was increased
    EXPECT_EQ(1, m.GetPayload());

    EXPECT_THROW(m.SetBVal(Message::maxPayload, 0xFF), Message::ArgumentException);
    EXPECT_THROW(m.GetBVal(Message::maxPayload), Message::ArgumentException);
}

TEST(Message, ULVal)
{
    Message m;
    m.SetULVal(0, 0xFF00FF00);
    EXPECT_EQ(0xFF00FF00, m.GetULVal(0));
    // Check that payload was increased
    EXPECT_EQ(4, m.GetPayload());

    EXPECT_THROW(m.SetULVal(Message::maxPayload - 3, 0xFF), Message::ArgumentException);
    EXPECT_THROW(m.GetULVal(Message::maxPayload - 3), Message::ArgumentException);
}

TEST(Message, LVal)
{
    Message m;
    m.SetLVal(0, -21526781);
    EXPECT_EQ(-21526781, m.GetLVal(0));
    // Check that payload was increased
    EXPECT_EQ(4, m.GetPayload());

    EXPECT_THROW(m.SetLVal(Message::maxPayload - 3, 0xFF), Message::ArgumentException);
    EXPECT_THROW(m.GetLVal(Message::maxPayload - 3), Message::ArgumentException);
}

TEST(Message, UIVal)
{
    Message m;
    m.SetUIVal(0, 0xF0F0);
    EXPECT_EQ(0xF0F0, m.GetUIVal(0));
    // Check that payload was increased
    EXPECT_EQ(2, m.GetPayload());

    EXPECT_THROW(m.SetUIVal(Message::maxPayload - 1, 0xFF), Message::ArgumentException);
    EXPECT_THROW(m.GetUIVal(Message::maxPayload - 1), Message::ArgumentException);
}

TEST(Message, IVal)
{
    Message m;
    m.SetIVal(0, -6335);
    EXPECT_EQ(-6335, m.GetIVal(0));
    // Check that payload was increased
    EXPECT_EQ(2, m.GetPayload());

    EXPECT_THROW(m.SetIVal(Message::maxPayload - 1, 0xFF), Message::ArgumentException);
    EXPECT_THROW(m.GetIVal(Message::maxPayload - 1), Message::ArgumentException);
}

TEST(Message, Str)
{
    Message m;
    m.SetStr(0, "test string");
    EXPECT_EQ("test string", m.GetStr(0));
    // Check that payload was increased
    EXPECT_EQ(12, m.GetPayload());

    std::string s = "this is a very long string which unfortunately does not fit within a message";
    s.resize(Message::maxPayload + 1, 'E');
    EXPECT_THROW(m.SetStr(0, s), Message::ArgumentException);
    EXPECT_THROW(m.GetStr(Message::maxPayload), Message::ArgumentException);
}
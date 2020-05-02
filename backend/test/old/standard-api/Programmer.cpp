#include "standard-api/Programmer.h"

#include <gtest/gtest.h>

#include "../mocks/MockNodeCommunication.h"
#include "standard-api/communication/MessageTypes.h"

TEST(Programmer, GetLineMsg)
{
    MockNodeCommunication nc{2};
    Programmer p{nc};
    // Hex file line format: ":[size/2][address/4][type/2][data/2*size][checksum/2](\n)"
    // Empty line
    {
        std::string s;
        std::string::const_iterator begin = s.begin();
        NodePath destination{1, 2, 1};
        Message expected = Messages::Nak(destination, NodePath());

        EXPECT_EQ(expected, p.GetLineMsg(begin, s.end(), destination));
        EXPECT_EQ(s.end(), begin);
    }
    // Line does not start with ':'
    {
        std::string s = "AAEF0023";
        std::string::const_iterator begin = s.begin();
        NodePath destination{1, 2, 1};

        EXPECT_THROW(p.GetLineMsg(begin, s.end(), destination), std::invalid_argument);
    }
    // Line too short
    {
        std::string s = ":00"
                        "0000"
                        "00"
                        ""
                        "0";
        std::string::const_iterator begin = s.begin();
        NodePath destination{1, 2, 1};

        EXPECT_THROW(p.GetLineMsg(begin, s.end(), destination), std::invalid_argument);
    }
    // Line too short with data
    {
        std::string s = ":08"
                        "0000"
                        "00"
                        "01234"
                        "00";
        std::string::const_iterator begin = s.begin();
        NodePath destination{1, 2, 1};

        EXPECT_THROW(p.GetLineMsg(begin, s.end(), destination), std::invalid_argument);
    }
    // Line without data (with nl)
    {
        std::string s = ":00"
                        "FFEF"
                        "10"
                        ""
                        "01\n";
        std::string::const_iterator begin = s.begin();
        NodePath destination{1, 2, 1};
        Message expected = Messages::ProgramData(destination, NodePath(), 0, 0xFFEF, 0x10, nullptr, 1);

        EXPECT_EQ(expected, p.GetLineMsg(begin, s.end(), destination));
        EXPECT_EQ(s.end(), begin);
    }
    // Line without data (without nl)
    {
        std::string s = ":00"
                        "FFEF"
                        "10"
                        ""
                        "01";
        std::string::const_iterator begin = s.begin();
        NodePath destination{1, 2, 1};
        Message expected = Messages::ProgramData(destination, NodePath(), 0, 0xFFEF, 0x10, nullptr, 1);

        EXPECT_EQ(expected, p.GetLineMsg(begin, s.end(), destination));
        EXPECT_EQ(s.end(), begin);
    }
    // Line with data (no nl)
    {
        std::string s = ":08"
                        "FFEF"
                        "10"
                        "0011223344556677"
                        "01";
        std::string::const_iterator begin = s.begin();
        std::vector<uint8_t> data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
        NodePath destination{1, 2, 1};
        Message expected = Messages::ProgramData(destination, NodePath(), 0x08, 0xFFEF, 0x10, data.data(), 1);

        EXPECT_EQ(expected, p.GetLineMsg(begin, s.end(), destination));
        EXPECT_EQ(s.end(), begin);
    }
    // Line with data (nl)
    {
        std::string s = ":08"
                        "FFEF"
                        "10"
                        "0011223344556677"
                        "01\n";
        std::string::const_iterator begin = s.begin();
        std::vector<uint8_t> data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
        NodePath destination{1, 2, 1};
        Message expected = Messages::ProgramData(destination, NodePath(), 0x08, 0xFFEF, 0x10, data.data(), 1);

        EXPECT_EQ(expected, p.GetLineMsg(begin, s.end(), destination));
        EXPECT_EQ(s.end(), begin);
    }
}

TEST(Programmer, Program)
{
    using namespace ::testing;
    MockNodeCommunication nc{2};
    Programmer p{nc};

    // Empty file
    {
        NodePath destination = {2, 1};
        MessageSequence s{{Messages::StartProgramming(destination, NodePath())}};

        EXPECT_CALL(nc, SendMessageSequence(s));
        p.Program("", destination);

        Mock::VerifyAndClearExpectations(&nc);
    }
    // One line
    {
        NodePath destination = {2, 1};
        MessageSequence s{{Messages::StartProgramming(destination, NodePath()),
            Messages::ProgramData(destination, NodePath(), 0, 0xFFEF, 0x10, nullptr, 1)}};

        EXPECT_CALL(nc, SendMessageSequence(s));
        p.Program(":00"
                  "FFEF"
                  "10"
                  ""
                  "01",
            destination);

        Mock::VerifyAndClearExpectations(&nc);
    }
    // Error
    {
        NodePath destination = {2, 1};
        EXPECT_THROW(p.Program(":00"
                               "FFEF",
                         destination),
            std::logic_error);

        Mock::VerifyAndClearExpectations(&nc);
    }
    // Multiple lines
    {
        NodePath destination = {2, 1};
        std::vector<uint8_t> data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

        MessageSequence s{{Messages::StartProgramming(destination, NodePath()),
            Messages::ProgramData(destination, NodePath(), 0, 0xFFEF, 0x10, nullptr, 1),
            Messages::ProgramData(destination, NodePath(), 0x08, 0xFFEF, 0x10, data.data(), 0x50),
            Messages::ProgramData(destination, NodePath(), 0x08, 0xFFEF, 0x04, data.data(), 1)}};

        EXPECT_CALL(nc, SendMessageSequence(s));
        p.Program(":00"
                  "FFEF"
                  "10"
                  ""
                  "01\n"
                  ":08"
                  "FFEF"
                  "10"
                  "0011223344556677"
                  "50\n"
                  ":08"
                  "FFEF"
                  "04"
                  "0011223344556677"
                  "01",
            destination);

        Mock::VerifyAndClearExpectations(&nc);
    }
}
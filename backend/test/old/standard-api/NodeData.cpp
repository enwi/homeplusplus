#include "standard-api/NodeData.h"

#include <gtest/gtest.h>

#include "NodeSerializeUtils.h"

#include "../mocks/MockDBHandler.h"
#include "../mocks/MockNodeCommunication.h"
#include "../mocks/MockNodeManager.h"
#include "api/Resources.h"
#include "database/DBNodeSerialize.h"
#include "events/EventSystem.h"
#include "events/Events.h"
#include "standard-api/communication/MessageTypes.h"

TEST(StateConversion, StateToString)
{
    // Empty state
    {
        const std::vector<uint8_t> state;
        EXPECT_EQ("", StateToString(state.begin(), state.end()));
    }
    // Short state (1 byte)
    {
        const uint8_t value = 32;
        const std::vector<uint8_t> state{32};
        EXPECT_EQ(std::to_string(value), StateToString(state.begin(), state.end()));
    }
    // Short state (2 bytes)
    {
        const int32_t value = 0x50FA;
        const std::vector<uint8_t> state{value & 0xFF, (value >> 8) & 0xFF};
        EXPECT_EQ(std::to_string(value), StateToString(state.begin(), state.end()));
    }
    // Short state (3 bytes)
    {
        const int32_t value = 0x50FA03;
        const std::vector<uint8_t> state{value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF};
        EXPECT_EQ(std::to_string(value), StateToString(state.begin(), state.end()));
    }
    // Short state (4 bytes)
    {
        const int32_t value = 0x4050FA03;
        const std::vector<uint8_t> state{value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF};
        EXPECT_EQ(std::to_string(value), StateToString(state.begin(), state.end()));
    }
    // Short state (4 bytes, with leading zeroes)
    {
        const int32_t value = 0x0000FA03;
        const std::vector<uint8_t> state{value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF};
        EXPECT_EQ(std::to_string(value), StateToString(state.begin(), state.end()));
    }
    // Long state (5 bytes)
    {
        const std::vector<uint8_t> state{0x00, 0xFE, 0x32, 0x44, 0x00};
        EXPECT_EQ("0x00fe324400", StateToString(state.begin(), state.end()));
    }
    // Long state (10 bytes)
    {
        const std::vector<uint8_t> state{0x00, 0xFE, 0x32, 0x44, 0x00, 0x64, 0x14, 0x42, 0x78, 0xFF};
        EXPECT_EQ("0x00fe32440064144278ff", StateToString(state.begin(), state.end()));
    }
}

TEST(StateConversion, StringToState)
{
    // Empty string
    {
        EXPECT_EQ(std::vector<uint8_t>(), StringToState(""));
    }
    // Decimal string
    {
        const int32_t val = 25644754;
        EXPECT_EQ(std::vector<uint8_t>({val & 0xFF, (val >> 8) & 0xFF, (val >> 16) & 0xFF, (val >> 24) & 0xFF}),
            StringToState(std::to_string(val)));
    }
    // Negative decimal string
    {
        const int32_t val = -2564;
        EXPECT_EQ(std::vector<uint8_t>({val & 0xFF, (val >> 8) & 0xFF, (val >> 16) & 0xFF, (val >> 24) & 0xFF}),
            StringToState(std::to_string(val)));
    }
    // Conversion fail
    {
        EXPECT_THROW(StringToState("hhE23"), std::invalid_argument);
    }
    // Hex string (short)
    {
        EXPECT_EQ(std::vector<uint8_t>({0x00, 0x3E}), StringToState("0x003E"));
    }
    // Hex string (long)
    {
        EXPECT_EQ(std::vector<uint8_t>({0x00, 0x3E, 0x53, 0x45, 0x23, 0x42, 0x3E, 0x00}),
            StringToState("0x003E534523423E00"));
    }
    // Odd size in hex string
    {
        EXPECT_THROW(StringToState("0x3"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x345"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x34560"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x03456"), std::invalid_argument);
    }
    // Hex conversion error
    {
        EXPECT_THROW(StringToState("0x3X"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x345X"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x3H56O"), std::invalid_argument);
        EXPECT_THROW(StringToState("0xHi"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x 3"), std::invalid_argument);
        EXPECT_THROW(StringToState("0x30 2 5 6"), std::invalid_argument);
    }
}

TEST(NodeSensor, DefaultConstructor)
{
    {
        const Sensor s;
        EXPECT_EQ("", s.GetName());
        EXPECT_EQ("", s.GetLocation());
        EXPECT_EQ("", s.GetState());
        EXPECT_EQ(0, s.m_interval);
        EXPECT_EQ(0, s.m_pin);
        EXPECT_EQ(0, s.m_type);
        EXPECT_FALSE(s.IsDeleted());
    }
    {
        const Sensor s{};
        EXPECT_EQ("", s.GetName());
        EXPECT_EQ("", s.GetLocation());
        EXPECT_EQ("", s.GetState());
        EXPECT_EQ(0, s.m_interval);
        EXPECT_EQ(0, s.m_pin);
        EXPECT_EQ(0, s.m_type);
        EXPECT_FALSE(s.IsDeleted());
    }
}

TEST(NodeSensor, Constructor)
{
    {
        const Sensor s{0, 0, "abc", "def", 10, 3, 4};
        EXPECT_EQ("abc", s.GetName());
        EXPECT_EQ("def", s.GetLocation());
        EXPECT_EQ("", s.GetState());
        EXPECT_EQ(4, s.m_interval);
        EXPECT_EQ(3, s.m_pin);
        EXPECT_EQ(10, s.m_type);
        EXPECT_FALSE(s.IsDeleted());
    }
    {
        const Sensor s(19, 3, "abc", "", 0, 2, 3);
        EXPECT_EQ("abc", s.GetName());
        EXPECT_EQ("", s.GetLocation());
        EXPECT_EQ("", s.GetState());
        EXPECT_EQ(3, s.m_interval);
        EXPECT_EQ(2, s.m_pin);
        EXPECT_EQ(0, s.m_type);
        EXPECT_FALSE(s.IsDeleted());
    }
}

TEST(NodeSensor, Equality)
{
    // Check for equals on const objects
    const Sensor d;
    EXPECT_EQ(d, Sensor(0, 0, "", "", 0, 0, 0));
    EXPECT_NE(d, Sensor(10, 0, "", "", 0, 0, 0));
    EXPECT_NE(d, Sensor(0, 10, "", "", 0, 0, 0));
    EXPECT_NE(d, Sensor(0, 0, "a", "", 0, 0, 0));
    EXPECT_NE(d, Sensor(0, 0, "", "b", 0, 0, 0));
    EXPECT_NE(d, Sensor(0, 0, "", "", 3, 0, 0));
    EXPECT_NE(d, Sensor(0, 0, "", "", 0, 4, 0));
    EXPECT_NE(d, Sensor(0, 0, "", "", 0, 0, 1));

    EXPECT_EQ(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 31, "1", "2", 3, 2, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(0, 31, "1", "2", 3, 2, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 0, "1", "2", 3, 2, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 31, "123", "2", 3, 2, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 31, "1", "23as", 3, 2, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 31, "1", "2", 33, 2, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 31, "1", "2", 3, 0, 3));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 31, "1", "2", 3, 2, 0));
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), d);
    EXPECT_NE(Sensor(10, 31, "1", "2", 3, 2, 3), Sensor(10, 0, "", "", 3, 2, 3));
}

TEST(NodeSensor, Parse)
{
    EXPECT_EQ(Sensor(4, 3, "name", "loc", 1, 3, 2),
        Sensor::Parse(
            {{"name", "name"}, {"location", "loc"}, {"state", ""}, {"type", 1}, {"pin", 3}, {"listener", 2}}, 4, 3));
    EXPECT_EQ(Sensor(0, 0, "", "", 0, 0, 1),
        Sensor::Parse({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}, {"listener", 1},
                          {"excessfield", "ignored"}},
            0, 0));
    EXPECT_THROW(Sensor::Parse("wrong json type", 0, 0), std::domain_error);
    EXPECT_THROW(Sensor::Parse({{"missing", "fields"}}, 0, 0), std::out_of_range);
    EXPECT_THROW(
        Sensor::Parse(
            {{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}, {"missing", "listener"}}, 0, 0),
        std::out_of_range);
    EXPECT_THROW(
        Sensor::Parse(
            {{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}, {"listener", "no int"}}, 0, 0),
        std::domain_error);
}

TEST(NodeSensor, ToJson)
{
    EXPECT_EQ(nlohmann::json(
                  {{"name", "name"}, {"location", "loc"}, {"state", ""}, {"type", 1}, {"pin", 3}, {"listener", 2}}),
        Sensor(4, 3, "name", "loc", 1, 3, 2).ToJson());
    EXPECT_EQ(nlohmann::json({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}, {"listener", 1}}),
        Sensor(0, 0, "", "", 0, 0, 1).ToJson());
    EXPECT_NE(nlohmann::json({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}, {"listener", 1}}),
        Sensor(0, 0, "abc", "", 0, 0, 1).ToJson());

    // Parse results in same sensor
    {
        const Sensor s(1, 21, "n", "l", 5, 1, 3);
        EXPECT_EQ(s, Sensor::Parse(s.ToJson(), 1, 21));
    }
    {
        const Sensor s(0, 0, "ad", "se", 1, 4, 2);
        EXPECT_EQ(s, Sensor::Parse(s.ToJson(), 0, 0));
    }
}

TEST(NodeSensor, Deleted)
{
    EXPECT_EQ(Sensor::Deleted(), Sensor::Deleted());
    EXPECT_TRUE(Sensor::Deleted().IsDeleted());
    EXPECT_EQ(Sensor(0, 0, "DELETED", "DELETED", 0, 0, 0), Sensor::Deleted());
    EXPECT_EQ(Sensor::Deleted(), Sensor::Parse(Sensor::Deleted().ToJson(), 0, 0));
    // Equality fixes also comparing nodeId and sensor broke all Sensor == Deleted checks!
}

TEST(NodeSensor, Get)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    NodeManager mOld = Res::NodeManager();

    try
    {
        MockDBHandler dbHandler;
        dbHandler.UseDefaults();
        MockNodeCommunication nc{1};
        DBNodeSerialize ns{dbHandler};
        NodeManager mocked{dbHandler, nc, ns};
        Res::NodeManager() = mocked;

        // TODO: Use MockNodeManager instead of copy-pasting test code from NodeManager

        const uint16_t nodeId = 1;
        const uint8_t sensorId = 2;
        Sensor s{nodeId, sensorId, "n", "l", 1, 2, 3};

        // Node not found
        {
            EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());

            EXPECT_EQ("", s.Get());
            Mock::VerifyAndClearExpectations(&dbHandler);
        }
        // Sensor not found
        {
            NodeData n{nodeId, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
            ExpectGetNode(dbHandler, n);
            EXPECT_EQ("", s.Get());
            Mock::VerifyAndClearExpectations(&dbHandler);
        }
        // Sensor found
        {
            // TODO: Implement some way of testing whether the correct value is returned
        }
    }
    catch (...)
    {
        Res::NodeManager() = mOld;
        throw;
    }
    Res::NodeManager() = mOld;
}

TEST(NodeActor, DefaultConstructor)
{
    {
        const Actor a;
        EXPECT_EQ("", a.GetName());
        EXPECT_EQ("", a.GetLocation());
        EXPECT_EQ("", a.GetState());
        EXPECT_EQ(0, a.m_pin);
        EXPECT_EQ(0, a.m_type);
        EXPECT_FALSE(a.IsDeleted());
    }
    {
        const Actor a{};
        EXPECT_EQ("", a.GetName());
        EXPECT_EQ("", a.GetLocation());
        EXPECT_EQ("", a.GetState());
        EXPECT_EQ(0, a.m_pin);
        EXPECT_EQ(0, a.m_type);
        EXPECT_FALSE(a.IsDeleted());
    }
}

TEST(NodeActor, Constructor)
{
    {
        const Actor a{0, 0, "abc", "def", 10, 3};
        EXPECT_EQ("abc", a.GetName());
        EXPECT_EQ("def", a.GetLocation());
        EXPECT_EQ("", a.GetState());
        EXPECT_EQ(3, a.m_pin);
        EXPECT_EQ(10, a.m_type);
        EXPECT_FALSE(a.IsDeleted());
    }
    {
        const Actor a(19, 3, "abc", "", 0, 2);
        EXPECT_EQ("abc", a.GetName());
        EXPECT_EQ("", a.GetLocation());
        EXPECT_EQ("", a.GetState());
        EXPECT_EQ(2, a.m_pin);
        EXPECT_EQ(0, a.m_type);
        EXPECT_FALSE(a.IsDeleted());
    }
}

TEST(NodeActor, Equality)
{
    // Check for equals on const objects
    const Actor d;
    EXPECT_EQ(d, Actor(0, 0, "", "", 0, 0));
    EXPECT_NE(d, Actor(10, 0, "", "", 0, 0));
    EXPECT_NE(d, Actor(0, 10, "", "", 0, 0));
    EXPECT_NE(d, Actor(0, 0, "a", "", 0, 0));
    EXPECT_NE(d, Actor(0, 0, "", "b", 0, 0));
    EXPECT_NE(d, Actor(0, 0, "", "", 3, 0));
    EXPECT_NE(d, Actor(0, 0, "", "", 0, 4));

    EXPECT_EQ(Actor(10, 31, "1", "2", 3, 2), Actor(10, 31, "1", "2", 3, 2));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(0, 31, "1", "2", 3, 2));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(10, 0, "1", "2", 3, 2));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(10, 31, "123", "2", 3, 2));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(10, 31, "1", "23as", 3, 2));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(10, 31, "1", "2", 33, 2));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(10, 31, "1", "2", 3, 0));
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), d);
    EXPECT_NE(Actor(10, 31, "1", "2", 3, 2), Actor(10, 0, "", "", 3, 2));
}

TEST(NodeActor, Parse)
{
    EXPECT_EQ(Actor(4, 3, "name", "loc", 1, 3),
        Actor::Parse({{"name", "name"}, {"location", "loc"}, {"state", ""}, {"type", 1}, {"pin", 3}}, 4, 3));
    EXPECT_EQ(Actor(0, 0, "", "", 0, 0),
        Actor::Parse(
            {{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}, {"excessfield", "ignored"}}, 0,
            0));
    EXPECT_THROW(Actor::Parse("wrong json type", 0, 0), std::domain_error);
    EXPECT_THROW(Actor::Parse({{"missing", "fields"}}, 0, 0), std::out_of_range);
    EXPECT_THROW(Actor::Parse({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"missing", "pin"}}, 0, 0),
        std::out_of_range);
    EXPECT_THROW(Actor::Parse({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", "no int"}}, 0, 0),
        std::domain_error);
}

TEST(NodeActor, ToJson)
{
    EXPECT_EQ(nlohmann::json({{"name", "name"}, {"location", "loc"}, {"state", ""}, {"type", 1}, {"pin", 3}}),
        Actor(4, 3, "name", "loc", 1, 3).ToJson());
    EXPECT_EQ(nlohmann::json({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}}),
        Actor(0, 0, "", "", 0, 0).ToJson());
    EXPECT_NE(nlohmann::json({{"name", ""}, {"location", ""}, {"state", ""}, {"type", 0}, {"pin", 0}}),
        Actor(0, 0, "abc", "", 0, 0).ToJson());

    // Parse results in same sensor
    {
        const Actor a(1, 21, "n", "l", 5, 1);
        EXPECT_EQ(a, Actor::Parse(a.ToJson(), 1, 21));
    }
    {
        const Actor a(0, 0, "ad", "se", 1, 4);
        EXPECT_EQ(a, Actor::Parse(a.ToJson(), 0, 0));
    }
}

TEST(NodeActor, Deleted)
{
    EXPECT_EQ(Actor::Deleted(), Actor::Deleted());
    EXPECT_TRUE(Actor::Deleted().IsDeleted());
    EXPECT_EQ(Actor(0, 0, "DELETED", "DELETED", 0, 0), Actor::Deleted());
    EXPECT_EQ(Actor::Deleted(), Actor::Parse(Actor::Deleted().ToJson(), 0, 0));
    // Equality fixes also comparing nodeId and actorId broke all Actor == Deleted checks!
}

TEST(NodeActor, Set)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    NodeManager mOld = Res::NodeManager();

    try
    {
        MockDBHandler dbHandler;
        dbHandler.UseDefaults();
        MockNodeCommunication nc{1};
        DBNodeSerialize ns{dbHandler};
        NodeManager mocked{dbHandler, nc, ns};
        Res::NodeManager() = mocked;

        // TODO: Use MockNodeManager instead of copy-pasting test code from NodeManager

        const uint16_t nodeId = 1;
        const uint8_t actorId = 1;
        Actor a{nodeId, actorId, "n", "l", 1, 2};

        // Node not found
        {
            EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
            EXPECT_THROW(a.Set("0"), std::logic_error);
            Mock::VerifyAndClearExpectations(&dbHandler);
        }
        // Actor not found
        {
            NodeData n{nodeId, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
            ExpectGetNode(dbHandler, n);
            EXPECT_THROW(a.Set("0"), std::logic_error);
            Mock::VerifyAndClearExpectations(&dbHandler);
        }
        // Actor found
        {
            NodeData n{nodeId, "name", "location", {}, {{}, a}, "state", NodePath(1, 1), 1};
            ExpectGetNode(dbHandler, n);
            const std::vector<uint8_t> value{3, 0, 0, 0};
            EXPECT_CALL(nc, QueueMessage(Messages::SetActorValue(n.m_path, NodePath(), actorId, value), 5, 0));
            a.Set(StateToString(value.begin(), value.end()));
            Mock::VerifyAndClearExpectations(&dbHandler);
        }
        // Not convertible to int
        {
            EXPECT_THROW(a.Set("abcd"), std::logic_error);
        }
    }
    catch (...)
    {
        Res::NodeManager() = mOld;
        throw;
    }
    Res::NodeManager() = mOld;
}

TEST(NodeData, DefaultConstructor)
{
    const NodeData n;
    EXPECT_EQ(0, n.GetId());
    EXPECT_EQ("Undefined", n.GetName());
    EXPECT_EQ("Undefined", n.GetLocation());
    EXPECT_EQ("No state", n.GetState());
    EXPECT_EQ(1, n.m_type);
    EXPECT_EQ(NodePath(), n.m_path);
    EXPECT_FALSE(n.IsEmpty());
    EXPECT_TRUE(n.m_actors.empty());
    EXPECT_TRUE(n.m_sensors.empty());
}

TEST(NodeData, Constructor)
{
    {
        std::vector<Sensor> s{{}, Sensor::Deleted()};
        std::vector<Actor> a{Actor::Deleted(), {}};
        const NodeData n(3, "name", "location", s, a, "state", NodePath(5, 3), 2);
        EXPECT_EQ(3, n.GetId());
        EXPECT_EQ("name", n.GetName());
        EXPECT_EQ("location", n.GetLocation());
        EXPECT_EQ("state", n.GetState());
        EXPECT_EQ(2, n.m_type);
        EXPECT_EQ(NodePath(5, 3), n.m_path);
        EXPECT_FALSE(n.IsEmpty());
        EXPECT_EQ(a, n.m_actors);
        EXPECT_EQ(s, n.m_sensors);
    }
    {
        std::vector<Sensor> s{{}, Sensor::Deleted(), {}};
        std::vector<Actor> a{Actor::Deleted(), {}};
        const NodeData n{3, "a", "b", s, a, "c", NodePath(3, 2)};
        EXPECT_EQ(3, n.GetId());
        EXPECT_EQ("a", n.GetName());
        EXPECT_EQ("b", n.GetLocation());
        EXPECT_EQ("c", n.GetState());
        EXPECT_EQ(1, n.m_type);
        EXPECT_EQ(NodePath(3, 2), n.m_path);
        EXPECT_FALSE(n.IsEmpty());
        EXPECT_EQ(a, n.m_actors);
        EXPECT_EQ(s, n.m_sensors);
    }
}

TEST(NodeData, Equality)
{
    const NodeData d;
    EXPECT_EQ(d, NodeData(d));
    EXPECT_EQ(d, NodeData(0, "Undefined", "Undefined", {}, {}, "No state", NodePath()));
    EXPECT_NE(d, NodeData(1, "Undefined", "Undefined", {}, {}, "No state", NodePath()));
    EXPECT_NE(d, NodeData(0, "a", "Undefined", {}, {}, "No state", NodePath()));
    EXPECT_NE(d, NodeData(0, "Undefined", "a", {}, {}, "No state", NodePath()));
    EXPECT_NE(d, NodeData(0, "Undefined", "Undefined", {Sensor{}}, {}, "No state", NodePath()));
    EXPECT_NE(d, NodeData(0, "Undefined", "Undefined", {}, {Actor{}}, "No state", NodePath()));
    EXPECT_NE(d, NodeData(0, "Undefined", "Undefined", {}, {}, "a", NodePath()));
    EXPECT_NE(d, NodeData(0, "Undefined", "Undefined", {}, {}, "No state", NodePath(1, 1)));

    const NodeData n(43, "asd", "def", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 2), 2);
    EXPECT_EQ(n, NodeData(43, "asd", "def", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 2), 2));
    EXPECT_NE(n, NodeData(42, "asd", "def", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 2), 2));
    EXPECT_NE(n, NodeData(43, "asdf", "def", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 2), 2));
    EXPECT_NE(n, NodeData(43, "asd", "dedf", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 2), 2));
    // Only sizes of actors and sensors are checked
    EXPECT_NE(n, NodeData(43, "asd", "def", {{}}, {Actor{}}, "ghi", NodePath(3, 2), 2));
    EXPECT_NE(n, NodeData(43, "asd", "def", {{}, {}}, {}, "ghi", NodePath(3, 2), 2));
    EXPECT_NE(n, NodeData(43, "asd", "def", {Sensor::Deleted(), {}}, {Actor{}}, "", NodePath(3, 2), 2));
    EXPECT_NE(n, NodeData(43, "asd", "def", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 3), 2));
    EXPECT_NE(n, NodeData(43, "asd", "def", {Sensor::Deleted(), {}}, {Actor{}}, "ghi", NodePath(3, 2), 1));
}

TEST(NodeData, LessGreater)
{
    const NodeData d{1, "", "", {}, {}, "", NodePath()};
    EXPECT_LT(d, NodeData(2, "", "", {}, {}, "", NodePath()));
    EXPECT_GT(NodeData(2, "", "", {}, {}, "", NodePath()), d);

    EXPECT_LT(d, NodeData(3, "a", "b", {Sensor()}, {Actor()}, "state", NodePath(3, 2)));
    EXPECT_GT(NodeData(3, "a", "b", {Sensor()}, {Actor()}, "state", NodePath(3, 2)), d);
}

TEST(NodeData, Empty)
{
    EXPECT_FALSE(NodeData().IsEmpty());
    EXPECT_TRUE(NodeData::EmptyNode().IsEmpty());
    EXPECT_FALSE(NodeData(1, "name", "loc", {}, {}, "", NodePath()).IsEmpty());
    EXPECT_EQ(NodeData::EmptyNode(),
        NodeData(0, "THIS_NODE_IS_EMPTY", "THIS_NODE_IS_EMPTY", {}, {}, "THIS_NODE_IS_EMPTY", NodePath(), 0));
}

TEST(NodeData, GetSensorsActors)
{
    {
        const NodeData d{1, "", "", {}, {}, "", NodePath()};
        auto s = d.GetSensors();
        auto a = d.GetActors();
        EXPECT_EQ(s.begin(), s.end());
        EXPECT_EQ(a.begin(), a.end());
    }
    {
        NodeData d;
        auto s = d.GetSensors();
        auto a = d.GetActors();
        EXPECT_EQ(s.begin(), s.end());
        EXPECT_EQ(a.begin(), a.end());
    }
    {
        const NodeData d{1, "", "", {Sensor::Deleted()}, {Actor::Deleted()}, "", NodePath()};
        auto s = d.GetSensors();
        auto a = d.GetActors();
        ASSERT_NE(s.begin(), s.end());
        ASSERT_NE(a.begin(), a.end());
        EXPECT_EQ(Sensor::Deleted(), static_cast<const Sensor&>(*s.begin()));
        EXPECT_EQ(Actor::Deleted(), static_cast<const Actor&>(*a.begin()));
        EXPECT_EQ(std::next(s.begin()), s.end());
    }
    {
        const NodeData d{1, "", "", {Sensor()}, {Actor()}, "", NodePath()};
        auto s = d.GetSensors();
        auto a = d.GetActors();
        ASSERT_NE(s.begin(), s.end());
        ASSERT_NE(a.begin(), a.end());
        EXPECT_EQ(Sensor(), static_cast<const Sensor&>(*s.begin()));
        EXPECT_EQ(Actor(), static_cast<const Actor&>(*a.begin()));
        EXPECT_EQ(std::next(s.begin()), s.end());
    }
}

TEST(NodeData, Parse)
{
    {
        NodeData d;
        d.Parse({
            {"id", 1}, {"name", "n"}, {"location", "l"}, {"sensors", nlohmann::json::array()},
            {"actors", nlohmann::json::array()}, {"state", "s"}, {"path", {{"path", 3}, {"distance", 1}}},
            {"type", "node2x2"} // type 2
        });
        EXPECT_EQ(NodeData(1, "n", "l", {}, {}, "s", NodePath(3, 1), 2), d);
    }
    {
        NodeData d;
        d.Parse({
            {"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
            {"actors", nlohmann::json::array()}, {"state", "c"}, {"path", {{"path", 1}, {"distance", 1}}},
            {"type", "node"} // type 1
        });
        EXPECT_EQ(NodeData(3, "a", "b", {}, {}, "c", NodePath(1, 1), 1), d);
    }
    {
        NodeData d;
        d.Parse({
            {"id", 3}, {"name", "a"}, {"location", "b"},
            {"sensors", nlohmann::json::array({Sensor::Deleted().ToJson(), Sensor().ToJson()})},
            {"actors", nlohmann::json::array({Actor::Deleted().ToJson(), Actor().ToJson()})}, {"state", "c"},
            {"path", {{"path", 1}, {"distance", 1}}}, {"type", "node_ro"} // type 0
        });
        // once again, only sizes of sensors and actors are compared
        EXPECT_EQ(
            NodeData(3, "a", "b", {Sensor::Deleted(), Sensor()}, {Actor::Deleted(), Actor()}, "c", NodePath(1, 1), 0),
            d);
    }
    {
        NodeData d;
        d.Parse({{"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
            {"actors", nlohmann::json::array()}, {"state", "c"}, {"path", {{"path", 1}, {"distance", 1}}},
            {"type", "node_ro"}, // type 0
            {"additionalfield", "ignored"}});
        EXPECT_EQ(NodeData(3, "a", "b", {}, {}, "c", NodePath(1, 1), 0), d);
    }
    // Unknown type is changed to 1
    {
        NodeData d;
        d.Parse({{"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
            {"actors", nlohmann::json::array()}, {"state", "c"}, {"path", {{"path", 1}, {"distance", 1}}},
            {"type", "some unknown type"}});
        EXPECT_EQ(NodeData(3, "a", "b", {}, {}, "c", NodePath(1, 1), 1), d);
    }
    {
        NodeData d;
        EXPECT_THROW(d.Parse("wrong json type"), std::domain_error);
    }
    {
        NodeData d;
        EXPECT_THROW(d.Parse({{"fields", "missing"}}), std::out_of_range);
    }
    {
        NodeData d;
        EXPECT_THROW(d.Parse({{"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
                         {"actors", nlohmann::json::array()}, {"state", "c"}, {"path", {{"path", 1}, {"distance", 1}}},
                         {"missing", "type"}}),
            std::out_of_range);
    }
    {
        NodeData d;
        EXPECT_THROW(
            d.Parse({{"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
                {"actors", nlohmann::json::array()}, {"state", "c"}, {"path", "wrong type"}, {"type", "node"}}),
            std::domain_error);
    }
    {
        NodeData d;
        EXPECT_THROW(d.Parse({{"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
                         {"actors", nlohmann::json::array()}, {"state", "c"},
                         {"path", {{"path", "wrong type"}, {"distance", 1}}}, {"type", "node"}}),
            std::domain_error);
    }
    {
        NodeData d;
        EXPECT_THROW(d.Parse({{"id", "wrong type"}, {"name", "a"}, {"location", "b"},
                         {"sensors", nlohmann::json::array()}, {"actors", nlohmann::json::array()}, {"state", "c"},
                         {"path", {{"path", 1}, {"distance", 1}}}, {"type", "node"}}),
            std::domain_error);
    }
    {
        NodeData d;
        EXPECT_THROW(d.Parse({{"id", 3}, {"name", "a"}, {"location", "b"}, {"sensors", nlohmann::json::array()},
                         {"actors", nlohmann::json::array()}, {"state", "c"},
                         {"path", {{"missing", "path"}, {"distance", 1}}}, {"type", "node"}}),
            std::out_of_range);
    }
}

TEST(NodeData, ToJson)
{
    EXPECT_EQ(nlohmann::json({{"id", 0}, {"name", "Undefined"}, {"location", "Undefined"},
                  {"sensors", nlohmann::json::array()}, {"actors", nlohmann::json::array()}, {"state", "No state"},
                  {"path", {{"path", 0}, {"distance", 0}}}, {"type", "node"}}),
        NodeData().ToJson());
    EXPECT_EQ(nlohmann::json({{"id", 3}, {"name", "n"}, {"location", "l"}, {"sensors", nlohmann::json::array()},
                  {"actors", nlohmann::json::array()}, {"state", "s"}, {"path", {{"path", 3}, {"distance", 1}}},
                  {"type", "node_ro"}}),
        NodeData(3, "n", "l", {}, {}, "s", NodePath(3, 1), 0).ToJson());
    EXPECT_EQ(nlohmann::json(
                  {{"id", 3}, {"name", "n"}, {"location", "l"}, {"sensors", nlohmann::json::array({Sensor().ToJson()})},
                      {"actors", nlohmann::json::array({Actor::Deleted().ToJson(), Actor().ToJson()})}, {"state", "s"},
                      {"path", {{"path", 3}, {"distance", 1}}}, {"type", "node2x2"}}),
        NodeData(3, "n", "l", {Sensor()}, {Actor::Deleted(), Actor()}, "s", NodePath(3, 1), 2).ToJson());
    EXPECT_EQ(nlohmann::json({{"id", 3}, {"name", "n"}, {"location", "l"},
                  {"sensors", nlohmann::json::array({Sensor::Deleted().ToJson(), Sensor().ToJson()})},
                  {"actors", nlohmann::json::array({Actor().ToJson()})}, {"state", "s"},
                  {"path", {{"path", 3}, {"distance", 1}}}, {"type", "node3x3"}}),
        NodeData(3, "n", "l", {Sensor::Deleted(), Sensor()}, {Actor()}, "s", NodePath(3, 1), 3).ToJson());
    // Unknown type is changed to "node"
    EXPECT_EQ(nlohmann::json({{"id", 3}, {"name", "n"}, {"location", "l"},
                  {"sensors", nlohmann::json::array({Sensor::Deleted().ToJson(), Sensor().ToJson()})},
                  {"actors", nlohmann::json::array({Actor().ToJson()})}, {"state", "s"},
                  {"path", {{"path", 3}, {"distance", 1}}}, {"type", "node"}}),
        NodeData(3, "n", "l", {Sensor::Deleted(), Sensor()}, {Actor()}, "s", NodePath(3, 1), 0xFF).ToJson());
    {
        const NodeData d(3, "n", "l", {Sensor::Deleted(), Sensor()}, {Actor()}, "s", NodePath(3, 1), 3);
        NodeData p;
        p.Parse(d.ToJson());
        EXPECT_EQ(d, p);
    }
    {
        const NodeData d;
        // Call parse on not default-constructed node
        NodeData p(3, "n", "l", {Sensor::Deleted(), Sensor()}, {Actor()}, "s", NodePath(3, 1), 3);
        p.Parse(d.ToJson());
        EXPECT_EQ(d, p);
    }
    {
        const NodeData d{1, "", "", {Sensor()}, {Actor()}, "", NodePath()};
        // Call parse on not default-constructed node
        NodeData p(3, "n", "l", {Sensor::Deleted(), Sensor()}, {Actor()}, "s", NodePath(3, 1), 3);
        p.Parse(d.ToJson());
        EXPECT_EQ(d, p);
    }
}

TEST(NodeData, Getters)
{
    std::vector<Sensor> s{Sensor{3, 0, "name", "location", 3, 2, 1}, Sensor::Deleted()};
    std::vector<Actor> a{Actor::Deleted(), {}};
    const NodeData n(3, "name", "location", s, a, "state", NodePath(5, 3), 2);
    EXPECT_EQ(3, n.GetId());
    EXPECT_EQ("name", n.GetName());
    EXPECT_EQ("location", n.GetLocation());
    EXPECT_EQ("state", n.GetState());
    EXPECT_EQ(NodeData::s_type, n.GetType());

    auto sensors = n.GetSensors();
    auto itS = sensors.begin();
    auto actors = n.GetActors();
    auto itA = actors.begin();
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        ASSERT_NE(itS, sensors.end());
        EXPECT_EQ(s[i], static_cast<const Sensor&>(*itS));
        ++itS;
    }
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        ASSERT_NE(itA, actors.end());
        EXPECT_EQ(a[i], static_cast<const Actor&>(*itA));
        ++itA;
    }
}

// TODO: TEST(NodeData, GetUI)

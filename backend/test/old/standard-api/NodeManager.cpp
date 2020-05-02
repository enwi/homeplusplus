#include "standard-api/NodeManager.h"

#include <gtest/gtest.h>

#include "NodeSerializeUtils.h"

#include "../mocks/MockDBHandler.h"
#include "../mocks/MockNodeCommunication.h"
#include "api/Resources.h"
#include "database/DBNodeSerialize.h"
#include "events/EventSystem.h"
#include "events/Events.h"
#include "standard-api/communication/MessageTypes.h"

// TODO: Verify that correct events are handled everywhere

TEST(NodeManager, GetState)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    NodeData n{2, "name", "loc", {}, {}, "state", NodePath(1, 1), 0};
    // Not charging
    {
        ExpectGetNode(dbHandler, n);
        EXPECT_CALL(nc, QueueMessage(Messages::GetState(n.m_path, NodePath()), 5, 0));
        std::future<std::string> f = nm.GetState(2);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        ::Message response{NodePath(), n.m_path, true, 2, NodeCommands::RETURN_STATE};
        response.SetBVal(0, 0x01);
        response.SetBVal(1, 50);
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(response, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ("Battery at 50%", f.get());
    }
    // Charging
    {
        ExpectGetNode(dbHandler, n);
        EXPECT_CALL(nc, QueueMessage(Messages::GetState(n.m_path, NodePath()), 5, 0));
        std::future<std::string> f = nm.GetState(2);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        ::Message response{NodePath(), n.m_path, true, 2, NodeCommands::RETURN_STATE};
        response.SetBVal(0, 0x02);
        response.SetBVal(1, 20);
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(response, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ("Charging (20%)", f.get());
    }
    // Other
    {
        ExpectGetNode(dbHandler, n);
        EXPECT_CALL(nc, QueueMessage(Messages::GetState(n.m_path, NodePath()), 5, 0));
        std::future<std::string> f = nm.GetState(2);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        ::Message response{NodePath(), n.m_path, true, 2, NodeCommands::RETURN_STATE};
        response.SetBVal(0, 0x03);
        response.SetBVal(1, 50);
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(response, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ("Error!", f.get());
    }
    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_));
        EXPECT_CALL(nc, QueueMessage(Messages::GetState(n.m_path, NodePath()), 5, 0)).Times(0);
        std::future<std::string> f = nm.GetState(2);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ("Error! Bananananana", f.get());
    }
    // Not charging, irrelevant messages
    {
        ExpectGetNode(dbHandler, n);
        EXPECT_CALL(nc, QueueMessage(Messages::GetState(n.m_path, NodePath()), 5, 0));
        std::future<std::string> f = nm.GetState(2);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        // Irrelevant messages
        // Wrong node
        ::Message irrelevant1{NodePath(), NodePath(2, 1), true, 2, NodeCommands::RETURN_STATE};
        irrelevant1.SetBVal(0, 0x01);
        irrelevant1.SetBVal(1, 50);
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(irrelevant1, 1));

        // Wrong command
        ::Message irrelevant2{NodePath(), NodePath(1, 1), true, 2, NodeCommands::RETURN_SENSOR_STATE};
        irrelevant2.SetBVal(0, 0x01);
        irrelevant2.SetLVal(1, 50);
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(irrelevant2, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        ::Message response{NodePath(), n.m_path, true, 2, NodeCommands::RETURN_STATE};
        response.SetBVal(0, 0x01);
        response.SetBVal(1, 50);
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(response, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ("Battery at 50%", f.get());
    }
}

TEST(NodeManager, AddActor)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // No actor existing
    {
        NodeData n{2, "name", "loc", {}, {}, "state", NodePath(1, 1), 0};
        Actor a{0, 0, "actor", "al", ::Types::RELAY, Pins::D5};
        const uint8_t nextActorId = 0;
        ExpectGetNode(dbHandler, n);

        EXPECT_CALL(
            nc, QueueMessage(Messages::SetActorType(n.m_path, NodePath(), nextActorId, a.m_type, a.m_pin), 5, 0));
        uint8_t actorId = nm.AddActor(2, a);

        EXPECT_EQ(nextActorId, actorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor existing
    {
        NodeData n{2, "name", "loc", {}, {Actor()}, "state", NodePath(1, 1), 0};
        Actor a{0, 0, "actor", "al", ::Types::RELAY, Pins::D5};
        const uint8_t nextActorId = 1;
        ExpectGetNode(dbHandler, n);

        EXPECT_CALL(
            nc, QueueMessage(Messages::SetActorType(n.m_path, NodePath(), nextActorId, a.m_type, a.m_pin), 5, 0));
        uint8_t actorId = nm.AddActor(2, a);

        EXPECT_EQ(nextActorId, actorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Deleted actor existing
    {
        NodeData n{2, "name", "loc", {}, {Actor::Deleted(), Actor()}, "state", NodePath(1, 1), 0};
        Actor a{0, 0, "actor", "al", ::Types::RELAY, Pins::D5};
        const uint8_t nextActorId = 0;
        ExpectGetNode(dbHandler, n);

        EXPECT_CALL(
            nc, QueueMessage(Messages::SetActorType(n.m_path, NodePath(), nextActorId, a.m_type, a.m_pin), 5, 0));
        uint8_t actorId = nm.AddActor(2, a);

        EXPECT_EQ(nextActorId, actorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());

        EXPECT_THROW(nm.AddActor(2, Actor()), std::logic_error);
    }
}

TEST(NodeManager, ChangeActor)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_THROW(nm.ChangeActor(1, 0, Actor()), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor not found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        EXPECT_THROW(nm.ChangeActor(n.m_id, 0, Actor()), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor found, difference for node
    {
        NodeData n{1, "name", "location", {}, {Actor(1, 0, "a", "b", 1, 2)}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        Actor a{0, 0, "c", "d", 2, 3};
        EXPECT_CALL(nc, QueueMessage(Messages::SetActorType(n.m_path, NodePath(), 0, 2, 3), 5, 0));
        nm.ChangeActor(n.m_id, 0, a);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor found, no difference for node
    {
        NodeData n{1, "name", "location", {}, {Actor(1, 0, "a", "b", 1, 2)}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        // Only names different
        Actor a{0, 0, "c", "d", 1, 2};
        EXPECT_CALL(nc, QueueMessage(_, _, _)).Times(0);
        nm.ChangeActor(n.m_id, 0, a);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeManager, SetActorValue)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_THROW(nm.SetActorValue(1, 0, {}), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor not found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        EXPECT_THROW(nm.SetActorValue(1, 0, {}), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor found
    {
        NodeData n{1, "name", "location", {}, {Actor(1, 0, "a", "b", 1, 2)}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        const uint8_t actorId = 0;
        const std::vector<uint8_t> value{3};
        EXPECT_CALL(nc, QueueMessage(Messages::SetActorValue(n.m_path, NodePath(), actorId, value), 5, 0));
        nm.SetActorValue(1, actorId, value);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeManager, RemoveActor)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_THROW(nm.RemoveActor(1, 0), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor not found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        EXPECT_THROW(nm.RemoveActor(1, 0), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Actor found
    {
        NodeData n{1, "name", "location", {}, {Actor(1, 0, "a", "b", 1, 2)}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        const uint8_t actorId = 0;
        EXPECT_CALL(nc, QueueMessage(Messages::SetActorType(n.m_path, NodePath(), actorId, 0, 0), 5, 0));
        nm.RemoveActor(n.m_id, actorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeManager, AddSensor)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // No sensor existing (listener)
    {
        NodeData n{2, "name", "loc", {}, {}, "state", NodePath(1, 1), 0};
        Sensor s{0, 0, "sensor", "sl", ::Types::RELAY, Pins::D5, 4};
        const uint8_t nextSensorId = 0;
        ExpectGetNode(dbHandler, n);

        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), nextSensorId, s.m_type, s.m_pin),
            Messages::AddSensorListener(n.m_path, NodePath(), nextSensorId, s.m_interval)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        uint8_t sensorId = nm.AddSensor(2, s);

        EXPECT_EQ(nextSensorId, sensorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // No sensor existing (no listener)
    {
        NodeData n{2, "name", "loc", {}, {}, "state", NodePath(1, 1), 0};
        Sensor s{0, 0, "sensor", "sl", ::Types::RELAY, Pins::D5, 0};
        const uint8_t nextSensorId = 0;
        ExpectGetNode(dbHandler, n);

        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), nextSensorId, s.m_type, s.m_pin)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        uint8_t sensorId = nm.AddSensor(2, s);

        EXPECT_EQ(nextSensorId, sensorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor existing
    {
        NodeData n{2, "name", "loc", {Sensor()}, {}, "state", NodePath(1, 1), 0};
        Sensor s{0, 0, "sensor", "sl", ::Types::RELAY, Pins::D5, 4};
        const uint8_t nextSensorId = 1;
        ExpectGetNode(dbHandler, n);

        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), nextSensorId, s.m_type, s.m_pin),
            Messages::AddSensorListener(n.m_path, NodePath(), nextSensorId, s.m_interval)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        uint8_t sensorId = nm.AddSensor(2, s);

        EXPECT_EQ(nextSensorId, sensorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Deleted sensor existing
    {
        NodeData n{2, "name", "loc", {Sensor::Deleted(), Sensor()}, {}, "state", NodePath(1, 1), 0};
        Sensor s{0, 0, "sensor", "sl", ::Types::RELAY, Pins::D5, 4};
        const uint8_t nextSensorId = 0;
        ExpectGetNode(dbHandler, n);

        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), nextSensorId, s.m_type, s.m_pin),
            Messages::AddSensorListener(n.m_path, NodePath(), nextSensorId, s.m_interval)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        uint8_t sensorId = nm.AddSensor(2, s);

        EXPECT_EQ(nextSensorId, sensorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());

        EXPECT_THROW(nm.AddSensor(2, Sensor()), std::logic_error);
    }
}

TEST(NodeManager, ChangeSensor)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_THROW(nm.ChangeSensor(1, 0, Sensor()), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor not found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        EXPECT_THROW(nm.ChangeSensor(n.m_id, 0, Sensor()), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found, difference for node (type)
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "s", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        Sensor s{0, 0, "c", "d", 2, 2, 3};
        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), 0, s.m_type, s.m_pin)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        nm.ChangeSensor(n.m_id, 0, s);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found, difference for node (pin)
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "s", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        Sensor s{0, 0, "c", "d", 1, 3, 3};
        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), 0, s.m_type, s.m_pin)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        nm.ChangeSensor(n.m_id, 0, s);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found, difference for node (listener)
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "s", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        Sensor s{0, 0, "c", "d", 1, 2, 4};
        MessageSequence seq{{Messages::AddSensorListener(n.m_path, NodePath(), 0, s.m_interval)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        nm.ChangeSensor(n.m_id, 0, s);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found, difference for node (type + pin + listener)
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "s", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        Sensor s{0, 0, "c", "d", 3, 4, 5};
        MessageSequence seq{{Messages::SetSensorType(n.m_path, NodePath(), 0, s.m_type, s.m_pin),
            Messages::AddSensorListener(n.m_path, NodePath(), 0, s.m_interval)}};
        EXPECT_CALL(nc, SendMessageSequence(seq));
        nm.ChangeSensor(n.m_id, 0, s);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found, no difference for node
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "s", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        // Only names different
        Sensor s{0, 0, "c", "d", 1, 2, 3};
        EXPECT_CALL(nc, SendMessageSequence(_)).Times(0);
        nm.ChangeSensor(n.m_id, 0, s);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeManager, GetSensorValue)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        std::future<std::vector<uint8_t>> f = nm.GetSensorValue(1, 0);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ(std::vector<uint8_t>(), f.get());
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor not found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        std::future<std::vector<uint8_t>> f = nm.GetSensorValue(1, 0);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ(std::vector<uint8_t>(), f.get());
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "a", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        const uint8_t sensorId = 0;
        const std::vector<uint8_t> value{0xAB, 0x64, 0xFF};
        EXPECT_CALL(nc, QueueMessage(Messages::GetSensorValue(n.m_path, NodePath(), sensorId), 5, 0));
        std::future<std::vector<uint8_t>> f = nm.GetSensorValue(n.m_id, sensorId);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));
        Mock::VerifyAndClearExpectations(&dbHandler);

        // Node is queried again in callback
        ExpectGetNode(dbHandler, n);
        // Callback will insert changed sensor (not checked yet)
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        // Release of other savepoints
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());
        ::Message response{NodePath(), n.m_path, true, 2, NodeCommands::RETURN_SENSOR_VALUE};
        response.SetBVal(0, sensorId);
        for (std::size_t i = 0; i < value.size(); ++i)
        {
            response.SetBVal(i + 1, value[i]);
        }
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(response, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ(value, f.get());
    }
    // Sensor found, irrelevant messages
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "a", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        const uint8_t sensorId = 0;
        const std::vector<uint8_t> value{0xAB, 0x64, 0xFF};
        EXPECT_CALL(nc, QueueMessage(Messages::GetSensorValue(n.m_path, NodePath(), sensorId), 5, 0));
        std::future<std::vector<uint8_t>> f = nm.GetSensorValue(n.m_id, sensorId);

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));
        Mock::VerifyAndClearExpectations(&dbHandler);

        // Node is queried again in callback
        ExpectGetNode(dbHandler, n);
        // Callback will insert changed sensor (not checked yet)
        EXPECT_CALL(dbHandler, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(dbHandler, GetSavepoint(_)).Times(AnyNumber());
        // Release of other savepoints
        EXPECT_CALL(dbHandler.db, ExecuteStatement(_)).Times(AnyNumber());

        // Irrelevant messages
        // Wrong node
        ::Message irrelevant1{NodePath(), NodePath(2, 1), true, 2, NodeCommands::RETURN_SENSOR_VALUE};
        irrelevant1.SetBVal(0, sensorId);
        for (std::size_t i = 0; i < value.size(); ++i)
        {
            irrelevant1.SetBVal(i + 1, value[i]);
        }
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(irrelevant1, 1));

        // Wrong sensor
        ::Message irrelevant2{NodePath(), NodePath(1, 1), true, 2, NodeCommands::RETURN_SENSOR_VALUE};
        irrelevant2.SetBVal(0, sensorId + 1);
        for (std::size_t i = 0; i < value.size(); ++i)
        {
            irrelevant2.SetBVal(i + 1, value[i]);
        }
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(irrelevant2, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        ::Message response{NodePath(), n.m_path, true, 2, NodeCommands::RETURN_SENSOR_VALUE};
        response.SetBVal(0, sensorId);
        for (std::size_t i = 0; i < value.size(); ++i)
        {
            response.SetBVal(i + 1, value[i]);
        }
        Res::EventSystem().HandleEvent(Events::NodeMessageEvent(response, 1));

        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_EQ(value, f.get());
    }
}

TEST(NodeManager, RemoveSensor)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_THROW(nm.RemoveSensor(1, 0), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor not found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        EXPECT_THROW(nm.RemoveSensor(1, 0), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Sensor found
    {
        NodeData n{1, "name", "location", {Sensor(1, 0, "a", "b", 1, 2, 3)}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        const uint8_t sensorId = 0;
        EXPECT_CALL(nc, QueueMessage(Messages::SetSensorType(n.m_path, NodePath(), sensorId, 0, 0), 5, 0));
        nm.RemoveSensor(n.m_id, sensorId);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
}

TEST(NodeManager, RemoveNode)
{
    using namespace ::testing;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc(10);
    MockDBHandler dbHandler;
    dbHandler.UseDefaults();

    DBNodeSerialize ns{dbHandler};
    NodeManager nm{dbHandler, nc, ns};

    // Node not found
    {
        EXPECT_CALL(dbHandler, GetROStatement(_)).Times(AnyNumber());
        EXPECT_THROW(nm.RemoveNode(1), std::logic_error);
        Mock::VerifyAndClearExpectations(&dbHandler);
    }
    // Node found
    {
        NodeData n{1, "name", "location", {}, {}, "state", NodePath(1, 1), 1};
        ExpectGetNode(dbHandler, n);
        EXPECT_CALL(nc, QueueMessage(Messages::DeleteNode(n.m_path, NodePath()), 5, 0));
        nm.RemoveNode(1);
    }
}

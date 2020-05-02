#include "database/DBEventHandler.h"

#include <gtest/gtest.h>

#include "../mocks/MockActionSerialize.h"
#include "../mocks/MockDBHandler.h"
#include "../mocks/MockEventHandler.h"
#include "../mocks/MockNodeSerialize.h"
#include "../mocks/MockRuleSerialize.h"

TEST(DBEventHandler, ShouldExecuteOn)
{
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    DBEventHandler h{db, ns, as, rs};

    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::nodeChange));
    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::actionChange));
    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::ruleChange));
    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::sensorChange));
    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::actorChange));
    EXPECT_FALSE(h.ShouldExecuteOn(EventTypes::nodeMessage));
    EXPECT_FALSE(h.ShouldExecuteOn(EventTypes::error));
}

TEST(DBEventHandler, HandleNodeChangeEvent)
{
    using namespace ::testing;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    DBEventHandler h{db, ns, as, rs};

    // Add
    {
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(NodeData::EmptyNode(), changed, Events::NodeFields::ADD);

        EXPECT_CALL(ns, AddNode(changed));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // All
    {
        NodeData old{2, "n0", "l0", {}, {}, "s0", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ALL);

        EXPECT_CALL(ns, AddNode(changed));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Remove
    {
        NodeData old{2, "n0", "l0", {}, {}, "s0", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, NodeData::EmptyNode(), Events::NodeFields::REMOVE);

        EXPECT_CALL(ns, RemoveNode(old.m_id));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Name
    {
        NodeData old{2, "n0", "l0", {}, {}, "s0", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::NAME);

        EXPECT_CALL(ns, AddNodeOnly(changed));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Location
    {
        NodeData old{2, "n0", "l0", {}, {}, "s0", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::LOCATION);

        EXPECT_CALL(ns, AddNodeOnly(changed));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // State
    {
        NodeData old{2, "n0", "l0", {}, {}, "s0", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::STATE);

        EXPECT_CALL(ns, AddNodeOnly(changed));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Path
    {
        NodeData old{2, "n0", "l0", {}, {}, "s0", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::PATH);

        EXPECT_CALL(ns, AddNodeOnly(changed));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // TODO: Sensors, Actors
}

TEST(DBEventHandler, HandleNodeChangeEventSensor)
{
    using namespace ::testing;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    DBEventHandler h{db, ns, as, rs};

    auto eh = std::make_shared<MockEventHandler>();

    Res::EventSystem().AddHandler(eh);
    CleanupEventHandler cleanup{Res::EventSystem(), eh};

    // Nothing changed
    {
        NodeData old{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, HandleEvent(_)).Times(0);
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Sensor added
    {
        NodeData old{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {Sensor(2, 0, "sn", "sl", 0, 0, 0)}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::ADD && casted.GetOld().m_nodeId == changed.m_id
                && casted.GetOld().m_sensor == Sensor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_property == 0 && casted.GetChanged().m_sensor == changed.m_sensors[0];
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Multiple sensors added
    {
        NodeData old{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {Sensor(2, 0, "sn", "sl", 0, 0, 0), Sensor(2, 1, "sn1", "sl1", 0, 0, 0)}, {}, "s",
            NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange)).Times(2);
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::ADD && casted.GetOld().m_nodeId == changed.m_id
                && casted.GetOld().m_sensor == Sensor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_property == 0 && casted.GetChanged().m_sensor == changed.m_sensors[0];
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::ADD && casted.GetOld().m_nodeId == changed.m_id
                && casted.GetOld().m_sensor == Sensor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_property == 1 && casted.GetChanged().m_sensor == changed.m_sensors[1];
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Sensor changed (currently always triggers ALL)
    {
        NodeData old{2, "n", "l", {Sensor(2, 0, "sn0", "sl0", 0, 1, 0)}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {Sensor(2, 0, "sn", "sl", 0, 0, 0)}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::ALL && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 0 && casted.GetOld().m_sensor == old.m_sensors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 0
                && casted.GetChanged().m_sensor == changed.m_sensors[0];
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Sensor removed
    {
        NodeData old{2, "n", "l", {Sensor(2, 0, "sn0", "sl0", 0, 1, 0)}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 0 && casted.GetOld().m_sensor == old.m_sensors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 0
                && casted.GetChanged().m_sensor == Sensor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Sensor removed (not at end)
    {
        NodeData old{2, "n", "l", {Sensor(2, 0, "sn0", "sl0", 0, 1, 0), Sensor(2, 1, "s1n0", "s1l0", 0, 2, 0)}, {}, "s",
            NodePath(1, 1)};
        NodeData changed{
            2, "n", "l", {Sensor::Deleted(), Sensor(2, 1, "s1n0", "s1l0", 0, 2, 0)}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 0 && casted.GetOld().m_sensor == old.m_sensors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 0
                && casted.GetChanged().m_sensor == Sensor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Multiple removed
    {
        NodeData old{2, "n", "l",
            {Sensor(2, 0, "sn0", "sl0", 0, 1, 0), Sensor(2, 1, "s1n0", "s1l0", 0, 2, 0),
                Sensor(2, 2, "s2n0", "s2l0", 0, 3, 0)},
            {}, "s", NodePath(1, 1)};
        NodeData changed{
            2, "n", "l", {Sensor::Deleted(), Sensor(2, 1, "s1n0", "s1l0", 0, 2, 0)}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange)).Times(2);
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 0 && casted.GetOld().m_sensor == old.m_sensors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 0
                && casted.GetChanged().m_sensor == Sensor::Deleted();
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 2 && casted.GetOld().m_sensor == old.m_sensors[2]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 2
                && casted.GetChanged().m_sensor == Sensor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Sensor 0 removed, sensor 1 changed, sensor 2 added
    {
        NodeData old{2, "n", "l", {Sensor(2, 0, "sn0", "sl0", 0, 1, 0), Sensor(2, 1, "s1n0", "s1l0", 0, 2, 0)}, {}, "s",
            NodePath(1, 1)};
        NodeData changed{2, "n", "l",
            {Sensor::Deleted(), Sensor(2, 1, "a", "b", 4, 2, 1), Sensor(2, 2, "s2n0", "s2l0", 0, 3, 0)}, {}, "s",
            NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::SENSORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::sensorChange)).Times(3);
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::ADD && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_sensor == Sensor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_property == 2 && casted.GetChanged().m_sensor == changed.m_sensors[2];
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::ALL && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 1 && casted.GetOld().m_sensor == old.m_sensors[1]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 1
                && casted.GetChanged().m_sensor == changed.m_sensors[1];
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::sensorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::DeviceChangeEvent&>(e);
            return casted.GetChangedFields() == Events::SensorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_property == 0 && casted.GetOld().m_sensor == old.m_sensors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_property == 0
                && casted.GetChanged().m_sensor == Sensor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
}

TEST(DBEventHandler, HandleNodeChangeEventActor)
{
    using namespace ::testing;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    DBEventHandler h{db, ns, as, rs};

    auto eh = std::make_shared<MockEventHandler>();

    Res::EventSystem().AddHandler(eh);
    CleanupEventHandler cleanup{Res::EventSystem(), eh};

    // Nothing changed
    {
        NodeData old{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, HandleEvent(_)).Times(0);
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Actor added
    {
        NodeData old{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {Actor(2, 0, "an", "al", 0, 0)}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::ADD && casted.GetOld().m_nodeId == changed.m_id
                && casted.GetOld().m_actor == Actor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_actorId == 0 && casted.GetChanged().m_actor == changed.m_actors[0];
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Multiple actors added
    {
        NodeData old{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{
            2, "n", "l", {}, {Actor(2, 0, "an", "al", 0, 0), Actor(2, 1, "an1", "al1", 0, 0)}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange)).Times(2);
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::ADD && casted.GetOld().m_nodeId == changed.m_id
                && casted.GetOld().m_actor == Actor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_actorId == 0 && casted.GetChanged().m_actor == changed.m_actors[0];
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::ADD && casted.GetOld().m_nodeId == changed.m_id
                && casted.GetOld().m_actor == Actor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_actorId == 1 && casted.GetChanged().m_actor == changed.m_actors[1];
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Actor changed (currently always triggers ALL)
    {
        NodeData old{2, "n", "l", {}, {Actor(2, 0, "an0", "al0", 0, 1)}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {Actor(2, 0, "an", "al", 0, 0)}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::ALL && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 0 && casted.GetOld().m_actor == old.m_actors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 0
                && casted.GetChanged().m_actor == changed.m_actors[0];
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Actor removed
    {
        NodeData old{2, "n", "l", {}, {Actor(2, 0, "an0", "al0", 0, 1)}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 0 && casted.GetOld().m_actor == old.m_actors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 0
                && casted.GetChanged().m_actor == Actor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Actor removed (not at end)
    {
        NodeData old{
            2, "n", "l", {}, {Actor(2, 0, "an0", "al0", 0, 1), Actor(2, 1, "a1n0", "a1l0", 0, 2)}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {Actor::Deleted(), Actor(2, 1, "a1n0", "a1l0", 0, 2)}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 0 && casted.GetOld().m_actor == old.m_actors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 0
                && casted.GetChanged().m_actor == Actor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Multiple removed
    {
        NodeData old{2, "n", "l", {},
            {Actor(2, 0, "sn0", "sl0", 0, 1), Actor(2, 1, "s1n0", "s1l0", 0, 2), Actor(2, 2, "s2n0", "s2l0", 0, 3)},
            "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {}, {Actor::Deleted(), Actor(2, 1, "s1n0", "s1l0", 0, 2)}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange)).Times(2);
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 0 && casted.GetOld().m_actor == old.m_actors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 0
                && casted.GetChanged().m_actor == Actor::Deleted();
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 2 && casted.GetOld().m_actor == old.m_actors[2]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 2
                && casted.GetChanged().m_actor == Actor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
    // Actor 0 removed, actor 1 changed, actor 2 added
    {
        NodeData old{
            2, "n", "l", {}, {Actor(2, 0, "sn0", "sl0", 0, 1), Actor(2, 1, "s1n0", "s1l0", 0, 2)}, "s", NodePath(1, 1)};
        NodeData changed{2, "n", "l", {},
            {Actor::Deleted(), Actor(2, 1, "a", "b", 4, 2), Actor(2, 2, "s2n0", "s2l0", 0, 3)}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(old, changed, Events::NodeFields::ACTORS);

        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actorChange)).Times(3);
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::ADD && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actor == Actor::Deleted() && casted.GetChanged().m_nodeId == changed.m_id
                && casted.GetChanged().m_actorId == 2 && casted.GetChanged().m_actor == changed.m_actors[2];
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::ALL && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 1 && casted.GetOld().m_actor == old.m_actors[1]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 1
                && casted.GetChanged().m_actor == changed.m_actors[1];
        })));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actorChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActorChangeEvent&>(e);
            return casted.GetChangedFields() == Events::ActorFields::REMOVE && casted.GetOld().m_nodeId == old.m_id
                && casted.GetOld().m_actorId == 0 && casted.GetOld().m_actor == old.m_actors[0]
                && casted.GetChanged().m_nodeId == changed.m_id && casted.GetChanged().m_actorId == 0
                && casted.GetChanged().m_actor == Actor::Deleted();
        })));
        h.HandleNodeChangeEvent(e);
        Mock::VerifyAndClearExpectations(eh.get());
    }
}

TEST(DBEventHandler, HandleActionChangeEvent)
{
    using namespace ::testing;
    using ::Action;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    DBEventHandler h{db, ns, as, rs};

    // Add (id 0)
    {
        const std::size_t newId = 2;
        Action changed{0, "n", "l", 0, {}};
        Events::ActionChangeEvent e(Action(), changed, Events::ActionFields::ADD);

        auto eh = std::make_shared<MockEventHandler>();
        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::actionChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::actionChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::ActionChangeEvent&>(e);
            // Id is changed after insert
            Action newAction = changed;
            newAction.SetId(newId);
            return casted.GetChangedFields() == Events::ActionFields::ALL
                && casted.GetOld().ToJson() == Action().ToJson() && casted.GetChanged().ToJson() == newAction.ToJson();
        })));

        Res::EventSystem().AddHandler(eh);
        CleanupEventHandler cleanup{Res::EventSystem(), eh};

        EXPECT_CALL(as, AddAction(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })))
            .WillOnce(Return(newId));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Add (id not 0)
    {
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(Action(), changed, Events::ActionFields::ADD);

        // Inserted with known id
        EXPECT_CALL(as, AddAction(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // All
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(old, changed, Events::ActionFields::ALL);

        EXPECT_CALL(as, AddAction(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // SubActions
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(old, changed, Events::ActionFields::SUB_ACTIONS);

        EXPECT_CALL(as, AddAction(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Remove
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Events::ActionChangeEvent e(old, Action(), Events::ActionFields::REMOVE);

        EXPECT_CALL(as, RemoveAction(old.GetId()));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Name
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(old, changed, Events::ActionFields::NAME);

        EXPECT_CALL(as, AddActionOnly(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Icon
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(old, changed, Events::ActionFields::ICON);

        EXPECT_CALL(as, AddActionOnly(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Color
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(old, changed, Events::ActionFields::COLOR);

        EXPECT_CALL(as, AddActionOnly(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Visible
    {
        Action old{1, "n0", "l0", 1, {}, false};
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(old, changed, Events::ActionFields::VISIBLE);

        EXPECT_CALL(as, AddActionOnly(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleActionChangeEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
}

TEST(DBEventHandler, HandleRuleChangeEvent)
{
    using namespace ::testing;
    using ::Action;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    EventSystem ev;
    DBEventHandler h{db, ns, as, rs};

    // Add (id 0)
    {
        const std::size_t newId = 2;
        Rule changed{0, "n", "l", 0, nullptr, Action()};
        Events::RuleChangeEvent e(Rule(), changed, Events::RuleFields::ADD);

        auto eh = std::make_shared<MockEventHandler>();
        EXPECT_CALL(*eh, ShouldExecuteOn(EventTypes::ruleChange));
        EXPECT_CALL(*eh, HandleEvent(Truly([&](const EventBase& e) {
            if (e.GetType() != EventTypes::ruleChange)
            {
                return false;
            }
            const auto& casted = dynamic_cast<const Events::RuleChangeEvent&>(e);
            // Id is changed after insert
            Rule newRule = changed;
            newRule.SetId(newId);
            return casted.GetChangedFields() == Events::RuleFields::ALL && casted.GetOld().ToJson() == Rule().ToJson()
                && casted.GetChanged().ToJson() == newRule.ToJson();
        })));

        Res::EventSystem().AddHandler(eh);
        CleanupEventHandler cleanup{Res::EventSystem(), eh};

        // TODO: Find a way to avoid adding Rules twice
        EXPECT_CALL(rs, AddRule(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })))
            .WillOnce(Invoke([&](Rule& r) { r.SetId(newId); }));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Add (id not 0)
    {
        Rule changed{1, "n", "l", 0, nullptr, Action()};
        Events::RuleChangeEvent e(Rule(), changed, Events::RuleFields::ADD);

        // Inserted with known id
        EXPECT_CALL(rs, AddRule(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // All
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::ALL);

        EXPECT_CALL(rs, AddRule(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Condition
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::CONDITION);

        EXPECT_CALL(rs, AddRule(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Effect
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::EFFECT);

        EXPECT_CALL(rs, AddRule(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Remove
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Events::RuleChangeEvent e(old, Rule(), Events::RuleFields::REMOVE);

        EXPECT_CALL(
            rs, RemoveRule(Matcher<const Rule&>(Truly([&](const Rule& r) { return r.ToJson() == old.ToJson(); }))));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Name
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::NAME);

        EXPECT_CALL(rs, AddRuleOnly(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Icon
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::ICON);

        EXPECT_CALL(rs, AddRuleOnly(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Color
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::COLOR);

        EXPECT_CALL(rs, AddRuleOnly(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Enabled
    {
        Rule old{1, "n0", "l0", 1, nullptr, Action(), false};
        Rule changed{0, "n", "l", 0, nullptr, Action(), true};
        Events::RuleChangeEvent e(old, changed, Events::RuleFields::ENABLED);

        EXPECT_CALL(rs, AddRuleOnly(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleRuleChangeEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
}

TEST(DBEventHandler, HandleActorChangeEvent)
{
    using namespace ::testing;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    EventSystem ev;
    DBEventHandler h{db, ns, as, rs};

    MockActorSerialize actorSer;
    EXPECT_CALL(ns, GetActorSerialize()).WillRepeatedly(ReturnRef(actorSer));

    // Add
    {
        const uint16_t nodeId = 2;
        const uint8_t actorId = 1;
        Actor a{nodeId, actorId, "n", "l", 1, 2};
        Events::ActorChangeEvent e{{nodeId, actorId, Actor::Deleted()}, {nodeId, actorId, a}, Events::ActorFields::ADD};

        EXPECT_CALL(actorSer, UpdateActor(nodeId, actorId, a));

        h.HandleActorChangeEvent(e);
        Mock::VerifyAndClearExpectations(&actorSer);
    }
    // All
    {
        const uint16_t nodeId = 2;
        const uint8_t actorId = 1;
        Actor old{nodeId, actorId, "n0", "l0", 3, 4};
        Actor a{nodeId, actorId, "n", "l", 1, 2};
        Events::ActorChangeEvent e{{nodeId, actorId, Actor::Deleted()}, {nodeId, actorId, a}, Events::ActorFields::ADD};

        EXPECT_CALL(actorSer, UpdateActor(nodeId, actorId, a));

        h.HandleActorChangeEvent(e);
        Mock::VerifyAndClearExpectations(&actorSer);
    }
    // Remove
    {
        const uint16_t nodeId = 2;
        const uint8_t actorId = 1;
        Actor a{nodeId, actorId, "n", "l", 1, 2};
        Events::ActorChangeEvent e{
            {nodeId, actorId, a}, {nodeId, actorId, Actor::Deleted()}, Events::ActorFields::REMOVE};

        EXPECT_CALL(actorSer, UpdateActor(nodeId, actorId, Actor::Deleted()));

        h.HandleActorChangeEvent(e);
        Mock::VerifyAndClearExpectations(&actorSer);
    }
}

TEST(DBEventHandler, HandleSensorChangeEvent)
{
    using namespace ::testing;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    EventSystem ev;
    DBEventHandler h{db, ns, as, rs};

    MockSensorSerialize sensorSer;
    EXPECT_CALL(ns, GetSensorSerialize()).WillRepeatedly(ReturnRef(sensorSer));

    // TODO: Verify that logs are updated correctly
    db.UseDefaults();
    EXPECT_CALL(db, GetSavepoint(_)).Times(AnyNumber());
    EXPECT_CALL(db, GetStatement(_)).Times(AnyNumber());
    EXPECT_CALL(db.db, ExecuteStatement(_)).Times(AnyNumber());

    // Add
    {
        const uint16_t nodeId = 2;
        const uint8_t sensorId = 1;
        Sensor s{nodeId, sensorId, "n", "l", 1, 2, 0};
        Events::DeviceChangeEvent e{
            {nodeId, sensorId, Sensor::Deleted()}, {nodeId, sensorId, s}, Events::SensorFields::ADD};

        EXPECT_CALL(sensorSer, UpdateSensor(nodeId, sensorId, s));

        h.HandleSensorChangeEvent(e);
        Mock::VerifyAndClearExpectations(&sensorSer);
    }
    // All
    {
        const uint16_t nodeId = 2;
        const uint8_t sensorId = 1;
        Sensor old{nodeId, sensorId, "n0", "l0", 3, 4, 0};
        Sensor s{nodeId, sensorId, "n", "l", 1, 2, 0};
        Events::DeviceChangeEvent e{
            {nodeId, sensorId, Sensor::Deleted()}, {nodeId, sensorId, s}, Events::SensorFields::ADD};

        EXPECT_CALL(sensorSer, UpdateSensor(nodeId, sensorId, s));

        h.HandleSensorChangeEvent(e);
        Mock::VerifyAndClearExpectations(&sensorSer);
    }
    // Remove
    {
        const uint16_t nodeId = 2;
        const uint8_t sensorId = 1;
        Sensor s{nodeId, sensorId, "n", "l", 1, 2, 0};
        Events::DeviceChangeEvent e{
            {nodeId, sensorId, s}, {nodeId, sensorId, Sensor::Deleted()}, Events::SensorFields::REMOVE};

        EXPECT_CALL(sensorSer, UpdateSensor(nodeId, sensorId, Sensor::Deleted()));

        h.HandleSensorChangeEvent(e);
        Mock::VerifyAndClearExpectations(&sensorSer);
    }
}

TEST(DBEventHandler, HandleEvent)
{
    using namespace ::testing;
    MockNodeSerialize ns;
    MockActionSerialize as;
    MockRuleSerialize rs;
    MockDBHandler db;
    db.UseDefaults();
    EventSystem ev;
    DBEventHandler h{db, ns, as, rs};

    // Node change, forwarded
    {
        NodeData changed{2, "n", "l", {}, {}, "s", NodePath(1, 1)};
        Events::NodeChangeEvent e(NodeData::EmptyNode(), changed, Events::NodeFields::ADD);

        EXPECT_CALL(ns, AddNode(changed));
        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&ns);
    }
    // Action change, forwarded
    {
        using ::Action;
        Action changed{1, "n", "l", 0, {}};
        Events::ActionChangeEvent e(Action(), changed, Events::ActionFields::ADD);

        // Inserted with known id
        EXPECT_CALL(as, AddAction(Truly([&](const Action& a) { return a.ToJson() == changed.ToJson(); })));
        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&as);
    }
    // Rule change, forwarded
    {
        Rule changed{1, "n", "l", 0, nullptr, ::Action()};
        Events::RuleChangeEvent e(Rule(), changed, Events::RuleFields::ADD);

        // Inserted with known id
        EXPECT_CALL(rs, AddRule(Truly([&](const Rule& r) { return r.ToJson() == changed.ToJson(); })));
        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&rs);
    }
    // Sensor change, forwarded
    {
        MockSensorSerialize sensorSer;
        EXPECT_CALL(ns, GetSensorSerialize()).WillRepeatedly(ReturnRef(sensorSer));

        // TODO: Verify that logs are updated correctly
        EXPECT_CALL(db, GetSavepoint(_)).Times(AnyNumber());
        EXPECT_CALL(db, GetStatement(_)).Times(AnyNumber());
        EXPECT_CALL(db.db, ExecuteStatement(_)).Times(AnyNumber());

        const uint16_t nodeId = 2;
        const uint8_t sensorId = 1;
        Sensor s{nodeId, sensorId, "n", "l", 1, 2, 0};
        Events::DeviceChangeEvent e{
            {nodeId, sensorId, Sensor::Deleted()}, {nodeId, sensorId, s}, Events::SensorFields::ADD};

        EXPECT_CALL(sensorSer, UpdateSensor(nodeId, sensorId, s));

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&sensorSer);
    }
    // Actor change, forwarded
    {
        MockActorSerialize actorSer;
        EXPECT_CALL(ns, GetActorSerialize()).WillRepeatedly(ReturnRef(actorSer));

        const uint16_t nodeId = 2;
        const uint8_t actorId = 1;
        Actor a{nodeId, actorId, "n", "l", 1, 2};
        Events::ActorChangeEvent e{{nodeId, actorId, Actor::Deleted()}, {nodeId, actorId, a}, Events::ActorFields::ADD};

        EXPECT_CALL(actorSer, UpdateActor(nodeId, actorId, a));

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&actorSer);
    }
    // Unknown type
    {
        EXPECT_THROW(h.HandleEvent(Events::ErrorEvent("test", "test")), std::logic_error);
    }
}
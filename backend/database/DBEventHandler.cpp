#include "DBEventHandler.h"

#include <ctime>
#include <stdexcept>

#include "../utility/RDPAlgorithm.h"

DBEventHandler::DBEventHandler(DBHandler& dbHandler, IActionSerialize& actionSer, IRuleSerialize& ruleSer)
    : m_dbHandler(&dbHandler), m_actionSer(&actionSer), m_ruleSer(&ruleSer)
{}

PostEventState DBEventHandler::HandleEvent(const EventBase& e)
{
    // if (e.GetType() == EventTypes::nodeChange)
    //{
    //    const Events::NodeChangeEvent& casted = EventCast<Events::NodeChangeEvent>(e);
    //    HandleNodeChangeEvent(casted);
    //    return PostEventState::handled;
    //}
    // else
    if (e.GetType() == EventTypes::actionChange)
    {
        const Events::ActionChangeEvent& casted = EventCast<Events::ActionChangeEvent>(e);
        HandleActionChangeEvent(casted);
        return PostEventState::handled;
    }
    else if (e.GetType() == EventTypes::ruleChange)
    {
        const Events::RuleChangeEvent& casted = EventCast<Events::RuleChangeEvent>(e);
        HandleRuleChangeEvent(casted);
        return PostEventState::handled;
    }
    return PostEventState::notHandled;
}
/*
void DBEventHandler::HandleNodeChangeEvent(const Events::NodeChangeEvent& e)
{
    switch (e.GetChangedFields())
    {
    case Events::NodeFields::ALL:
    case Events::NodeFields::ADD:
        Res::Logger().Debug("Node Changed");
        m_nodeSer->AddNode(e.GetChanged());
        break;
    case Events::NodeFields::REMOVE:
        Res::Logger().Debug("Remove node");
        m_nodeSer->RemoveNode(e.GetOld().m_id);
        break;
    case Events::NodeFields::NAME:
    case Events::NodeFields::LOCATION:
    case Events::NodeFields::STATE:
    case Events::NodeFields::PATH:
        Res::Logger().Debug("Node field Changed");
        // Only need to change the node, no sensors or actors
        m_nodeSer->AddNodeOnly(e.GetChanged());
        break;
    case Events::NodeFields::SENSORS:
    {
        const std::vector<Sensor>& old = e.GetOld().m_sensors;
        const std::vector<Sensor>& changed = e.GetChanged().m_sensors;
        const unsigned int maxSize = std::max(old.size(), changed.size());
        for (std::size_t i = 0; i < maxSize; ++i)
        {
            if (i < changed.size())
            {
                // Compare every common Sensor to check if they differ or were added
                if (i >= old.size())
                {
                    // Sensor was added
                    Res::EventSystem().HandleEvent(
                        Events::DeviceChangeEvent({e.GetOld().m_id, (uint8_t)i, Sensor::Deleted()},
                            {e.GetChanged().m_id, (uint8_t)i, changed[i]}, Events::SensorFields::ADD));
                }
                else if (old[i] != changed[i])
                {
                    // Sensor was changed
                    if (changed[i].IsDeleted())
                    {
                        Res::EventSystem().HandleEvent(Events::DeviceChangeEvent({e.GetOld().m_id, (uint8_t)i, old[i]},
                            {e.GetChanged().m_id, (uint8_t)i, changed[i]}, Events::SensorFields::REMOVE));
                    }
                    else
                    {
                        // TODO: Change this and find out which field changed
                        Res::EventSystem().HandleEvent(Events::DeviceChangeEvent({e.GetOld().m_id, (uint8_t)i, old[i]},
                            {e.GetChanged().m_id, (uint8_t)i, changed[i]}, Events::SensorFields::ALL));
                    }
                }
            }
            else
            {
                // Sensor(s) were removed
                Res::EventSystem().HandleEvent(Events::DeviceChangeEvent({e.GetOld().m_id, (uint8_t)i, old[i]},
                    {e.GetChanged().m_id, (uint8_t)i, Sensor::Deleted()}, Events::SensorFields::REMOVE));
            }
        }
        break;
    }
    case Events::NodeFields::ACTORS:
    {
        // TODO: Move this into an own EventHandler
        // Find changed actors, fire an ActorChangeEvent
        const std::vector<Actor>& old = e.GetOld().m_actors;
        const std::vector<Actor>& changed = e.GetChanged().m_actors;
        unsigned int i = 0;
        unsigned int maxSize = std::max(old.size(), changed.size());
        for (; i < maxSize; ++i)
        {
            if (i < changed.size())
            {
                // Compare every common Actor to check if they differ or were added
                if (i >= old.size())
                {
                    // Actor was added
                    Res::EventSystem().HandleEvent(
                        Events::ActorChangeEvent({e.GetOld().m_id, (uint8_t)i, Actor::Deleted()},
                            {e.GetChanged().m_id, (uint8_t)i, changed[i]}, Events::ActorFields::ADD));
                }
                else if (old[i] != changed[i])
                {
                    // Actor was changed
                    if (changed[i] == Actor::Deleted())
                    {
                        Res::EventSystem().HandleEvent(Events::ActorChangeEvent({e.GetOld().m_id, (uint8_t)i, old[i]},
                            {e.GetChanged().m_id, (uint8_t)i, changed[i]}, Events::ActorFields::REMOVE));
                    }
                    else
                    {
                        // TODO: Change this and find out which field changed
                        Res::EventSystem().HandleEvent(Events::ActorChangeEvent({e.GetOld().m_id, (uint8_t)i, old[i]},
                            {e.GetChanged().m_id, (uint8_t)i, changed[i]}, Events::ActorFields::ALL));
                    }
                }
            }
            else
            {
                // Actor(s) were removed
                Res::EventSystem().HandleEvent(Events::ActorChangeEvent({e.GetOld().m_id, (uint8_t)i, old[i]},
                    {e.GetChanged().m_id, (uint8_t)i, Actor::Deleted()}, Events::ActorFields::REMOVE));
            }
        }
        break;
    }
    default:
        Res::Logger().Debug("Something in a node changed, but no flag was set!");
        break;
    }
}
*/

void DBEventHandler::HandleActionChangeEvent(const Events::ActionChangeEvent& e)
{
    switch (e.GetChangedFields())
    {
    case Events::ActionFields::ADD:
    {
        if (e.GetChanged().GetId() == 0)
        {
            // Add the action first, then fire this event again with a changed id
            Action action = e.GetChanged();
            action.SetId(m_actionSer->AddAction(action, e.GetUser().value()));
            if (action.GetId() != 0)
            {
                // Extra check to avoid infinite recursion if something goes wrong
                // TODO: Find a way to avoid adding Actions twice (again with ALL event)
                Events::ActionChangeEvent e2(e.GetOld(), action, Events::ActionFields::ALL);
                Res::EventSystem().HandleEvent(e2);
            }
        }
        else
        {
            m_actionSer->AddAction(e.GetChanged(), e.GetUser().value());
        }
        break;
    }
    case Events::ActionFields::ALL:
    case Events::ActionFields::SUB_ACTIONS:
    {
        m_actionSer->AddAction(e.GetChanged(), e.GetUser().value());
        break;
    }
    case Events::ActionFields::REMOVE:
        // Old contains the previous action, new will be empty
        m_actionSer->RemoveAction(e.GetOld().GetId(), e.GetUser().value());
        break;
    case Events::ActionFields::NAME:
    case Events::ActionFields::ICON:
    case Events::ActionFields::COLOR:
    case Events::ActionFields::VISIBLE:
        m_actionSer->AddActionOnly(e.GetChanged(), e.GetUser().value());
        break;
    }
}

void DBEventHandler::HandleRuleChangeEvent(const Events::RuleChangeEvent& e)
{
    switch (e.GetChangedFields())
    {
    case Events::RuleFields::ADD:
    {
        if (e.GetChanged().GetId() == 0)
        {
            // Add the rule first, then fire the event again with changed ids
            Rule copy = e.GetChanged();
            m_ruleSer->AddRule(copy, e.GetUser().value());
            if (copy.GetId() != 0)
            {
                // Extra check to avoid infinite recursion if something goes wrong
                // TODO: Find a way to avoid adding Rules twice (again with ALL event)
                Events::RuleChangeEvent e2(e.GetOld(), copy, Events::RuleFields::ALL);
                Res::EventSystem().HandleEvent(e2);
            }
        }
        else
        {
            Rule copy = e.GetChanged();
            m_ruleSer->AddRule(copy, e.GetUser().value());
        }
        break;
    }
    case Events::RuleFields::ALL:
    case Events::RuleFields::CONDITION:
    case Events::RuleFields::EFFECT:
    {
        Rule copy(e.GetChanged());
        m_ruleSer->AddRule(copy, e.GetUser().value());
        break;
    }
    case Events::RuleFields::REMOVE:
        // Old contains the previous rule, new will be empty
        m_ruleSer->RemoveRule(e.GetOld(), e.GetUser().value());
        break;
    case Events::RuleFields::NAME:
    case Events::RuleFields::ICON:
    case Events::RuleFields::COLOR:
    case Events::RuleFields::ENABLED:
        m_ruleSer->AddRuleOnly(e.GetChanged(), e.GetUser().value());
        break;
    }
}

/*
void DBEventHandler::HandleSensorChangeEvent(const Events::DeviceChangeEvent& e)
{
    CompressSensorLog();
    if (e.GetChanged().m_sensor.m_type == Types::TEMPERATURE_DIG)
    {
        // Average out fluctuations
        try
        {
            long long val = std::stoi(e.GetChanged().m_sensor.m_state);
            DBSavepoint savepoint = m_dbHandler->GetSavepoint("DBEventHandler_HandleSensorChangeEvent");
            {
                DBResult result
                    = m_dbHandler
                          ->GetStatement(
                              "SELECT changetime, val, rowid FROM sensor_log WHERE sensor_uid=(SELECT sensor_uid FROM "
                              "sensors WHERE node_id=?1 AND sensor_id=?2) ORDER BY changetime DESC LIMIT 2;")
                          .Execute({e.GetChanged().m_nodeId, e.GetChanged().m_property});
                std::vector<std::pair<long long, long long>> vals;
                if (result.NextRow())
                {
                    vals.push_back(std::make_pair(result.GetColumnInt64(0), result.GetColumnInt64(1)));
                    long long rowid = result.GetColumnInt64(2);
                    if (result.NextRow())
                    {
                        vals.push_back(std::make_pair(result.GetColumnInt64(0), result.GetColumnInt64(1)));

                        // If 2nd last val is +- 0.1°C than 3rd val and less than 20s older than it, average it using
                        // the three points
                        if (vals[0].second > (vals[1].second - 10) && vals[0].second < (vals[1].second + 10)
                            && vals[0].first < (vals[1].first + 300))
                        {
                            vals[0].second = static_cast<long long>(
                                std::round(vals[1].second / 3.0 + vals[0].second / 3.0 + val / 3.0));
                            m_dbHandler->GetStatement("UPDATE sensor_log SET val = ?1 WHERE rowid = ?2;")
                                .ExecuteAll({vals[0].second, rowid});
                        }
                    }
                }
            }
            savepoint.Release();
        }
        catch (const std::exception& e)
        {
            Res::Logger().Warning(std::string("Exception in HandleSensorChangeEvent: ") + e.what());
        }
    }
    // Update the sensor
    m_nodeSer->GetSensorSerialize().UpdateSensor(
        e.GetChanged().m_nodeId, e.GetChanged().m_property, e.GetChanged().m_sensor);
}

void DBEventHandler::HandleActorChangeEvent(const Events::ActorChangeEvent& e)
{
    // This only needs to update the Actor, regardless of the type
    m_nodeSer->GetActorSerialize().UpdateActor(
        e.GetChanged().m_nodeId, e.GetChanged().m_actorId, e.GetChanged().m_actor);
}
*/
void DBEventHandler::CompressSensorLog()
{ /*
     DBSavepoint savepoint = m_dbHandler->GetSavepoint("DBEventHandler_CompressSensorLog");
     DBResult result
         = m_dbHandler
               ->GetStatement("SELECT changetime FROM sensor_log JOIN sensor_log_compressed ON sensor_log.rowid = "
                              "sensor_log_compressed.sensor_log_id ORDER BY changetime DESC LIMIT 1;")
               .Execute();
     std::time_t startTime = 0;
     if (result.NextRow())
     {
         startTime = result.GetColumnInt64(0);
     }
     if (startTime
         < (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() + std::chrono::seconds(120))))
     {
         result = m_dbHandler
                      ->GetStatement(
                          "SELECT changetime, val FROM sensor_log WHERE changetime >= ?1 ORDER BY changetime ASC;")
                      .Execute({static_cast<long long>(startTime)});
         std::vector<std::pair<long long, long long>> results;
         while (result.NextRow())
         {
             results.emplace_back(result.GetColumnInt64(0), result.GetColumnInt64(1));
         }

         std::vector<std::pair<long long, long long>> compressedValues
             = rdp<std::vector<std::pair<long long, long long>>>(results.begin(), results.end(), 50.0);
         if (!compressedValues.empty())
         {
             DBStatement statement
                 = m_dbHandler->GetStatement("INSERT INTO sensor_log_compressed(sensor_log_id) SELECT rowid FROM "
                                             "sensor_log WHERE changetime = ?1 AND val = ?2;");
             for (const auto& p : compressedValues)
             {
                 statement.ExecuteAll({p.first, p.second});
                 statement.Reset();
             }
         }
     }
     savepoint.Release();*/
}

void DBEventHandler::CompressActorLog() {}
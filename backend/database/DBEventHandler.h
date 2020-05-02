#ifndef _DB_EVENT_HANDLER_H
#define _DB_EVENT_HANDLER_H

#include "DBHandler.h"

#include "../api/IActionSerialize.h"
#include "../api/IRuleSerialize.h"
#include "../events/EventSystem.h"
#include "../events/Events.h"
class DBEventHandler : public EventHandler<EventBase>
{
public:
    DBEventHandler(DBHandler& dbHandler, IActionSerialize& actionSer, IRuleSerialize& ruleSer);

    // Updates changed Node/Action/Rule/Sensor/Actor in the database
    PostEventState HandleEvent(const EventBase& e) override;

    // void HandleNodeChangeEvent(const Events::NodeChangeEvent& e);
    void HandleActionChangeEvent(const Events::ActionChangeEvent& e);
    void HandleRuleChangeEvent(const Events::RuleChangeEvent& e);
    void HandleSensorChangeEvent(const Events::DeviceChangeEvent& e);
    // void HandleActorChangeEvent(const Events::ActorChangeEvent& e);
    void CompressSensorLog();
    void CompressActorLog();

private:
    DBHandler* m_dbHandler;
    IActionSerialize* m_actionSer;
    IRuleSerialize* m_ruleSer;
};

#endif
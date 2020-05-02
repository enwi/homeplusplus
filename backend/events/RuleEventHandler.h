#ifndef _RULE_EVENT_HANDLER_H
#define _RULE_EVENT_HANDLER_H
#include <condition_variable>
#include <functional>
#include <thread>

#include <assert.h>

#include "Events.h"

#include "../api/ActionStorage.h"
#include "../api/IRuleSerialize.h"
#include "../api/Resources.h"
#include "../communication/WebsocketCommunication.h"
#include "../database/DBHandler.h"

bool IsConditionTimeBased(const RuleConditions::RuleCondition& c);

std::chrono::system_clock::time_point NextExecutionTime(const RuleConditions::RuleCondition& c);

struct RuleExecutionTimeCompare
{
    bool operator()(const Rule& rhs, const Rule& lhs)
    {
        return NextExecutionTime(rhs.GetCondition()) > NextExecutionTime(lhs.GetCondition());
    }
};

// Checks if Rules are satisfied after Event occurred
class RuleEventHandler : public EventHandler<EventBase>
{
public:
    RuleEventHandler(const ActionStorage& actionStorage, WebsocketChannelAccessor notificationsChannel,
        DeviceRegistry& deviceReg, IRuleSerialize& ruleSer);

    ~RuleEventHandler();

    // Checks all Rules if they are satisfied after Event
    PostEventState HandleEvent(const EventBase& e) override;

private:
    // Returns true if changes were made
    bool CheckRules(const EventBase& e);
    void Run();

private:
    ActionStorage m_actionStorage;
    WebsocketChannelAccessor m_notificationsChannel;
    DeviceRegistry* m_deviceReg;
    IRuleSerialize* m_ruleSer;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_shutdownThread = false;
    std::vector<Rule> m_timedRules;
};

#endif

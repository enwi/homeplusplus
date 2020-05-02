#include "RuleEventHandler.h"

bool IsConditionTimeBased(const RuleConditions::RuleCondition& c)
{
    // TODO: Find out which type is actually RuleTimeCondition
    if (c.GetType() == 2)
    {
        return true;
    }
    else if (c.HasChilds())
    {
        for (const RuleConditions::RuleCondition* child : c.GetChilds())
        {
            if (IsConditionTimeBased(*child))
            {
                return true;
            }
        }
    }
    return false;
}

std::chrono::system_clock::time_point NextExecutionTime(const RuleConditions::RuleCondition& c)
{
    using std::chrono::system_clock;
    // TODO: Find out which type is actually RuleTimeCondition
    if (c.GetType() == 2)
    {
        return dynamic_cast<const RuleConditions::RuleTimeCondition&>(c).GetNextExecutionTime();
    }
    else
    {
        system_clock::time_point next {std::chrono::seconds(0)};
        for (const RuleConditions::RuleCondition* child : c.GetChilds())
        {
            system_clock::time_point t = NextExecutionTime(*child);
            if (t != system_clock::time_point(std::chrono::seconds(0)))
            {
                if (next != system_clock::time_point(std::chrono::seconds(0)))
                {
                    next = std::min(t, next);
                }
                else
                {
                    next = t;
                }
            }
        }
        return next;
    }
}

RuleEventHandler::RuleEventHandler(const ActionStorage& actionStorage, WebsocketChannelAccessor notificationsChannel,
    DeviceRegistry& deviceReg, IRuleSerialize& ruleSer)
    : m_actionStorage(actionStorage),
      m_notificationsChannel(notificationsChannel),
      m_deviceReg(&deviceReg),
      m_ruleSer(&ruleSer)
{
    // TODO: Change UserId to something like SystemUser
    std::vector<Rule> rules = m_ruleSer->GetAllRules(Filter(), UserId::Dummy());
    for (Rule& r : rules)
    {
        if (IsConditionTimeBased(r.GetCondition()))
        {
            m_timedRules.push_back(std::move(r));
        }
        std::make_heap(m_timedRules.begin(), m_timedRules.end(), RuleExecutionTimeCompare {});
    }
    m_thread = std::thread(&RuleEventHandler::Run, this);
}

RuleEventHandler::~RuleEventHandler()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shutdownThread = true;
    }
    m_cv.notify_all();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

// Checks all Rules if they are satisfied after Event
PostEventState RuleEventHandler::HandleEvent(const EventBase& e)
{
    // Do not Check rules in the background, because it results in weird errors
    // std::thread t(&RuleEventHandler::CheckRules, this, e.Clone());
    // t.detach();
    bool handled = false;
    if (e.GetType() == EventTypes::ruleChange)
    {
        // Check rule again, because it might have changed
        const Events::RuleChangeEvent& casted = EventCast<Events::RuleChangeEvent>(e);
        // If the condition changed, check to see if it is satisfied now
        if (casted.GetChangedFields() == Events::RuleFields::CONDITION)
        {
            if (casted.GetChanged().IsSatisfied())
            {
                auto notificationsChannel = m_notificationsChannel.Get();
                handled = true;
                // TODO: Find out who the rule belongs to
                casted.GetChanged().GetEffect().Execute(
                    m_actionStorage, notificationsChannel, *m_deviceReg, UserId::Dummy());
            }
            if (IsConditionTimeBased(casted.GetChanged().GetCondition()))
            {
                handled = true;
                std::lock_guard<std::mutex> lock(m_mutex);

                auto it = std::find_if(m_timedRules.begin(), m_timedRules.end(),
                    [id = casted.GetChanged().GetId()](const Rule& r) { return r.GetId() == id; });
                if (it != m_timedRules.end())
                {
                    *it = casted.GetChanged();
                }
                else
                {
                    m_timedRules.push_back(casted.GetChanged());
                }
                std::make_heap(m_timedRules.begin(), m_timedRules.end(), RuleExecutionTimeCompare {});

                if (m_timedRules.front().GetId() == casted.GetChanged().GetId())
                {
                    // Notify worker thread that there is a new rule with a new lowest execution time
                    m_cv.notify_all();
                }
            }
        }
        else if (casted.GetChangedFields() == Events::RuleFields::ADD
            && IsConditionTimeBased(casted.GetChanged().GetCondition()))
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = std::find_if(m_timedRules.begin(), m_timedRules.end(),
                [id = casted.GetChanged().GetId()](const Rule& r) { return r.GetId() == id; });
            if (it != m_timedRules.end())
            {
                *it = casted.GetChanged();
            }
            else
            {
                m_timedRules.push_back(casted.GetChanged());
            }
            std::make_heap(m_timedRules.begin(), m_timedRules.end(), RuleExecutionTimeCompare {});
            if (m_timedRules.front().GetId() == casted.GetChanged().GetId())
            {
                // Notify worker thread that there is a new rule with a new lowest execution time
                m_cv.notify_all();
            }
        }
        else if (casted.GetChangedFields() == Events::RuleFields::REMOVE)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = std::find_if(m_timedRules.begin(), m_timedRules.end(),
                [id = casted.GetChanged().GetId()](const Rule& r) { return r.GetId() == id; });
            if (it != m_timedRules.end())
            {
                m_timedRules.erase(it);
            }
            std::make_heap(m_timedRules.begin(), m_timedRules.end(), RuleExecutionTimeCompare {});
        }
        CheckRules(e);
    }
    else if (e.GetType() == EventTypes::devicePropertyChange)
    {
        handled = CheckRules(e);
    }
    return handled ? PostEventState::handled : PostEventState::notHandled;
}

bool RuleEventHandler::CheckRules(const EventBase& e)
{
    bool handled = false;
    // Has to be shared_ptr, otherwise e will be destroyed as this method runs asynchronously
    // Wait until this event was definitely handled completely
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // No, see HandleEvent()
    Res::Logger().Debug("Checking rules");
    // TODO: Make this EFFICIENT, only check rules if the right event fired
    std::vector<Rule> rules = m_ruleSer->GetAllRules(Filter(), UserId::Dummy());
    for (auto it = rules.begin(); it != rules.end(); ++it)
    {
        if (!it->HasCondition())
        {
            Res::Logger().Error("Rule without condition!");
            continue;
        }
        // Check if it should execute for that event and check if it is satisfied
        if (it->IsEnabled() && it->GetCondition().ShouldExecuteOn(e.GetType())
            && it->GetCondition().IsSatisfiedAfterEvent(e))
        {
            auto notificationsChannel = m_notificationsChannel.Get();
            handled = true;
            // TODO: Use creator of rule as user
            it->GetEffect().Execute(m_actionStorage, notificationsChannel, *m_deviceReg, UserId::Dummy());
        }
    }
    return handled;
}

void RuleEventHandler::Run()
{
    using std::chrono::system_clock;
    std::unique_lock<std::mutex> lock(m_mutex);
    system_clock::time_point nextCheck {std::chrono::seconds(0)};
    while (!m_shutdownThread)
    {
        if (!m_timedRules.empty())
        {
            const Rule& r = m_timedRules.front();
            if (r.IsSatisfied())
            {
                Rule copy = std::move(m_timedRules.front());
                std::pop_heap(m_timedRules.begin(), m_timedRules.end(), RuleExecutionTimeCompare {});
                m_timedRules.pop_back();

                auto notificationsChannel = m_notificationsChannel.Get();
                lock.unlock();
                try
                {
                    // Unlock while effects are processed
                    // TODO: Use creator of rule as user
                    copy.GetEffect().Execute(m_actionStorage, notificationsChannel, *m_deviceReg, UserId::Dummy());
                }
                catch (const std::exception& e)
                {
                    Res::Logger().Error(
                        "RuleEventHandler", std::string("Exception while executing timed rule effect: ") + e.what());
                }
                lock.lock();

                // If rule was one-time only, do not add it back
                if (NextExecutionTime(copy.GetCondition()) > system_clock::now())
                {
                    m_timedRules.push_back(std::move(copy));
                    std::push_heap(m_timedRules.begin(), m_timedRules.end(), RuleExecutionTimeCompare {});
                }
            }
            nextCheck = NextExecutionTime(m_timedRules.front().GetCondition());
            Res::Logger().Debug("[RuleEventHandler] Waiting for"
                + std::to_string(
                    std::chrono::duration_cast<std::chrono::seconds>(nextCheck - system_clock::now()).count())
                + "s");
        }
        else
        {
            nextCheck = system_clock::time_point(std::chrono::seconds(0));
            Res::Logger().Debug("[RuleEventHandler] Waiting for timed rules");
        }
        if (nextCheck > system_clock::now())
        {
            // No predicate, spurious wakeups dont matter
            m_cv.wait_until(lock, nextCheck);
        }
        else if (nextCheck.time_since_epoch() == std::chrono::seconds(0))
        {
            // No rule should be checked
            // No predicate, spurious wakeups dont matter
            m_cv.wait(lock);
        }
        // Otherwise continue with next rule
    }
}

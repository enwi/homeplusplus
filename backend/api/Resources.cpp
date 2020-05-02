#include "Resources.h"

#include "Action.h"
#include "Rule.h"

#include "../events/EventSystem.h"
#include "../utility/Logger.h"

EventSystem& Res::EventSystem()
{
    static ::EventSystem evSys {};
    return evSys;
}

Logger& Res::Logger()
{
    static ::Logger logger {};
    return logger;
}

RuleConditions::Registry& Res::ConditionRegistry()
{
    static RuleConditions::Registry registry {};
    return registry;
}

SubActionRegistry& Res::ActionRegistry()
{
    static SubActionRegistry registry {};
    return registry;
}

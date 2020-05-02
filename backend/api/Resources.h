#ifndef _RESOURCES_H
#define _RESOURCES_H

// Forward declaration
namespace RuleConditions
{
    class Registry;
} // namespace RuleConditions

// Static resource class
class Res
{
public:
    // Globale EventSystem instance
    static class EventSystem& EventSystem();
    // Global Logger instance
    static class Logger& Logger();
    // Global RuleCondition registry
    static RuleConditions::Registry& ConditionRegistry();
    // Global Action registry
    static class SubActionRegistry& ActionRegistry();

private:
    Res() {}
    Res(const Res& /*unused*/) = delete;
};

#endif
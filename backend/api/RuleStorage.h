#pragma once

#include <absl/types/optional.h>

#include "IRuleSerialize.h"
#include "Rule.h"
#include "RulePermissions.h"
#include "User.h"

#include "../events/Events.h"

class RuleStorage
{
public:
    // The references must be valid for the lifetime of the instance or any copies
    RuleStorage(IRuleSerialize& ruleSerialize, EventEmitter<Events::RuleChangeEvent>& eventEmitter)
        : m_ruleSerialize(&ruleSerialize), m_eventEmitter(&eventEmitter)
    {}

    void AddRule(Rule rule, UserId user);
    void UpdateRule(Rule rule, UserId user);
    void UpdateRuleHeader(const Rule& rule, UserId user);

    std::vector<Rule> GetAllRules(const Filter& filter, UserId user);
    absl::optional<Rule> GetRule(uint64_t ruleId, UserId user);

    void RemoveRule(uint64_t ruleId, UserId user);

private:
    IRuleSerialize* m_ruleSerialize;
    EventEmitter<Events::RuleChangeEvent>* m_eventEmitter;
};
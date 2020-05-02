#ifndef _I_RULE_SERIALIZE_H
#define _I_RULE_SERIALIZE_H

#include "Filter.h"
#include "Rule.h"
#include "User.h"

// Tag type to ensure a transaction persists
class UserHeldTransaction;

class IRuleConditionSerialize
{
public:
    virtual ~IRuleConditionSerialize() = default;

    // Adds the RuleCondition, modifies its id
    virtual void AddRuleCondition(RuleConditions::RuleCondition& condition, UserId user) = 0;
    virtual void AddRuleCondition(RuleConditions::RuleCondition& condition, const UserHeldTransaction&) = 0;
    // Returns the RuleCondition with the given id
    virtual RuleConditions::Ptr GetRuleCondition(uint64_t conditionId, UserId user) const = 0;
    virtual RuleConditions::Ptr GetRuleCondition(uint64_t conditionId, const UserHeldTransaction&) const = 0;

    // Inserts/updates condition. Modifies condition's and chilren's ids
    virtual void InsertRuleCondition(RuleConditions::RuleCondition& condition, UserId user) = 0;
    virtual void InsertRuleCondition(RuleConditions::RuleCondition& condition, const UserHeldTransaction&) = 0;
    // Removes condition
    virtual void RemoveRuleCondition(const RuleConditions::RuleCondition& condition, UserId user) = 0;
    virtual void RemoveRuleCondition(const RuleConditions::RuleCondition& condition, const UserHeldTransaction&) = 0;
};

class IRuleSerialize
{
public:
    virtual ~IRuleSerialize() = default;

    // Returns the Rule with the given id
    virtual absl::optional<Rule> GetRule(uint64_t ruleId, UserId user) const = 0;
    virtual absl::optional<Rule> GetRule(uint64_t ruleId, const UserHeldTransaction&) const = 0;
    // Returns all rules. Careful when there are too many of them
    virtual std::vector<Rule> GetAllRules(const Filter& filter, UserId user) const = 0;
    virtual std::vector<Rule> GetAllRules(const Filter& filter, const UserHeldTransaction&) const = 0;
    // Adds a Rule, modifies the rule with updated ids
    virtual void AddRule(Rule& rule, UserId user) = 0;
    virtual void AddRule(Rule& rule, const UserHeldTransaction&) = 0;
    // Updates a Rule, modifies the rule with updated ids
    virtual void UpdateRule(Rule& rule, UserId user) = 0;
    virtual void UpdateRule(Rule& rule, const UserHeldTransaction&) = 0;
    // Updates a Rule without updating RuleConditions and Actions. Faster than AddRule()
    virtual uint64_t AddRuleOnly(const Rule& rule, UserId user) = 0;
    virtual uint64_t AddRuleOnly(const Rule& rule, const UserHeldTransaction&) = 0;
    // Removes the Rule and all RuleConditions
    virtual void RemoveRule(uint64_t ruleId, UserId user) = 0;
    virtual void RemoveRule(uint64_t ruleId, const UserHeldTransaction&) = 0;
    // Removes the Rule and all RuleConditions. Faster than RemoveRule(id)
    virtual void RemoveRule(const Rule& rule, UserId user) = 0;
    virtual void RemoveRule(const Rule& rule, const UserHeldTransaction&) = 0;

    // Returns the IRuleConditionSerialize used to get conditions for the rules
    virtual IRuleConditionSerialize& GetConditionSerialize() = 0;
    // Returns the IRuleConditionSerialize used to get conditions for the rules, const version
    virtual const IRuleConditionSerialize& GetConditionSerialize() const = 0;
};

#endif
#ifndef DB_RULE_SERIALIZE_H
#define DB_RULE_SERIALIZE_H

#include "HeldTransaction.h"
#include "RulesTable.h"

#include "../api/IActionSerialize.h"
#include "../api/IRuleSerialize.h"
#include "../api/RulePermissions.h"

// Functions to save/load RuleConditions to/from database
class DBRuleConditionSerialize : public IRuleConditionSerialize
{
public:
    explicit DBRuleConditionSerialize(DBHandler& dbHandler) : m_dbHandler(dbHandler) {}

    // Adds the RuleCondition, modifies its id
    void AddRuleCondition(RuleConditions::RuleCondition& condition, UserId user) override;
    void AddRuleCondition(RuleConditions::RuleCondition& condition, const UserHeldTransaction&) override;
    // Returns the RuleCondition with the given id
    RuleConditions::Ptr GetRuleCondition(uint64_t conditionId, UserId user) const override;
    RuleConditions::Ptr GetRuleCondition(uint64_t conditionId, const UserHeldTransaction&) const override;

    // Inserts/updates condition. Modifies condition's and chilren's ids
    void InsertRuleCondition(RuleConditions::RuleCondition& condition, UserId user) override;
    void InsertRuleCondition(RuleConditions::RuleCondition& condition, const UserHeldTransaction&) override;
    // Removes condition
    void RemoveRuleCondition(const RuleConditions::RuleCondition& condition, UserId user) override;
    void RemoveRuleCondition(const RuleConditions::RuleCondition& condition, const UserHeldTransaction&) override;

private:
    DBHandler& m_dbHandler;
};

// Functions to save/load Rules to/from database
class DBRuleSerialize : public IRuleSerialize
{
public:
    using RuleSelectResult
        = decltype(GetSelectResult(RulesTable(), RulesTable().ruleId, RulesTable().ruleName, RulesTable().ruleIconName,
            RulesTable().ruleColor, RulesTable().conditionId, RulesTable().actionId, RulesTable().ruleEnabled));

public:
    DBRuleSerialize(DBHandler& dbHandler, IActionSerialize& actionSer)
        : m_dbHandler(dbHandler), m_actionSerialize(actionSer), m_condSerialize(dbHandler)
    {}

    // Returns the Rule with the given id
    absl::optional<Rule> GetRule(uint64_t ruleId, UserId user) const override;
    absl::optional<Rule> GetRule(uint64_t ruleId, const UserHeldTransaction&) const override;
    // Returns all rules. Careful when there are too many of them
    std::vector<Rule> GetAllRules(const Filter& filter, UserId user) const override;
    std::vector<Rule> GetAllRules(const Filter& filter, const UserHeldTransaction&) const override;
    // Adds/updates a Rule, modifies the rule with updated ids
    void AddRule(Rule& rule, UserId user) override;
    void AddRule(Rule& rule, const UserHeldTransaction&) override;
    void UpdateRule(Rule& rule, UserId user) override;
    void UpdateRule(Rule& rule, const UserHeldTransaction&) override;
    // Updates a Rule without updating RuleConditions and Actions. Faster than AddRule()
    uint64_t AddRuleOnly(const Rule& rule, UserId user) override;
    uint64_t AddRuleOnly(const Rule& rule, const UserHeldTransaction&) override;
    // Removes the Rule and all RuleConditions
    void RemoveRule(uint64_t ruleId, UserId user) override;
    void RemoveRule(uint64_t ruleId, const UserHeldTransaction&) override;
    // Removes the Rule and all RuleConditions. Faster than RemoveRule(id)
    void RemoveRule(const Rule& rule, UserId user) override;
    void RemoveRule(const Rule& rule, const UserHeldTransaction&) override;

    const DBRuleConditionSerialize& GetConditionSerialize() const override { return m_condSerialize; }
    DBRuleConditionSerialize& GetConditionSerialize() override { return m_condSerialize; }

    std::vector<Rule> GetRulesFromQuery(
        DBHandler::DatabaseConnection& db, RuleSelectResult result, const UserHeldTransaction&) const;

private:
    RulePermissions m_rulePermissions;
    DBHandler& m_dbHandler;
    IActionSerialize& m_actionSerialize;
    DBRuleConditionSerialize m_condSerialize;
};

#endif
#include "RuleStorage.h"

void RuleStorage::AddRule(Rule rule, UserId user)
{
    m_ruleSerialize->AddRule(rule, user);
    m_eventEmitter->EmitEvent(Events::RuleChangeEvent(Rule(), rule, Events::RuleFields::ADD));
}

void RuleStorage::UpdateRule(Rule rule, UserId user)
{
	m_ruleSerialize->UpdateRule(rule, user);
	// TODO: Pass proper old rule or remove the field
	m_eventEmitter->EmitEvent(Events::RuleChangeEvent(Rule(), rule, Events::RuleFields::ALL));
}

void RuleStorage::UpdateRuleHeader(const Rule& rule, UserId user)
{
    m_ruleSerialize->AddRuleOnly(rule, user);
    m_eventEmitter->EmitEvent(Events::RuleChangeEvent(Rule(), rule, Events::RuleFields::NAME));
}

std::vector<Rule> RuleStorage::GetAllRules(const Filter& filter, UserId user)
{
    return m_ruleSerialize->GetAllRules(filter, user);
}

absl::optional<Rule> RuleStorage::GetRule(uint64_t ruleId, UserId user)
{
    return m_ruleSerialize->GetRule(ruleId, user);
}

void RuleStorage::RemoveRule(uint64_t ruleId, UserId user)
{
    m_ruleSerialize->RemoveRule(ruleId, user);

    // TODO: Pass proper action for remove
    m_eventEmitter->EmitEvent(
        Events::RuleChangeEvent(Rule(ruleId, "2", "", 0, 0, Action()), Rule(), Events::RuleFields::REMOVE));
}

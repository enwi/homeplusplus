#ifndef MOCK_RULE_SERIALIZE_H
#define MOCK_RULE_SERIALIZE_H

#include <gmock/gmock.h>

#include "api/IRuleSerialize.h"
#include "database/HeldTransaction.h"

class MockConditionSerialize : public IRuleConditionSerialize
{
public:
    MOCK_METHOD2(AddRuleCondition, void(RuleConditions::RuleCondition&, UserId));
	MOCK_METHOD2(AddRuleCondition, void(RuleConditions::RuleCondition&, const UserHeldTransaction&));
    MOCK_CONST_METHOD2(GetRuleCondition, RuleConditions::Ptr(uint64_t, UserId));
	MOCK_CONST_METHOD2(GetRuleCondition, RuleConditions::Ptr(uint64_t, const UserHeldTransaction&));
    MOCK_METHOD2(InsertRuleCondition, void(RuleConditions::RuleCondition&, UserId));
	MOCK_METHOD2(InsertRuleCondition, void(RuleConditions::RuleCondition&, const UserHeldTransaction&));
    MOCK_METHOD2(RemoveRuleCondition, void(const RuleConditions::RuleCondition&, UserId));
	MOCK_METHOD2(RemoveRuleCondition, void(const RuleConditions::RuleCondition&, const UserHeldTransaction&));
};

class MockRuleSerialize : public IRuleSerialize
{
public:
    MOCK_CONST_METHOD2(GetRule, absl::optional<Rule>(uint64_t, UserId));
	MOCK_CONST_METHOD2(GetRule, absl::optional<Rule>(uint64_t, const UserHeldTransaction&));
    MOCK_CONST_METHOD2(GetAllRules, std::vector<Rule>(const Filter&, UserId));
	MOCK_CONST_METHOD2(GetAllRules, std::vector<Rule>(const Filter&, const UserHeldTransaction&));
    MOCK_METHOD2(AddRule, void(Rule&, UserId));
	MOCK_METHOD2(AddRule, void(Rule&, const UserHeldTransaction&));
	MOCK_METHOD2(UpdateRule, void(Rule&, UserId));
	MOCK_METHOD2(UpdateRule, void(Rule&, const UserHeldTransaction&));
    MOCK_METHOD2(AddRuleOnly, uint64_t(const Rule&, UserId));
	MOCK_METHOD2(AddRuleOnly, uint64_t(const Rule&, const UserHeldTransaction&));
    MOCK_METHOD2(RemoveRule, void(uint64_t, UserId));
	MOCK_METHOD2(RemoveRule, void(uint64_t, const UserHeldTransaction&));
    MOCK_METHOD2(RemoveRule, void(const Rule&, UserId));
	MOCK_METHOD2(RemoveRule, void(const Rule&, const UserHeldTransaction&));
    MOCK_METHOD0(GetConditionSerialize, IRuleConditionSerialize&());
    MOCK_CONST_METHOD0(GetConditionSerialize, const IRuleConditionSerialize&());
};

#endif
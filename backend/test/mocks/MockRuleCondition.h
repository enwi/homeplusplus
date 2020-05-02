#ifndef MOCK_RULE_CONDITION_H
#define MOCK_RULE_CONDITION_H

#include <gmock/gmock.h>

#include "JsonWrapper.h"

#include "api/IRuleSerialize.h"
#include "api/Rule.h"

class MockRuleCondition : public RuleConditions::RuleCondition
{
public:
    explicit MockRuleCondition(std::size_t type = 0) : RuleCondition(type) {}

    MOCK_METHOD4(Parse,
        void(const IRuleConditionSerialize& condSer, const RuleConditions::Registry& registry,
            const RuleConditionsRow& result, const UserHeldTransaction&));
    void Parse(const RuleConditions::Registry& registry, const nlohmann::json& json) override
    {
        ParseImpl(registry, JsonWrapper(json));
    }
    MOCK_METHOD2(ParseImpl, void(const RuleConditions::Registry& registry, const JsonWrapper& json));
    MOCK_CONST_METHOD0(HasChilds, bool());
    MOCK_CONST_METHOD0(GetChilds, std::vector<const RuleCondition*>());
    MOCK_METHOD0(GetChilds, std::vector<RuleCondition*>());
    MOCK_CONST_METHOD0(GetFields, std::array<int64_t, 5>());
    MOCK_CONST_METHOD0(IsSatisfied, bool());
	nlohmann::json ToJson() const { return ToJsonImpl().get(); }
    MOCK_CONST_METHOD0(ToJsonImpl, JsonWrapper());
    MOCK_CONST_METHOD1(Clone, Ptr(const RuleConditions::Registry&));
    MOCK_CONST_METHOD1(Create, Ptr(const RuleConditions::Registry&));
    MOCK_CONST_METHOD1(ShouldExecuteOn, bool(EventType));
    MOCK_CONST_METHOD1(IsSatisfiedAfterEvent, bool(const EventBase&));
};

#endif
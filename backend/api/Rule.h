#ifndef _RULE_H
#define _RULE_H
#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <json.hpp>

#include "Action.h"
#include "Device.h"

#include "api/rule.pb.h"

#include "../database/DBHandler.h"
#include "../database/RulesTable.h"
#include "../events/EventSystem.h"
#include "../utility/FactoryRegistry.h"

class IRuleConditionSerialize;
class UserHeldTransaction;

namespace RuleConditions
{
    // Base class for RuleConditions
    class RuleCondition
    {
    public:
        typedef std::unique_ptr<RuleCondition> Ptr;

        using RuleConditionsRow = decltype(GetSelectRow(RuleConditionsTable(), RuleConditionsTable().conditionId,
            RuleConditionsTable().conditionType, RuleConditionsTable().conditionData));

    public:
        // Construct from type
        explicit RuleCondition(uint64_t type = 0) : m_id(0), m_type(type) {}
        virtual ~RuleCondition() = default;

        // Parse from DBResult
        virtual void Parse(const IRuleConditionSerialize& condSer, const class Registry& registry,
            const RuleConditionsRow& result, const UserHeldTransaction&);
        // Parse from Json
        virtual void Parse(const class Registry& registry, const nlohmann::json& json);
		virtual void Deserialize(const class Registry& registry, const messages::RuleCondition& msg);

        // Returns true if there are child conditions
        virtual bool HasChilds() const { return false; }
        // Returns child conditions
        virtual std::vector<const RuleCondition*> GetChilds() const { return std::vector<const RuleCondition*>(); }
        // Returns mutable list of child conditions
        virtual std::vector<RuleCondition*> GetChilds() { return std::vector<RuleCondition*>(); }
        // Returns true if condition is met
        virtual bool IsSatisfied() const { return false; }
        // Returns id of this condition. Id of 0 means not assigned yet
        virtual uint64_t GetId() const { return m_id; }
        // Returns type of this condition
        virtual uint64_t GetType() const { return m_type; }
        // Set id
        virtual void SetId(uint64_t id) { m_id = id; }

        // Returns Json representation which this condition can be parsed from
        virtual nlohmann::json ToJson() const;
		// IncludeChildren determines whether children are completely serialized or only saved as id (used in database)
		// messages serialized without includeChildren cannot be deserialized (currently)
		virtual messages::RuleCondition Serialize(bool includeChildren) const;

        // Returns a new instance of child class
        virtual Ptr Clone(const class Registry& registry) const;
        // Copies child class
        virtual Ptr Create(const class Registry& registry) const;

        // Returns true if IsSatisfiedAfterEvent() should be called for type
        virtual bool ShouldExecuteOn(EventType type) const = 0;
        // Checks if condition is met after the event
        virtual bool IsSatisfiedAfterEvent(const EventBase& e) const = 0;

    protected:
        uint64_t m_id;
        uint64_t m_type;
    };

    typedef RuleCondition::Ptr Ptr;

    // Holds factory function and config filename for RuleConditions
    struct RuleConditionInfo
    {
        // Factory function, argument is type
        std::function<Ptr(uint64_t)> function;
        // Name
        std::string name;
        // Construct empty
        RuleConditionInfo() = default;
        // Construct from nullptr, same as default
        RuleConditionInfo(std::nullptr_t /*unused*/) {}
        // Construct from factory function and config filename
        RuleConditionInfo(std::function<Ptr(uint64_t)> function, std::string name)
            : function(function), name(std::move(name))
        {}
        // Required to determine if not set
        bool operator==(std::nullptr_t /*unused*/) const { return function == nullptr; }
        bool operator!=(std::nullptr_t /*unused*/) const { return !(*this == nullptr); }
    };

    // Registry for RuleConditions
    class Registry : public FactoryRegistry<RuleConditionInfo>
    {
    public:
        // Registers default conditions (ids 0-3). Must be called once before anything else is registered
        void RegisterDefaultConditions();
        // Creates RuleCondition of given type
        Ptr GetCondition(uint64_t type) const;
        // Result format: id, type, val1, val2, comp, additional1, additional2
        Ptr ParseCondition(const IRuleConditionSerialize& condSer, const RuleCondition::RuleConditionsRow& result,
            const UserHeldTransaction&) const;
        // Parse condition from Json
        Ptr ParseCondition(const nlohmann::json& json) const;
		Ptr DeserializeCondition(const messages::RuleCondition& msg) const;
        // Returns all registered RuleConditions
        const std::vector<RuleConditionInfo>& GetRegistered() const;
    };

    // Constant condition which is always true or false
    class RuleConstantCondition : public RuleCondition
    {
    public:
        explicit RuleConstantCondition(uint64_t type = 0, bool state = false) : RuleCondition(type), m_state(state) {}

        void Parse(const IRuleConditionSerialize& condSer, const Registry& registry, const RuleConditionsRow& result,
            const UserHeldTransaction&) override;
        void Parse(const Registry& registry, const nlohmann::json& json) override;
		void Deserialize(const class Registry& registry, const messages::RuleCondition& msg) override;

        bool IsSatisfied() const override { return m_state; }

        nlohmann::json ToJson() const override;
		messages::RuleCondition Serialize(bool includeChildren) const override;

        Ptr Clone(const Registry& registry) const override;
        bool ShouldExecuteOn(EventType /*type*/) const override { return true; }
        bool IsSatisfiedAfterEvent(const EventBase& /*e*/) const override { return IsSatisfied(); }

    private:
        bool m_state;
    };

    // Condition to logically link two child conditions
    class RuleCompareCondition : public RuleCondition
    {
    public:
        enum class Operator
        {
            AND = 0,
            OR,
            NAND,
            NOR,
            // EQUAL = XNOR
            EQUAL,
            // NOT_EQUAL = XOR
            NOT_EQUAL
        };

    public:
        explicit RuleCompareCondition(uint64_t type = 0) : RuleCompareCondition(type, nullptr, nullptr, Operator::AND)
        {}
        RuleCompareCondition(uint64_t type, Ptr&& left, Ptr&& right, Operator compare)
            : RuleCondition(type), m_left(std::move(left)), m_right(std::move(right)), m_compare(compare)
        {}

        void Parse(const IRuleConditionSerialize& condSer, const Registry& registry, const RuleConditionsRow& result,
            const UserHeldTransaction&) override;
        void Parse(const Registry& registry, const nlohmann::json& json) override;
		void Deserialize(const class Registry& registry, const messages::RuleCondition& msg) override;

        bool HasChilds() const override { return m_left != nullptr || m_right != nullptr; }

        std::vector<const RuleCondition*> GetChilds() const override;

        std::vector<RuleCondition*> GetChilds() override;

        bool IsSatisfied() const override;

        nlohmann::json ToJson() const override;
		messages::RuleCondition Serialize(bool includeChildren) const override;

        Ptr Clone(const Registry& registry) const override;

        bool ShouldExecuteOn(EventType type) const override
        {
            return (m_left != nullptr ? m_left->ShouldExecuteOn(type) : false)
                || (m_right != nullptr ? m_right->ShouldExecuteOn(type) : false);
        }
        bool IsSatisfiedAfterEvent(const EventBase& e) const override;

    private:
        Ptr m_left;
        Ptr m_right;
        Operator m_compare;
    };

    // Condition checking current time
    class RuleTimeCondition : public RuleCondition
    {
    public:
        enum class Operator
        {
            equals = 0,
            notEquals,
            // Greater and less ignore time2
            greater,
            // Greater and less ignore time2
            less,
            inRange
        };
        enum class Type
        {
            // saved in int: hour * 3600 + min * 60 + sec
            hourMinSec = 0,
            hour,
            dayWeek,
            dayMonth,
            dayYear,
            month,
            year,
            // Time value
            absolute
        };

    public:
        explicit RuleTimeCondition(uint64_t type = 0) : RuleTimeCondition(type, 0, 0, Operator::equals, Type::year) {}
        // Range: from t1 - t2, other: t1 +- t2
        RuleTimeCondition(uint64_t type, time_t time1, time_t time2, Operator compare, Type timeType)
            : RuleCondition(type), m_time1(time1), m_time2(time2), m_compare(compare), m_timeType(timeType)
        {}

        void Parse(const IRuleConditionSerialize& condSer, const Registry& registry, const RuleConditionsRow& result,
            const UserHeldTransaction&) override;
        void Parse(const Registry& registry, const nlohmann::json& json) override;
		void Deserialize(const class Registry& registry, const messages::RuleCondition& msg) override;

        bool IsSatisfied() const override;

        nlohmann::json ToJson() const override;
		messages::RuleCondition Serialize(bool includeChildren) const override;

        Ptr Clone(const Registry& registry) const override;

        bool ShouldExecuteOn(EventType /*type*/) const override { return true; }
        bool IsSatisfiedAfterEvent(const EventBase& /*e*/) const override { return IsSatisfied(); }
        std::chrono::system_clock::time_point GetNextExecutionTime() const;

    private:
        time_t m_time1;
        time_t m_time2;
        Operator m_compare;
        Type m_timeType;
    };

    // Condition checking sensor state
    class RuleDeviceCondition : public RuleCondition
    {
    public:
        enum class Operator
        {
            EQUALS = 0,
            NOT_EQUALS,
            GREATER,
            LESS,
            IN_RANGE
        };

    public:
        RuleDeviceCondition(uint64_t type = 0)
            : RuleCondition(type),
              m_deviceReg(nullptr),
              m_deviceId({}),
              m_val1(0),
              m_val2(0),
              m_compare(Operator::EQUALS)
        {}
        // Range: from v1 - v2, other: v1 +- v2
        RuleDeviceCondition(uint64_t type, DeviceRegistry& deviceReg, DeviceId deviceId, absl::string_view property,
            int val1, int val2, Operator compare)
            : RuleCondition(type),
              m_deviceReg(&deviceReg),
              m_deviceId(deviceId),
              m_property(property),
              m_val1(val1),
              m_val2(val2),
              m_compare(compare)
        {}

        void Parse(const IRuleConditionSerialize& condSer, const Registry& registry, const RuleConditionsRow& result,
            const UserHeldTransaction&) override;
        void Parse(const Registry& registry, const nlohmann::json& json) override;
		void Deserialize(const class Registry& registry, const messages::RuleCondition& msg) override;

        bool IsSatisfied() const override;

        nlohmann::json ToJson() const override;
		messages::RuleCondition Serialize(bool includeChildren) const override;

        Ptr Clone(const Registry& registry) const override;
        bool ShouldExecuteOn(EventType type) const override;
        bool IsSatisfiedAfterEvent(const EventBase& e) const override;

    private:
        bool Compare(int compareValue) const;

    private:
        DeviceRegistry* m_deviceReg;
        DeviceId m_deviceId;
        std::string m_property;
        int m_val1;
        int m_val2;
        Operator m_compare;
    };

} // namespace RuleConditions

class Rule
{
public:
    // Construct empty Rule, should not be used
    Rule() : Rule(0, "", "", 0, nullptr, Action()) {}
    // Construct Rule
    Rule(uint64_t id, std::string name, std::string icon, unsigned int color,
        std::unique_ptr<RuleConditions::RuleCondition>&& condition, Action effect, bool enabled = true)
        : m_id(id),
          m_name(std::move(name)),
          m_icon(std::move(icon)),
          m_color(color),
          m_condition(std::move(condition)),
          m_effect(std::move(effect)),
          m_enabled(enabled)
    {}
    // Copy Rule
    Rule(const Rule& other)
        : m_id(other.m_id),
          m_name(other.m_name),
          m_icon(other.m_icon),
          m_color(other.m_color),
          m_condition(nullptr),
          m_effect(other.m_effect),
          m_enabled(other.m_enabled)
    {
        if (other.m_condition)
        {
            m_condition = other.m_condition->Clone(Res::ConditionRegistry());
        }
    }
    Rule& operator=(const Rule& other)
    {
        m_id = other.m_id;
        m_name = other.m_name;
        m_icon = other.m_icon;
        m_color = other.m_color;
        m_effect = other.m_effect;
        m_enabled = other.m_enabled;
        m_condition.reset();
        if (other.m_condition)
        {
            m_condition = other.m_condition->Clone(Res::ConditionRegistry());
        }
        return *this;
    }
    Rule(Rule&& other) = default;
    Rule& operator=(Rule&& rhs) = default;

    // Checks if enabled and condition is satisfied
    bool IsSatisfied() const { return m_enabled && m_condition->IsSatisfied(); }

    // Returns Rule id. 0 means not assigned
    uint64_t GetId() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    const std::string& GetIcon() const { return m_icon; }
    unsigned int GetColor() const { return m_color; }
    // Returns condition. Condition must be set (not default-constructed or nullptr)
    const RuleConditions::RuleCondition& GetCondition() const
    {
        assert(m_condition != nullptr);
        return *m_condition;
    }
    // Returns if Rule is enabled. If disabled, IsSatisfied() always returns false
    bool IsEnabled() const { return m_enabled; }

    // Returns whether condition is set (save to call GetCondition())
    bool HasCondition() const { return m_condition != nullptr; }
    // Returns mutable condition. Condition must be set (not default-constructed or nullptr)
    RuleConditions::RuleCondition& GetCondition()
    {
        assert(m_condition != nullptr);
        return *m_condition;
    }
    // Returns the Action representing the effect
    const Action& GetEffect() const { return m_effect; }

    void SetId(uint64_t id) { m_id = id; }
    void SetName(std::string name) { m_name = std::move(name); }
    void SetIcon(std::string icon) { m_icon = std::move(icon); }
    void SetCondition(RuleConditions::Ptr&& condition) { m_condition = std::move(condition); }
    void SetEffect(Action effect) { m_effect = std::move(effect); }
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    // Returns Json of node
    nlohmann::json ToJson() const;
	messages::Rule Serialize() const;

    // Returns Rule parsed from Json
    static Rule Parse(const nlohmann::json& json);
	static Rule Deserialize(const messages::Rule& msg, const RuleConditions::Registry& condReg, 
		const SubActionRegistry& actionReg);

    // Only compares condition ids, not all fields
    bool operator==(const Rule& other) const;
    bool operator!=(const Rule& other) const { return !(*this == other); }

private:
    uint64_t m_id;
    std::string m_name;
    std::string m_icon;
    unsigned int m_color;
    RuleConditions::Ptr m_condition;
    Action m_effect;
    bool m_enabled;
};

#endif

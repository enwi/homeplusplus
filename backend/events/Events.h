#ifndef _EVENTS_H
#define _EVENTS_H

#include "../api/Action.h"
#include "../api/Device.h"
#include "../api/Rule.h"
#include "../api/User.h"
#include "../events/EventSystem.h"

// Strange error
#ifdef GetMessage
#undef GetMessage
#endif

namespace EventTypes
{
    // Error occurred
    constexpr EventType error = GetEventType("error");
    // An action changed
    constexpr EventType actionChange = GetEventType("action_change");
    // A rule changed
    constexpr EventType ruleChange = GetEventType("rule_change");
    // A device changed
    constexpr EventType deviceChange = GetEventType("device_change");
    // A property on a device changed
    constexpr EventType devicePropertyChange = GetEventType("device_property_change");
} // namespace EventTypes

namespace Events
{
    class ErrorEvent : public Event<ErrorEvent>
    {
    public:
        ErrorEvent() : ErrorEvent("", "") {}
        ErrorEvent(std::string message, std::string place) : m_msg(std::move(message)), m_place(std::move(place)) {}

        EventType GetType() const override { return EventTypes::error; }

        const std::string& GetMessage() const { return m_msg; }
        const std::string& GetPlace() const { return m_place; }

    private:
        std::string m_msg;
        std::string m_place;
    };

    // Template for a change event, holds old state, changed fields and new state
    template <typename T, uint64_t ET, typename ChangeIndicator>
    class ChangeEvent : public Event<ChangeEvent<T, ET, ChangeIndicator>>
    {
    public:
        ChangeEvent() = default;
        ChangeEvent(
            const T& old, const T& changed, ChangeIndicator changedFields, absl::optional<UserId> user = absl::nullopt)
            : m_old(old), m_changed(changed), m_changedFields(changedFields), m_user(std::move(user))
        {}

        EventType GetType() const override { return EventType(ET); }

        const T& GetOld() const { return m_old; }
        const T& GetChanged() const { return m_changed; }
        const ChangeIndicator& GetChangedFields() const { return m_changedFields; }
        const absl::optional<UserId>& GetUser() const { return m_user; }

    private:
        T m_old;
        T m_changed;
        ChangeIndicator m_changedFields;
        absl::optional<UserId> m_user;
    };

    // Possible Action changes
    enum class ActionFields
    {
        NAME,
        ICON,
        COLOR,
        SUB_ACTIONS,
        VISIBLE,
        ALL,
        ADD,
        REMOVE
    };
    // Possible Rule changes
    enum class RuleFields
    {
        NAME,
        ICON,
        COLOR,
        CONDITION,
        EFFECT,
        ENABLED,
        ALL,
        ADD,
        REMOVE
    };
    // Possible Device changes (excluding properties, see DevicePropertyChangeEvent for that)
    enum class DeviceFields
    {
        NAME,
        GROUPS,
        ALL,
        ADD,
        REMOVE
    };

    // Container for nodeId, sensorId and Sensor
    struct SensorContainer
    {
        uint16_t m_nodeId;
        uint8_t m_property;
        // Sensor m_sensor;
    };

    // typedef ChangeEvent<NodeData, EventTypes::nodeChange, NodeFields> NodeChangeEvent;
    typedef ChangeEvent<Action, EventTypes::actionChange, ActionFields> ActionChangeEvent;
    typedef ChangeEvent<Rule, EventTypes::ruleChange, RuleFields> RuleChangeEvent;
    typedef ChangeEvent<Device, EventTypes::deviceChange, DeviceFields> DeviceChangeEvent;
    typedef ChangeEvent<Device, EventTypes::devicePropertyChange, std::string> DevicePropertyChangeEvent;
} // namespace Events
#endif
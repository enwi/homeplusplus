#ifndef UI_EVENTS_H_INCLUDED
#define UI_EVENTS_H_INCLUDED

#include "EventSystem.h"
#include "UserInterface.h"

namespace EventTypes
{
    constexpr EventType deviceUIChange = GetEventType("device_ui_change");
    constexpr EventType ruleUIChange = GetEventType("rule_ui_change");
    constexpr EventType actionUIChange = GetEventType("action_ui_change");
} // namespace EventTypes

namespace UIEvents
{
    template <typename UI, EventType type>
    class UIChangeEvent : public Event<UIChangeEvent<UI, type>>
    {
    public:
        UIChangeEvent() = default;
        UIChangeEvent(const UI& ui, uint64_t id, uint64_t index, const UserInterface::UIItem::Change& change)
            : m_ui(ui), m_id(id), m_index(index), m_change(change)
        {}
        EventType GetType() const override { return type; }
        const UI& GetUI() const { return m_ui; }
        uint64_t GetId() const { return m_id; }
        uint64_t GetIndex() const { return m_index; }
        const UserInterface::UIItem::Change& GetChange() const { return m_change; }

    private:
        UI m_ui;
        uint64_t m_id;
        uint64_t m_index;
        UserInterface::UIItem::Change m_change;
    };

    using DeviceUIChangeEvent = UIChangeEvent<UserInterface::DeviceUI, EventTypes::deviceUIChange>;
    using RuleUIChangeEvent = UIChangeEvent<UserInterface::RuleUI, EventTypes::ruleUIChange>;
    using ActionUIChangeEvent = UIChangeEvent<UserInterface::ActionUI, EventTypes::actionUIChange>;
} // namespace UIEvents

#endif
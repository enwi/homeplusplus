#pragma once

#include <absl/types/optional.h>

#include "Action.h"
#include "Filter.h"
#include "IActionSerialize.h"

#include "../events/EventSystem.h"
#include "../events/Events.h"

class ActionStorage
{
public:
    // The references must be valid for the lifetime of the instance or any copies
    ActionStorage(IActionSerialize& actionSerialize, EventEmitter<Events::ActionChangeEvent>& eventEmitter)
        : m_actionSerialize(&actionSerialize), m_eventEmitter(&eventEmitter)
    {}
    uint64_t AddAction(const Action& action, UserId user);
    void UpdateAction(const Action& action, UserId user);
    void UpdateActionHeader(const Action& action, UserId user);

    absl::optional<Action> GetAction(uint64_t actionId, UserId user) const;
    std::vector<Action> GetAllActions(const Filter& filter, UserId user) const;

    void RemoveAction(uint64_t actionId, UserId user);

private:
    IActionSerialize* m_actionSerialize;
    EventEmitter<Events::ActionChangeEvent>* m_eventEmitter;
};
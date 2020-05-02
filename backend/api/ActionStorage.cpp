#include "ActionStorage.h"

uint64_t ActionStorage::AddAction(const Action& action, UserId user)
{
    uint64_t id = m_actionSerialize->AddAction(action, user);
	Action changedCopy = action;
	changedCopy.SetId(id);
    m_eventEmitter->EmitEvent(Events::ActionChangeEvent(Action(), changedCopy, Events::ActionFields::ADD));
    return id;
}

void ActionStorage::UpdateAction(const Action& action, UserId user)
{
    // TODO: implement actual update function
    m_actionSerialize->AddAction(action, user);
    // TODO: either pass proper old action or remove that field
    m_eventEmitter->EmitEvent(Events::ActionChangeEvent(Action(), action, Events::ActionFields::ALL));
}

void ActionStorage::UpdateActionHeader(const Action& action, UserId user)
{
    m_actionSerialize->AddActionOnly(action, user);
    m_eventEmitter->EmitEvent(Events::ActionChangeEvent(Action(), action, Events::ActionFields::NAME));
}

absl::optional<Action> ActionStorage::GetAction(uint64_t actionId, UserId user) const
{
    return m_actionSerialize->GetAction(actionId, user);
}

std::vector<Action> ActionStorage::GetAllActions(const Filter& filter, UserId user) const
{
    return m_actionSerialize->GetAllActions(filter, user);
}

void ActionStorage::RemoveAction(uint64_t actionId, UserId user)
{
    m_actionSerialize->RemoveAction(actionId, user);
    // TODO: Pass proper action for remove
    m_eventEmitter->EmitEvent(
        Events::ActionChangeEvent(Action(actionId, "", "", 0, {}), Action(), Events::ActionFields::REMOVE));
}
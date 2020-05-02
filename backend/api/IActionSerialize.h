#ifndef _I_ACTION_SERIALIZE_H
#define _I_ACTION_SERIALIZE_H

#include <absl/types/optional.h>

#include "Action.h"
#include "Filter.h"
#include "User.h"

// Just a tag type to ensure a transaction persists
class UserHeldTransaction;

class IActionSerialize
{
public:
    virtual ~IActionSerialize() = default;

    // Returns the Action with the given id
    virtual absl::optional<Action> GetAction(uint64_t actionId, UserId user) const = 0;
    virtual absl::optional<Action> GetAction(uint64_t actionId, const UserHeldTransaction&) const = 0;
    // Returns all Actions. Careful when there are too many of them
    virtual std::vector<Action> GetAllActions(const Filter& filter, UserId user) const = 0;
    virtual std::vector<Action> GetAllActions(const Filter& filter, const UserHeldTransaction&) const = 0;
    // Adds an Action, returns the id
    virtual uint64_t AddAction(const Action& action, UserId user) = 0;
    virtual uint64_t AddAction(const Action& action, const UserHeldTransaction&) = 0;
    // Updates an Action without updating SubActions. Should be preferred over AddAction(), because it is a LOT faster.
    // Returns the actionId, either of action or the newly found
    virtual uint64_t AddActionOnly(const Action& action, UserId user) = 0;
    virtual uint64_t AddActionOnly(const Action& action, const UserHeldTransaction&) = 0;
    // Removes the Action
    virtual void RemoveAction(uint64_t actionId, UserId user) = 0;
    virtual void RemoveAction(uint64_t actionId, const UserHeldTransaction&) = 0;
};

#endif
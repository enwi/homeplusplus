#ifndef DB_ACTION_SERIALIZE_H
#define DB_ACTION_SERIALIZE_H

#include "ActionsTable.h"
#include "DBHandler.h"

#include "HeldTransaction.h"

#include "../api/IActionSerialize.h"

// Class to read/write Actions from/to database
class DBActionSerialize : public IActionSerialize
{
public:
    explicit DBActionSerialize(DBHandler& dbHandler) : m_dbHandler(dbHandler) {}

    // Returns the Action with the given id
    absl::optional<Action> GetAction(uint64_t actionId, UserId user) const override;
    absl::optional<Action> GetAction(uint64_t actionId, const UserHeldTransaction&) const override;
    // Returns all Actions. Careful when there are too many of them
    std::vector<Action> GetAllActions(const Filter& filter, UserId user) const override;
    std::vector<Action> GetAllActions(const Filter& filter, const UserHeldTransaction&) const override;
    // Adds an Action, returns the id
    uint64_t AddAction(const Action& action, UserId user) override;
    uint64_t AddAction(const Action& action, const UserHeldTransaction&) override;
    // Updates an Action without updating SubActions. Should be preferred over AddAction(), because it is a LOT faster.
    // Returns the actionId, either of action or the newly found
    uint64_t AddActionOnly(const Action& action, UserId user) override;
    uint64_t AddActionOnly(const Action& action, const UserHeldTransaction&) override;
    // Removes the Action
    void RemoveAction(uint64_t actionId, UserId user) override;
    void RemoveAction(uint64_t actionId, const UserHeldTransaction&) override;

private:
    DBHandler& m_dbHandler;
};

#endif
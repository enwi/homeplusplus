#include "DBActionSerialize.h"

#include <sqlpp11/functions.h>
#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>
#include <sqlpp11/transaction.h>
#include <sqlpp11/update.h>

namespace
{
    constexpr ActionsTable actions;
    constexpr SubActionsTable subActions;
    using Result = decltype(GetSelectResult(actions, actions.actionId, actions.actionName, actions.actionIconName,
        actions.actionColor, actions.actionVisible));
    std::vector<Action> GetActionsFromQuery(DBHandler::DatabaseConnection& db, Result result, const UserHeldTransaction& transaction)
    {
        std::vector<Action> actions;
        for (const SelectRow<Result>& row : result)
        {
            int64_t id = row.actionId;
            std::string name = row.actionName;
            std::string icon = row.actionIconName;
            unsigned int color = static_cast<unsigned int>(row.actionColor);
            bool visible = row.actionVisible;
            actions.emplace_back(id, name, icon, color, std::vector<SubAction>(), visible);
        }
        auto preparedSelect = db.prepare(select(subActions.actionType, subActions.data, subActions.timeout, subActions.transition)
                                             .from(subActions)
                                             .where(subActions.actionId == parameter(subActions.actionId)));
        for (Action& action : actions)
        {
            std::vector<SubAction> subActions;
            preparedSelect.params.actionId = action.GetId();
            for (const auto& row : db(preparedSelect))
            {
                subActions.push_back(Res::ActionRegistry().Parse(db, row, transaction));
            }
            action.SetActions(std::move(subActions));
        }
        return actions;
    }

    template <typename T, typename Db>
    std::remove_reference_t<T> CommitAndReturn(T&& value, sqlpp::transaction_t<Db>& transaction)
    {
        transaction.commit();
        return std::forward<T>(value);
    }
} // namespace

absl::optional<Action> DBActionSerialize::GetAction(uint64_t actionId, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetAction(actionId, { user, transaction }), transaction);
}

absl::optional<Action> DBActionSerialize::GetAction(uint64_t actionId, const UserHeldTransaction& transaction) const
{
    auto& db = m_dbHandler.GetDatabase();
    auto result = GetActionsFromQuery(db,
        db(select(
            actions.actionId, actions.actionName, actions.actionIconName, actions.actionColor, actions.actionVisible)
                .from(actions)
                .where(actions.actionId == actionId)), transaction);
    if (result.size() == 0)
    {
        return absl::nullopt;
    }
    else if (result.size() > 1)
    {
        Res::Logger().Warning("Got more than one result from getAction(). Taking the first one.");
    }
    return result.front();
}

std::vector<Action> DBActionSerialize::GetAllActions(const Filter& filter, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetAllActions(filter, { user, transaction }), transaction);
}

std::vector<Action> DBActionSerialize::GetAllActions(const Filter& filter, const UserHeldTransaction& transaction) const
{
    auto& db = m_dbHandler.GetDatabase();
    auto result = db(
        select(actions.actionId, actions.actionName, actions.actionIconName, actions.actionColor, actions.actionVisible)
            .from(actions)
            .unconditionally());
    return GetActionsFromQuery(db, std::move(result), transaction);
}

uint64_t DBActionSerialize::AddAction(const Action& action, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(AddAction(action, { user, transaction }), transaction);
}

uint64_t DBActionSerialize::AddAction(const Action& action, const UserHeldTransaction& transaction)
{
    auto& db = m_dbHandler.GetDatabase();
    const int64_t actionId = AddActionOnly(action, transaction );

    // Add SubActions
    if (action.GetId() != 0)
    {
        // Delete previous sub actions
        db(remove_from(subActions).where(subActions.actionId == action.GetId()));
    }
    // Scope for the statement
    {
        auto preparedInsert = db.prepare(
            insert_into(subActions)
                .set(subActions.actionId = actionId, subActions.actionType = parameter(subActions.actionType),
                    subActions.data = parameter(subActions.data),
                    subActions.timeout = parameter(subActions.timeout),
                    subActions.transition = parameter(subActions.transition)));
		std::vector<uint8_t> data;
        for (const SubAction& subAction : action.GetSubActions())
        {
			messages::SubAction msg = subAction.Serialize();
			data.resize(msg.data().ByteSizeLong());
			msg.data().SerializeToArray(data.data(), data.size());

            preparedInsert.params.actionType = msg.type();
            preparedInsert.params.timeout = msg.timeout();
            preparedInsert.params.transition = msg.transition();
			preparedInsert.params.data = data;
            db(preparedInsert);
        }
    }
    return actionId;
}

uint64_t DBActionSerialize::AddActionOnly(const Action& action, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(AddActionOnly(action, { user, transaction }), transaction);
}

uint64_t DBActionSerialize::AddActionOnly(const Action& action, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();

    int64_t actionId = action.GetId();
    bool actionExists = actionId != 0;
    if (actionExists)
    {
        auto existsResult = db(select(actions.actionId).from(actions).where(actions.actionId == action.GetId()));
        actionExists = !existsResult.empty();
    }
    if (!actionExists)
    {
		// The Action does not exist, insert it
		if (actionId == 0)
        {
			// Choose next available id
            actionId = db(insert_into(actions).set(actions.actionName = action.GetName(),
                actions.actionIconName = action.GetIcon(), actions.actionColor = action.GetColor(),
                actions.actionVisible = action.GetVisibility()));
        }
        else
        {
			// Use preset id
			actionId = db(insert_into(actions).set(actions.actionId = actionId, actions.actionName = action.GetName(),
				actions.actionIconName = action.GetIcon(), actions.actionColor = action.GetColor(),
				actions.actionVisible = action.GetVisibility()));
        }
    }
    else
    {
        // The Action exists, update it
        db(update(actions)
                .set(actions.actionName = action.GetName(), actions.actionIconName = action.GetIcon(),
                    actions.actionColor = action.GetColor(), actions.actionVisible = action.GetVisibility())
                .where(actions.actionId == action.GetId()));
    }
    return actionId;
}

void DBActionSerialize::RemoveAction(uint64_t actionId, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
	RemoveAction(actionId, { user, transaction });
    transaction.commit();
}

void DBActionSerialize::RemoveAction(uint64_t actionId, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();

    db(remove_from(actions).where(actions.actionId == actionId));
}

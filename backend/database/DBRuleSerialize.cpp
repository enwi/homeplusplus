#include "DBRuleSerialize.h"

#include <sqlpp11/insert.h>
#include <sqlpp11/remove.h>
#include <sqlpp11/select.h>
#include <sqlpp11/update.h>

namespace
{
    constexpr RulesTable rules;
    constexpr RuleConditionsTable ruleConditions;
    template <typename T, typename Db>
    std::remove_reference_t<T> CommitAndReturn(T&& value, sqlpp::transaction_t<Db>& transaction)
    {
        transaction.commit();
        return std::forward<T>(value);
    }
} // namespace

absl::optional<Rule> DBRuleSerialize::GetRule(uint64_t ruleId, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetRule(ruleId, {user, transaction}), transaction);
}

absl::optional<Rule> DBRuleSerialize::GetRule(uint64_t ruleId, const UserHeldTransaction& transaction) const
{
    auto& db = m_dbHandler.GetDatabase();

    std::vector<Rule> result = GetRulesFromQuery(db,
        db(select(rules.ruleId, rules.ruleName, rules.ruleIconName, rules.ruleColor, rules.conditionId, rules.actionId,
            rules.ruleEnabled)
                .from(rules)
                .where(rules.ruleId == ruleId)),
        transaction);

    if (result.empty())
    {
        return absl::nullopt;
    }
    if (result.size() > 1)
    {
        Res::Logger().Warning("Got more than one result from GetRule(). Taking the first one.");
    }
    return result.front();
}

std::vector<Rule> DBRuleSerialize::GetAllRules(const Filter& filter, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetAllRules(filter, {user, transaction}), transaction);
}

std::vector<Rule> DBRuleSerialize::GetAllRules(const Filter& filter, const UserHeldTransaction& transaction) const
{
    auto& db = m_dbHandler.GetDatabase();
    return GetRulesFromQuery(db,
        db(select(rules.ruleId, rules.ruleName, rules.ruleIconName, rules.ruleColor, rules.conditionId, rules.actionId,
            rules.ruleEnabled)
                .from(rules)
                .unconditionally()),
        transaction);
}

void DBRuleSerialize::AddRule(Rule& rule, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    AddRule(rule, {user, transaction});
    transaction.commit();
}

void DBRuleSerialize::AddRule(Rule& rule, const UserHeldTransaction& transaction)
{
    auto& db = m_dbHandler.GetDatabase();
    m_rulePermissions.VerifyGeneralPermission(RulePermissions::Permission::addRule, transaction.GetUser());
    int64_t ruleId = 0;

    // Add Condition & Effect
    if (!rule.HasCondition())
    {
        Res::Logger().Error("Tried to add Rule without condition!");
        throw std::invalid_argument("Rule without condition in DBHandler::AddRule");
    }
    if (rule.GetCondition().GetId() != 0)
    {
        // Delete previous condition
        m_condSerialize.RemoveRuleCondition(rule.GetCondition(), transaction);
    }
    Action effect = rule.GetEffect();
    effect.SetId(m_actionSerialize.AddAction(effect, transaction));
    rule.SetEffect(effect);

    m_condSerialize.InsertRuleCondition(rule.GetCondition(), transaction);

    // Insert rule AFTER the condition to obey constraints
    ruleId = AddRuleOnly(rule, transaction);
    rule.SetId(ruleId);
}

void DBRuleSerialize::UpdateRule(Rule& rule, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    UpdateRule(rule, {user, transaction});
    transaction.commit();
}

void DBRuleSerialize::UpdateRule(Rule& rule, const UserHeldTransaction& transaction)
{
    // TODO: Change this to only work if rule can be edited by user
    AddRule(rule, transaction);
}

uint64_t DBRuleSerialize::AddRuleOnly(const Rule& rule, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(AddRuleOnly(rule, {user, transaction}), transaction);
}

uint64_t DBRuleSerialize::AddRuleOnly(const Rule& rule, const UserHeldTransaction& transaction)
{
    auto& db = m_dbHandler.GetDatabase();
    int64_t ruleId = rule.GetId();
    bool ruleExists = ruleId != 0;
	if (ruleExists) {
		auto existsResult = db(select(rules.ruleId).from(rules).where(rules.ruleId == rule.GetId()));
		ruleExists = !existsResult.empty();
	}
    if (!ruleExists)
    {
        // The Rule does not exist, insert it
		if (ruleId == 0) {
			ruleId = db(insert_into(rules).set(rules.ruleName = rule.GetName(), rules.ruleIconName = rule.GetIcon(),
				rules.ruleColor = rule.GetColor(), rules.conditionId = rule.GetCondition().GetId(),
				rules.actionId = rule.GetEffect().GetId(), rules.ruleEnabled = rule.IsEnabled()));
		}
		else
		{
			// Use preset id
			ruleId = db(insert_into(rules).set(rules.ruleId = rule.GetId(),rules.ruleName = rule.GetName(), rules.ruleIconName = rule.GetIcon(),
				rules.ruleColor = rule.GetColor(), rules.conditionId = rule.GetCondition().GetId(),
				rules.actionId = rule.GetEffect().GetId(), rules.ruleEnabled = rule.IsEnabled()));
		}
    }
    else
    {
        db(update(rules)
                .set(rules.ruleName = rule.GetName(), rules.ruleIconName = rule.GetIcon(),
                    rules.ruleColor = rule.GetColor(), rules.conditionId = rule.GetCondition().GetId(),
                    rules.actionId = rule.GetEffect().GetId(), rules.ruleEnabled = rule.IsEnabled())
                .where(rules.ruleId == rule.GetId()));
    }
    return ruleId;
}

void DBRuleSerialize::RemoveRule(uint64_t ruleId, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    RemoveRule(ruleId, {user, transaction});
    transaction.commit();
}

void DBRuleSerialize::RemoveRule(uint64_t ruleId, const UserHeldTransaction& transaction)
{
    absl::optional<Rule> rule = GetRule(ruleId, transaction);
    if (rule)
    {
        RemoveRule(*rule, transaction);
    }
    else
    {
        Res::Logger().Warning("DBRuleSerialize", "Rule to be removed could not be found");
    }
}

void DBRuleSerialize::RemoveRule(const Rule& rule, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    RemoveRule(rule, {user, transaction});
    transaction.commit();
}

void DBRuleSerialize::RemoveRule(const Rule& rule, const UserHeldTransaction& transaction)
{
    auto& db = m_dbHandler.GetDatabase();
    // requires transaction

    // Delete the rule first, then the condition and effect because of constraints
    db(remove_from(rules).where(rules.ruleId == rule.GetId()));

    m_condSerialize.RemoveRuleCondition(rule.GetCondition(), transaction);
    m_actionSerialize.RemoveAction(rule.GetEffect().GetId(), transaction);
}

std::vector<Rule> DBRuleSerialize::GetRulesFromQuery(
    DBHandler::DatabaseConnection& db, RuleSelectResult result, const UserHeldTransaction& transaction) const
{
    std::vector<Rule> rules;
    std::vector<int64_t> conditionIds;
    std::vector<int64_t> actionIds;
    for (const auto& row : result)
    {
        Rule rule(row.ruleId, row.ruleName, row.ruleIconName, row.ruleColor, nullptr, Action(), row.ruleEnabled);
        rules.push_back(std::move(rule));
        conditionIds.push_back(row.conditionId);
        actionIds.push_back(row.actionId);
    }
    for (unsigned int i = 0; i < rules.size(); i++)
    {
        rules[i].SetEffect(m_actionSerialize.GetAction(actionIds[i], transaction).value_or(Action()));
        rules[i].SetCondition(m_condSerialize.GetRuleCondition(conditionIds[i], transaction));
    }
    return rules;
}

void DBRuleConditionSerialize::AddRuleCondition(RuleConditions::RuleCondition& condition, UserId user)
{

    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    AddRuleCondition(condition, {user, transaction});
    transaction.commit();
}

void DBRuleConditionSerialize::AddRuleCondition(
    RuleConditions::RuleCondition& condition, const UserHeldTransaction& transaction)
{
    // TODO: Remove these, better: check if something changed
    RemoveRuleCondition(condition, transaction);
    InsertRuleCondition(condition, transaction);
}

RuleConditions::Ptr DBRuleConditionSerialize::GetRuleCondition(uint64_t conditionId, UserId user) const
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    return CommitAndReturn(GetRuleCondition(conditionId, {user, transaction}), transaction);
}

RuleConditions::Ptr DBRuleConditionSerialize::GetRuleCondition(
    uint64_t conditionId, const UserHeldTransaction& transaction) const
{
    auto& db = m_dbHandler.GetDatabase();

    RuleConditions::Ptr result;
    {
        auto rows = db(select(ruleConditions.conditionId, ruleConditions.conditionType, ruleConditions.conditionData)
                           .from(ruleConditions)
                           .where(ruleConditions.conditionId == conditionId));
        // Execute the result and check if a condition was found
        if (rows.empty())
        {
            Res::Logger().Warning("Did not find rule condition: " + std::to_string(conditionId));
            throw std::out_of_range(
                "DBRuleConditionSerialize::GetRuleCondition: Did not find rule condition: " + std::to_string(conditionId));
        }
        else
        {
            result = Res::ConditionRegistry().ParseCondition(*this, rows.front(), transaction);
        }
    }
    return result;
}

void DBRuleConditionSerialize::InsertRuleCondition(RuleConditions::RuleCondition& condition, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    InsertRuleCondition(condition, {user, transaction});
    transaction.commit();
}

void DBRuleConditionSerialize::InsertRuleCondition(RuleConditions::RuleCondition& condition, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();

    {
        auto insertStatement
            = db.prepare(insert_into(ruleConditions)
                             .set(ruleConditions.conditionId = parameter(ruleConditions.conditionId),
                                 ruleConditions.conditionType = parameter(ruleConditions.conditionType),
                                 ruleConditions.conditionData = parameter(ruleConditions.conditionData)));
        auto insertStatementNoId
            = db.prepare(insert_into(ruleConditions)
                             .set(ruleConditions.conditionType = parameter(ruleConditions.conditionType),
                                 ruleConditions.conditionData = parameter(ruleConditions.conditionData)));

        // Recursive function to insert
        std::function<void(RuleConditions::RuleCondition*)> rec = [&](RuleConditions::RuleCondition* c) {
            // Add childs first, because fields might depend on them
            if (c->HasChilds())
            {
                std::vector<RuleConditions::RuleCondition*> childs = c->GetChilds();
                for (const auto child : childs)
                {
                    rec(child);
                }
            }
			messages::RuleCondition msg = c->Serialize(false);
			std::vector<uint8_t> data(msg.data().ByteSizeLong());
			msg.data().SerializeToArray(data.data(), data.size());
            if (c->GetId() != 0)
            {
                insertStatement.params.conditionId = c->GetId();
                insertStatement.params.conditionType = c->GetType();
				insertStatement.params.conditionData = data;
                db(insertStatement);
            }
            else
            {
                insertStatementNoId.params.conditionType = c->GetType();
				insertStatementNoId.params.conditionData = data;
                c->SetId(db(insertStatementNoId));
            }
        };
        rec(&condition);
    }
}

void DBRuleConditionSerialize::RemoveRuleCondition(const RuleConditions::RuleCondition& condition, UserId user)
{
    auto transaction = sqlpp::start_transaction(m_dbHandler.GetDatabase());
    RemoveRuleCondition(condition, {user, transaction});
    transaction.commit();
}

void DBRuleConditionSerialize::RemoveRuleCondition(
    const RuleConditions::RuleCondition& condition, const UserHeldTransaction&)
{
    auto& db = m_dbHandler.GetDatabase();
    {
        auto remove = db.prepare(
            remove_from(ruleConditions).where(ruleConditions.conditionId == parameter(ruleConditions.conditionId)));
        if (!condition.HasChilds())
        {
            if (condition.GetId() != 0)
            {
                remove.params.conditionId = condition.GetId();
                db(remove);
            }
            return;
        }
        const std::function<void(const RuleConditions::RuleCondition*)> rec
            = [&](const RuleConditions::RuleCondition* c) {
                  // Only remove if it exists
                  if (c->GetId() != 0)
                  {
                      remove.params.conditionId = c->GetId();
                      db(remove);
                  }
                  if (c->HasChilds())
                  {
                      std::vector<const RuleConditions::RuleCondition*> childs = c->GetChilds();
                      for (const auto child : childs)
                      {
                          // Call recursively
                          rec(child);
                      }
                  }
              };
        rec(&condition);
    }
}

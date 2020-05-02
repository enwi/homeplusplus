#include "DBHandler.h"

#include <algorithm>
#include <exception>
#include <map>
#include <stdexcept>

#include "ActionsTable.h"
#include "DevicesTable.h"
#include "RulesTable.h"
#include "UsersTable.h"

DBHandler::DBHandler(const std::string& filename) : m_filename(filename), m_sqliteDatabase(filename, 5000) {}

void DBHandler::CreateTables()
{
    auto& db = m_sqliteDatabase.GetDatabase();
    auto transaction = sqlpp::start_transaction(db);

    db.execute(ActionsTable::createStatement);
    db.execute(SubActionsTable::createStatement);
    db.execute(DevicesTable::createStatement);
    db.execute(DeviceGroupsTable::createStatement);
    db.execute(PropertiesTable::createStatement);
    db.execute(PropertiesLogTable::createStatement);
    db.execute(RuleConditionsTable::createStatement);
    db.execute(RulesTable::createStatement);
    db.execute(UsersTable::createStatement);

    transaction.commit();
}

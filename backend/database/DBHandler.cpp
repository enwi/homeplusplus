#include "DBHandler.h"

#include <algorithm>
#include <exception>
#include <map>
#include <stdexcept>

#include <sqlpp11/insert.h>
#include <sqlpp11/limit.h>

#include "ActionsTable.h"
#include "DevicesTable.h"
#include "RulesTable.h"
#include "UsersTable.h"

namespace
{
    constexpr UsersTable users;
} // namespace

DBHandler::DBHandler(const std::string& filename) : m_filename(filename), m_sqliteDatabase(filename, 5000) {}

void DBHandler::CreateTables(const Authenticator& authenticator)
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

    auto result = db(select(users.userId).from(users).unconditionally().limit(1u));
    if (result.empty())
    {
        Res::Logger().Info(
            "DBHandler", "This is likely to be the first start, so the default user \'admin\' will be generated");
        const std::string userName = "admin";
        const std::string password = "admin";
        const std::string hash = authenticator.CreatePasswordHash(password);
        const uint64_t userId = 0x00;
        const uint8_t userPriority = 0x00;

        db(insert_into(users).set(users.userId = userId, users.userPwhash = hash, users.userName = userName,
            users.userPriority = userPriority));
    }

    transaction.commit();
}

#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

#include <sqlite3.h>
#include <sqlpp11/sqlite3/connection.h>

class SqliteDatabase
{
public:
    SqliteDatabase(const std::string& filename, int busyTimeout, bool readOnly = false);

    sqlpp::sqlite3::connection& GetDatabase() { return m_database; }

private:
    sqlpp::sqlite3::connection m_database;
};

#endif
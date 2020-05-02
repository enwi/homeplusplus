#include "SQLiteDatabase.h"

namespace sql = sqlpp::sqlite3;

SqliteDatabase::SqliteDatabase(const std::string& filename, int busyTimeout, bool readOnly)
    : m_database(sql::connection_config(
          filename, readOnly ? SQLITE_OPEN_READONLY : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)))
{
    sqlite3_busy_timeout(m_database.native_handle(), busyTimeout);
    m_database.execute("PRAGMA foreign_keys = ON;");
}

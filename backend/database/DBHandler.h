#ifndef _DB_HANDLER_H
#define _DB_HANDLER_H
#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include <absl/types/variant.h>
#include <sqlpp11/select.h>

#include "SQLiteDatabase.h"

#include "../api/Resources.h"
#include "../communication/Authenticator.h"
#include "../utility/Logger.h"

class DBValue
{
public:
    friend bool operator==(const DBValue& lhs, const DBValue& rhs) { return lhs.m_value == rhs.m_value; }
    friend bool operator!=(const DBValue& lhs, const DBValue& rhs) { return lhs.m_value != rhs.m_value; }

public:
    DBValue() : m_value(absl::in_place_index_t<0> {}, 0) {}
    // Any 64 bit int
    template <typename T, std::enable_if_t<std::is_integral<T>::value && (sizeof(T) > sizeof(int))>* = nullptr>
    DBValue(T i) : m_value(static_cast<int64_t>(i))
    {}
    // Any 32 bit int or less
    template <typename T, std::enable_if_t<std::is_integral<T>::value && (sizeof(T) <= sizeof(int))>* = nullptr>
    DBValue(T i) : m_value(static_cast<int>(i))
    {}
    DBValue(const std::string& str) : m_value(str) {}
    DBValue(const void* ptr, size_t len)
        : m_value(
            std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(ptr), reinterpret_cast<const uint8_t*>(ptr) + len))
    {}

    bool IsInt() const { return absl::holds_alternative<int>(m_value); }
    bool IsInt64() const { return absl::holds_alternative<int64_t>(m_value); }
    bool IsString() const { return absl::holds_alternative<std::string>(m_value); }
    bool IsBytes() const { return absl::holds_alternative<std::vector<uint8_t>>(m_value); }

    int GetInt() const { return absl::get<int>(m_value); }
    int64_t GetInt64() const
    {
        // Also work if only int is saved
        const int64_t* i = absl::get_if<int64_t>(&m_value);
        if (i != nullptr)
        {
            return *i;
        }
        return absl::get<int>(m_value);
    }
    const std::string& GetStr() const { return absl::get<std::string>(m_value); }
    const std::vector<uint8_t>& GetBytes() const { return absl::get<std::vector<uint8_t>>(m_value); }

private:
    absl::variant<int, int64_t, std::string, std::vector<uint8_t>> m_value;
};

class DBHandler
{
public:
    using DatabaseConnection = sqlpp::sqlite3::connection;
    using Transaction = sqlpp::transaction_t<DatabaseConnection>;

    // Creates an DBHandler by the arguments
    explicit DBHandler(const std::string& filename);

    void CreateTables(const Authenticator& authenticator);

    DatabaseConnection& GetDatabase() { return m_sqliteDatabase.GetDatabase(); }

protected:
    // The filename of the database
    std::string m_filename;
    SqliteDatabase m_sqliteDatabase;
    mutable std::recursive_mutex m_mutex;
};

template <typename Table, typename... Columns>
using SelectResult = decltype(std::declval<DBHandler::DatabaseConnection>()(
    sqlpp::select(std::declval<Columns>()...).from(std::declval<Table>()).unconditionally()));

template <typename Table, typename... Columns>
constexpr auto GetSelectResult(Table table, Columns... columns) -> SelectResult<Table, Columns...>
{}
template <typename SelectResult>
using SelectRow = typename SelectResult::iterator::value_type;
template <typename Table, typename... Columns>
constexpr auto GetSelectRow(Table table, Columns... columns) -> SelectRow<SelectResult<Table, Columns...>>
{}

#endif

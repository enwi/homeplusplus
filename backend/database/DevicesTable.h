#pragma once

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/table.h>

namespace Devices_
{
    struct DeviceId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceId;
                T& operator()() { return deviceId; }
                const T& operator()() const { return deviceId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct DeviceName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceName;
                T& operator()() { return deviceName; }
                const T& operator()() const { return deviceName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
    struct DeviceIcon
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_icon";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceIcon;
                T& operator()() { return deviceIcon; }
                const T& operator()() const { return deviceIcon; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
    struct DeviceApi
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_api";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceApi;
                T& operator()() { return deviceApi; }
                const T& operator()() const { return deviceApi; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
    struct DeviceType
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_type";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceType;
                T& operator()() { return deviceType; }
                const T& operator()() const { return deviceType; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
} // namespace Devices_

struct DevicesTable : sqlpp::table_t<DevicesTable, Devices_::DeviceId, Devices_::DeviceName, Devices_::DeviceIcon,
                          Devices_::DeviceApi, Devices_::DeviceType>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "devices";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T nodes;
            T& operator()() { return nodes; }
            const T& operator()() const { return nodes; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS devices(device_id INTEGER PRIMARY KEY NOT NULL, "
          "device_name VARCHAR NOT NULL, device_icon VARCHAR NOT NULL, device_api VARCHAR NOT NULL, device_type "
          "VARCHAR NOT NULL);";
};

namespace DeviceGroups_
{
    struct DeviceId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceId;
                T& operator()() { return deviceId; }
                const T& operator()() const { return deviceId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct GroupName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "group_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T groupName;
                T& operator()() { return groupName; }
                const T& operator()() const { return groupName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
} // namespace DeviceGroups_

struct DeviceGroupsTable : sqlpp::table_t<DeviceGroupsTable, DeviceGroups_::DeviceId, DeviceGroups_::GroupName>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "device_groups";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T deviceGroups;
            T& operator()() { return deviceGroups; }
            const T& operator()() const { return deviceGroups; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS device_groups(device_id INTEGER PRIMARY KEY NOT NULL REFERENCES "
          "devices(device_id) ON DELETE "
          "CASCADE, group_name VARCHAR NOT NULL, UNIQUE(device_id, group_name));";
};

namespace Properties_
{
    struct PropertyUid
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "property_uid";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T propertyUid;
                T& operator()() { return propertyUid; }
                const T& operator()() const { return propertyUid; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct DeviceId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "device_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T deviceId;
                T& operator()() { return deviceId; }
                const T& operator()() const { return deviceId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct PropertyKey
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "property_key";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T propertyKey;
                T& operator()() { return propertyKey; }
                const T& operator()() const { return propertyKey; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
    struct PropertyValue
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "property_value";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T propertyValue;
                T& operator()() { return propertyValue; }
                const T& operator()() const { return propertyValue; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::blob, sqlpp::tag::can_be_null>;
    };
    struct PropertyDate
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "property_date";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T propertyDate;
                T& operator()() { return propertyDate; }
                const T& operator()() const { return propertyDate; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::require_insert>;
    };
} // namespace Properties_

struct PropertiesTable : sqlpp::table_t<PropertiesTable, Properties_::PropertyUid, Properties_::DeviceId,
                             Properties_::PropertyKey, Properties_::PropertyValue>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "properties";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T properties;
            T& operator()() { return properties; }
            const T& operator()() const { return properties; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS properties(property_uid INTEGER PRIMARY KEY NOT NULL,"
          "device_id INTEGER NOT NULL REFERENCES devices(device_id) ON DELETE CASCADE,"
          "property_key VARCHAR NOT NULL,"
          "property_value BLOB,"
          "UNIQUE(device_id, property_key));";
};

struct PropertiesLogTable : sqlpp::table_t<PropertiesLogTable, Properties_::DeviceId, Properties_::PropertyKey,
                                Properties_::PropertyValue, Properties_::PropertyDate>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "propertieslog";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T propertieslog;
            T& operator()() { return propertieslog; }
            const T& operator()() const { return propertieslog; }
        };
    };
    static constexpr const char* createStatement = "CREATE TABLE IF NOT EXISTS propertieslog(device_id INTEGER NOT "
                                                   "NULL REFERENCES devices(device_id) ON DELETE CASCADE,"
                                                   "property_key VARCHAR NOT NULL,"
                                                   "property_value BLOB,"
                                                   "property_date DATETIME NOT NULL,"
                                                   "UNIQUE(device_id, property_key, property_date));";
};

#pragma once
#include <sqlpp11/column_types.h>
#include <sqlpp11/table.h>

namespace Actions_
{
    struct ActionId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionId;
                T& operator()() { return actionId; }
                const T& operator()() const { return actionId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct ActionName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionName;
                T& operator()() { return actionName; }
                const T& operator()() const { return actionName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct ActionIconName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_icon_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionIconName;
                T& operator()() { return actionIconName; }
                const T& operator()() const { return actionIconName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct ActionColor
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_color";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionColor;
                T& operator()() { return actionColor; }
                const T& operator()() const { return actionColor; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct ActionVisible
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_visible";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionVisible;
                T& operator()() { return actionVisible; }
                const T& operator()() const { return actionVisible; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::boolean, sqlpp::tag::can_be_null>;
    };
} // namespace Actions_

struct ActionsTable : sqlpp::table_t<ActionsTable, Actions_::ActionId, Actions_::ActionName, Actions_::ActionIconName,
                          Actions_::ActionColor, Actions_::ActionVisible>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "actions";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T actions;
            T& operator()() { return actions; }
            const T& operator()() const { return actions; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS actions(action_id INTEGER PRIMARY KEY NOT NULL, action_name VARCHAR, action_icon_name "
          "VARCHAR, action_color INTEGER, action_visible INTEGER DEFAULT 1);";
};

namespace SubActions_
{
    struct SubActionId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "sub_action_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T subActionId;
                T& operator()() { return subActionId; }
                const T& operator()() const { return subActionId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct ActionId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionId;
                T& operator()() { return actionId; }
                const T& operator()() const { return actionId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct ActionType
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "action_type";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T actionType;
                T& operator()() { return actionType; }
                const T& operator()() const { return actionType; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct Data
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "data";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T data;
                T& operator()() { return data; }
                const T& operator()() const { return data; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::blob, sqlpp::tag::can_be_null>;
    };
    struct Timeout
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "timeout";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T timeout;
                T& operator()() { return timeout; }
                const T& operator()() const { return timeout; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Transition
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "transition";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T transition;
                T& operator()() { return transition; }
                const T& operator()() const { return transition; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
} // namespace SubActions_

struct SubActionsTable
    : sqlpp::table_t<SubActionsTable, SubActions_::SubActionId, SubActions_::ActionId, SubActions_::ActionType,
          SubActions_::Data, SubActions_::Timeout, SubActions_::Transition>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "sub_actions";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T subActions;
            T& operator()() { return subActions; }
            const T& operator()() const { return subActions; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS sub_actions(sub_action_id INTEGER PRIMARY KEY NOT NULL, action_id INTEGER NOT NULL REFERENCES "
          "actions(action_id) ON DELETE CASCADE, action_type INTEGER NOT NULL, data BLOB, timeout INTEGER DEFAULT 0, transition INTEGER DEFAULT 0);";
};
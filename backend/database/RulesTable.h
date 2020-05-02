#pragma once

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/table.h>

namespace Rules_
{
    struct RuleId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "rule_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T ruleId;
                T& operator()() { return ruleId; }
                const T& operator()() const { return ruleId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct RuleName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "rule_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T ruleName;
                T& operator()() { return ruleName; }
                const T& operator()() const { return ruleName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct RuleIconName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "rule_icon_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T ruleIconName;
                T& operator()() { return ruleIconName; }
                const T& operator()() const { return ruleIconName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct RuleColor
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "rule_color";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T ruleColor;
                T& operator()() { return ruleColor; }
                const T& operator()() const { return ruleColor; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct ConditionId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "condition_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T conditionId;
                T& operator()() { return conditionId; }
                const T& operator()() const { return conditionId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
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
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct RuleEnabled
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "rule_enabled";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T ruleEnabled;
                T& operator()() { return ruleEnabled; }
                const T& operator()() const { return ruleEnabled; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::boolean, sqlpp::tag::can_be_null>;
    };
} // namespace Rules_

struct RulesTable : sqlpp::table_t<RulesTable, Rules_::RuleId, Rules_::RuleName, Rules_::RuleIconName,
                        Rules_::RuleColor, Rules_::ConditionId, Rules_::ActionId, Rules_::RuleEnabled>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "rules";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T rules;
            T& operator()() { return rules; }
            const T& operator()() const { return rules; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS rules(rule_id INTEGER PRIMARY KEY NOT NULL, rule_name VARCHAR, rule_icon_name VARCHAR, "
          "rule_color INTEGER, condition_id INTEGER REFERENCES rule_conditions(condition_id) ON DELETE SET NULL, "
          "action_id INTEGER REFERENCES actions(action_id) ON DELETE SET NULL, rule_enabled INTEGER DEFAULT 1);";
};
namespace RuleConditions_
{
    struct ConditionId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "condition_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T conditionId;
                T& operator()() { return conditionId; }
                const T& operator()() const { return conditionId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct ConditionType
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "condition_type";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T conditionType;
                T& operator()() { return conditionType; }
                const T& operator()() const { return conditionType; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct ConditionData
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "condition_data";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T conditionData;
                T& operator()() { return conditionData; }
                const T& operator()() const { return conditionData; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::blob, sqlpp::tag::can_be_null>;
    };
} // namespace RuleConditions_

struct RuleConditionsTable
    : sqlpp::table_t<RuleConditionsTable, RuleConditions_::ConditionId, RuleConditions_::ConditionType,
          RuleConditions_::ConditionData>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "rule_conditions";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T ruleConditions;
            T& operator()() { return ruleConditions; }
            const T& operator()() const { return ruleConditions; }
        };
    };
    static constexpr const char* createStatement
        = "CREATE TABLE IF NOT EXISTS rule_conditions(condition_id INTEGER PRIMARY KEY NOT NULL, condition_type INTEGER NOT NULL, "
          "condition_data BLOB);";
};

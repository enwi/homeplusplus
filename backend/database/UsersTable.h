#pragma once

#include <sqlpp11/char_sequence.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/table.h>

namespace Users_
{
    struct UserId
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "user_id";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T userId;
                T& operator()() { return userId; }
                const T& operator()() const { return userId; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct UserName
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "user_name";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T userName;
                T& operator()() { return userName; }
                const T& operator()() const { return userName; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::require_insert>;
    };
    struct UserPwhash
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "user_pwhash";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T userPwhash;
                T& operator()() { return userPwhash; }
                const T& operator()() const { return userPwhash; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null>;
    };
    struct UserPriority
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "user_priority";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T userPriority;
                T& operator()() { return userPriority; }
                const T& operator()() const { return userPriority; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer>;
    };
    struct LastLogin
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "last_login";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T lastLogin;
                T& operator()() { return lastLogin; }
                const T& operator()() const { return lastLogin; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::can_be_null>;
    };
    struct LoginAttempts
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "login_attempts";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T loginAttempts;
                T& operator()() { return loginAttempts; }
                const T& operator()() const { return loginAttempts; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Picture
    {
        struct _alias_t
        {
            static constexpr const char _literal[] = "picture";
            using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
            template <typename T>
            struct _member_t
            {
                T picture;
                T& operator()() { return picture; }
                const T& operator()() const { return picture; }
            };
        };
        using _traits = sqlpp::make_traits<sqlpp::blob, sqlpp::tag::can_be_null>;
    };
} // namespace Users_

struct UsersTable : sqlpp::table_t<UsersTable, Users_::UserId, Users_::UserName, Users_::UserPwhash,
                        Users_::UserPriority, Users_::LastLogin, Users_::LoginAttempts, Users_::Picture>
{
    struct _alias_t
    {
        static constexpr const char _literal[] = "users";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template <typename T>
        struct _member_t
        {
            T users;
            T& operator()() { return users; }
            const T& operator()() const { return users; }
        };
    };
	static constexpr const char* createStatement = "CREATE TABLE IF NOT EXISTS users(user_id INTEGER PRIMARY KEY NOT NULL, user_name VARCHAR NOT NULL, user_pwhash VARCHAR, user_priority INTEGER NOT NULL DEFAULT 0, last_login TIMESTAMP, login_attempts INTEGER DEFAULT 0, picture BLOB DEFAULT NULL);";
};
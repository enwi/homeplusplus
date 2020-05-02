#pragma once

#include <type_traits>

#include <sqlpp11/transaction.h>
#include "../api/User.h"

template <typename T>
struct IsTransaction : std::false_type
{};

template <typename T>
struct IsTransaction<sqlpp::transaction_t<T>> : std::true_type
{};

class HeldTransaction
{
public:
    template <typename T, typename Enable = std::enable_if_t<IsTransaction<T>::value>>
    HeldTransaction(T& transaction)
    {}
	HeldTransaction(HeldTransaction&&) = delete;
};

class UserHeldTransaction
{
public:
    template <typename T, typename Enable = std::enable_if_t<IsTransaction<T>::value>>
    UserHeldTransaction(UserId user, T& transaction) : m_user(user), m_transaction(transaction)
    {}
    const HeldTransaction& GetTransaction() const { return m_transaction; }
    UserId GetUser() const { return m_user; }

private:
    UserId m_user;
    HeldTransaction m_transaction;
};

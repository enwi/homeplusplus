#ifndef MOCK_ACTION_SERIALIZE_H
#define MOCK_ACTION_SERIALIZE_H

#include <gmock/gmock.h>

#include "api/IActionSerialize.h"
#include "database/HeldTransaction.h"


class MockActionSerialize : public IActionSerialize
{
public:
    MOCK_CONST_METHOD2(GetAction, absl::optional<Action>(uint64_t, UserId user));
    MOCK_CONST_METHOD2(GetAction, absl::optional<Action>(uint64_t, const UserHeldTransaction&));
    MOCK_CONST_METHOD2(GetAllActions, std::vector<Action>(const Filter& filter, UserId user));
    MOCK_CONST_METHOD2(GetAllActions, std::vector<Action>(const Filter& filter, const UserHeldTransaction&));
    MOCK_METHOD2(AddAction, uint64_t(const Action&, UserId user));
    MOCK_METHOD2(AddAction, uint64_t(const Action&, const UserHeldTransaction&));
    MOCK_METHOD2(AddActionOnly, uint64_t(const Action&, UserId user));
    MOCK_METHOD2(AddActionOnly, uint64_t(const Action&, const UserHeldTransaction&));
    MOCK_METHOD2(RemoveAction, void(uint64_t, UserId user));
    MOCK_METHOD2(RemoveAction, void(uint64_t, const UserHeldTransaction&));
};

#endif
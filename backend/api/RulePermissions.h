#pragma once

#include "User.h"

class RulePermissions
{
public:
    enum class Permission
    {
        addRule,
        viewRule,
        editRule,
        removeRule
    };

public:
    // Throws exception if permission is not granted
    void VerifyGeneralPermission(Permission permission, UserId user) const {}
    void VerifySpecificPermission(Permission permission, uint64_t ruleId, UserId user) const {}
};
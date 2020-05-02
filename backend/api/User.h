#pragma once

#include <cstdint>

class UserId
{
public:
    explicit UserId(int64_t id) : m_id(id) {}

    int64_t IActuallyReallyNeedTheIntegerNow() const { return m_id; }
    static UserId Dummy(int64_t id = 0) { return UserId(id); }

	friend bool operator==(UserId lhs, UserId rhs) {
		return lhs.m_id == rhs.m_id;
	}
private:
    int64_t m_id;
};
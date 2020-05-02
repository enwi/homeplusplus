#pragma once

#include <chrono>
#include <string>
#include <absl/strings/string_view.h>

#include <json.hpp>

class Authenticator
{
public:
    bool ValidatePassword(const std::string& password, const std::string& hash) const;
    std::string CreateJWTToken(nlohmann::json payload) const;
    nlohmann::json ValidateJWTToken(const std::string& encoded) const;
	std::string CreatePasswordHash(absl::string_view password) const;

public:
    static constexpr std::chrono::seconds s_tokenExpiration = std::chrono::hours(2);
	static constexpr unsigned int s_hashRounds = 6400;
};
#ifndef _COOKIE_VERIFY_H
#define _COOKIE_VERIFY_H

#include <chrono>
#include <string>

namespace CookieVerify
{
    // Verifies cookie of the format name="version:key_version:timestamp:name:value:signature"
    bool CheckCookie(const std::string& cookie);
    // Creates signature using cookieSecret, used to compare to cookie signature
    std::string CreateSignature(const std::string& str);

    constexpr static const char* s_cookieName = "Nodeduino.user";
    // Max age is 31 days
    constexpr static std::chrono::duration<long long> s_maxCookieAge = std::chrono::hours(31 * 24);
    constexpr static const char* s_cookieSecret = "_HAHA_ICH_SOLLTE_EIGENTLICH_ZUFAELLIG_GENERIERT_WERDEN_0815";
} // namespace CookieVerify

#endif
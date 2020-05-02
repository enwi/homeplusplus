#include "CookieVerify.h"

#include <memory>
#include <vector>

#include <cryptopp/hex.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>

#include "../api/Resources.h"
#include "../utility/Logger.h"

bool CookieVerify::CheckCookie(const std::string& cookie)
{
    // Before = is the name
    size_t startCookie = cookie.find("=\"");
    // Ignore leading whitespace
    size_t whitespace = cookie.find_first_not_of(' ');
    if (startCookie == std::string::npos || cookie.substr(whitespace, startCookie - whitespace) != s_cookieName)
    {
        // This is not the correct cookie
        return false;
    }
    // Res::Logger().Debug("Start cookie: " + std::to_string(startCookie));

    if (cookie.length() < startCookie + 2 || cookie.at(startCookie + 2) != '2')
    {
        // Wrong version
        return false;
    }

    // Fields are: key_version, timestamp, name, value, signature
    std::vector<std::string> fields;

    const auto getField = [](std::string& s) {
        size_t colonPos = s.find(':');
        unsigned long len = std::stoul(s.substr(0, colonPos));
        if (colonPos == std::string::npos || s.at(colonPos + len + 1) != '|')
        {
            // Malformed cookie value
            return std::string();
        }
        std::string field = s.substr(colonPos + 1, len);
        // Res::Logger().Debug(field);
        // Remove used field
        s.erase(0, colonPos + len + 2);
        return field;
    };

    try
    {
        // 2 chars for '="', 2 chars for value, cut off last "
        std::string rest = cookie.substr(startCookie + 4, cookie.rfind('\"') - startCookie - 4);
        // key_version
        fields.push_back(getField(rest));
        // timestamp
        fields.push_back(getField(rest));
        // name
        fields.push_back(getField(rest));
        // value
        fields.push_back(getField(rest));
        // signature (is no field with len:value, so add it directly)
        fields.push_back(std::move(rest));
    }
    catch (const std::exception& e)
    {
        // Malformed cookie
        return false;
    }
    // Create signature of whole cookie string to check it
    // Start after the =" and cut off the signature + the " at the end
    std::string signString = cookie.substr(startCookie + 2, cookie.size() - startCookie - 3 - fields.back().size());
    // Res::Logger().Debug("String to check: " + signString);
    std::string expectedSignature = CreateSignature(signString);
    // Res::Logger().Debug("Expected signature: " + expectedSignature + ", got signature " + fields.back());

    const std::string& containedSignature = fields.back();
    // Compare signatures
    if (expectedSignature.size() != containedSignature.size())
    {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < expectedSignature.size(); ++i)
    {
        // xor all values to be time independent
        result |= expectedSignature[i] ^ containedSignature[i];
    }
    if (result != 0)
    {
        Res::Logger().Debug("Signatures not equal");
        return false;
    }
    // Check name
    if (fields[2] != s_cookieName)
    {
        Res::Logger().Debug("Names not equal" + fields[2] + "/" + s_cookieName);
        return false;
    }
    using std::chrono::system_clock;
    // Check timestamp
    return !(std::stoll(fields[1]) < system_clock::to_time_t(system_clock::now() - s_maxCookieAge));
}

std::string CookieVerify::CreateSignature(const std::string& str)
{
    using namespace CryptoPP;
    HMAC<SHA256> hmac(reinterpret_cast<const byte*>(s_cookieSecret), std::strlen(s_cookieSecret));
    std::string result;
    StringSource ssrc(str.c_str(), true, new HashFilter(hmac, new HexEncoder(new StringSink(result), false)));
    return result;
}
#include "Authenticator.h"

#include <absl/strings/str_cat.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>

constexpr std::chrono::seconds Authenticator::s_tokenExpiration;

namespace
{
    constexpr CryptoPP::byte key[] = {0x6A, 0x2D, 0xFC, 0xA3, 0x4B, 0xE8, 0xAE, 0x82, 0x96, 0x8B, 0x63, 0xE9, 0x6C, 0xD1, 0x9A,
        0xFE, 0x73, 0xC0, 0x2B, 0x09, 0xF8, 0xEA, 0x20, 0xAC, 0xF4, 0x26, 0x8B, 0x03, 0x5E, 0x2E, 0xE2, 0xEA};
} // namespace

bool Authenticator::ValidatePassword(const std::string& password, const std::string& hash) const
{
    using namespace CryptoPP;

    // For clarity
    using ByteString = std::basic_string<byte>;

    if (hash.empty() || hash.front() != '$')
    {
        return false;
    }
    std::size_t beginRounds = hash.find('$', 1);
    if (beginRounds == -1 || hash.substr(1, beginRounds - 1) != "pbkdf2-sha256")
    {
        return false;
    }
    std::size_t beginSalt = hash.find('$', beginRounds + 1);
    if (beginSalt == -1)
    {
        return false;
    }
    std::size_t beginChecksum = hash.find('$', beginSalt + 1);
    if (beginChecksum == -1)
    {
        return false;
    }
    std::size_t rounds = 0;
    try
    {
        rounds = std::stoul(hash.substr(beginRounds + 1, beginSalt - beginRounds - 1));
    }
    catch (...)
    {
        return false;
    }
    std::string salt = hash.substr(beginSalt + 1, beginChecksum - beginSalt - 1);
    std::string checksum = hash.substr(beginChecksum + 1);

    PKCS5_PBKDF2_HMAC<SHA256> pbkdf2;

    // 32 bytes = 256 bits
    SecByteBlock derived(32);

    ByteString rawSalt;
    StringSource(salt, true, new Base64Decoder(new StringSinkTemplate<ByteString>(rawSalt)));
    ByteString rawChecksum;
    StringSource(checksum, true, new Base64Decoder(new StringSinkTemplate<ByteString>(rawChecksum)));

    pbkdf2.DeriveKey(derived.data(), derived.size(), 0, (const byte*)password.data(), password.size(), rawSalt.data(),
        rawSalt.size(), rounds);

    if (derived.size() != rawChecksum.size())
    {
        // Something is wrong
        return false;
    }
    return VerifyBufsEqual(rawChecksum.data(), derived.data(), rawChecksum.size());
}

std::string Authenticator::CreateJWTToken(nlohmann::json payload) const
{
    using namespace CryptoPP;
    nlohmann::json header{{"alg", "HS256"}, {"typ", "JWT"}};

    if (payload.find("exp") != payload.end())
    {
        throw std::logic_error("Authenticator::CreateJWTToken must not be given expiration time");
    }
    int64_t expires = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch() + s_tokenExpiration)
                          .count();
    payload["exp"] = expires;

    // Base64-URL encode header
    std::string encodedHeader;
    StringSource(header.dump(), true, new Base64URLEncoder(new StringSink(encodedHeader)));
    // Base64-URL encode payload
    std::string encodedJson;
    StringSource(payload.dump(), true, new Base64URLEncoder(new StringSink(encodedJson)));

    std::string encodedPayload = encodedHeader + '.' + encodedJson;
    HMAC<SHA256> hmacSha256{key, sizeof(key)};
    std::string signature;
    StringSource(encodedPayload, true, new HashFilter(hmacSha256, new Base64URLEncoder(new StringSink(signature))));
    return absl::StrCat(encodedPayload, ".", signature);
}

nlohmann::json Authenticator::ValidateJWTToken(const std::string& encoded) const
{
    using namespace CryptoPP;

    std::size_t payloadBegin = encoded.find('.');
    if (payloadBegin == -1)
    {
        return nullptr;
    }
    std::size_t signatureBegin = encoded.find('.', payloadBegin + 1);
    if (signatureBegin == -1)
    {
        return nullptr;
    }

    std::string encodedHeader = encoded.substr(0, payloadBegin);
    std::string encodedPayload = encoded.substr(payloadBegin + 1, signatureBegin - payloadBegin - 1);
    std::string encodedSignature = encoded.substr(signatureBegin + 1);

    std::string header;
    StringSource(encodedHeader, true, new Base64URLDecoder(new StringSink(header)));
    std::string payload;
    StringSource(encodedPayload, true, new Base64URLDecoder(new StringSink(payload)));

    std::string signature;
    StringSource(encodedSignature, true, new Base64URLDecoder(new StringSink(signature)));

    try
    {
        nlohmann::json headerJson = nlohmann::json::parse(header);

        if (headerJson.at("typ") != "JWT" || headerJson.at("alg") != "HS256")
        {
            return nullptr;
        }

        // Verify signature
        bool result = false;
        HMAC<SHA256> hmacSha256(key, sizeof(key));
        StringSource(encoded.substr(0, signatureBegin) + signature, true,
            new HashVerificationFilter(hmacSha256, new ArraySink((byte*)&result, sizeof(result)),
                HashVerificationFilter::PUT_RESULT | HashVerificationFilter::HASH_AT_END));
        if (!result)
        {
            return nullptr;
        }

        nlohmann::json payloadJson = nlohmann::json::parse(payload);

        // Check expiration time, with 2 minutes margin for clock skew
        std::chrono::system_clock::time_point expires
            = std::chrono::system_clock::time_point(std::chrono::seconds(payloadJson.at("exp")));
        if (std::chrono::system_clock::now() > expires + std::chrono::minutes(2))
        {
            return nullptr;
        }

        // Return contained information
        return payloadJson;
    }
    catch (...)
    {
        return nullptr;
    }

    return nullptr;
}

std::string Authenticator::CreatePasswordHash(absl::string_view password) const
{
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;

    CryptoPP::SecByteBlock derived(32);

    CryptoPP::SecByteBlock rawSalt(16);
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(rawSalt.data(), rawSalt.size());
    std::string salt;
    CryptoPP::ArraySource(
        rawSalt.data(), rawSalt.size(), true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(salt), false));

    pbkdf2.DeriveKey(derived.data(), derived.size(), 0, reinterpret_cast<const CryptoPP::byte*>(password.data()), password.size(),
        rawSalt.data(), rawSalt.size(), s_hashRounds);

    std::string hash;
    CryptoPP::ArraySource(
        derived.data(), derived.size(), true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(hash), false));
    return absl::StrCat("$pbkdf2-sha256$", s_hashRounds, "$", salt, "$", hash);
}

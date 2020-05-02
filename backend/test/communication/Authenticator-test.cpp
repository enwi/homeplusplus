#include <gtest/gtest.h>

#include "communication/Authenticator.h"

TEST(Authenticator, CreateJWTToken)
{
    Authenticator a;
    nlohmann::json payload = {{"test", "a"}, {"b", "c"}};

    std::string token = a.CreateJWTToken(payload);

    nlohmann::json validated = a.ValidateJWTToken(token);
    EXPECT_EQ(payload.at("test"), validated.at("test"));
    EXPECT_EQ(payload.at("b"), validated.at("b"));
    EXPECT_EQ(payload.size(), validated.size() - 1);
    // Expiry time is set properly
    EXPECT_LE(validated.at("exp"),
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch() + Authenticator::s_tokenExpiration)
            .count());

    // Create token with explicit expiration time fails
    EXPECT_THROW(a.CreateJWTToken({{"exp", 12300}}), std::logic_error);
}

TEST(Authenticator, ValidateJWTToken)
{
    Authenticator a;

    // Successful validation is already tested in CreateJWTToken
    EXPECT_EQ(nullptr, a.ValidateJWTToken(""));
    EXPECT_EQ(nullptr, a.ValidateJWTToken("."));
    EXPECT_EQ(nullptr, a.ValidateJWTToken(".."));
    EXPECT_EQ(nullptr, a.ValidateJWTToken("asv.asdf.23"));
    EXPECT_EQ(nullptr, a.ValidateJWTToken("17689.189.1olaksf"));
    // incorrect signature
    EXPECT_EQ(nullptr,
        a.ValidateJWTToken("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
                           "e30.Et9HFtf9R3GEMA0IICOfFMVXY7kkTX1wr4qCyhIf58U"));
}

// TODO: TEST(Authenticator, ValidatePassword)
// TODO: TEST(Authenticator, CreatePasswordHash)
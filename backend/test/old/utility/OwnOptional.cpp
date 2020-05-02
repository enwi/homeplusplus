#include "utility/OwnOptional.h"

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(OwnOptional, Constructor)
{
    OwnOptional<int> optional1;
    EXPECT_FALSE(static_cast<bool>(optional1));

    OwnOptional<std::vector<std::string>> optional2{std::vector<std::string>{"A", "b", "C", "d", "EfGhIJ5"}};
    EXPECT_TRUE(static_cast<bool>(optional2));
}

TEST(OwnOptional, CopyConstructor)
{
    using namespace ::testing;
    const OwnOptional<int> optional1{24};
    EXPECT_TRUE(static_cast<bool>(optional1));

    OwnOptional<int> optional2{optional1};
    EXPECT_TRUE(static_cast<bool>(optional2));
    EXPECT_EQ(24, optional2.Get());

    OwnOptional<std::vector<std::string>> optional3{std::vector<std::string>{"A", "b", "C", "d", "EfGhIJ5"}};
    EXPECT_TRUE(static_cast<bool>(optional3));

    OwnOptional<std::vector<std::string>> optional4{std::move(optional3)};
    EXPECT_TRUE(static_cast<bool>(optional4));
    EXPECT_THAT(optional4.Get(), ElementsAre("A", "b", "C", "d", "EfGhIJ5"));
}

TEST(OwnOptional, OperatorEquals)
{
    using namespace ::testing;
    const OwnOptional<int> optional1{24};
    EXPECT_TRUE(static_cast<bool>(optional1));

    OwnOptional<int> optional2{33};
    EXPECT_TRUE(static_cast<bool>(optional1));
    EXPECT_EQ(33, optional2.Get());
    optional2 = optional1;
    EXPECT_TRUE(static_cast<bool>(optional2));
    EXPECT_EQ(24, optional2.Get());

    OwnOptional<std::vector<std::string>> optional3{std::vector<std::string>{"A", "b", "C", "d", "EfGhIJ5"}};
    EXPECT_TRUE(static_cast<bool>(optional3));

    OwnOptional<std::vector<std::string>> optional4{std::vector<std::string>{"1", "2", "#", "$", "5^7*9)"}};
    EXPECT_TRUE(static_cast<bool>(optional4));
    EXPECT_THAT(optional4.Get(), ElementsAre("1", "2", "#", "$", "5^7*9)"));
    optional4 = std::move(optional3);
    EXPECT_TRUE(static_cast<bool>(optional4));
    EXPECT_THAT(optional4.Get(), ElementsAre("A", "b", "C", "d", "EfGhIJ5"));
}

TEST(OwnOptional, Get)
{
    using namespace ::testing;
    OwnOptional<int> optional1;
    EXPECT_FALSE(static_cast<bool>(optional1));
    EXPECT_THROW(optional1.Get(), std::logic_error);

    const OwnOptional<int> optional2{};
    EXPECT_FALSE(static_cast<bool>(optional2));
    EXPECT_THROW(optional2.Get(), std::logic_error);

    OwnOptional<std::vector<std::string>> optional3{std::vector<std::string>{"A", "b", "C", "d", "EfGhIJ5"}};
    EXPECT_TRUE(static_cast<bool>(optional3));
    EXPECT_THAT(optional3.Get(), ElementsAre("A", "b", "C", "d", "EfGhIJ5"));

    const OwnOptional<std::vector<std::string>> optional4{std::vector<std::string>{"A", "b", "C", "d", "EfGhIJ5"}};
    EXPECT_TRUE(static_cast<bool>(optional4));
    EXPECT_THAT(optional4.Get(), ElementsAre("A", "b", "C", "d", "EfGhIJ5"));
}

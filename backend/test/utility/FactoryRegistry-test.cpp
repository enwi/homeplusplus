#include "utility/FactoryRegistry.h"

#include <gtest/gtest.h>

template <typename Factory>
class FactoryDerived : public FactoryRegistry<Factory>
{
public:
    // Provide access to protected member
    const std::vector<Factory>& GetFactories() const { return FactoryRegistry<Factory>::m_factories; }
};

void testFun() {}

using FactoryT = FactoryDerived<std::function<void()>>;

TEST(FactoryRegistry, DefaultConstructor)
{
    FactoryT f;
    FactoryT f2{};
    EXPECT_TRUE(f.GetFactories().empty());
    EXPECT_TRUE(f2.GetFactories().empty());
    EXPECT_EQ(0, f.GetFreeTypeId());
    EXPECT_EQ(0, f2.GetFreeTypeId());
}

TEST(FactoryRegistry, CopyConstructor)
{
    FactoryT f;
    EXPECT_TRUE(f.Register([] {}, 0));
    EXPECT_TRUE(f.Register([] {}, 2));
    EXPECT_TRUE(f.Register([] {}, 5));
    EXPECT_TRUE(f.Register([] {}, 3));
    FactoryT f2 = f;
    // Cannot compare std::function, only size
    EXPECT_EQ(f.GetFactories().size(), f2.GetFactories().size());
    FactoryT f3{f};
    EXPECT_EQ(f.GetFactories().size(), f3.GetFactories().size());
}

TEST(FactoryRegistry, MoveConstructor)
{
    FactoryT f;
    EXPECT_TRUE(f.Register([] {}, 0));
    EXPECT_TRUE(f.Register([] {}, 2));
    EXPECT_TRUE(f.Register([] {}, 5));
    EXPECT_TRUE(f.Register([] {}, 3));
    std::size_t s = f.GetFactories().size();
    FactoryT f2 = std::move(f);
    EXPECT_EQ(s, f2.GetFactories().size());
    FactoryT f3{std::move(f2)};
    EXPECT_EQ(s, f3.GetFactories().size());
}

TEST(FactoryRegistry, Register)
{
    FactoryT f;
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);

    // Lambda
    EXPECT_TRUE(f.Register([] {}, 0));
    EXPECT_EQ(1, f.GetFactories().size());
    EXPECT_NE(nullptr, f.GetFactories().at(0));

    // Normal function
    EXPECT_TRUE(f.Register(testFun, 1));

    // With empty types inbetween
    EXPECT_TRUE(f.Register([] {}, 3));
    EXPECT_TRUE(f.Register([] {}, 2));
    EXPECT_EQ(4, f.GetFactories().size());
    EXPECT_NE(nullptr, f.GetFactories().at(3));

    // Re-register on same type returns false
    EXPECT_FALSE(f.Register([] {}, 0));
    // Register nullptr
    EXPECT_FALSE(f.Register(nullptr, 4));
}

TEST(FactoryRegistry, GetFreeTypeId)
{
    FactoryT f;
    EXPECT_EQ(0, f.GetFreeTypeId());
    EXPECT_TRUE(f.Register([] {}, 0));
    EXPECT_EQ(1, f.GetFreeTypeId());
    EXPECT_TRUE(f.Register([] {}, 3));
    EXPECT_EQ(1, f.GetFreeTypeId());
    f.Remove(0);
    EXPECT_EQ(0, f.GetFreeTypeId());
}

TEST(FactoryRegistry, Remove)
{
    FactoryT f;
    EXPECT_TRUE(f.Register([] {}, 0));
    EXPECT_TRUE(f.Register([] {}, 2));
    EXPECT_TRUE(f.Register([] {}, 5));
    EXPECT_TRUE(f.Register([] {}, 3));
    f.Remove(0);
    EXPECT_EQ(nullptr, f.GetFactories()[0]);
    // Remove non-existant, should not do anything
    f.Remove(1);
    // Remove should reduce size
    f.Remove(5);
    EXPECT_EQ(4, f.GetFactories().size());
    f.Remove(2);
    f.Remove(3);
    EXPECT_TRUE(f.GetFactories().empty());
}

TEST(FactoryRegistry, RemoveAll)
{
    FactoryT f;
    EXPECT_TRUE(f.Register([] {}, 0));
    EXPECT_TRUE(f.Register([] {}, 2));
    EXPECT_TRUE(f.Register([] {}, 5));
    EXPECT_TRUE(f.Register([] {}, 3));
    f.RemoveAll();
    EXPECT_TRUE(f.GetFactories().empty());
    // Remove all from empty
    f.RemoveAll();
}

TEST(FactoryRegistry, OtherFunctors)
{
    // Basic, non-functional class
    struct Functor
    {
        Functor() = default;
        Functor(std::nullptr_t) {}
        bool operator()(const std::string& s) const { return !s.empty(); }
        // This does not work, except in this specific test case
        bool operator==(std::nullptr_t) const { return false; }
    };
    // Test compilation with any functor comparable to nullptr
    FactoryDerived<Functor> f;
    EXPECT_TRUE(f.Register(Functor{}, 0));
    EXPECT_NO_THROW(f.GetFactories().at(0)("hi"));
}
#include "standard-api/NodePath.h"

#include <gtest/gtest.h>

TEST(NodePath, Constructor)
{
    {
        // Default constructor
        NodePath p;
        EXPECT_EQ(0, p.GetDistance());
        EXPECT_EQ(0, p.GetPath());
        EXPECT_TRUE(p.IsGateway());
    }
    {
        NodePath p(0, 1);
        EXPECT_EQ(1, p.GetDistance());
        EXPECT_EQ(0, p.GetPath());
        EXPECT_FALSE(p.IsGateway());
    }
    {
        NodePath p(1751, 6);
        EXPECT_EQ(6, p.GetDistance());
        EXPECT_EQ(1751, p.GetPath());
        EXPECT_FALSE(p.IsGateway());
    }
    {
        // Initializer list constructor (empty)
        NodePath p{};
        EXPECT_TRUE(p.IsGateway());
    }
    {
        // Initializer list constructor
        NodePath p({1, 2, 1, 3, 0, 2});
        EXPECT_EQ(6, p.GetDistance());
        EXPECT_EQ(1, p.GetNthHop(0));
        EXPECT_EQ(2, p.GetNthHop(1));
        EXPECT_EQ(1, p.GetNthHop(2));
        EXPECT_EQ(3, p.GetNthHop(3));
        EXPECT_EQ(0, p.GetNthHop(4));
        EXPECT_EQ(2, p.GetNthHop(5));
        EXPECT_EQ(255, p.GetNthHop(6));
    }
    {
        // Initializer list throw
        EXPECT_THROW(NodePath({1, 45, 3}), std::invalid_argument);
        EXPECT_THROW(NodePath({4, 3}), std::invalid_argument);
    }
    {
        // Vector constructor (should be equivalent to initializer list)
        std::vector<uint8_t> v{1, 2, 1, 3, 0, 2};
        NodePath p1{1, 2, 1, 3, 0, 2};
        NodePath p2(std::move(v));
        EXPECT_EQ(p1, p2);
    }
}

TEST(NodePath, Distance)
{
    EXPECT_EQ(1, NodePath(0, 1).GetDistance());
    EXPECT_EQ(1, NodePath(1, 1).GetDistance());
    EXPECT_EQ(1, NodePath(2, 1).GetDistance());
    EXPECT_EQ(1, NodePath(3, 1).GetDistance());
    EXPECT_EQ(2, NodePath(4, 2).GetDistance());
    EXPECT_EQ(3, NodePath(16, 3).GetDistance());
    EXPECT_EQ(3, NodePath(17, 3).GetDistance());
    EXPECT_EQ(4, NodePath(64, 4).GetDistance());
    EXPECT_EQ(6, NodePath(1751, 6).GetDistance());
}

TEST(NodePath, NthHop)
{
    EXPECT_EQ(0, NodePath(0, 1).GetNthHop(0));
    EXPECT_EQ(1, NodePath(1, 1).GetNthHop(0));
    EXPECT_EQ(2, NodePath(2, 1).GetNthHop(0));
    EXPECT_EQ(3, NodePath(23, 3).GetNthHop(0));
    EXPECT_EQ(1, NodePath(23, 3).GetNthHop(1));
    EXPECT_EQ(1, NodePath(23, 3).GetNthHop(2));
    EXPECT_EQ(1, NodePath(340, 5).GetNthHop(4));
    EXPECT_EQ(0xFF, NodePath(340, 5).GetNthHop(5));
    EXPECT_EQ(0xFF, NodePath().GetNthHop(0));
}

TEST(NodePath, GetHops)
{
    EXPECT_EQ(std::vector<uint8_t>{1}, NodePath(1, 1).GetHops());
    EXPECT_EQ(std::vector<uint8_t>{}, NodePath().GetHops());
    EXPECT_EQ(std::vector<uint8_t>({1, 2, 2, 1, 2}), NodePath({1, 2, 2, 1, 2}).GetHops());
}

TEST(NodePath, CommonPath)
{
    EXPECT_EQ(NodePath({1, 2}).GetCommonPath(NodePath({1, 2, 1})), NodePath({1, 2}));
    EXPECT_EQ(NodePath(), NodePath().GetCommonPath(NodePath({1, 1})));
    EXPECT_EQ(NodePath(), NodePath({3, 1}).GetCommonPath(NodePath({1, 2})));
    EXPECT_EQ(NodePath({1, 2}), NodePath({1, 2, 1}).GetCommonPath(NodePath({1, 2})));
    EXPECT_EQ(NodePath({1, 2, 1}), NodePath({1, 2, 1}).GetCommonPath(NodePath({1, 2, 1})));
    EXPECT_EQ(NodePath({1, 2}), NodePath({1, 2, 3}).GetCommonPath(NodePath({1, 2, 1})));
}

TEST(NodePath, NextToGateway)
{
    EXPECT_EQ(NodePath(), NodePath().NextToGateway());
    EXPECT_EQ(NodePath(), NodePath(1, 1).NextToGateway());
    EXPECT_EQ(NodePath(1, 1), NodePath({1, 2}).NextToGateway());
    EXPECT_EQ(NodePath({1, 2}), NodePath({1, 2, 1}).NextToGateway());
}

TEST(NodePath, NextToThis)
{
    EXPECT_EQ(NodePath(), NodePath().NextToThis(NodePath()));
    EXPECT_EQ(NodePath({1, 2}), NodePath().NextToThis(NodePath({1, 2, 1})));
    EXPECT_EQ(NodePath({1}), NodePath({1, 2, 1}).NextToThis(NodePath()));
    EXPECT_EQ(NodePath({1}), NodePath({1, 2}).NextToThis(NodePath()));
    EXPECT_EQ(NodePath({2}), NodePath({1, 2}).NextToThis(NodePath({2, 1})));
    EXPECT_EQ(NodePath({1, 2, 3}), NodePath({1, 2, 3, 1}).NextToThis(NodePath({1, 2})));
    EXPECT_EQ(NodePath(), NodePath({1}).NextToThis(NodePath({3})));
    EXPECT_EQ(NodePath({1}), NodePath({1}).NextToThis(NodePath()));
}
TEST(NodePath, NextFromThis)
{
    EXPECT_EQ(NodePath(), NodePath().NextFromThis(NodePath()));
    EXPECT_EQ(NodePath({1, 2}), NodePath({1, 2, 1}).NextFromThis(NodePath()));
    EXPECT_EQ(NodePath({1}), NodePath().NextFromThis(NodePath({1, 2, 1})));
    EXPECT_EQ(NodePath({1}), NodePath().NextFromThis(NodePath({1, 2})));
    EXPECT_EQ(NodePath({2}), NodePath({2, 1}).NextFromThis(NodePath({1, 2})));
    EXPECT_EQ(NodePath({1, 2, 3}), NodePath({1, 2}).NextFromThis(NodePath({1, 2, 3, 1})));
    EXPECT_EQ(NodePath(), NodePath({3}).NextFromThis(NodePath({1})));
    EXPECT_EQ(NodePath({1}), NodePath().NextFromThis(NodePath({1})));
}

TEST(NodePath, SubParentNode)
{
    EXPECT_TRUE(NodePath().IsParentNodeOf({1}));
    EXPECT_TRUE(NodePath({1, 2}).IsSubNodeOf({1}));
    EXPECT_FALSE(NodePath({1, 2, 3}).IsSubNodeOf({1, 2, 3, 2}));
    EXPECT_FALSE(NodePath({1, 2, 3}).IsParentNodeOf({1, 2}));
    EXPECT_FALSE(NodePath({1, 3, 1}).IsParentNodeOf({2, 1}));
}

TEST(NodePath, equality)
{
    EXPECT_TRUE(NodePath() == NodePath());
    EXPECT_TRUE(NodePath() == NodePath({}));
    EXPECT_TRUE(NodePath(1, 2) == NodePath(1, 2));
    EXPECT_TRUE(NodePath(3, 4) == NodePath(3, 4));
    EXPECT_TRUE(NodePath(0, 0) == NodePath());
    EXPECT_TRUE(NodePath(0, 1) == NodePath({0}));
    EXPECT_FALSE(NodePath(0, 1) == NodePath(0, 2));
    EXPECT_FALSE(NodePath(3, 2) == NodePath(4, 2));
    EXPECT_FALSE(NodePath(0, 1) == NodePath({2}));
    EXPECT_FALSE(NodePath() == NodePath(0, 2));
    EXPECT_FALSE(NodePath({1}) == NodePath({3, 2}));
    // Inverted for !=
    EXPECT_FALSE(NodePath() != NodePath());
    EXPECT_FALSE(NodePath() != NodePath({}));
    EXPECT_FALSE(NodePath(1, 2) != NodePath(1, 2));
    EXPECT_FALSE(NodePath(3, 4) != NodePath(3, 4));
    EXPECT_FALSE(NodePath(0, 0) != NodePath());
    EXPECT_FALSE(NodePath(0, 1) != NodePath({0}));
    EXPECT_TRUE(NodePath(0, 1) != NodePath(0, 2));
    EXPECT_TRUE(NodePath(3, 2) != NodePath(4, 2));
    EXPECT_TRUE(NodePath(0, 1) != NodePath({2}));
    EXPECT_TRUE(NodePath() != NodePath(0, 2));
    EXPECT_TRUE(NodePath({1}) != NodePath({3, 2}));
}

TEST(NodePath, comparison)
{
    // Ordering by <, >, <=, >=
    //(dist << 16 | path)
    EXPECT_TRUE(NodePath() < NodePath(0, 1));
    EXPECT_TRUE(NodePath({1, 2, 1}) > NodePath({1, 2}));
    EXPECT_FALSE(NodePath({1, 2, 1}) < NodePath({0, 1}));
    EXPECT_TRUE(NodePath() <= NodePath());
    EXPECT_TRUE(NodePath() >= NodePath());

    EXPECT_TRUE(NodePath(32, 4) > NodePath(31, 4));
    EXPECT_FALSE(NodePath(32, 4) < NodePath(31, 4));
    EXPECT_TRUE(NodePath(32, 4) >= NodePath(31, 4));
    EXPECT_FALSE(NodePath(32, 4) <= NodePath(31, 4));

    EXPECT_FALSE(NodePath(32, 4) > NodePath(31, 5));
    EXPECT_TRUE(NodePath(32, 4) < NodePath(31, 5));
    EXPECT_FALSE(NodePath(32, 4) >= NodePath(31, 5));
    EXPECT_TRUE(NodePath(32, 4) <= NodePath(31, 5));
}

TEST(NodePath, ToString)
{
    EXPECT_EQ("Path:3-0-0-0-x-x-x-x", NodePath(3, 4).ToString());
    EXPECT_EQ("Path:0-0-2-0-x-x-x-x", NodePath(32, 4).ToString());
    EXPECT_EQ("Path:0-x-x-x-x-x-x-x", NodePath(0, 1).ToString());
}

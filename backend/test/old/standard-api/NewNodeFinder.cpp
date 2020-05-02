#include "standard-api/NewNodeFinder.h"

#include <gtest/gtest.h>

#include "NodeSerializeUtils.h"

#include "../mocks/MockDBHandler.h"
#include "../mocks/MockNodeCommunication.h"
#include "database/DBNodeSerialize.h"
#include "standard-api/communication/MessageTypes.h"

TEST(NewNodeFinder, FindAvailableNodes)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc{23};
    MockDBHandler db;
    DBNodeSerialize ns{db};
    NewNodeFinder f{nc, ns};

    // No node found
    {
        EXPECT_CALL(
            nc, QueueMessage(Messages::FindNode(NodePath(), NodePath()), 0, NodeCommunication::s_broadcastAddr));

        NewNodeSet found = f.FindAvailableNodes();
        EXPECT_TRUE(found.GetPaths().empty());
    }
    // Node found, gateway first receiver
    {
        const uint8_t randId = 23;
        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc, QueueMessage(Messages::FindNode(NodePath(), NodePath()), 0, NodeCommunication::s_broadcastAddr))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    ::Message m(NodePath(), NodePath(), false, 1, NodeCommands::REQUEST_CONNECTION);
                    m.SetBVal(0, randId);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m, 0));
                });
                return std::future<bool>();
            }));
        NewNodeSet found = f.FindAvailableNodes();
        const auto& paths = found.GetPaths();
        EXPECT_THAT(paths, UnorderedElementsAre(std::make_pair(randId, NodePath())));
    }
    // Node found, node first receiver
    {
        const uint8_t randId = 23;
        const NodePath firstReceive{3, 2};
        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc, QueueMessage(Messages::FindNode(NodePath(), NodePath()), 0, NodeCommunication::s_broadcastAddr))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    ::Message m(NodePath(), firstReceive, false, 4, NodeCommands::REQUEST_CONNECTION);
                    m.SetBVal(0, randId);
                    // Set path of first receiver
                    m.SetUIVal(1, firstReceive.GetPath());
                    m.SetBVal(3, firstReceive.GetDistance());
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m, 1));
                });
                return std::future<bool>();
            }));
        NewNodeSet found = f.FindAvailableNodes();
        const auto& paths = found.GetPaths();
        EXPECT_THAT(paths, UnorderedElementsAre(std::make_pair(randId, firstReceive)));
    }
    // Node found multiple times
    {
        const uint8_t randId = 23;
        const NodePath firstReceive{3, 2};
        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc, QueueMessage(Messages::FindNode(NodePath(), NodePath()), 0, NodeCommunication::s_broadcastAddr))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    ::Message m(NodePath(), firstReceive, false, 4, NodeCommands::REQUEST_CONNECTION);
                    m.SetBVal(0, randId);
                    // Set path of first receiver
                    m.SetUIVal(1, firstReceive.GetPath());
                    m.SetBVal(3, firstReceive.GetDistance());
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m, 1));
                    // second time received by gateway
                    ::Message m2(NodePath(), NodePath(), false, 1, NodeCommands::REQUEST_CONNECTION);
                    m2.SetBVal(0, randId);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m2, 0));
                });
                return std::future<bool>();
            }));
        NewNodeSet found = f.FindAvailableNodes();
        const auto& paths = found.GetPaths();
        EXPECT_THAT(
            paths, UnorderedElementsAre(std::make_pair(randId, NodePath()), std::make_pair(randId, firstReceive)));
    }
    // More than one node found
    {
        const uint8_t randId = 23;
        const uint8_t randId2 = 43;
        const NodePath firstReceive{3, 2};
        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc, QueueMessage(Messages::FindNode(NodePath(), NodePath()), 0, NodeCommunication::s_broadcastAddr))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    ::Message m(NodePath(), firstReceive, false, 4, NodeCommands::REQUEST_CONNECTION);
                    m.SetBVal(0, randId);
                    // Set path of first receiver
                    m.SetUIVal(1, firstReceive.GetPath());
                    m.SetBVal(3, firstReceive.GetDistance());
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m, 1));
                    // second time received by gateway
                    ::Message m2(NodePath(), NodePath(), false, 1, NodeCommands::REQUEST_CONNECTION);
                    m2.SetBVal(0, randId2);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m2, 0));
                });
                return std::future<bool>();
            }));
        NewNodeSet found = f.FindAvailableNodes();
        const auto& paths = found.GetPaths();
        EXPECT_THAT(
            paths, UnorderedElementsAre(std::make_pair(randId2, NodePath()), std::make_pair(randId, firstReceive)));
    }
}

TEST(NewNodeFinder, GetPath)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc{23};
    MockDBHandler db;
    db.UseDefaults();
    DBNodeSerialize ns{db};
    NewNodeFinder f{nc, ns};

    // Node not contained in list
    {
        NewNodeSet found{std::multimap<uint8_t, NodePath>()};
        EXPECT_CALL(db, GetROStatement(_)).Times(AnyNumber());
        EXPECT_EQ(NodePath(), f.GetPath(12, found));
    }
    // Node contained in list, has space
    {
        const uint8_t randId = 12;
        const NodePath p{1};
        // One child node of Node at path p
        NodeData child0{1, "", "", {}, {}, "", NodePath{1, 0}};
        NewNodeSet found{std::multimap<uint8_t, NodePath>{{randId, p}}};
        EXPECT_CALL(db, GetROStatement(_)).Times(AnyNumber());
        EXPECT_EQ(NodePath({1, 0}), f.GetPath(randId, found));
    }
    // Node contained in list, has space
    {
        const uint8_t randId = 12;
        const NodePath p{1};
        // One child node of Node at path p
        NodeData child0{1, "", "", {}, {}, "", NodePath{1, 0}};
        NewNodeSet found{std::multimap<uint8_t, NodePath>{{randId, p}}};
        ExpectGetNodePath(db, child0);
        EXPECT_EQ(NodePath({1, 1}), f.GetPath(randId, found));
    }
    // Node contained in list, has no space
    {
        const uint8_t randId = 12;
        const NodePath p{1};
        // four child nodes of Node at path p, which is the maximum
        NodeData child0{1, "", "", {}, {}, "", NodePath{1, 0}};
        NodeData child1{2, "", "", {}, {}, "", NodePath{1, 1}};
        NodeData child2{3, "", "", {}, {}, "", NodePath{1, 2}};
        NodeData child3{4, "", "", {}, {}, "", NodePath{1, 3}};
        NewNodeSet found{std::multimap<uint8_t, NodePath>{{randId, p}}};

        // In reverse order, because it retires
        ExpectGetNodePath(db, child3);
        ExpectGetNodePath(db, child2);
        ExpectGetNodePath(db, child1);
        ExpectGetNodePath(db, child0);
        EXPECT_EQ(NodePath(), f.GetPath(randId, found));
    }
    // Node contained multiple times, one has no space, other has
    {
        const uint8_t randId = 12;
        const NodePath p{1};
        const NodePath p1{2};
        // four child nodes of Node at path p, which is the maximum
        NodeData child0{1, "", "", {}, {}, "", NodePath{1, 0}};
        NodeData child1{2, "", "", {}, {}, "", NodePath{1, 1}};
        NodeData child2{3, "", "", {}, {}, "", NodePath{1, 2}};
        NodeData child3{4, "", "", {}, {}, "", NodePath{1, 3}};
        NewNodeSet found{std::multimap<uint8_t, NodePath>{{randId, p}, {randId, p1}}};

        // In reverse order, because it retires
        ExpectGetNodePath(db, child3);
        ExpectGetNodePath(db, child2);
        ExpectGetNodePath(db, child1);
        ExpectGetNodePath(db, child0);
        EXPECT_EQ(NodePath({2, 0}), f.GetPath(randId, found));
    }
    // Node contained multiple times, no one has space
    {
        const uint8_t randId = 12;
        const NodePath p{1};
        const NodePath p1{2};
        // four child nodes of Node at path p, which is the maximum
        NodeData child0{1, "", "", {}, {}, "", NodePath{1, 0}};
        NodeData child1{2, "", "", {}, {}, "", NodePath{1, 1}};
        NodeData child2{3, "", "", {}, {}, "", NodePath{1, 2}};
        NodeData child3{4, "", "", {}, {}, "", NodePath{1, 3}};
        NodeData child10{1, "", "", {}, {}, "", NodePath{2, 0}};
        NodeData child11{2, "", "", {}, {}, "", NodePath{2, 1}};
        NodeData child12{3, "", "", {}, {}, "", NodePath{2, 2}};
        NodeData child13{4, "", "", {}, {}, "", NodePath{2, 3}};
        NewNodeSet found{std::multimap<uint8_t, NodePath>{{randId, p}, {randId, p1}}};

        // In reverse order, because it retires
        ExpectGetNodePath(db, child13);
        ExpectGetNodePath(db, child12);
        ExpectGetNodePath(db, child11);
        ExpectGetNodePath(db, child10);
        ExpectGetNodePath(db, child3);
        ExpectGetNodePath(db, child2);
        ExpectGetNodePath(db, child1);
        ExpectGetNodePath(db, child0);
        EXPECT_EQ(NodePath(), f.GetPath(randId, found));
    }
}

TEST(NewNodeFinder, AddNode)
{
    using namespace ::testing;
    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    MockNodeCommunication nc{23};
    MockDBHandler db;
    db.UseDefaults();
    DBNodeSerialize ns{db};
    NewNodeFinder f{nc, ns};
    // Fail, direct send
    {
        const uint8_t randId = 23;
        const NodeData node{3, "", "", {}, {}, "", NodePath{1}};

        EXPECT_CALL(nc,
            QueueMessage(Messages::AssignId(
                             node.m_path.NextToGateway(), NodePath(), randId, node.m_id, nc.GetBaseId(), node.m_path),
                0, NodeCommunication::s_broadcastAddr));
        EXPECT_FALSE(f.AddNode(randId, node));
        Mock::VerifyAndClearExpectations(&nc);
    }
    // Fail, indirect send
    {
        const uint8_t randId = 23;
        const NodeData node{3, "", "", {}, {}, "", NodePath{1, 0}};

        EXPECT_CALL(nc,
            SendMessage(Messages::AssignId(
                            node.m_path.NextToGateway(), NodePath(), randId, node.m_id, nc.GetBaseId(), node.m_path),
                0));
        EXPECT_FALSE(f.AddNode(randId, node));
        Mock::VerifyAndClearExpectations(&nc);
    }
    // Success, direct send
    {
        const uint8_t randId = 23;
        const NodeData node{3, "", "", {}, {}, "", NodePath{1}};

        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc,
            QueueMessage(Messages::AssignId(
                             node.m_path.NextToGateway(), NodePath(), randId, node.m_id, nc.GetBaseId(), node.m_path),
                0, NodeCommunication::s_broadcastAddr))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    ::Message m(NodePath(), NodePath(), false, 1, NodeCommands::ACK);
                    m.SetBVal(0, randId);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m, 0));
                });
                return std::future<bool>();
            }));
        EXPECT_TRUE(f.AddNode(randId, node));
        Mock::VerifyAndClearExpectations(&nc);
    }
    // Success, direct send with irrelevant messages
    {
        const uint8_t randId = 23;
        const NodeData node{3, "", "", {}, {}, "", NodePath{1}};

        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc,
            QueueMessage(Messages::AssignId(
                             node.m_path.NextToGateway(), NodePath(), randId, node.m_id, nc.GetBaseId(), node.m_path),
                0, NodeCommunication::s_broadcastAddr))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    // Wrong randId
                    ::Message m1(NodePath(), NodePath(), false, 1, NodeCommands::ACK);
                    m1.SetBVal(0, 0);
                    // Wrong command
                    ::Message m2(NodePath(), NodePath(), false, 1, NodeCommands::NAK);
                    m2.SetBVal(0, randId);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m1, 0));
                    ::Message m3(NodePath(), NodePath(), false, 1, NodeCommands::ACK);
                    m3.SetBVal(0, randId);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m1, 0));
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m2, 0));
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m3, 0));
                });
                return std::future<bool>();
            }));
        EXPECT_TRUE(f.AddNode(randId, node));
        Mock::VerifyAndClearExpectations(&nc);
    }
    // Success, indirect send
    {
        const uint8_t randId = 23;
        const NodeData node{3, "", "", {}, {}, "", NodePath{1, 0}};

        // TODO: find a better way to insert messages into the event system at the right time (after the handler was
        // added)
        std::future<void> future;
        EXPECT_CALL(nc,
            SendMessage(Messages::AssignId(
                            node.m_path.NextToGateway(), NodePath(), randId, node.m_id, nc.GetBaseId(), node.m_path),
                0))
            .WillOnce(InvokeWithoutArgs([&]() {
                future = std::async(std::launch::async, [=]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    ::Message m(NodePath(), NodePath{1}, false, 1, NodeCommands::ACK);
                    m.SetBVal(0, randId);
                    Res::EventSystem().HandleEvent(Events::NodeMessageEvent(m, 0));
                });
                return std::future<bool>();
            }));
        EXPECT_TRUE(f.AddNode(randId, node));
        Mock::VerifyAndClearExpectations(&nc);
    }
}
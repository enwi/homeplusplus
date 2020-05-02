#include "events/WebsocketEventHandler.h"

#include <gtest/gtest.h>

#include "../communication/TestWebsocketCommunication.h"
#include "communication/WebsocketCommunication.h"
#include "events/Events.h"

TEST(WebsocketEventHandler, ShouldExecuteOn)
{
    WebsocketCommunication wc{};
    WebsocketEventHandler h{wc};

    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::nodeChange));
    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::actionChange));
    EXPECT_TRUE(h.ShouldExecuteOn(EventTypes::ruleChange));

    EXPECT_FALSE(h.ShouldExecuteOn(EventTypes::nodeMessage));
    EXPECT_FALSE(h.ShouldExecuteOn(EventTypes::error));
}

TEST(WebsocketEventHandler, HandleEventNodeChange)
{
    using namespace ::testing;
    TestWebsocketCommunication wc;
    WebsocketEventHandler h{wc.ws};

    WebsocketCommunication::server::connection_ptr c0 = std::make_shared<MockWebsocketConnection>();
    WebsocketCommunication::server::connection_ptr c1 = std::make_shared<MockWebsocketConnection>();
    wc.WSOnOpen(c0);
    wc.WSOnOpen(c1);

    // Node change
    {
        NodeData n{23, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed{23, "n1", "l1", {}, {}, "s1", NodePath(1, 1)};

        Events::NodeChangeEvent e{n, changed, Events::NodeFields::ALL};

        EXPECT_CALL(wc.GetServer(), send(_, "Node:" + changed.ToJson().dump() + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Node add
    {
        NodeData n = NodeData::EmptyNode();
        NodeData changed{23, "n1", "l1", {}, {}, "s1", NodePath(1, 1)};

        Events::NodeChangeEvent e{n, changed, Events::NodeFields::ADD};

        EXPECT_CALL(wc.GetServer(), send(_, "Node:" + changed.ToJson().dump() + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Node remove
    {
        NodeData n{23, "n", "l", {}, {}, "s", NodePath(1, 1)};
        NodeData changed = NodeData::EmptyNode();

        Events::NodeChangeEvent e{n, changed, Events::NodeFields::REMOVE};

        EXPECT_CALL(wc.GetServer(), send(_, "DELETE_NODE;" + std::to_string(n.m_id) + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketEventHandler, HandleEventActionChange)
{
    using namespace ::testing;
    using ::Action;
    TestWebsocketCommunication wc;
    WebsocketEventHandler h{wc.ws};

    WebsocketCommunication::server::connection_ptr c0 = std::make_shared<MockWebsocketConnection>();
    WebsocketCommunication::server::connection_ptr c1 = std::make_shared<MockWebsocketConnection>();
    wc.WSOnOpen(c0);
    wc.WSOnOpen(c1);

    // Action change
    {
        Action a{
            23,
            "n",
            "l",
            2,
            {},
        };
        Action changed{23, "n1", "l1", 2, {}};

        Events::ActionChangeEvent e{a, changed, Events::ActionFields::ALL};

        EXPECT_CALL(wc.GetServer(), send(_, "Action:" + changed.ToJson().dump() + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Action add
    {
        Action a;
        Action changed{23, "n1", "l1", 2, {}};

        Events::ActionChangeEvent e{a, changed, Events::ActionFields::ADD};

        EXPECT_CALL(wc.GetServer(), send(_, "Action:" + changed.ToJson().dump() + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Action remove
    {
        Action a{
            23,
            "n",
            "l",
            2,
            {},
        };
        Action changed;

        Events::ActionChangeEvent e{a, changed, Events::ActionFields::REMOVE};

        EXPECT_CALL(wc.GetServer(), send(_, "DELETE_ACTION;" + std::to_string(a.GetId()) + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}

TEST(WebsocketEventHandler, HandleEventRuleChange)
{
    using namespace ::testing;
    using ::Action;
    TestWebsocketCommunication wc;
    WebsocketEventHandler h{wc.ws};
    EventSystem ev;

    WebsocketCommunication::server::connection_ptr c0 = std::make_shared<MockWebsocketConnection>();
    WebsocketCommunication::server::connection_ptr c1 = std::make_shared<MockWebsocketConnection>();
    wc.WSOnOpen(c0);
    wc.WSOnOpen(c1);

    // Rule change
    {
        Rule r{23, "n", "l", 2, nullptr, Action()};
        Rule changed{23, "n1", "l1", 2, nullptr, Action()};

        Events::RuleChangeEvent e{r, changed, Events::RuleFields::ALL};

        EXPECT_CALL(wc.GetServer(), send(_, "Rule:" + changed.ToJson().dump() + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Rule add
    {
        Rule r;
        Rule changed{23, "n1", "l1", 2, nullptr, Action()};

        Events::RuleChangeEvent e{r, changed, Events::RuleFields::ADD};

        EXPECT_CALL(wc.GetServer(), send(_, "Rule:" + changed.ToJson().dump() + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
    // Rule remove
    {
        Rule r{23, "n", "l", 2, nullptr, Action()};
        Rule changed;

        Events::RuleChangeEvent e{r, changed, Events::RuleFields::REMOVE};

        EXPECT_CALL(wc.GetServer(), send(_, "DELETE_RULE;" + std::to_string(r.GetId()) + "\n", _)).Times(2);

        h.HandleEvent(e);
        Mock::VerifyAndClearExpectations(&wc.GetServer());
    }
}
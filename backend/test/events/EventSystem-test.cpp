#include <gtest/gtest.h>

#include "../mocks/MockEventHandler.h"
#include "events/EventSystem.h"
#include "events/Events.h"

TEST(EventSystem, AddHandler)
{
    using namespace ::testing;
    EventSystem evSys;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    std::shared_ptr<MockEventHandler> h = std::make_shared<MockEventHandler>();
    std::shared_ptr<MockEventHandler> h2 = std::make_shared<MockEventHandler>();

    EXPECT_CALL(*h2, HandleEvent(_)).Times(0);

    evSys.AddHandler(h);
    EXPECT_NO_THROW(evSys.AddHandler(nullptr));

    EXPECT_CALL(*h, HandleEvent(_))
        .Times(3)
        .WillOnce(Invoke([&](const EventBase& e) {
            evSys.AddHandler(h2);
            // Check that add is only performed after handler completed
            evSys.HandleEvent(e);
			return PostEventState::handled;
        }))
        .WillRepeatedly(Return(PostEventState::handled));

    evSys.HandleEvent(Events::ErrorEvent("test", "test"));

    // h2 is only invoked in second HandleEvent
    EXPECT_CALL(*h2, HandleEvent(_)).Times(1);
    evSys.HandleEvent(Events::ErrorEvent("test", "test"));
}

TEST(EventSystem, RemoveHandler)
{
    using namespace ::testing;
    EventSystem evSys;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    std::shared_ptr<MockEventHandler> h = std::make_shared<MockEventHandler>();
    std::shared_ptr<MockEventHandler> h2 = std::make_shared<MockEventHandler>();

    EXPECT_CALL(*h2, HandleEvent(_)).Times(2);

    evSys.AddHandler(h);
    evSys.AddHandler(h2);
    EXPECT_NO_THROW(evSys.RemoveHandler(nullptr));

    EXPECT_CALL(*h, HandleEvent(_)).Times(3).WillOnce(Invoke([&](const EventBase& e) {
        evSys.RemoveHandler(h2.get());
        // Check that remove is only performed after handler completed
        evSys.HandleEvent(e);
        return PostEventState::handled;
    })).WillRepeatedly(Return(PostEventState::handled));

    evSys.HandleEvent(Events::ErrorEvent("test", "test"));

    // h2 is no longer invoked in second HandleEvent
    EXPECT_CALL(*h2, HandleEvent(_)).Times(0);
    evSys.HandleEvent(Events::ErrorEvent("test", "test"));

    evSys.RemoveHandler(h.get());
    evSys.HandleEvent(Events::ErrorEvent("test", "test"));
}

TEST(EventSystem, HandleEvent)
{
    using namespace ::testing;
    EventSystem evSys;

    Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
    Res::Logger().SetFileLevel(Logger::LEVEL_NONE);

    std::shared_ptr<MockEventHandler> h = std::make_shared<MockEventHandler>();

    Events::ErrorEvent event("error", "place");

    evSys.AddHandler(h);

    {
        EXPECT_CALL(*h, HandleEvent(_)).WillOnce(Invoke([&](const EventBase& e) {
            const Events::ErrorEvent& casted = EventCast<Events::ErrorEvent>(e);
            EXPECT_EQ("error", casted.GetMessage());
            EXPECT_EQ("place", casted.GetPlace());
            EXPECT_EQ(EventTypes::error, casted.GetType());
			return PostEventState::handled;
        }));
    }
    evSys.HandleEvent(event);

    // Recursion
    EXPECT_CALL(*h, HandleEvent(_))
        .Times(2)
        .WillOnce(Invoke([&](const EventBase& e) {
            evSys.HandleEvent(e);
            return PostEventState::handled;
        }))
        .WillRepeatedly(Return(PostEventState::handled));
    evSys.HandleEvent(event);
}

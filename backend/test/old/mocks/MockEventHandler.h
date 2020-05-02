#ifndef MOCK_EVENT_HANDLER_H
#define MOCK_EVENT_HANDLER_H

#include <gmock/gmock.h>

#include "events/EventSystem.h"

class MockEventHandler : public EventHandler
{
public:
    MockEventHandler()
    {
        // Return true by default instead of false
        ON_CALL(*this, ShouldExecuteOn(::testing::_)).WillByDefault(::testing::Return(true));
    }
    MOCK_CONST_METHOD1(ShouldExecuteOn, bool(EventType));
    MOCK_METHOD1(HandleEvent, PostEventState(const EventBase&));
};

// Removes event handler, even if an exception is thrown somewhere
class CleanupEventHandler
{
public:
    CleanupEventHandler(EventSystem& evSys, const EventHandler& eventHandler) : evSys(evSys), e(&eventHandler) {}
    CleanupEventHandler(EventSystem& evSys, const std::shared_ptr<EventHandler>& eventHandler)
        : evSys(evSys), e(eventHandler.get())
    {}
    ~CleanupEventHandler() { evSys.RemoveHandler(e); }

private:
    EventSystem& evSys;
    const EventHandler* e;
};

#endif
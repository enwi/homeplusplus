#ifndef MOCK_EVENT_HANDLER_H
#define MOCK_EVENT_HANDLER_H

#include <gmock/gmock.h>

#include "events/EventSystem.h"

class MockEventHandler : public EventHandler<EventBase>
{
public:
    MOCK_METHOD1(HandleEvent, PostEventState(const EventBase&));
};

// Removes event handler, even if an exception is thrown somewhere
class CleanupEventHandler
{
public:
    CleanupEventHandler(EventSystem& evSys, const EventHandler<EventBase>& eventHandler)
        : evSys(evSys), e(&eventHandler)
    {}
    CleanupEventHandler(EventSystem& evSys, const std::shared_ptr<EventHandler<EventBase>>& eventHandler)
        : evSys(evSys), e(eventHandler.get())
    {}
    ~CleanupEventHandler() { evSys.RemoveHandler(e); }

private:
    EventSystem& evSys;
    const EventHandler<EventBase>* e;
};

#endif
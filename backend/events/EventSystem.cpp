#include "EventSystem.h"

#include <algorithm>
#include <cassert>

#include "../api/Resources.h"
#include "../utility/Logger.h"

void EventSystem::AddHandler(EventHandler<EventBase>::Ptr handler)
{
    if (handler != nullptr)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_inLoop)
        {
            m_deferredAdd.push_back(std::move(handler));
        }
        else
        {
            m_handlers.push_back(std::move(handler));
        }
    }
}

void EventSystem::RemoveHandler(const EventHandler<EventBase>* handler)
{
    if (handler != nullptr)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_inLoop)
        {
            Res::Logger().Warning("Removing event handlers inside event loop. Use PostEventState for this");
            m_deferredRemove.push_back(handler);
        }
        else
        {
            auto pos = std::find_if(m_handlers.begin(), m_handlers.end(),
                [handler](const EventHandler<EventBase>::Ptr& r) { return r.get() == handler; });
            if (pos != m_handlers.end())
            {
                m_handlers.erase(pos);
            }
            else
            {
                Res::Logger().Warning("Tried to remove an EventHandler which does not exist!");
            }
        }
    }
}

void EventSystem::HandleEvent(const EventBase& e)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    Res::Logger().Debug("Handle event with type: " + std::to_string(static_cast<int>(e.GetType())));
    bool innerLoop = false;
    {
        if (m_inLoop)
        {
            innerLoop = true;
        }
        m_inLoop = true;
        size_t handledCount = 0;
        for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it)
        {
            assert(*it != nullptr);
            try
            {
                PostEventState state = (*it)->HandleEvent(e);
                if ((state & PostEventState::handled) != PostEventState::notHandled)
                {
                    ++handledCount;
                }
                if ((state & PostEventState::shouldRemove) != PostEventState::notHandled)
                {
                    m_deferredRemove.push_back(it->get());
                }
                if ((state & PostEventState::consumeEvent) != PostEventState::notHandled)
                {
                    // Do not execute more event handlers
                    break;
                }
            }
            catch (const std::exception& exc)
            {
                Res::Logger().Error("Exception in EventSystem HandleEvent: EventType: "
                    + std::to_string(static_cast<int>(e.GetType())) + " Message: " + exc.what());
            }
        }
        if (handledCount == 0)
        {
            Res::Logger().Debug("Unhandled event with type: " + std::to_string(static_cast<int>(e.GetType())));
        }
        if (!innerLoop)
        {
            m_inLoop = false;
        }
    }
    // Prevent from adding when handling an event from within an event handler,
    // when the list is still being iterated over
    if (!innerLoop)
    {
        for (auto it = m_deferredRemove.begin(); it != m_deferredRemove.end(); ++it)
        {
            assert(*it != nullptr);
            auto pos = std::find_if(m_handlers.begin(), m_handlers.end(),
                [&it](const EventHandler<EventBase>::Ptr& r) { return r.get() == *it; });
            if (pos != m_handlers.end())
            {
                m_handlers.erase(pos);
            }
        }
        m_deferredRemove.clear();
        for (auto it = m_deferredAdd.begin(); it != m_deferredAdd.end(); ++it)
        {
            assert(*it != nullptr);
            m_handlers.push_back(std::move(*it));
        }
        m_deferredAdd.clear();
    }
}
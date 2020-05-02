#ifndef _EVENT_SYSTEM_H
#define _EVENT_SYSTEM_H
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using EventType = uint64_t;

namespace EventTypes
{
    namespace detail
    {
        /*
        The MIT License(MIT)

            Copyright(c) 2015, 2016 Ben Deane

            Permission is hereby granted, free of charge, to any person obtaining a copy
            of this software and associated documentation files(the "Software"), to deal
            in the Software without restriction, including without limitation the rights
            to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
            copies of the Software, and to permit persons to whom the Software is
            furnished to do so, subject to the following conditions :

        The above copyright notice and this permission notice shall be included in
            all copies or substantial portions of the Software.

            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
            IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
            FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
            AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
            LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
            OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
            THE SOFTWARE.
        */
        constexpr uint64_t fnv1(uint64_t h, const char* s)
        {
            return (*s == 0) ? h : fnv1((h * 1099511628211ull) ^ static_cast<uint64_t>(*s), s + 1);
        }
        constexpr uint64_t fnv1(const char* s)
        {
            return true ? fnv1(14695981039346656037ull, s) : throw std::runtime_error("Hash");
        }
    } // namespace detail

    // Returns hash of name
    constexpr EventType GetEventType(const char* name) { return static_cast<EventType>(detail::fnv1(name)); }
} // namespace EventTypes

// Base class for events. Events should inherit from Event<Derived>
class EventBase
{
public:
    typedef std::shared_ptr<EventBase> Ptr;

    virtual ~EventBase() = default;

    virtual EventType GetType() const = 0;

    virtual Ptr Clone() const = 0;
};

template <typename Derived>
class Event : public EventBase
{
public:
    Ptr Clone() const override
    {
        static_assert(std::is_copy_constructible<Derived>::value,
            "Event<Derived>: Derived must be copy constructible! Inherit from EventBase and implement Create() and "
            "Clone() instead.");
        return std::make_shared<Derived>(static_cast<const Derived&>(*this));
    }
};

template <typename DerivedType>
const DerivedType& EventCast(const EventBase& base)
{
    return dynamic_cast<const DerivedType&>(base);
}

// Can be combined by bitwise or and and
enum class PostEventState
{
    // Default, the Event was not handled
    notHandled = 0,
    // The event was handled
    handled = 1,
    // The EventHandler should be removed from the EventSystem
    shouldRemove = 2,
    // An error occurred
    error = 4,
    // The event is consumed and no further EventHandlers are executed on it
    // Order of execution is unspecified
    consumeEvent = 8
};

inline PostEventState operator|(PostEventState lhs, PostEventState rhs)
{
    return static_cast<PostEventState>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
inline PostEventState operator&(PostEventState lhs, PostEventState rhs)
{
    return static_cast<PostEventState>(static_cast<int>(lhs) & static_cast<int>(rhs));
}
inline PostEventState& operator|=(PostEventState& lhs, PostEventState rhs)
{
    return (lhs = static_cast<PostEventState>(static_cast<int>(lhs) | static_cast<int>(rhs)));
}
inline PostEventState& operator&=(PostEventState& lhs, PostEventState rhs)
{
    return (lhs = static_cast<PostEventState>(static_cast<int>(lhs) & static_cast<int>(rhs)));
}

// Base class for event handlers
template <typename Event>
class EventHandler
{
public:
    typedef std::shared_ptr<EventHandler> Ptr;

public:
    virtual ~EventHandler() = default;

    virtual PostEventState HandleEvent(const Event& e) = 0;
};

// Event system storing EventHandlers to handle Events
class EventSystem
{
public:
    // Adds handler
    void AddHandler(EventHandler<EventBase>::Ptr handler);
    // Removes handler (pointer needs to be equal)
    void RemoveHandler(const EventHandler<EventBase>* handler);

    // Calls all handlers for e's type
    void HandleEvent(const EventBase& e);

private:
    std::recursive_mutex m_mutex;
    std::vector<EventHandler<EventBase>::Ptr> m_handlers;
    bool m_inLoop = false;
    std::vector<const EventHandler<EventBase>*> m_deferredRemove;
    std::vector<EventHandler<EventBase>::Ptr> m_deferredAdd;
};

template <typename Event, typename... Args>
class EventEmitter
{
public:
    using Handler = std::function<PostEventState(const Event&, Args...)>;

public:
    void AddHandler(Handler handler)
    {
        if (handler)
        {
            m_handlers.push_back(std::move(handler));
        }
    }

    void EmitEvent(const Event& e, Args... args)
    {
        size_t handledCount = 0;
        for (auto it = m_handlers.begin(); it != m_handlers.end();)
        {
            PostEventState state = (*it)(e, args...);
            if ((state & PostEventState::handled) != PostEventState::notHandled)
            {
                ++handledCount;
            }
            if ((state & PostEventState::shouldRemove) != PostEventState::notHandled)
            {
                it = m_handlers.erase(it);
            }
            else
            {
                ++it;
            }
            if ((state & PostEventState::consumeEvent) != PostEventState::notHandled)
            {
                // Do not execute more event handlers
                break;
            }
        }
    }

private:
    std::vector<Handler> m_handlers;
};

#endif
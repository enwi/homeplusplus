#include "Active.h"

Active::Active() : m_thread(&Active::Run, this) {}

Active::~Active()
{
    Send([&] { m_done = true; });
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void Active::Send(Message m)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messages.push(std::move(m));
    m_cvMessages.notify_one();
}

void Active::Run()
{
    while (!m_done)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cvMessages.wait(lock, [&]() { return m_messages.size() != 0; });
        while (m_messages.size() > 0)
        {
            Message m = m_messages.front();
            m_messages.pop();
            lock.unlock();
            m();
            lock.lock();
        }
    }
}

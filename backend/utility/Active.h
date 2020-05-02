#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

class Active
{
public:
    using Message = std::function<void()>;

public:
    Active();
    ~Active();

    Active(Active&& other) = delete;
    Active& operator=(Active&& other) = delete;

    void Send(Message m);

private:
    // Called by thread
    void Run();

private:
    // Must only be set from thread
    bool m_done = false;
    std::mutex m_mutex;
    std::condition_variable m_cvMessages;
    std::queue<Message> m_messages;
    std::thread m_thread;
};

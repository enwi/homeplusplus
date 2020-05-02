#ifndef _LOAD_REQUEST_HANDLER_H
#define _LOAD_REQUEST_HANDLER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>

#include "Events.h"
#include "SocketEvents.h"

#include "../communication/WebsocketChannel.h"
#include "../communication/WebsocketCommunication.h"
#include "../config/Timings.h"

class LoadRequestHandler
{
public:
    explicit LoadRequestHandler(WebsocketChannelAccessor statsChannelAccessor)
        : m_channelAccessor(std::move(statsChannelAccessor))
    {}
    // Stops thread
    ~LoadRequestHandler() { StopThread(); }

    PostEventState HandleEvent(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel);
    PostEventState HandleLoadRequest(WebsocketChannel::connection_hdl connection, WebsocketChannel& channel);

    // Starts thread, thread must not be running already
    void StartThread();
    // Stops thread if running
    void StopThread();

private:
    void Run();
    // Reads CPU load
    std::pair<bool, double> GetCPULoad();
    // Reads total and available memory, returns 0,0 on failure
    std::pair<bool, std::pair<long long, long long>> GetMemory();
    // Reads CPU temperature
    std::pair<bool, double> GetTemp();

private:
    // Update interval for CPU load and memory
    static constexpr std::chrono::duration<long long> s_loadDuration = Timings::LoadRequestHandlerLoadDuration();
    // Update interval for temperature
    static constexpr std::chrono::duration<long long> s_tempDuration = Timings::LoadRequestHandlerTempDuration();

    WebsocketChannelAccessor m_channelAccessor;
    std::thread m_thread;
    std::mutex m_mutex;
    bool m_running = false;
    std::condition_variable m_cvRunning;
    std::vector<std::pair<time_t, double>> m_tempData;
    std::chrono::system_clock::time_point m_lastLoadCheck;
    std::chrono::system_clock::time_point m_lastTempCheck;

    // Needed to calculate cpu load
#ifdef __APPLE__
    unsigned long long _previousTotalTicks = 0;
    unsigned long long _previousIdleTicks = 0;
#else
    long long m_prevIdle = 0;
    long long m_prevTotal = 0;
#endif
    static constexpr const char* m_cpuloadPath = "/proc/stat";
    static constexpr const char* m_memPath = "/proc/meminfo";
    static constexpr const char* m_tempPath = "/sys/class/thermal/thermal_zone0/temp";
};

#endif

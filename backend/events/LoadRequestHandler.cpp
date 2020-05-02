#include "LoadRequestHandler.h"

#include <array>
#include <cstdio>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include <absl/strings/str_split.h>

#ifdef __APPLE__
#include <mach/mach_error.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_types.h>
#include <mach/vm_map.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#include "../config/Config.h"

constexpr std::chrono::duration<long long> LoadRequestHandler::s_loadDuration;
constexpr std::chrono::duration<long long> LoadRequestHandler::s_tempDuration;

PostEventState LoadRequestHandler::HandleEvent(const WebsocketChannel::EventVariant& event, WebsocketChannel& channel)
{
    if (absl::holds_alternative<Events::SocketMessageEvent>(event))
    {
        const Events::SocketMessageEvent& messageEvent = absl::get<Events::SocketMessageEvent>(event);

        nlohmann::json payload = messageEvent.GetJsonPayload();
        if (payload.at("command") == "LOAD_REQ")
        {
            return HandleLoadRequest(messageEvent.GetConnection(), channel);
        }
    }
    else if (absl::holds_alternative<Events::SocketConnectEvent>(event))
    {
        // Return previous data on first connection
        return HandleLoadRequest(absl::get<Events::SocketConnectEvent>(event).GetConnection(), channel);
    }
    return PostEventState::notHandled;
}

PostEventState LoadRequestHandler::HandleLoadRequest(
    WebsocketChannel::connection_hdl connection, WebsocketChannel& channel)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::pair<bool, double> load = GetCPULoad();
    nlohmann::json loadResponse;
    if (load.first)
    {
        loadResponse["load"] = load.second;
    }
    std::pair<bool, std::pair<long long, long long>> memory = GetMemory();
    if (memory.first)
    {
        loadResponse["memory"] = {{"total", memory.second.first}, {"available", memory.second.second}};
    }
    if (!loadResponse.empty())
    {
        channel.Send(connection, loadResponse);
    }
    for (const std::pair<time_t, double>& p : m_tempData)
    {
        nlohmann::json tempResponse{{"temperature", {{"time", p.first}, {"temperature", p.second}}}};
        channel.Send(connection, tempResponse);
    }
    return PostEventState::handled;
}

void LoadRequestHandler::StartThread()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    assert(!m_running);
    m_running = true;
    m_thread = std::thread(&LoadRequestHandler::Run, this);
}

void LoadRequestHandler::StopThread()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_running)
    {
        m_running = false;
        m_cvRunning.notify_all();
        lock.unlock();
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }
}

void LoadRequestHandler::Run()
{
    using std::chrono::system_clock;
    std::unique_lock<std::mutex> lock(m_mutex);
    Res::Logger().Info("LoadRequestHandler", "Thread started");
    m_lastLoadCheck = m_lastTempCheck = system_clock::now();
    while (!m_cvRunning.wait_until(lock, std::min(m_lastLoadCheck + s_loadDuration, m_lastTempCheck + s_tempDuration),
        [this]() { return !m_running; }))
    {
        try
        {
            system_clock::time_point now = system_clock::now();
            if (now >= m_lastLoadCheck + s_loadDuration)
            {
                WebsocketChannel& statsChannel = m_channelAccessor.Get();
                if (statsChannel.HasSubscribers())
                {
                    // Only get load if someone is listening
                    std::pair<bool, double> load = GetCPULoad();
                    nlohmann::json loadResponse;
                    if (load.first)
                    {
                        loadResponse["load"] = load.second;
                    }
                    std::pair<bool, std::pair<long long, long long>> memory = GetMemory();
                    if (memory.first)
                    {
                        loadResponse["memory"] = {{"total", memory.second.first}, {"available", memory.second.second}};
                    }
                    statsChannel.Broadcast(loadResponse);
                }
                m_lastLoadCheck = now;
            }
            if (now >= m_lastTempCheck + s_tempDuration)
            {
                WebsocketChannel& statsChannel = m_channelAccessor.Get();
                std::pair<bool, double> temp = GetTemp();
                if (temp.first)
                {
                    time_t time = system_clock::to_time_t(system_clock::now());
                    if (m_tempData.size() >= 30)
                    {
                        m_tempData.erase(m_tempData.begin());
                    }
                    m_tempData.emplace_back(time, temp.second);
                    if (statsChannel.HasSubscribers())
                    {
                        // Only need to create json if clients are listening
                        // Temperature is still recorded in m_tempData
                        nlohmann::json tempResponse{{"temperature", {{"time", time}, {"temperature", temp.second}}}};
                        statsChannel.Broadcast(tempResponse);
                    }
                }
                m_lastTempCheck = now;
            }
        }
        catch (const websocketpp::exception& e)
        {
            Res::Logger().Error("LoadRequestHandler", std::string("Websocket exception: ") + e.what());
        }
        catch (const std::exception& e)
        {
            Res::Logger().Error("LoadRequestHandler", std::string("Exception: ") + e.what());
        }
    }
    Res::Logger().Info("LoadRequestHandler", "Thread shutting down.");
}

std::pair<bool, double> LoadRequestHandler::GetCPULoad()
{
#ifdef __APPLE__
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS)
    {
        unsigned long long totalTicks = 0;
        for (int i = 0; i < CPU_STATE_MAX; i++)
            totalTicks += cpuinfo.cpu_ticks[i];
        unsigned long long idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
        unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
        unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;
        float ret
            = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);
        _previousTotalTicks = totalTicks;
        _previousIdleTicks = idleTicks;
        return std::make_pair(true, ret);
    }
#else
    std::ifstream file(m_cpuloadPath);
    if (file)
    {
        std::string line;
        std::getline(file, line);
        file.close();
        // user(2) nice(3) system(4) idle(5) iowait(6) irq(7) softirq(8) steal(9) guest(10) guest_nice(11)
        std::vector<std::string> split = absl::StrSplit(line, ' ');
        if (split.size() >= 9)
        {
            const long long idle = std::stoll(split[5]) + std::stoll(split[6]);
            const long long nonIdle = std::stoll(split[2]) + std::stoll(split[3]) + std::stoll(split[4])
                + std::stoll(split[7]) + std::stoll(split[8]) + std::stoll(split[9]);
            const long long total = idle + nonIdle;
            // Difference: Actual - Previous
            const long long totalD = total - m_prevTotal;
            const long long idleD = idle - m_prevIdle;
            m_prevIdle = idle;
            m_prevTotal = total;
            return std::make_pair(true, static_cast<double>(totalD - idleD) / static_cast<double>(totalD));
        }
    }
#endif
    return std::make_pair(false, std::numeric_limits<double>::quiet_NaN());
}

std::pair<bool, std::pair<long long, long long>> LoadRequestHandler::GetMemory()
{
#ifdef __APPLE__
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size)
        && KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO, (host_info64_t)&vm_stats, &count))
    {
        // long long free_memory = (int64_t)vm_stats.free_count * (int64_t)page_size;
        long long used_memory
            = ((int64_t)vm_stats.active_count + (int64_t)vm_stats.inactive_count + (int64_t)vm_stats.wire_count)
            * (int64_t)page_size;

        int mib[2];
        int64_t physical_memory;
        mib[0] = CTL_HW;
        mib[1] = HW_MEMSIZE;
        size_t length = sizeof(int64_t);
        sysctl(mib, 2, &physical_memory, &length, NULL, 0);
        return std::make_pair(true, std::make_pair(physical_memory, physical_memory - used_memory));
    }
#else
    std::ifstream file(m_memPath);
    if (file)
    {
        std::string line;
        std::getline(file, line);
        long long total = std::stoll(line.substr(line.find_first_of("0123456789")));
        // Skip second line only if we are not using an orange pi
        //! \todo It would be better to search for MemTotal and MemFree instead of relying on the order of values
        if (!configuration::USE_ORANGE_PI)
        {
            std::getline(file, line);
        }
        std::getline(file, line);
        long long available = std::stoll(line.substr(line.find_first_of("0123456789")));
        return std::make_pair(true, std::make_pair(total, available));
    }
#endif
    return std::make_pair(false, std::make_pair(0, 0));
}

std::pair<bool, double> LoadRequestHandler::GetTemp()
{

#if !defined(_MSC_VER)
    std::ifstream file(m_tempPath);
    if (file)
    {
        std::string line;
        std::getline(file, line);
        return std::make_pair(true, configuration::USE_ORANGE_PI ? std::stod(line) : std::stod(line) / 1000.0);
    }
#endif
    return std::make_pair(false, 0.0);
}

#ifndef TIMINGS_H
#define TIMINGS_H

#include <chrono>

namespace Timings
{
    constexpr std::chrono::duration<long long> LoadRequestHandlerLoadDuration() { return std::chrono::seconds(1); }
    constexpr std::chrono::duration<long long> LoadRequestHandlerTempDuration() { return std::chrono::seconds(10); }

#ifdef C_PLUS_PLUS_TESTING
    constexpr std::chrono::steady_clock::duration NodeCommunicationSendDelay() { return std::chrono::milliseconds(0); }
#else
    constexpr std::chrono::steady_clock::duration NodeCommunicationSendDelay()
    {
        return std::chrono::milliseconds(150);
    }
#endif

    constexpr std::chrono::steady_clock::duration NodeCommunicationCheckInterval()
    {
        return std::chrono::milliseconds(10);
    }

#ifdef C_PLUS_PLUS_TESTING
    constexpr std::chrono::duration<double> NewNodeFinderDuration() { return std::chrono::milliseconds(20); }
#else
    constexpr std::chrono::duration<double> NewNodeFinderDuration() { return std::chrono::seconds(2); }
#endif
} // namespace Timings

#endif
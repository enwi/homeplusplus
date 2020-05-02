#include "SignalHandler.h"

#ifndef _MSC_VER
#include <thread>

#include <pthread.h>
#endif

#include "Main.h"

#ifdef _MSC_VER

// Needs to be down here, otherwise include conflicts
#define WIN32_LEAN_AND_MEAN
#include <consoleapi.h>

std::atomic<Main*> g_main = nullptr;

BOOL __stdcall CtrlHandler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_CLOSE_EVENT)
    {
        if (g_main != nullptr)
        {
            (*g_main).Stop();
            return TRUE;
        }
    }
    return FALSE;
}

#endif

SignalHandler::SignalHandler() : m_main(nullptr)
{
#ifndef _MSC_VER
    // Block SIGINT and SIGTERM, which will be passed over to child threads of the main thread
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    int s = pthread_sigmask(SIG_BLOCK, &set, nullptr);
    if (s != 0)
    {
        std::cerr << "Error blocking signals: " << s << std::endl;
    }
    std::thread t = std::thread(std::mem_fn(&SignalHandler::Run), this);
    t.detach();
#else
    assert(g_main == nullptr);
    SetConsoleCtrlHandler(CtrlHandler, TRUE);
#endif
}

SignalHandler::~SignalHandler()
{
#ifdef _MSC_VER
    if (m_main != nullptr)
    {
        // No longer want to shutdown Main
        g_main = nullptr;
    }
#endif
}

void SignalHandler::SetMain(Main& main)
{
    m_main = &main;
#ifdef _MSC_VER
    g_main = &main;
#endif
}

void SignalHandler::Run()
{
#ifndef _MSC_VER
    std::cout << "Starting signal handler thread" << std::endl;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    int sig = -1;
    while (sig != SIGINT && sig != SIGTERM)
    {
        // Wait for SIGINT and SIGTERM
        int s = sigwait(&set, &sig);
        if (s != 0)
        {
            std::cout << "Error waiting for signals: " << s << std::endl;
        }
        else if (sig == SIGINT || sig == SIGTERM)
        {
            std::cout << "Signal handler stops main loop!" << std::endl;
            // Stop main loop
            if (m_main != nullptr)
            {
                (*m_main).Stop();
            }
        }
    }
#endif
}

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#include <atomic>
#include <csignal>

// Class to handle SIGINT and SIGTERM, stopping the main loop
class SignalHandler
{
public:
    // Sets up signal handler
    SignalHandler();
    ~SignalHandler();
    // Sets Main instance which should be stopped
    void SetMain(class Main& main);

private:
    void Run();

private:
    std::atomic<class Main*> m_main;
};

#endif
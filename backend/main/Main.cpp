#include "Main.h"

#include <chrono>
#include <csignal>
#include <ctime>
#include <iostream>
#include <thread>

#include <json.hpp>

#include "SignalHandler.h"

#include "../api/Action.h"
#include "../api/Resources.h"
#include "../api/Rule.h"
#include "../core-api/CoreDeviceAPI.h"
#include "../database/SQLiteDatabase.h"
#include "../events/Events.h"
#include "../plugins/hue-api/HueAPI.h"
#include "../plugins/tasmota-api/TasmotaAPI.h"
#include "main/test.pb.h"
#ifndef _MSC_VER
#include "../utility/LinuxFileControl.h"
#endif
#ifdef HOMEPLUSPLUS_REMOTE_SOCKET
#include "../remotesocket-api/RemoteSocketAPI.h"
#endif

#ifndef MAIN_CPP_NO_MAIN_FUNCTION
int main(int argc, char* argv[])
{
    // Signal handler needs to come before everything else
    SignalHandler signalHandler;
    {
        Arguments args = ParseArguments(argc, argv);

        Main m(args);
        signalHandler.SetMain(m);
        // Must be unique, otherwise light updates will not work
        UserId hueAPIId = UserId::Dummy(153523);
        m.AddDeviceAPI(std::make_unique<HueAPI>(hueAPIId));
        m.AddDeviceAPI(std::make_unique<TasmotaAPI>(UserId::Dummy(16738), m.GetDeviceStorage(), m.GetMQTT()));
#ifdef HOMEPLUSPLUS_REMOTE_SOCKET
        m.AddDeviceAPI(std::make_unique<RemoteSocketAPI>(
            UserId::Dummy(21633), m.GetDBHandler(), m.GetSocketComm(), m.GetAuthenticator()));
#endif
        return m.Run();
    }
}
#endif // MAIN_CPP_NO_MAIN_FUNCTION

Main::Main(const Arguments& args)
    : m_dbHandler(args.m_directory + "/database/data.db"),
      m_socketComm(&m_authenticator),
      m_shutdown(false),
      m_apiConfigDir(args.m_directory + "/config/plugins"),
      m_actionSer(m_dbHandler),
      m_ruleSer(m_dbHandler, m_actionSer),
      m_deviceSer(m_dbHandler, m_deviceTypes),
      m_deviceReg(m_deviceSer, m_deviceEvents, m_propertyEvents)
{
    Res::Logger().Open(args.m_logDir, args.m_logLevel, args.m_consoleLogLevel);
    // Res::NodeManager() = NodeManager(m_dbHandler, m_nodeComm);

    AddDeviceAPI(std::make_unique<CoreDeviceAPI>(m_dbHandler, m_deviceReg, m_deviceTypes, m_socketComm, m_actionSer,
        m_ruleSer, m_authenticator, m_deviceEvents, m_propertyEvents));

    // Just in case Main is run twice
    Res::ConditionRegistry().RemoveAll();
    Res::ActionRegistry().RemoveAll();
}

Main::~Main()
{
    Res::Logger().Close();
}

void Main::AddDeviceAPI(std::unique_ptr<IDeviceAPI>&& api)
{
    m_deviceReg.RegisterDeviceAPI(std::move(api));
}

int Main::Run()
{
    try
    {
        // Cache logger, because every static call requires an atomic read to check whether it was initialized
        auto& log = Res::Logger();

        log.Info("Setup started.");
        // TODO: open database here
        m_dbHandler.CreateTables(m_authenticator);
        m_deviceReg.InitAPIs(
            m_apiConfigDir, Res::EventSystem(), Res::ConditionRegistry(), Res::ActionRegistry(), m_deviceTypes);

        log.Info("Main loop started");
        m_deviceReg.Start();
        m_socketComm.Start();

        // Wait until shutdown message arrives
        std::unique_lock<std::mutex> lock(m_shutdownMutex);
        m_cvShutdown.wait(lock, [this] { return m_shutdown; });

        m_socketComm.Stop();

        m_deviceReg.Shutdown();
    }
    catch (const std::exception& e)
    {
        Res::Logger().Severe(std::string("Exception in Main::Run: ") + e.what());
        return 1;
    }
    catch (...)
    {
        Res::Logger().Severe("Unknown exception in Main::Run");
        return 1;
    }
    return 0;
}

void Main::Stop()
{
    std::lock_guard<std::mutex> lock(m_shutdownMutex);
    m_shutdown = true;
    m_cvShutdown.notify_all();
}

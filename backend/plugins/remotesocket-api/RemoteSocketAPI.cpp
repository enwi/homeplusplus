#include "RemoteSocketAPI.h"

#include "RemoteSocketType.h"

void RemoteSocketAPI::Initialize(nlohmann::json& config)
{
    if (config.count("pin") == 0)
    {
        config["pin"] = 9;
    }
    m_transmitter.Init(config["pin"]);
}

void RemoteSocketAPI::RegisterEventHandlers(EventSystem& evSys) {}

void RemoteSocketAPI::Shutdown()
{
    m_transmitter.Shutdown();
}

void RemoteSocketAPI::RegisterDeviceTypes(DeviceTypeRegistry& registry)
{
    registry.AddDeviceType(std::make_unique<RemoteSocketType>(m_transmitter));
}

void RemoteSocketAPI::SynchronizeDevices(DeviceStorage& storage)
{
    // Devices cannot be synchronized
}

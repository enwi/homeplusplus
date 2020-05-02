#include "DeviceRegistry.h"

#include <algorithm>
#include <iomanip>
#include <stdexcept>

#include "Action.h"
#include "IDeviceSerialize.h"
#include "Rule.h"

#include "../events/EventSystem.h"

DeviceRegistry::DeviceRegistry(IDeviceSerialize& deviceSer, EventEmitter<Events::DeviceChangeEvent>& eventEmitter,
    EventEmitter<Events::DevicePropertyChangeEvent>& propertyChanges)
    : m_storage(deviceSer, eventEmitter, propertyChanges)
{}

void DeviceRegistry::RegisterDeviceAPI(std::unique_ptr<IDeviceAPI>&& api)
{
    m_registered.push_back(std::move(api));
}

void DeviceRegistry::RemoveDeviceAPI(const std::string& api)
{
    auto pos = std::find_if(m_registered.begin(), m_registered.end(),
        [&](const std::unique_ptr<IDeviceAPI>& a) { return a->GetAPIId() == api; });
    if (pos != m_registered.end())
    {
        m_registered.erase(pos);
    }
}

const std::vector<std::unique_ptr<IDeviceAPI>>& DeviceRegistry::GetAllAPIs() const
{
    return m_registered;
}

std::vector<std::unique_ptr<IDeviceAPI>>& DeviceRegistry::GetAllAPIs()
{
    return m_registered;
}

DeviceStorage& DeviceRegistry::GetStorage()
{
    return m_storage;
}

const IDeviceAPI& DeviceRegistry::GetAPI(const std::string& apiId) const
{
    auto res = std::find_if(m_registered.begin(), m_registered.end(),
        [&](const std::unique_ptr<IDeviceAPI>& api) { return api->GetAPIId() == apiId; });
    if (res == m_registered.end())
    {
        throw std::out_of_range("DeviceAPI not found!");
    }
    return **res;
}

IDeviceAPI& DeviceRegistry::GetAPI(const std::string& apiId)
{
    auto res = std::find_if(m_registered.begin(), m_registered.end(),
        [&](const std::unique_ptr<IDeviceAPI>& api) { return api->GetAPIId() == apiId; });
    if (res == m_registered.end())
    {
        throw std::out_of_range("DeviceAPI not found!");
    }
    return **res;
}

void DeviceRegistry::InitAPIs(const std::string& configDir, EventSystem& evSys, RuleConditions::Registry& condReg,
    SubActionRegistry& subActReg, DeviceTypeRegistry& typeReg)
{
    auto& log = Res::Logger();
    log.Info(CLASSNAME, "Initializing " + std::to_string(m_registered.size()) + " device apis");

    for (auto it = m_registered.begin(); it != m_registered.end();)
    {
        nlohmann::json config = GetAPIConfig(configDir, (*it)->GetAPIId());
        try
        {
            (*it)->Initialize(config);
            (*it)->RegisterDeviceTypes(typeReg);
            (*it)->RegisterRuleConditions(condReg);
            (*it)->RegisterSubActions(subActReg);
            (*it)->RegisterEventHandlers(evSys);
            (*it)->SynchronizeDevices(m_storage);
        }
        catch (const std::exception& e)
        {
            log.Error(CLASSNAME, std::string("Failed to initialize api ") + (*it)->GetAPIId());
            log.Debug(CLASSNAME, std::string("Reason:") + e.what());
            SaveAPIConfig(configDir, (*it)->GetAPIId(), config);
            it = m_registered.erase(it);
            continue;
        }
        log.Info(CLASSNAME, std::string("Initialized api ") + (*it)->GetAPIId());
        SaveAPIConfig(configDir, (*it)->GetAPIId(), config);
        ++it;
    }
}

void DeviceRegistry::Start()
{
    for (auto it = m_registered.begin(); it != m_registered.end();)
    {
        try
        {
            (*it)->Start();
        }
        catch (const std::exception&)
        {
            Res::Logger().Error(CLASSNAME, std::string("Failed to start api ") + (*it)->GetAPIId());
            it = m_registered.erase(it);
            continue;
        }
        ++it;
    }
}

void DeviceRegistry::Shutdown()
{
    for (auto& api : m_registered)
    {
        try
        {
            api->Shutdown();
        }
        catch (const std::exception&)
        {
            Res::Logger().Error(CLASSNAME, std::string("Failed to shutdown api ") + api->GetAPIId());
        }
    }
}

nlohmann::json GetAPIConfig(const std::string& configDir, const char* apiName)
{
    std::ifstream stream(configDir + "/" + apiName + "_config.json");
    try
    {
        nlohmann::json result;
        if (stream)
        {
            stream >> result;
        }
        return result;
    }
    catch (const nlohmann::json::exception&)
    {
        // Ignore parse errors
        return {};
    }
}

void SaveAPIConfig(const std::string& configDir, const char* apiName, const nlohmann::json& value)
{
    std::ofstream stream(configDir + "/" + apiName + "_config.json");
    stream << std::setw(4) << value;
}

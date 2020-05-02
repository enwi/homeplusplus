#pragma once
#include <string>
#include <vector>

#include <json.hpp>

#include "DeviceAPI.h"
#include "DeviceStorage.h"

#include "../utility/SimpleRange.h"

namespace RuleConditions
{
    class Registry;
} // namespace RuleConditions

class DeviceRegistry
{
public:
    DeviceRegistry(class IDeviceSerialize& deviceSer, EventEmitter<Events::DeviceChangeEvent>& eventEmitter,
        EventEmitter<Events::DevicePropertyChangeEvent>& propertyEvents);
    // Register API
    void RegisterDeviceAPI(std::unique_ptr<IDeviceAPI>&& api);
    // Remove API with given id
    void RemoveDeviceAPI(const std::string& api);
    // Returns list of all registered apis
    const std::vector<std::unique_ptr<IDeviceAPI>>& GetAllAPIs() const;
    // Returns mutable list of all registered apis
    std::vector<std::unique_ptr<IDeviceAPI>>& GetAllAPIs();

    // Returns DeviceStorage to retrieve devices
    DeviceStorage& GetStorage();

    // Returns api with given id
    const class IDeviceAPI& GetAPI(const std::string& apiId) const;
    // Returns mutable api with given id
    class IDeviceAPI& GetAPI(const std::string& apiId);
    // Calls Initialize() and RegisterXXX() on each api, must be called before Start(). Removes api without calling
    // Shutdown() on error
    void InitAPIs(const std::string& configDir, class EventSystem& evSys, RuleConditions::Registry& condReg,
        class SubActionRegistry& subActReg, class DeviceTypeRegistry& typeReg);
    // Calls Start() on each api, must be called after Initialize(). Removes api without calling Shutdown() on error
    void Start();
    // Calls Shutdown() on each api, does NOT remove api on error
    void Shutdown();

private:
    std::vector<std::unique_ptr<IDeviceAPI>> m_registered;
    DeviceStorage m_storage;
    static constexpr const char* const CLASSNAME = "DeviceRegistry";
};

// Loads Json file apiName_config.json in configDir. Returns empty object on error
nlohmann::json GetAPIConfig(const std::string& configDir, const char* apiName);
// Saves Json file apiName_config.json in configDir
void SaveAPIConfig(const std::string& configDir, const char* apiName, const nlohmann::json& value);
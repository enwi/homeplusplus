#include "HueAPI.h"

#include <algorithm>
#include <memory>

#include <json.hpp>

HueAPI* HueAPI::s_instance;

#ifdef _MSC_VER
HueAPI::HueAPI(UserId apiUser)
    : handler(std::make_shared<WinHttpHandler>()), m_hue("", 80, "", handler), m_apiUser(apiUser)
{}
#else
HueAPI::HueAPI(UserId apiUser)
    : handler(std::make_shared<LinHttpHandler>()), m_hue("", 80, "", handler), m_apiUser(apiUser)
{}
#endif
void HueAPI::Initialize(nlohmann::json& config)
{
    s_instance = this;
    HueFinder finder(handler);
    if (config.count("hue_mac") && config.count("hue_username"))
    {
        finder.AddUsername(config["hue_mac"], config["hue_username"]);
    }

    if (config.count("hue_mac"))
    {
        std::vector<HueFinder::HueIdentification> bridges = finder.FindBridges();
        if (config.count("hue_ip"))
        {
            bridges.push_back({config["hue_ip"], config["hue_mac"]});
        }
        std::string mac = config["hue_mac"];
        auto pos = std::find_if(
            bridges.begin(), bridges.end(), [&](const HueFinder::HueIdentification& hue) { return hue.mac == mac; });
        if (pos != bridges.end())
        {
            m_hue = finder.GetBridge(*pos);
            config["hue_username"] = m_hue.getUsername();
            config.erase("error");
        }
        else
        {
            std::string error = "Did not find hue with specified mac, found: [";
            for (const HueFinder::HueIdentification& hue : bridges)
            {
                error += hue.mac;
                error += " at ";
                error += hue.ip;
                error += ", ";
            }
            error += "]";
            config["error"] = error;

            throw std::runtime_error("configuration error");
        }
    }
    else
    {
        config["error"] = "Missing required field: hue_mac";
        throw std::runtime_error("configuration error");
    }
}

void HueAPI::RegisterEventHandlers(EventSystem&) {}

void HueAPI::RegisterRuleConditions(RuleConditions::Registry&) {}

void HueAPI::RegisterSubActions(SubActionRegistry&)
{
    // m_setLightAction = registry.GetFreeTypeId();
    // registry.Register([](size_t type){ return std::make_shared<HueStuff::SetLightAction>(type); }, m_setLightAction);
}

void HueAPI::RegisterDeviceTypes(DeviceTypeRegistry& registry)
{
    auto type = std::make_unique<HueDeviceType>(m_hue, m_apiUser);
    m_hueLightType = type.get();
    registry.AddDeviceType(std::move(type));
}

void HueAPI::SynchronizeDevices(DeviceStorage& storage)
{
    std::vector<std::reference_wrapper<HueLight>> lights = m_hue.getAllLights();
    std::vector<Device> devices = storage.GetApiDevices(GetAPIId(), m_apiUser);
    std::sort(lights.begin(), lights.end(), [](HueLight& l, HueLight& r) { return l.getId() < r.getId(); });
    std::sort(devices.begin(), devices.end(),
        [](Device& l, Device& r) { return l.GetProperty("lightId") < r.GetProperty("lightId"); });

    auto begin = lights.begin();
    for (Device& d : devices)
    {
        int lightId = d.GetProperty("lightId");
        auto it = std::find_if(begin, lights.end(), [&](HueLight& l) { return l.getId() == lightId; });
        if (it == lights.end())
        {
            storage.RemoveDevice(d.GetId(), m_apiUser);
        }
        else
        {
            // Add missing lights inbetween
            std::for_each(begin, it, [&](HueLight& l) { storage.AddDevice(CreateDeviceFromLight(l), m_apiUser); });
            UpdateDeviceFromLight(d, storage, *it);
            storage.ChangeDevice(d, m_apiUser);
        }
        begin = it + 1;
    }
    std::for_each(begin, lights.end(), [&](HueLight& l) { storage.AddDevice(CreateDeviceFromLight(l), m_apiUser); });
}

Device HueAPI::CreateDeviceFromLight(const HueLight& light) const
{
    absl::flat_hash_map<std::string, nlohmann::json> propertyMap;
    propertyMap.emplace("lightId", light.getId());
    propertyMap.emplace("on", light.isOn());
    if (light.hasBrightnessControl())
    {
        propertyMap.emplace("brightness", light.getBrightness());
    }
    if (light.hasColorControl())
    {
        auto p = light.getColorHueSaturation();
        propertyMap.emplace("hue", p.first);
        propertyMap.emplace("saturation", p.second);
    }
    if (light.hasTemperatureControl())
    {
        propertyMap.emplace("colorTemperature", light.getColorTemperature());
    }
    return Device(light.getName(), "/plugin/HueAPI/" + m_hue.getPictureOfLight(light.getId()), {}, "hueLight",
        Properties::FromRawData(std::move(propertyMap), *m_hueLightType), GetAPIId());
}

void HueAPI::UpdateDeviceFromLight(Device& device, DeviceStorage& storage, const HueLight& light) const
{
    device.SetProperty("on", light.isOn(), storage, m_apiUser);
    if (light.hasBrightnessControl())
    {
        device.SetProperty("brightness", light.getBrightness(), storage, m_apiUser);
    }
    if (light.hasColorControl())
    {
        auto p = light.getColorHueSaturation();
        device.SetProperty("hue", p.first, storage, m_apiUser);
        device.SetProperty("saturation", p.second, storage, m_apiUser);
    }
    if (light.hasTemperatureControl())
    {
        device.SetProperty("colorTemperature", light.getColorTemperature(), storage, m_apiUser);
    }
}
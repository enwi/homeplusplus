#include "HueDeviceType.h"

HueDeviceType::HueDeviceType(Hue& hue, UserId apiUser)
    :m_hue(&hue), m_apiUser(apiUser)
{
    MetadataEntry lightIdMeta = MetadataEntry::Builder()
        .SetType(MetadataEntry::DataType::integer)
        .SetSave(MetadataEntry::DBSave::save)
        .Create();
    MetadataEntry onMeta = MetadataEntry::Builder()
        .SetType(MetadataEntry::DataType::boolean)
        .SetSave(MetadataEntry::DBSave::save_log)
        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
            | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
        .Create();
    MetadataEntry brightnessMeta = MetadataEntry::Builder()
        .SetType(MetadataEntry::DataType::integer)
        .SetSave(MetadataEntry::DBSave::save_log)
        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
            | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
        .SetOptional(true)
        .Create();
    MetadataEntry hueMeta = MetadataEntry::Builder()
        .SetType(MetadataEntry::DataType::integer)
        .SetSave(MetadataEntry::DBSave::save_log)
        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
            | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
        .SetOptional(true)
        .Create();
    MetadataEntry saturationMeta = MetadataEntry::Builder()
        .SetType(MetadataEntry::DataType::integer)
        .SetSave(MetadataEntry::DBSave::save_log)
        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
            | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
        .SetOptional(true)
        .Create();
    MetadataEntry colorTemperatureMeta
        = MetadataEntry::Builder()
        .SetType(MetadataEntry::DataType::integer)
        .SetSave(MetadataEntry::DBSave::save_log)
        .SetAccess(MetadataEntry::Access::userRead | MetadataEntry::Access::ruleRead
            | MetadataEntry::Access::userWrite | MetadataEntry::Access::actionWrite)
        .SetOptional(true)
        .Create();
    absl::flat_hash_map<std::string, MetadataEntry> entries{ {"lightId", lightIdMeta}, {"on", onMeta},
        {"brightness", brightnessMeta}, {"hue", hueMeta}, {"saturation", saturationMeta},
        {"colorTemperature", colorTemperatureMeta} };
    m_meta = Metadata(std::move(entries));
}

bool HueDeviceType::ValidateUpdate(absl::string_view property, const nlohmann::json & value, UserId user) const
{
    if (property == "lightId" || property == "on")
    {
        return true;
    }
    else if (property == "brightness")
    {
        int i = value;
        return i >= 0 && i <= 255;
    }
    else if (property == "hue")
    {
        int i = value;
        return i >= 0 && i <= 65535;
    }
    else if (property == "saturation")
    {
        int i = value;
        return i >= 0 && i <= 255;
    }
    else if (property == "colorTemperature")
    {
        int i = value;
        return i >= 153 && i <= 500;
    }
    return false;
}

void HueDeviceType::OnUpdate(absl::string_view property, Device & device, UserId user) const
{
    if (user == m_apiUser)
    {
        // The changes were caused by external api requests, light is already updated
        return;
    }
    if (property != "lightId")
    {
        int lightId = device.GetProperty("lightId");
        int brightness = device.GetProperty("brightness");
        HueLight light = m_hue->getLight(lightId);
        if (property == "on")
        {
            if (device.GetProperty("on"))
            {
                light.On();
            }
            else
            {
                light.Off();
            }
        }
        else if (property == "brightness")
        {
            if (light.hasBrightnessControl())
            {
                light.setBrightness(brightness);
            }
            else if (brightness != 0)
            {
                light.On();
            }
            else
            {
                light.Off();
            }
        }
        else if (property == "hue" && light.hasColorControl())
        {
            light.setColorHue(device.GetProperty("hue"));
        }
        else if (property == "saturation" && light.hasColorControl())
        {
            light.setColorSaturation(device.GetProperty("saturation"));
        }
        else if (property == "colorTemperature" && light.hasTemperatureControl())
        {
            light.setColorTemperature(device.GetProperty("colorTemperature"));
        }
    }
}



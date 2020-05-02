#include "TasmotaAPI.h"

#include <absl/strings/str_split.h>
#include <json.hpp>

// #include <functional>

void TasmotaAPI::Initialize(nlohmann::json& config)
{
    if (config.count("ip") == 0)
    {
        config["ip"] = "localhost";
    }

    if (config.count("port") == 0)
    {
        config["port"] = 1883;
    }

    m_ip = config["ip"].get<std::string>();
    m_port = config["port"];
}

void TasmotaAPI::RegisterEventHandlers(EventSystem& evSys) {}

void TasmotaAPI::Shutdown() {}

void TasmotaAPI::RegisterDeviceTypes(DeviceTypeRegistry& registry)
{
    auto type = std::make_unique<TasmotaDeviceType>(m_mqtt.GetClient(m_ip, m_port), m_apiUser);
    m_devciceType = type.get();
    registry.AddDeviceType(std::move(type));
}

void TasmotaAPI::SynchronizeDevices(DeviceStorage& storage)
{
    std::vector<Device> devices = storage.GetApiDevices(GetAPIId(), m_apiUser);
    for (Device& d : devices)
    {
        std::string nanme = d.GetName();
        if (knownDevices.find(nanme) == knownDevices.end())
        {
            knownDevices.insert({nanme, d.GetId()});
        }
    }

    auto& client = m_mqtt.GetClient(m_ip, m_port);
    client.Subscribe(
        "stat/#", [&](int messageId, const std::string& topic, const std::string& payload, int qos, bool retain) {
            return this->handleStatusMessages(messageId, topic, payload, qos, retain);
        });
    client.Subscribe(
        "tele/#", [&](int messageId, const std::string& topic, const std::string& payload, int qos, bool retain) {
            return this->handleTelemetryMessages(messageId, topic, payload, qos, retain);
        });
    // client.Subscribe("cmnd/#", this->);
}

bool TasmotaAPI::handleStatusMessages(
    int messageId, const std::string& topic, const std::string& payload, int qos, bool retain)
{
    std::vector<std::string> tokens = absl::StrSplit(topic, '/');
    if (tokens.size() == 3)
    {
        // std::string prefix = topic[0];
        std::string deviceName = tokens[1];
        std::string type = tokens[2];

        if (type == "RESULT")
        {
            auto deviceId = getDeviceId(deviceName);
            if (deviceId.has_value())
            {
                auto opDevice = m_storage.GetDevice(deviceId.value(), m_apiUser);
                if (opDevice.has_value())
                {
                    auto device = opDevice.value();
                    nlohmann::json json = nlohmann::json::parse(payload);
                    if (!handleStateUpdate(device, json))
                    {
                        Res::Logger().Debug("TasmotaAPI",
                            "Got unkownn stat RESULT data for device \"" + deviceName + "\" with type " + type
                                + " payload: " + json.dump(1));
                    }
                }
            }
        }

        // if (type == "STATUS" || type == "STATUS1" || type == "STATUS2" || type == "STATUS3" || type == "STATUS4"
        //     || type == "STATUS5" || type == "STATUS6" || type == "STATUS7" || type == "STATUS8" || type == "STATUS9"
        //     || type == "STATUS10" || type == "STATUS11")
        // {
        //     nlohmann::json json = nlohmann::json::parse(payload);
        //     Res::Logger().Debug("TasmotaAPI",
        //         "Got state for device \"" + deviceName + "\": with type " + type + " payload: " + json.dump(1));
        // }
    }
    else
    {
        std::string tokenString;
        if (!topic.empty())
        {
            // Convert all but the last element to avoid a trailing ","
            std::for_each(
                tokens.begin(), tokens.end() - 1, [&](const std::string& piece) { tokenString += piece + ", "; });

            // Now add the last element with no delimiter
            tokenString += tokens.back();
        }
        Res::Logger().Warning(
            "TasmotaAPI", "[stat] Unexpected number of tokens for topic " + topic + ", tokens: " + tokenString);
    }
    return false;
}

bool TasmotaAPI::handleTelemetryMessages(
    int messageId, const std::string& topic, const std::string& payload, int qos, bool retain)
{
    std::vector<std::string> tokens = absl::StrSplit(topic, '/');
    if (tokens.size() == 3)
    {
        // std::string prefix = topic[0];
        std::string deviceName = tokens[1];
        std::string type = tokens[2];

        if (!isDeviceKnown(deviceName))
        {
            handleUnknownDevice(deviceName);
        }

        if (type == "LWT")
        {
            // handle Last Will and Testament
            auto deviceId = getDeviceId(deviceName);
            if (deviceId.has_value())
            {
                auto opDevice = m_storage.GetDevice(deviceId.value(), m_apiUser);
                if (opDevice.has_value())
                {
                    auto device = opDevice.value();
                    device.SetProperty("online", payload == "Online", m_storage, m_apiUser);
                }
            }
        }
        else if (type == "SENSOR")
        {
            auto deviceId = getDeviceId(deviceName);
            if (deviceId.has_value())
            {
                auto opDevice = m_storage.GetDevice(deviceId.value(), m_apiUser);
                if (opDevice.has_value())
                {
                    auto device = opDevice.value();

                    nlohmann::json json = nlohmann::json::parse(payload);
                    if (!handleStateUpdate(device, json))
                    {
                        Res::Logger().Debug("TasmotaAPI",
                            "Got unkownn tele SENSOR data for device \"" + deviceName + "\" with type " + type
                                + " payload: " + json.dump(1));
                    }
                }
            }
        }
        else if (type == "STATE")
        {
            auto deviceId = getDeviceId(deviceName);
            if (deviceId.has_value())
            {
                auto opDevice = m_storage.GetDevice(deviceId.value(), m_apiUser);
                if (opDevice.has_value())
                {
                    auto device = opDevice.value();

                    nlohmann::json json = nlohmann::json::parse(payload);
                    if (!handleStateUpdate(device, json))
                    {
                        Res::Logger().Debug("TasmotaAPI",
                            "Got unkownn tele STATE data for device \"" + deviceName + "\" with type " + type
                                + " payload: " + json.dump(1));
                    }
                }
            }
        }

        // if (type == "INFO1" || type == "INFO2" || type == "INFO3")
        // {
        //     nlohmann::json json = nlohmann::json::parse(payload);
        //     Res::Logger().Debug("TasmotaAPI",
        //         "Got tele for device \"" + deviceName + "\" with type " + type + " payload: " + json.dump(1));
        // }
        // else
        // {
        //     // Res::Logger().Debug("TasmotaAPI",
        //     //     "Got tele for device \"" + deviceName + "\" with type " + type + " payload: \"" + payload
        //     //         + "\"");
        // }
    }
    else
    {
        std::string tokenString;
        if (!tokens.empty())
        {
            // Convert all but the last element to avoid a trailing ","
            std::for_each(
                tokens.begin(), tokens.end() - 1, [&](const std::string& piece) { tokenString += piece + ", "; });

            // Now add the last element with no delimiter
            tokenString += tokens.back();
        }
        Res::Logger().Warning(
            "TasmotaAPI", "[tele] Unexpected number of tokens for topic " + topic + ", tokens: " + tokenString);
    }
    return false;
}

void TasmotaAPI::handleUnknownDevice(std::string& name)
{
    knownDevices.insert({name, m_storage.AddDevice(CreateDevice(name), m_apiUser)});

    std::string payload {"10"};
    std::string topic {"cmnd/" + name + "/Status"};
    m_mqtt.Publish(topic, payload, m_ip, m_port);
}

bool TasmotaAPI::isDeviceKnown(std::string& name)
{
    return knownDevices.find(name) != knownDevices.end();
}

Device TasmotaAPI::CreateDevice(const std::string& name) const
{
    absl::flat_hash_map<std::string, nlohmann::json> propertyMap;
    // propertyMap.emplace("name", name);
    propertyMap.emplace("online", false);
    return Device(name, "/plugin/tasmota/tasmota.svg", {}, "tasmota",
        Properties::FromRawData(std::move(propertyMap), *m_devciceType), GetAPIId());
}

absl::optional<DeviceId> TasmotaAPI::getDeviceId(const std::string& name)
{
    auto id = knownDevices.find(name);
    if (id != knownDevices.end())
    {
        return id->second;
    }
    return absl::nullopt;
}

bool TasmotaAPI::handleStateUpdate(Device& device, const nlohmann::json& json)
{
    bool handled = false;
    // TODO need to handle removing properties
    // Actuators
    if (json.count("POWER1"))
    {
        device.SetProperty("POWER1", json["POWER1"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER2"))
    {
        device.SetProperty("POWER2", json["POWER2"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER3"))
    {
        device.SetProperty("POWER3", json["POWER3"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER4"))
    {
        device.SetProperty("POWER4", json["POWER4"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER5"))
    {
        device.SetProperty("POWER5", json["POWER5"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER6"))
    {
        device.SetProperty("POWER6", json["POWER6"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER7"))
    {
        device.SetProperty("POWER7", json["POWER7"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("POWER8"))
    {
        device.SetProperty("POWER8", json["POWER8"] == "ON", m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("Dimmer"))
    {
        device.SetProperty("Dimmer", json["Dimmer"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("Dimmer0"))
    {
        device.SetProperty("Dimmer0", json["Dimmer0"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("Dimmer1"))
    {
        device.SetProperty("Dimmer1", json["Dimmer1"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("Dimmer2"))
    {
        device.SetProperty("Dimmer2", json["Dimmer2"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("Color"))
    {
        device.SetProperty("Color", json["Color"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("White"))
    {
        device.SetProperty("White", json["White"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("CT"))
    {
        device.SetProperty("CT", json["CT"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("HSBColor"))
    {
        std::vector<std::string> values = absl::StrSplit(json["HSBColor"].get<std::string>(), ',');
        if (values.size() == 3)
        {
            device.SetProperty("HSBColorHue", std::stoi(values[0]), m_storage, m_apiUser);
            device.SetProperty("HSBColorSaturation", std::stoi(values[1]), m_storage, m_apiUser);
            device.SetProperty("HSBColorBrightness", std::stoi(values[2]), m_storage, m_apiUser);
            handled = true;
        }
    }
    if (json.count("Scheme"))
    {
        device.SetProperty("Scheme", json["Scheme"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("Speed"))
    {
        device.SetProperty("Speed", json["Speed"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterClose1"))
    {
        device.SetProperty("ShutterClose1", json["ShutterClose1"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterClose2"))
    {
        device.SetProperty("ShutterClose2", json["ShutterClose2"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterClose3"))
    {
        device.SetProperty("ShutterClose3", json["ShutterClose3"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterClose4"))
    {
        device.SetProperty("ShutterClose4", json["ShutterClose4"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterOpen1"))
    {
        device.SetProperty("ShutterOpen1", json["ShutterOpen1"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterOpen2"))
    {
        device.SetProperty("ShutterOpen2", json["ShutterOpen2"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterOpen3"))
    {
        device.SetProperty("ShutterOpen3", json["ShutterOpen3"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterOpen4"))
    {
        device.SetProperty("ShutterOpen4", json["ShutterOpen4"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterStop1"))
    {
        device.SetProperty("ShutterStop1", json["ShutterStop1"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterStop2"))
    {
        device.SetProperty("ShutterStop2", json["ShutterStop2"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterStop3"))
    {
        device.SetProperty("ShutterStop3", json["ShutterStop3"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterStop4"))
    {
        device.SetProperty("ShutterStop4", json["ShutterStop4"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterPosition1"))
    {
        device.SetProperty("ShutterPosition1", json["ShutterPosition1"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterPosition2"))
    {
        device.SetProperty("ShutterPosition2", json["ShutterPosition2"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterPosition3"))
    {
        device.SetProperty("ShutterPosition3", json["ShutterPosition3"], m_storage, m_apiUser);
        handled = true;
    }
    if (json.count("ShutterPosition4"))
    {
        device.SetProperty("ShutterPosition4", json["ShutterPosition4"], m_storage, m_apiUser);
        handled = true;
    }
    // Sensors
    if (json.count("ANALOG"))
    {
        handled = handlePropertyUpdate(device, "ANALOG", "A0", json);
        if (!handled)
        {
            handled = handlePropertyUpdate(device, "ANALOG", "Temperature", json);
            if (!handled)
            {
                handled = handlePropertyUpdate(device, "ANALOG", "Illuminance", json);
            }
        }
    }
    if (json.count("AM2301"))
    {
        handleTemperatureUpdate(device, "AM2301", json);
        handleHumidityUpdate(device, "AM2301", json);
    }
    if (json.count("APDS9960"))
    {
        // TODO add APDS9960Gesture
        // {"Time":"2019-10-31T21:34:25","APDS9960":{"None":1}}
        // {"Time":"2019-10-31T21:34:26","APDS9960":{"Right":1}}
        // {"Time":"2019-10-31T21:34:29","APDS9960":{"Down":1}}
        // {"Time":"2019-10-31T21:34:29","APDS9960":{"Right":1}}
        // {"Time":"2019-10-31T21:34:31","APDS9960":{"Left":1}}
        // {"Time":"2019-10-31T21:34:33","APDS9960":{"Up":1}}
        // {"Time":"2019-10-31T21:34:35","APDS9960":{"Down":1}}
        handlePropertyUpdate(device, "APDS9960", "Red", json);
        handlePropertyUpdate(device, "APDS9960", "Green", json);
        handlePropertyUpdate(device, "APDS9960", "Blue", json);
        handlePropertyUpdate(device, "APDS9960", "Ambient", json);
        handlePropertyUpdate(device, "APDS9960", "CCT", json);
        handlePropertyUpdate(device, "APDS9960", "Proximity", json);
        handled = true;
    }
    if (json.count("BH1750"))
    {
        handlePropertyUpdate(device, "BH1750", "Illuminance", json);
        handled = true;
    }
    if (json.count("BME280"))
    {
        // MQTT Example
        // {"Time":"2020-04-18T13:02:29","BME280":{"Temperature":22.3,"Humidity":36.8,"DewPoint":6.8,"Pressure":972.1},"PressureUnit":"hPa","TempUnit":"C"}
        handleTemperatureUpdate(device, "BME280", json);
        handleHumidityUpdate(device, "BME280", json);
        handlePressureUpdate(device, "BME280", json);
        handlePropertyUpdate(device, "BME280", "DewPoint", json);
        handled = true;
    }
    if (json.count("BME680"))
    {
        handleTemperatureUpdate(device, "BME680", json);
        handleHumidityUpdate(device, "BME680", json);
        handlePressureUpdate(device, "BME680", json);
        handlePropertyUpdate(device, "BME680", "Gas", json);
        handled = true;
    }
    if (json.count("DHT11"))
    {
        handleTemperatureUpdate(device, "DHT11", json);
        handleHumidityUpdate(device, "DHT11", json);
        handled = true;
    }
    // DS18x20 -> complex
    if (json.count("SR04"))
    {
        handlePropertyUpdate(device, "SR04", "Distance", json);
    }
    if (json.count("HTU21"))
    {
        handleTemperatureUpdate(device, "HTU21", json);
        handleHumidityUpdate(device, "HTU21", json);
        handled = true;
    }
    if (json.count("LM75AD"))
    {
        handleTemperatureUpdate(device, "LM75AD", json);
        handled = true;
    }
    if (json.count("MLX90614"))
    {
        if (json["MLX90614"].count("OBJTMP"))
        {
            device.SetProperty("MLX90614ObjectT", json["MLX90614"]["OBJTMP"], m_storage, m_apiUser);
        }
        if (json["MLX90614"].count("AMBTMP"))
        {
            device.SetProperty("MLX90614AmbientT", json["MLX90614"]["AMBTMP"], m_storage, m_apiUser);
        }
        handled = true;
    }
    if (json.count("MPU6050"))
    {
        handleTemperatureUpdate(device, "MPU6050", json);
        handlePropertyUpdate(device, "MPU6050", "AccelXAxis", json);
        handlePropertyUpdate(device, "MPU6050", "AccelYAxis", json);
        handlePropertyUpdate(device, "MPU6050", "AccelZAxis", json);
        handlePropertyUpdate(device, "MPU6050", "GyroXAxis", json);
        handlePropertyUpdate(device, "MPU6050", "GyroYAxis", json);
        handlePropertyUpdate(device, "MPU6050", "GyroZAxis", json);
        handlePropertyUpdate(device, "MPU6050", "Yaw", json);
        handlePropertyUpdate(device, "MPU6050", "Pitch", json);
        handlePropertyUpdate(device, "MPU6050", "Roll", json);
        handled = true;
    }
    if (json.count("PMS5003"))
    {
        handlePropertyUpdate(device, "PMS5003", "CF1", json);
        if (json["PMS5003"].count("CF2.5"))
        {
            device.SetProperty("PMS5003CF2_5", json["PMS5003"]["CF2.5"], m_storage, m_apiUser);
        }
        handlePropertyUpdate(device, "PMS5003", "CF10", json);

        handlePropertyUpdate(device, "PMS5003", "PM1", json);
        if (json["PMS5003"].count("PM2.5"))
        {
            device.SetProperty("PMS5003PM2_5", json["PMS5003"]["PM2.5"], m_storage, m_apiUser);
        }
        handlePropertyUpdate(device, "PMS5003", "PM10", json);

        if (json["PMS5003"].count("PB0.3"))
        {
            device.SetProperty("PMS5003PB0_3", json["PMS5003"]["PB0.3"], m_storage, m_apiUser);
        }
        if (json["PMS5003"].count("PB0.5"))
        {
            device.SetProperty("PMS5003PB0_5", json["PMS5003"]["PB0.5"], m_storage, m_apiUser);
        }
        handlePropertyUpdate(device, "PMS5003", "PB1", json);
        if (json["PMS5003"].count("PB2.5"))
        {
            device.SetProperty("PMS5003PB2_5", json["PMS5003"]["PB2.5"], m_storage, m_apiUser);
        }
        handlePropertyUpdate(device, "PMS5003", "PB5", json);
        handlePropertyUpdate(device, "PMS5003", "PB10", json);
    }
    if (json.count("PN532"))
    {
        handlePropertyUpdate(device, "PN532", "UID", json);
        if (json["PN532"].count("DATA"))
        {
            device.SetProperty("PN532Data", json["PN532"]["DATA"], m_storage, m_apiUser);
        }
        handled = true;
    }
    if (json.count("PIR1"))
    {
        device.SetProperty("PIR1", json["PIR1"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR2"))
    {
        device.SetProperty("PIR2", json["PIR2"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR3"))
    {
        device.SetProperty("PIR3", json["PIR3"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR4"))
    {
        device.SetProperty("PIR4", json["PIR4"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR5"))
    {
        device.SetProperty("PIR5", json["PIR5"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR6"))
    {
        device.SetProperty("PIR6", json["PIR6"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR7"))
    {
        device.SetProperty("PIR7", json["PIR7"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("PIR8"))
    {
        device.SetProperty("PIR8", json["PIR8"] == "ON", m_storage, m_apiUser);
    }
    if (json.count("SDS011"))
    {
        if (json["SDS011"].count("PM2.5"))
        {
            device.SetProperty("SDS011PM2_5", json["SDS011"]["PM2.5"], m_storage, m_apiUser);
        }
        handlePropertyUpdate(device, "SDS011", "PM10", json);
    }
    if (json.count("SHT3X"))
    {
        handleTemperatureUpdate(device, "SHT3X", json);
        handleHumidityUpdate(device, "SHT3X", json);
        handled = true;
    }
    if (json.count("TX23"))
    {
        if (json["TX23"].count("Speed"))
        {
            if (json["TX23"]["Speed"].count("Act"))
            {
                device.SetProperty("TX23SpeedAct", json["TX23"]["Speed"]["Act"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Speed"].count("Avg"))
            {
                device.SetProperty("TX23SpeedAvg", json["TX23"]["Speed"]["Avg"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Speed"].count("Min"))
            {
                device.SetProperty("TX23SpeedMin", json["TX23"]["Speed"]["Min"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Speed"].count("Max"))
            {
                device.SetProperty("TX23SpeedMax", json["TX23"]["Speed"]["Max"], m_storage, m_apiUser);
            }
        }
        if (json["TX23"].count("Dir"))
        {
            if (json["TX23"]["Dir"].count("Card"))
            {
                device.SetProperty("TX23DirCard", json["TX23"]["Dir"]["Card"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Dir"].count("Deg"))
            {
                device.SetProperty("TX23DirDeg", json["TX23"]["Dir"]["Deg"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Dir"].count("Avg"))
            {
                device.SetProperty("TX23DirAvg", json["TX23"]["Dir"]["Avg"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Dir"].count("AvgCard"))
            {
                device.SetProperty("TX23DirAvgCard", json["TX23"]["Dir"]["AvgCard"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Dir"].count("Min"))
            {
                device.SetProperty("TX23DirMin", json["TX23"]["Dir"]["Min"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Dir"].count("Max"))
            {
                device.SetProperty("TX23DirMax", json["TX23"]["Dir"]["Max"], m_storage, m_apiUser);
            }
            if (json["TX23"]["Dir"].count("Range"))
            {
                device.SetProperty("TX23DirRange", json["TX23"]["Dir"]["Range"], m_storage, m_apiUser);
            }
        }
        handled = true;
    }
    if (json.count("TSL2561"))
    {
        handlePropertyUpdate(device, "TSL2561", "Illuminance", json);
        handled = true;
    }
    if (json.count("VL53L0X"))
    {
        handlePropertyUpdate(device, "VL53L0X", "Distance", json);
        handled = true;
    }

    // Other
    // Prevent log output for normal state message
    if (json.count("MqttCount"))
    {
        handled = true;
    }
    return handled;
}

void TasmotaAPI::handleTemperatureUpdate(Device& device, const std::string& sensor, const nlohmann::json& json)
{
    handlePropertyUpdate(device, sensor, "Temperature", json);
}

void TasmotaAPI::handleHumidityUpdate(Device& device, const std::string& sensor, const nlohmann::json& json)
{
    handlePropertyUpdate(device, sensor, "Humidity", json);
}

void TasmotaAPI::handlePressureUpdate(Device& device, const std::string& sensor, const nlohmann::json& json)
{
    handlePropertyUpdate(device, sensor, "Pressure", json);
}

bool TasmotaAPI::handlePropertyUpdate(
    Device& device, const std::string& sensor, const std::string& property, const nlohmann::json& json)
{
    if (json[sensor].count(property))
    {
        device.SetProperty(sensor + property, json[sensor][property], m_storage, m_apiUser);
        return true;
    }
    return false;
}
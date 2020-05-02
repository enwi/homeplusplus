#pragma once

#include <atomic>
#include <set>
#include <string>
#include <thread>

#include "TasmotaDeviceType.h"

#include "../../api/DeviceAPI.h"
#include "../../api/DeviceStorage.h"
#include "../../communication/MQTT.h"

class TasmotaAPI : public IDeviceAPI
{
public:
    TasmotaAPI(UserId apiUser, DeviceStorage& deviceStorage, MQTT& mqtt)
        : m_apiUser {apiUser}, m_storage {deviceStorage}, m_mqtt(mqtt) {};

    void Initialize(nlohmann::json& config) override;
    void RegisterEventHandlers(EventSystem& evSys) override;
    void Shutdown() override;

    void RegisterDeviceTypes(DeviceTypeRegistry& registry) override;
    void SynchronizeDevices(DeviceStorage& storage) override;

    const char* GetAPIId() const noexcept override { return "TASMOTA_API0.0"; }

private:
    bool handleStatusMessages(
        int messageId, const std::string& topic, const std::string& payload, int qos, bool retain);
    bool handleTelemetryMessages(
        int messageId, const std::string& topic, const std::string& payload, int qos, bool retain);
    void handleUnknownDevice(std::string& name);
    bool isDeviceKnown(std::string& name);
    Device CreateDevice(const std::string& name) const;
    absl::optional<DeviceId> getDeviceId(const std::string& name);
    bool handleStateUpdate(Device& device, const nlohmann::json& json);
    void handleTemperatureUpdate(Device& device, const std::string& sensor, const nlohmann::json& json);
    void handleHumidityUpdate(Device& device, const std::string& sensor, const nlohmann::json& json);
    void handlePressureUpdate(Device& device, const std::string& sensor, const nlohmann::json& json);
    bool handlePropertyUpdate(
        Device& device, const std::string& sensor, const std::string& property, const nlohmann::json& json);

private:
    UserId m_apiUser;
    DeviceStorage& m_storage;
    MQTT& m_mqtt;
    TasmotaDeviceType* m_devciceType;
    /*!
     * \brief All known devices
     */
    absl::flat_hash_map<std::string, DeviceId> knownDevices;
    std::string m_ip;
    int m_port;
};
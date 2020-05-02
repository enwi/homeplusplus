#pragma once

#include "../../api/DeviceType.h"
#include "../../communication/MQTTClient.h"

class TasmotaDeviceType : public DeviceType
{
public:
    TasmotaDeviceType(MQTTClient& mqttClient, UserId apiUser);

    absl::string_view GetName() const override { return "tasmota"; }
    const Metadata& GetDeviceMetadata() const override { return m_meta; }
    bool ValidateUpdate(absl::string_view property, const nlohmann::json& value, UserId user) const override;
    void OnUpdate(absl::string_view property, Device& device, UserId user) const override;

private:
    Metadata m_meta;
    UserId m_apiUser;
    MQTTClient& m_MQTTClient;
};
#pragma once

#include "RemoteSocketTransmitter.h"

#include "../api/DeviceType.h"

class RemoteSocketType : public DeviceType
{
public:
    RemoteSocketType(RemoteSocketTransmitter& transmitter);

    absl::string_view GetName() const override { return "remoteSocket"; }

    const Metadata& GetDeviceMetadata() const override { return m_meta; }

    bool CanAddDevice() const override { return true; }

    bool CanRemoveDevice() const override { return true; }

    bool ValidateUpdate(absl::string_view property, const nlohmann::json& value, UserId user) const override;

    void OnUpdate(absl::string_view property, Device& device, UserId user) const override;

private:
    RemoteSocketTransmitter& m_transmitter;
    Metadata m_meta;
};
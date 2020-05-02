#pragma once

#include "Device.h"

class DeviceType
{
public:
    virtual ~DeviceType() = default;

    // Type name. Careful because return type is only a reference!
    virtual absl::string_view GetName() const = 0;

    // Returns the metadata for the device type (should be cached and reused)
    virtual const Metadata& GetDeviceMetadata() const = 0;
    // Whether the device can be added from the UI
    virtual bool CanAddDevice() const { return false; }
    // Whether the device can be removed from the UI
    virtual bool CanRemoveDevice() const { return false; }

    // Validates whether property can be set to value by user
    virtual bool ValidateUpdate(absl::string_view property, const nlohmann::json& value, UserId user) const = 0;
    // Called on update, should notify underlying API of the changes
    virtual void OnUpdate(absl::string_view property, Device& device, UserId user) const = 0;
};

class DeviceTypeRegistry
{
public:
    void AddDeviceType(std::unique_ptr<DeviceType> type);
    const DeviceType& GetDeviceType(absl::string_view type) const;
    bool HasDeviceType(absl::string_view type) const;
    std::vector<std::string> GetRegisteredTypes() const;

private:
    absl::flat_hash_map<absl::string_view, std::unique_ptr<DeviceType>> m_types;
};

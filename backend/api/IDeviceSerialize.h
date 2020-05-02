#pragma once

#include <vector>

#include "Device.h"
#include "Filter.h"
#include "User.h"

// Just a tag type to ensure a transaction persists
class UserHeldTransaction;

class IDeviceSerialize
{
public:
    virtual ~IDeviceSerialize() = default;

    // Returns the device data with the given id
    virtual absl::optional<Device::Data> GetDeviceData(DeviceId deviceId, UserId user) const = 0;
    virtual absl::optional<Device::Data> GetDeviceData(DeviceId deviceId, const UserHeldTransaction&) const = 0;
    // Adds a device, returns the id
    virtual DeviceId AddDevice(const Device::Data& deviceData, UserId user) = 0;
    virtual DeviceId AddDevice(const Device::Data& deviceData, const UserHeldTransaction&) = 0;
    // Updates an existing device
    virtual void UpdateDevice(DeviceId id, const Device::Data& data, UserId user) = 0;
    virtual void UpdateDevice(DeviceId id, const Device::Data& data, const UserHeldTransaction&) = 0;

    virtual std::vector<DeviceId> GetAPIDeviceIds(absl::string_view apiId, UserId user) const = 0;
    virtual std::vector<DeviceId> GetAPIDeviceIds(absl::string_view apiId, const UserHeldTransaction&) const = 0;
    virtual std::vector<DeviceId> GetAllDeviceIds(const Filter& filter, UserId user) const = 0;
    virtual std::vector<DeviceId> GetAllDeviceIds(const Filter& filter, const UserHeldTransaction&) const = 0;
    // Removes a device
    virtual void RemoveDevice(DeviceId deviceId, UserId user) = 0;
    virtual void RemoveDevice(DeviceId deviceId, const UserHeldTransaction&) = 0;

    virtual void SetDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user)
        = 0;
    virtual void SetDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, const UserHeldTransaction&)
        = 0;

    virtual void InsertDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user)
        = 0;
    virtual void InsertDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, const UserHeldTransaction&)
        = 0;

    virtual void LogDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user)
        = 0;
    virtual void LogDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, const UserHeldTransaction&)
        = 0;

    virtual nlohmann::json GetPropertyHistory(DeviceId deviceId, absl::string_view propertyKey,
        const std::chrono::system_clock::time_point& start,
        absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
        const Properties& properties, UserId user)
        = 0;
};
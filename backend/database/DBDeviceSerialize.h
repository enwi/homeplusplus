#pragma once

#include "DBHandler.h"
#include "HeldTransaction.h"

#include "../api/DeviceType.h"
#include "../api/IDeviceSerialize.h"

class DBDeviceSerialize : public IDeviceSerialize
{
public:
    explicit DBDeviceSerialize(DBHandler& dbHandler, DeviceTypeRegistry& types)
        : m_dbHandler(dbHandler), m_deviceTypes(types)
    {}

    // Returns the device data with the given id
    absl::optional<Device::Data> GetDeviceData(DeviceId deviceId, UserId user) const override;
    absl::optional<Device::Data> GetDeviceData(DeviceId deviceId, const UserHeldTransaction&) const override;
    // Adds a device, returns the id
    DeviceId AddDevice(const Device::Data& deviceData, UserId user) override;
    DeviceId AddDevice(const Device::Data& deviceData, const UserHeldTransaction&) override;
    // Updates an existing device
    void UpdateDevice(DeviceId id, const Device::Data& data, UserId user) override;
    void UpdateDevice(DeviceId id, const Device::Data& data, const UserHeldTransaction&) override;

    std::vector<DeviceId> GetAPIDeviceIds(absl::string_view apiId, UserId user) const override;
    std::vector<DeviceId> GetAPIDeviceIds(absl::string_view apiId, const UserHeldTransaction&) const override;
    std::vector<DeviceId> GetAllDeviceIds(const Filter& filter, UserId user) const override;
    std::vector<DeviceId> GetAllDeviceIds(const Filter& filter, const UserHeldTransaction&) const override;
    // Removes a device
    void RemoveDevice(DeviceId deviceId, UserId user) override;
    void RemoveDevice(DeviceId deviceId, const UserHeldTransaction&) override;

    // Assume property exists already, will not insert new one
    void SetDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user) override;
    void SetDeviceProperty(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties,
        const UserHeldTransaction&) override;

    void InsertDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user) override;
    void InsertDeviceProperty(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties,
        const UserHeldTransaction&) override;

    void LogDeviceProperty(
        DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user) override;
    void LogDeviceProperty(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties,
        const UserHeldTransaction&) override;

    nlohmann::json GetPropertyHistory(DeviceId deviceId, absl::string_view propertyKey,
        const std::chrono::system_clock::time_point& start,
        absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
        const Properties& properties, UserId user) override;

private:
    void InsertDeviceGroups(DeviceId id, const std::vector<std::string>& groups, const UserHeldTransaction&);
    void AddProperties(DeviceId deviceId, const Properties& properties, const UserHeldTransaction&);
    std::vector<std::string> GetDeviceGroups(DeviceId deviceId, const UserHeldTransaction&) const;
    Properties GetDeviceProperties(DeviceId deviceId, const DeviceType& meta, const UserHeldTransaction&) const;

private:
    DBHandler& m_dbHandler;
    DeviceTypeRegistry& m_deviceTypes;
};
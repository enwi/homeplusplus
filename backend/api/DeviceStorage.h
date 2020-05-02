#pragma once

#include "Device.h"

#include "../events/Events.h"

class DeviceStorage
{
public:
    // apiId must refer to static storage
    DeviceStorage(class IDeviceSerialize& deviceSer, EventEmitter<Events::DeviceChangeEvent>& eventEmitter,
        EventEmitter<Events::DevicePropertyChangeEvent>& propertyEvents);
    // Adds device as new, ignores id
    DeviceId AddDevice(const Device& d, UserId u);
    void RemoveDevice(DeviceId id, UserId u);
    // Changes existing device with same id
    void ChangeDevice(const Device& d, UserId u);
    absl::optional<Device> GetDevice(DeviceId id, UserId u);
    std::vector<Device> GetApiDevices(absl::string_view apiId, UserId u);
    std::vector<Device> GetAllDevices(UserId u);

    void SetDeviceProperty(DeviceId id, absl::string_view path, const Properties& properties, UserId user);
    void SetAndLogDeviceProperty(DeviceId id, absl::string_view path, const Properties& properties, UserId user);
    void InsertDeviceProperty(DeviceId id, absl::string_view path, const Properties& properties, UserId user);
    void InsertAndLogDeviceProperty(DeviceId id, absl::string_view path, const Properties& properties, UserId user);
    nlohmann::json GetPropertyHistory(DeviceId id, absl::string_view path,
        const std::chrono::system_clock::time_point& start,
        absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
        const Properties& properties, UserId user);

    // Removes all expired Data ptrs from cache
    void CleanupCache();

private:
    absl::flat_hash_map<DeviceId, std::weak_ptr<Device::Data>> m_cache;
    class IDeviceSerialize* m_serialize;
    EventEmitter<Events::DeviceChangeEvent>* m_eventEmitter;
    EventEmitter<Events::DevicePropertyChangeEvent>* m_propertyEvents;
};
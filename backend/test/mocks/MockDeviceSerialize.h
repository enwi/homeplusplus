#pragma once

#include <gmock/gmock.h>

#include "api/IDeviceSerialize.h"

class MockDeviceSerialize : public IDeviceSerialize
{
public:
    // Returns the device data with the given id
    MOCK_CONST_METHOD2(GetDeviceData, absl::optional<Device::Data>(DeviceId deviceId, UserId user));
    MOCK_CONST_METHOD2(GetDeviceData, absl::optional<Device::Data>(DeviceId deviceId, const UserHeldTransaction&));
    // Adds a device, returns the id
    MOCK_METHOD2(AddDevice, DeviceId(const Device::Data& deviceData, UserId user));
    MOCK_METHOD2(AddDevice, DeviceId(const Device::Data& deviceData, const UserHeldTransaction&));
    // Updates an existing device
    MOCK_METHOD3(UpdateDevice, void(DeviceId id, const Device::Data& data, UserId user));
    MOCK_METHOD3(UpdateDevice, void(DeviceId id, const Device::Data& data, const UserHeldTransaction&));

    MOCK_CONST_METHOD2(GetAPIDeviceIds, std::vector<DeviceId>(absl::string_view apiId, UserId user));
    MOCK_CONST_METHOD2(GetAPIDeviceIds, std::vector<DeviceId>(absl::string_view apiId, const UserHeldTransaction&));
    MOCK_CONST_METHOD2(GetAllDeviceIds, std::vector<DeviceId>(const Filter& filter, UserId user));
    MOCK_CONST_METHOD2(GetAllDeviceIds, std::vector<DeviceId>(const Filter& filter, const UserHeldTransaction&));
    // Removes a device
    MOCK_METHOD2(RemoveDevice, void(DeviceId deviceId, UserId user));
    MOCK_METHOD2(RemoveDevice, void(DeviceId deviceId, const UserHeldTransaction&));

    MOCK_METHOD4(SetDeviceProperty,
        void(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user));
    MOCK_METHOD4(SetDeviceProperty,
        void(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties,
            const UserHeldTransaction&));

    MOCK_METHOD4(InsertDeviceProperty,
        void(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user));
    MOCK_METHOD4(InsertDeviceProperty,
        void(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties,
            const UserHeldTransaction&));

    MOCK_METHOD4(LogDeviceProperty,
        void(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties, UserId user));
    MOCK_METHOD4(LogDeviceProperty,
        void(DeviceId deviceId, absl::string_view propertyKey, const Properties& properties,
            const UserHeldTransaction&));

    MOCK_METHOD7(GetPropertyHistory,
        nlohmann::json(DeviceId deviceId, absl::string_view propertyKey,
            const std::chrono::system_clock::time_point& start,
            absl::optional<const std::chrono::system_clock::time_point> end, std::time_t compression,
            const Properties& properties, UserId user));
};
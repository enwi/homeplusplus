#ifndef MOCK_DEVICE_API_H
#define MOCK_DEVICE_API_H

#include <gmock/gmock.h>

#include "api/DeviceAPI.h"

class MockDeviceAPI : public IDeviceAPI
{
public:
    MOCK_METHOD1(Initialize, void(nlohmann::json& config));
    MOCK_METHOD0(Start, void());
    MOCK_METHOD0(Shutdown, void());
    MOCK_METHOD1(RegisterEventHandlers, void(EventSystem&));
    MOCK_METHOD1(RegisterRuleConditions, void(RuleConditions::Registry&));
    MOCK_METHOD1(RegisterSubActions, void(SubActionRegistry&));
    MOCK_METHOD0(GetDevices, Range<IDevice>());
    MOCK_METHOD1(GetDevice, std::unique_ptr<IDevice>(std::size_t id));
    MOCK_CONST_METHOD0(GetAPIIdMock, const char*());
    const char* GetAPIId() const noexcept { return GetAPIIdMock(); };
};

#endif

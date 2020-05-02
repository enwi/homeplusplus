#ifndef MOCK_DEVICE_H
#define MOCK_DEVICE_H

#include <gmock/gmock.h>

#include "api/DeviceRegistry.h"

class MockActor : public IActor
{
public:
    MOCK_CONST_METHOD0(GetName, std::string());
    MOCK_CONST_METHOD0(GetLocation, std::string());
    MOCK_CONST_METHOD0(GetState, std::string());
    MOCK_METHOD1(Set, void(const std::string&));
};

class MockSensor : public ISensor
{
public:
    MOCK_CONST_METHOD0(GetName, std::string());
    MOCK_CONST_METHOD0(GetLocation, std::string());
    MOCK_CONST_METHOD0(GetState, std::string());
    MOCK_METHOD0(Get, std::string());
};

class MockDevice : public IDevice
{
public:
    MOCK_CONST_METHOD0(GetIdMock, std::size_t());
    std::size_t GetId() const noexcept { return GetIdMock(); };
    MOCK_CONST_METHOD0(GetTypeMock, std::size_t());
    std::size_t GetType() const noexcept { return GetTypeMock(); };
    MOCK_CONST_METHOD0(GetName, std::string());
    MOCK_CONST_METHOD0(GetLocation, std::string());
    MOCK_CONST_METHOD0(GetState, std::string());
    MOCK_CONST_METHOD0(GetSensors, Range<const ISensor>());
    MOCK_CONST_METHOD0(GetActors, Range<const IActor>());
    MOCK_METHOD0(GetSensors, Range<ISensor>());
    MOCK_METHOD0(GetActors, Range<IActor>());
    MOCK_CONST_METHOD0(ToJson, nlohmann::json());
    MOCK_METHOD1(Parse, void(const nlohmann::json& json));
};

#endif

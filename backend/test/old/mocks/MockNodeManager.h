#ifndef MOCK_NODE_MANAGER_H
#define MOCK_NODE_MANAGER_H

#include <gmock/gmock.h>

#include "standard-api/NodeManager.h"

class MockNodeManager : public NodeManager
{
public:
    MOCK_METHOD1(GetState, std::future<std::string>(uint16_t));
    MOCK_METHOD2(AddActor, uint8_t(uint16_t, Actor));
    MOCK_METHOD3(ChangeActor, void(uint16_t, uint8_t, Actor));
    MOCK_METHOD3(SetActorValue, void(uint16_t, uint8_t, const std::vector<uint8_t>&));
    MOCK_METHOD2(RemoveActor, void(uint16_t, uint8_t));
    MOCK_METHOD2(AddSensor, uint8_t(uint16_t, Sensor));
    MOCK_METHOD3(ChangeSensor, void(uint16_t, uint8_t, Sensor));
    MOCK_CONST_METHOD2(GetSensorValue, std::future<std::vector<uint8_t>>(uint16_t, uint8_t));
    MOCK_METHOD2(RemoveSensor, void(uint16_t, uint8_t));
    MOCK_METHOD1(RemoveNode, void(uint16_t));
};

#endif

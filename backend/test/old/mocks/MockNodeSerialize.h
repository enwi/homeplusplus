#ifndef MOCK_NODE_SERIALIZE_H
#define MOCK_NODE_SERIALIZE_H

#include <gmock/gmock.h>

#include "standard-api/INodeSerialize.h"

class MockActorSerialize : public IActorSerialize
{
public:
    MOCK_CONST_METHOD1(GetActors, std::vector<Actor>(uint16_t));
    MOCK_METHOD3(UpdateActor, void(uint16_t, uint8_t, const Actor&));
    MOCK_METHOD3(InsertActors, void(uint16_t, const std::vector<Actor>&, bool));
};

class MockSensorSerialize : public ISensorSerialize
{
public:
    MOCK_CONST_METHOD1(GetSensors, std::vector<Sensor>(uint16_t));
    MOCK_METHOD3(UpdateSensor, void(uint16_t, uint8_t, const Sensor&));
    MOCK_METHOD3(InsertSensors, void(uint16_t, const std::vector<Sensor>&, bool));
};

class MockNodeSerialize : public INodeSerialize
{
public:
    MOCK_CONST_METHOD1(GetNodeById, NodeData(uint16_t));
    MOCK_CONST_METHOD1(GetNodesByName, Range<NodeData>(const std::string&));
    MOCK_CONST_METHOD1(GetNodeByPath, NodeData(const NodePath&));
    MOCK_CONST_METHOD0(GetAllNodes, Range<NodeData>());
    MOCK_CONST_METHOD0(GetFreeNodeId, uint16_t());

    MOCK_METHOD1(AddNode, void(const NodeData&));
    MOCK_METHOD1(AddNodeOnly, void(const NodeData&));
    MOCK_METHOD1(RemoveNode, void(uint16_t));

    MOCK_METHOD0(GetActorSerialize, IActorSerialize&());
    MOCK_CONST_METHOD0(GetActorSerialize, const IActorSerialize&());
    MOCK_METHOD0(GetSensorSerialize, ISensorSerialize&());
    MOCK_CONST_METHOD0(GetSensorSerialize, const ISensorSerialize&());
};

#endif
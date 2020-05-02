#ifndef MOCK_NODE_COMMUNICATION_H
#define MOCK_NODE_COMMUNICATION_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockNRF.h"

#include "standard-api/communication/NodeCommunication.h"

inline std::unique_ptr<MockNRF> GetSetupMockNRF()
{
    std::unique_ptr<MockNRF> p = std::make_unique<MockNRF>();
    p->SetupForNRFTest();
    return p;
}

class MockNodeCommunication : public NodeCommunication
{
public:
    MockNodeCommunication(uint64_t baseId) : NodeCommunication(GetSetupMockNRF(), baseId) {}
    MOCK_METHOD2(SendMessage, std::future<bool>(const Message&, const uint64_t&));
    MOCK_METHOD3(QueueMessage, std::future<bool>(const Message&, uint8_t, uint64_t));
    MOCK_METHOD1(SendMessageSequence, std::future<bool>(MessageSequence));
    MOCK_METHOD2(SendMessageNow, bool(const Message&, const uint64_t&));
};

#endif
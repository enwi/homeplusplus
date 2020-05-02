#include "standard-api/communication/NodeCommunication.h"

#include <gtest/gtest.h>

#include "../SilenceCout.h"
#include "../mocks/MockNRF.h"
#include "../mocks/MockNodeCommunication.h"
#include "api/Resources.h"
#include "communication/nRFTests.h"
#include "events/EventSystem.h"
#include "standard-api/CallbackMessageHandler.h"
#include "standard-api/communication/MessageTypes.h"
#include "utility/Logger.h"

class TestNodeCommunication : public ::testing::Test
{
public:
    class VerifyLater
    {
    public:
        VerifyLater(const std::shared_ptr<bool>& p) : p(p) {}
        ~VerifyLater()
        {
            EXPECT_TRUE(p != nullptr);
            if (p)
            {
                EXPECT_TRUE(*p);
            }
        }

    private:
        std::shared_ptr<bool> p;
    };

public:
    TestNodeCommunication() : nrf(std::make_unique<MockNRF>())
    {
        // Disable logging
        Res::Logger().SetConsoleLevel(Logger::LEVEL_NONE);
        Res::Logger().SetFileLevel(Logger::LEVEL_NONE);
    }
    void SetupForNRFTest()
    {
        if (!nrf)
        {
            nrf = std::make_unique<MockNRF>();
        }
        nrf->SetupForNRFTest();
    }
    ::testing::ExpectationSet SetupInitExpectations()
    {
        using namespace ::testing;
        using namespace RF24;
        ExpectationSet fullyInitialized;
        // Features must be enabled before AA and DYNPD
        ExpectationSet featuresEnabled = EXPECT_CALL(*nrf,
            SPIWrite(Truly([](const void* cmd) {
                return *reinterpret_cast<const uint16_t*>(cmd) == (Commands::ACTIVATE << 8 | 0x73);
            }),
                2))
                                             .Times(AtLeast(1));
        fullyInitialized += *featuresEnabled.begin();
        fullyInitialized += EXPECT_CALL(*nrf, Write(Commands::FLUSH_RX)).Times(AtLeast(1));
        fullyInitialized += EXPECT_CALL(*nrf, Write(Commands::FLUSH_TX)).Times(AtLeast(1));
        constexpr uint8_t clearFlags = RegVals::Status::RX_DR | RegVals::Status::TX_DS | RegVals::Status::MAX_RT;
        fullyInitialized
            += EXPECT_CALL(*nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::STATUS.addr, clearFlags>), 2, _))
                   .Times(AtLeast(1));
        fullyInitialized += EXPECT_CALL(
            *nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::EN_RXADDR.addr, RegVals::EnRXAddr::ERX_NONE>), 2, _))
                                .Times(AtLeast(1));
        fullyInitialized
            += EXPECT_CALL(*nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::RF_CH.addr>), 2, _)).Times(AtLeast(1));
        fullyInitialized += EXPECT_CALL(
            *nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::EN_AA.addr, RegVals::EnAA::ENAA_ALL>), 2, _))
                                .Times(AtLeast(1));
        fullyInitialized += EXPECT_CALL(
            *nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::DYNPD.addr, RegVals::DynPD::DPL_ALL>), 2, _))
                                .Times(AtLeast(1))
                                .After(featuresEnabled);
        fullyInitialized += EXPECT_CALL(*nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::FEATURE.addr>), 2, _))
                                .Times(AtLeast(1))
                                .After(featuresEnabled);
        fullyInitialized
            += EXPECT_CALL(*nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::CONFIG.addr>), 2, _)).Times(AtLeast(1));
        fullyInitialized
            += EXPECT_CALL(*nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::RF_SETUP.addr>), 2, _)).Times(AtLeast(1));
        fullyInitialized += EXPECT_CALL(*nrf, SPIWrite(Truly(IsWriteRegisterCommand<Regs::SETUP_RETR.addr>), 2, _))
                                .Times(AtLeast(1));
        return fullyInitialized;
    }
    // Returns lambda that compares const void* to msg
    auto MsgEquals(const Message& msg)
    {
        return [msg](const void* p) {
            const uint8_t* pMsg = reinterpret_cast<const uint8_t*>(&msg);
            const uint8_t* pM = reinterpret_cast<const uint8_t*>(p);
            return std::equal(pMsg, pMsg + msg.GetSize(), pM, pM + msg.GetSize());
        };
    }
    // Returned ptr set to true when called
    VerifyLater ExpectHandleMessage(NodeCommunication& nc, const Message& msg)
    {
        std::shared_ptr<bool> p = std::make_shared<bool>(false);
        Res::EventSystem().AddHandler(
            std::make_shared<CallbackMessageHandler>(nc, [=](NodeCommunication&, const Message& m) {
                EXPECT_EQ(m, msg) << "Handled Message is different from received";
                *p = true;
                // Remove handler
                return true;
            }));
        return VerifyLater(p);
    }

public:
    std::unique_ptr<MockNRF> nrf;
};

TEST_F(TestNodeCommunication, Constructor)
{
    using namespace ::testing;
    // Suppress warnings
    EXPECT_CALL(*nrf, Write(_, _)).Times(AnyNumber());
    EXPECT_CALL(*nrf, SPIWrite(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*nrf, SPIWriteRead(_, _, _, _)).Times(AnyNumber());

    // Will only fail if nrf tests are actually done
    if (configuration::USE_NRF && configuration::TEST_NRF)
    {
        // Silence error messages
        SilenceCout silence;
        EXPECT_THROW(NodeCommunication(std::move(nrf), 0), std::system_error);
    }

    {
        // Register calls for tests first, because they are more generic
        SetupForNRFTest();
        // Expectations for initialization
        ExpectationSet fullyInitialized = SetupInitExpectations();

        const uint64_t baseId = 10;
        ExpectationSet pipesOpened;
        pipesOpened
            += EXPECT_CALL(*nrf, OpenReadingPipe(0, NodeCommunication::s_broadcastAddr)).After(fullyInitialized);
        pipesOpened += EXPECT_CALL(*nrf, OpenReadingPipe(1, baseId - 1)).After(fullyInitialized);
        EXPECT_CALL(*nrf, StartListening()).After(pipesOpened);

        NodeCommunication nc(std::move(nrf), baseId);
        EXPECT_EQ(baseId, nc.GetBaseId());
        EXPECT_FALSE(nc.IsRunning());
    }
    {
        // Register calls for tests first, because they are more generic
        SetupForNRFTest();
        // Expectations for initialization
        ExpectationSet fullyInitialized = SetupInitExpectations();

        const uint64_t baseId = 0xA5BAF2345;
        ExpectationSet pipesOpened;
        pipesOpened
            += EXPECT_CALL(*nrf, OpenReadingPipe(0, NodeCommunication::s_broadcastAddr)).After(fullyInitialized);
        pipesOpened += EXPECT_CALL(*nrf, OpenReadingPipe(1, baseId - 1)).After(fullyInitialized);
        EXPECT_CALL(*nrf, StartListening()).After(pipesOpened);

        NodeCommunication nc(std::move(nrf), baseId);
        EXPECT_EQ(baseId, nc.GetBaseId());
        EXPECT_FALSE(nc.IsRunning());
    }
}

TEST_F(TestNodeCommunication, CheckMessages)
{
    using namespace ::testing;
    SetupForNRFTest();
    MockNRF& rf = *nrf;
    NodeCommunication nc(std::move(nrf), 1);
    EXPECT_CALL(rf, Available(_)).WillOnce(Return(false)).RetiresOnSaturation();
    // Verify that no read occurs
    EXPECT_CALL(rf, Read(_, _)).Times(0);
    nc.CheckMessages();

    EXPECT_CALL(rf, Available(_)).WillOnce(Invoke([](uint8_t* p) {
        if (p)
        {
            *p = 1;
        }
        return true;
    }));
    EXPECT_CALL(rf, GetStatus())
        .WillOnce(Return(RF24::RegVals::Status::RX_P_NO_1))
        .WillOnce(Return(RF24::RegVals::Status::RX_P_NO_RX_FIFO_NOT_EMPTY));
    ::Message msg = Messages::AssignId(NodePath(), NodePath(), 0x2A, 1, 1, NodePath(1, 3));
    {
        InSequence seq;
        EXPECT_CALL(rf, StopListening());
        EXPECT_CALL(rf, GetPayloadLength()).WillOnce(Return(msg.GetSize()));
        EXPECT_CALL(rf, Read(_, msg.GetSize())).WillOnce(Invoke([&](void* out, uint8_t len) {
            std::copy_n(reinterpret_cast<const uint8_t*>(&msg), len, reinterpret_cast<uint8_t*>(out));
            return true;
        }));
        EXPECT_CALL(rf, StartListening());
    }
    {
        VerifyLater v = ExpectHandleMessage(nc, msg);
        nc.CheckMessages();
    }

    // Test len > 32
    EXPECT_CALL(rf, Available(_)).WillOnce(Invoke([](uint8_t* p) {
        if (p)
        {
            *p = 1;
        }
        return true;
    }));
    EXPECT_CALL(rf, GetStatus())
        .WillOnce(Return(RF24::RegVals::Status::RX_P_NO_1))
        .WillOnce(Return(RF24::RegVals::Status::RX_P_NO_RX_FIFO_NOT_EMPTY));
    {
        InSequence seq;
        EXPECT_CALL(rf, StopListening());
        EXPECT_CALL(rf, GetPayloadLength()).WillOnce(Return(33));
        EXPECT_CALL(rf, Write(RF24::Commands::FLUSH_RX));
        EXPECT_CALL(rf, StartListening());
    }
    nc.CheckMessages();
}

TEST_F(TestNodeCommunication, SendEmptyQueue)
{
    using namespace ::testing;
    SetupForNRFTest();
    MockNRF& rf = *nrf;
    NodeCommunication nc(std::move(nrf), 1);
    // Empty queue
    EXPECT_CALL(rf, StopListening()).Times(0);
    EXPECT_CALL(rf, Transmit(_, _, _)).Times(0);
    EXPECT_CALL(rf, Transmit(_, _)).Times(0);
    nc.SendQueue();
}

TEST_F(TestNodeCommunication, SendMessage)
{
    using namespace ::testing;
    SetupForNRFTest();
    MockNRF& rf = *nrf;
    NodeCommunication nc(std::move(nrf), 1);

    // Send message (success)
    {
        ::Message msg = Messages::SetSensorType(NodePath(1, 1), NodePath(), 1, 3, 2);
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.SendMessage(msg);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // Send message (fail)
    {
        ::Message msg = Messages::SetSensorType(NodePath(1, 1), NodePath(), 1, 3, 2);
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.SendMessage(msg);
        nc.SendQueue();
        // Result must NOT be ready (1 retry with SendMessage)
        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        // Retry sending
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        nc.SendQueue();

        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());
    }
    // Send message (success in retry)
    {
        ::Message msg = Messages::SetSensorType(NodePath(1, 1), NodePath(), 1, 3, 2);
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.SendMessage(msg);
        nc.SendQueue();
        // Result must NOT be ready (1 retry with SendMessage)
        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        // Retry sending
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        nc.SendQueue();

        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
}
TEST_F(TestNodeCommunication, SendMessageAddress)
{
    using namespace ::testing;
    SetupForNRFTest();
    MockNRF& rf = *nrf;
    NodeCommunication nc(std::move(nrf), 1);
    // Send message to specific address
    {
        ::Message msg = Messages::SetSensorType(NodePath(1, 1), NodePath(), 1, 3, 2);
        const uint64_t addr = 0xAAFE234;
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.SendMessage(msg, addr);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }

    // Send message to specific address (success in retry)
    {
        ::Message msg = Messages::SetSensorType(NodePath(1, 1), NodePath(), 1, 3, 2);
        const uint64_t addr = 0xAAFE234;
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.SendMessage(msg, addr);
        nc.SendQueue();
        // Result must NOT be ready (1 retry left)
        ASSERT_TRUE(f.valid());
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        // Retry sending, now successful
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
}

TEST_F(TestNodeCommunication, QueueMessage)
{
    using namespace ::testing;
    SetupForNRFTest();
    MockNRF& rf = *nrf;
    NodeCommunication nc(std::move(nrf), 1);
    // No retries (success)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.QueueMessage(msg, 0);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // No retries (fail)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.QueueMessage(msg, 0);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());

        // must be removed from queue after fail
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
    // 1 retry (success on 0)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.QueueMessage(msg, 1);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());

        // must be removed from queue
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
    // 3 retries (success on 3)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        // Called in reverse order, last one succeeds
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        for (int i = 0; i < 3; ++i)
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening()).RetiresOnSaturation();
            EXPECT_CALL(rf, OpenWritingPipe(2)).RetiresOnSaturation();
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize()))
                .WillOnce(Return(false))
                .RetiresOnSaturation();
            EXPECT_CALL(rf, StartListening()).RetiresOnSaturation();
        }
        std::future<bool> f = nc.QueueMessage(msg, 3);
        for (int i = 0; i < 3; ++i)
        {
            nc.SendQueue();
            // Result must not be ready
            ASSERT_TRUE(f.valid());
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));
        }
        // last retry succeeds
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());

        // must be removed from queue after success
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
    // 3 retries (fail)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        // Called in reverse order, last one fails
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(2));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        for (int i = 0; i < 3; ++i)
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening()).RetiresOnSaturation();
            EXPECT_CALL(rf, OpenWritingPipe(2)).RetiresOnSaturation();
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize()))
                .WillOnce(Return(false))
                .RetiresOnSaturation();
            EXPECT_CALL(rf, StartListening()).RetiresOnSaturation();
        }
        std::future<bool> f = nc.QueueMessage(msg, 3);
        for (int i = 0; i < 3; ++i)
        {
            nc.SendQueue();
            // Result must not be ready
            ASSERT_TRUE(f.valid());
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));
        }
        // last retry succeeds
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());

        // must be removed from queue after fail
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
}
TEST_F(TestNodeCommunication, QueueMessageAddress)
{
    using namespace ::testing;
    SetupForNRFTest();
    MockNRF& rf = *nrf;
    NodeCommunication nc(std::move(nrf), 1);
    // No retries (success)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        const uint64_t addr = 0xAFE0032AB;
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.QueueMessage(msg, 0, addr);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // No retries (fail)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        const uint64_t addr = 0xAFEEE32AB;
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.QueueMessage(msg, 0, addr);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());

        // must be removed from queue after fail
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
    // 1 retry (success on 0)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        const uint64_t addr = 0xEE;
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        std::future<bool> f = nc.QueueMessage(msg, 1, addr);
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());

        // must be removed from queue
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
    // 3 retries (success on 3)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        const uint64_t addr = 0xAFE0032AB;
        // Called in reverse order, last one succeeds
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(true));
            EXPECT_CALL(rf, StartListening());
        }
        for (int i = 0; i < 3; ++i)
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening()).RetiresOnSaturation();
            EXPECT_CALL(rf, OpenWritingPipe(addr)).RetiresOnSaturation();
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize()))
                .WillOnce(Return(false))
                .RetiresOnSaturation();
            EXPECT_CALL(rf, StartListening()).RetiresOnSaturation();
        }
        std::future<bool> f = nc.QueueMessage(msg, 3, addr);
        for (int i = 0; i < 3; ++i)
        {
            nc.SendQueue();
            // Result must not be ready
            ASSERT_TRUE(f.valid());
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));
        }
        // last retry succeeds
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());

        // must be removed from queue after success
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
    // 3 retries (fail)
    {
        ::Message msg = Messages::GetActorState(NodePath(1, 1), NodePath(), 0);
        const uint64_t addr = 0xAFE0032AB;
        // Called in reverse order, last one fails
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening());
            EXPECT_CALL(rf, OpenWritingPipe(addr));
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize())).WillOnce(Return(false));
            EXPECT_CALL(rf, StartListening());
        }
        for (int i = 0; i < 3; ++i)
        {
            InSequence s;
            EXPECT_CALL(rf, StopListening()).RetiresOnSaturation();
            EXPECT_CALL(rf, OpenWritingPipe(addr)).RetiresOnSaturation();
            EXPECT_CALL(rf, Transmit(Truly(MsgEquals(msg)), msg.GetSize()))
                .WillOnce(Return(false))
                .RetiresOnSaturation();
            EXPECT_CALL(rf, StartListening()).RetiresOnSaturation();
        }
        std::future<bool> f = nc.QueueMessage(msg, 3, addr);
        for (int i = 0; i < 3; ++i)
        {
            nc.SendQueue();
            // Result must not be ready
            ASSERT_TRUE(f.valid());
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));
        }
        // last retry succeeds
        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());

        // must be removed from queue after fail
        EXPECT_CALL(rf, StopListening()).Times(0);
        EXPECT_CALL(rf, OpenWritingPipe(_)).Times(0);
        EXPECT_CALL(rf, Transmit(_, _)).Times(0);
        nc.SendQueue();
    }
}

TEST(NodeCommunication, SendMessageSequence)
{
    using namespace ::testing;
    using ::Message;
    MockNodeCommunication nc{1};

    // Forward calls to actual implementation
    EXPECT_CALL(nc, SendMessageSequence(_)).WillRepeatedly(Invoke([&](MessageSequence s) {
        return nc.NodeCommunication::SendMessageSequence(std::move(s));
    }));

    // Empty message sequence
    {
        MessageSequence s{{}, 5};

        EXPECT_CALL(nc, SendMessageNow(_, _)).Times(0);

        std::future<bool> f = nc.SendMessageSequence(s);

        ASSERT_TRUE(f.valid());

        // Future will be set in SendQueue()
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // Message sequence with no tries
    {
        MessageSequence s{{Messages::Ack(NodePath(), NodePath())}, 0};

        EXPECT_CALL(nc, SendMessageNow(_, _)).Times(0);

        std::future<bool> f = nc.SendMessageSequence(s);

        ASSERT_TRUE(f.valid());

        // Future will be set in SendQueue()
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());
    }

    // One message, success
    {
        std::vector<Message> messages = {Messages::Ack(NodePath(1, 1), NodePath())};
        MessageSequence s{messages, 5};

        EXPECT_CALL(nc, SendMessageNow(messages[0], 0)).WillOnce(Return(true));

        std::future<bool> f = nc.SendMessageSequence(s);

        ASSERT_TRUE(f.valid());

        // Future will be set in SendQueue()
        EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

        nc.SendQueue();
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // One message, fail
    {
        std::vector<Message> messages = {Messages::Ack(NodePath(1, 1), NodePath())};
        MessageSequence s{messages, 5};

        EXPECT_CALL(nc, SendMessageNow(messages[0], 0)).Times(5).WillRepeatedly(Return(false));

        std::future<bool> f = nc.SendMessageSequence(s);

        for (int i = 0; i < 5; ++i)
        {
            ASSERT_TRUE(f.valid());

            // Future will be set in SendQueue() when all retries are over
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

            nc.SendQueue();
        }
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());
    }
    // One message, success in last try
    {
        std::vector<Message> messages = {Messages::Ack(NodePath(1, 1), NodePath())};
        MessageSequence s{messages, 5};

        EXPECT_CALL(nc, SendMessageNow(messages[0], 0))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(true));

        std::future<bool> f = nc.SendMessageSequence(s);

        for (int i = 0; i < 5; ++i)
        {
            ASSERT_TRUE(f.valid());

            // Future will be set in SendQueue() when all retries are over
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

            nc.SendQueue();
        }
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // Multiple messages, success
    {
        std::vector<Message> messages
            = {Messages::Ack(NodePath(1, 1), NodePath()), Messages::Ack(NodePath(2, 1), NodePath())};
        MessageSequence s{messages, 5};

        {
            InSequence s;
            EXPECT_CALL(nc, SendMessageNow(messages[0], 0)).WillOnce(Return(true));
            EXPECT_CALL(nc, SendMessageNow(messages[1], 0)).WillOnce(Return(true));
        }

        std::future<bool> f = nc.SendMessageSequence(s);

        for (int i = 0; i < 2; ++i)
        {
            ASSERT_TRUE(f.valid());

            // Future will be set in SendQueue() when all retries are over
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

            nc.SendQueue();
        }
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
    // Multiple messages, fail first message
    {
        std::vector<Message> messages
            = {Messages::Ack(NodePath(1, 1), NodePath()), Messages::Ack(NodePath(2, 1), NodePath())};
        MessageSequence s{messages, 5};

        EXPECT_CALL(nc, SendMessageNow(messages[0], 0)).Times(5).WillRepeatedly(Return(false));
        EXPECT_CALL(nc, SendMessageNow(messages[1], 0)).Times(0);

        std::future<bool> f = nc.SendMessageSequence(s);

        for (int i = 0; i < 5; ++i)
        {
            ASSERT_TRUE(f.valid());

            // Future will be set in SendQueue() when all retries are over
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

            nc.SendQueue();
        }
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());
    }
    // Multiple messages, fail second message
    {
        std::vector<Message> messages
            = {Messages::Ack(NodePath(1, 1), NodePath()), Messages::Ack(NodePath(2, 1), NodePath())};
        MessageSequence s{messages, 5};

        EXPECT_CALL(nc, SendMessageNow(messages[0], 0)).WillOnce(Return(true));
        EXPECT_CALL(nc, SendMessageNow(messages[1], 0)).Times(5).WillRepeatedly(Return(false));

        std::future<bool> f = nc.SendMessageSequence(s);

        for (int i = 0; i < 6; ++i)
        {
            ASSERT_TRUE(f.valid());

            // Future will be set in SendQueue() when all retries are over
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

            nc.SendQueue();
        }
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_FALSE(f.get());
    }
    // Multiple messages, retries first and second message
    {
        std::vector<Message> messages
            = {Messages::Ack(NodePath(1, 1), NodePath()), Messages::Ack(NodePath(2, 1), NodePath())};
        MessageSequence s{messages, 5};

        EXPECT_CALL(nc, SendMessageNow(messages[0], 0))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(true));
        EXPECT_CALL(nc, SendMessageNow(messages[1], 0))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(true));

        std::future<bool> f = nc.SendMessageSequence(s);

        for (int i = 0; i < 10; ++i)
        {
            ASSERT_TRUE(f.valid());

            // Future will be set in SendQueue() when all retries are over
            EXPECT_EQ(std::future_status::timeout, f.wait_for(std::chrono::seconds(0)));

            nc.SendQueue();
        }
        // Result must be ready
        ASSERT_TRUE(f.valid());
        // Assert, otherwise f.get() will block the test
        ASSERT_EQ(std::future_status::ready, f.wait_for(std::chrono::seconds(0)));
        EXPECT_TRUE(f.get());
    }
}
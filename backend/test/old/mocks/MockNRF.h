#ifndef MOCK_NRF_H
#define MOCK_NRF_H

#include <gmock/gmock.h>

#include "communication/nRF24L01.h"
#include "communication/nRFTests.h"

class MockNRF : public RF24::IRadio
{
public:
    MockNRF() = default;

    MOCK_METHOD0(GetStatus, uint8_t());
    MOCK_METHOD1(GetStatus, uint8_t(std::error_code& ec));
    MOCK_METHOD1(WriteRead, uint8_t(uint8_t val));
    MOCK_METHOD2(WriteRead, uint8_t(uint8_t val, std::error_code& ec));
    MOCK_METHOD1(Write, void(uint8_t val));
    MOCK_METHOD2(Write, std::error_code(uint8_t val, std::error_code& ec));
    MOCK_METHOD2(Transmit, bool(const void* buf, uint8_t len));
    MOCK_METHOD3(Transmit, bool(const void* buf, uint8_t len, std::error_code& ec));
    MOCK_METHOD1(Available, bool(uint8_t* pipe));
    MOCK_METHOD2(Available, bool(uint8_t* pipe, std::error_code& ec));
    MOCK_METHOD0(GetPayloadLength, uint8_t());
    MOCK_METHOD1(GetPayloadLength, uint8_t(std::error_code& ec));
    MOCK_METHOD2(Read, bool(void* buf, uint8_t len));
    MOCK_METHOD3(Read, bool(void* buf, uint8_t len, std::error_code& ec));
    MOCK_METHOD2(OpenReadingPipe, void(uint8_t pipe, uint64_t addr));
    MOCK_METHOD3(OpenReadingPipe, std::error_code(uint8_t pipe, uint64_t addr, std::error_code& ec));
    MOCK_METHOD1(CloseReadingPipe, void(uint8_t pipe));
    MOCK_METHOD2(CloseReadingPipe, std::error_code(uint8_t pipe, std::error_code& ec));
    MOCK_METHOD1(OpenWritingPipe, void(uint64_t addr));
    MOCK_METHOD2(OpenWritingPipe, std::error_code(uint64_t addr, std::error_code& ec));
    MOCK_METHOD0(StartListening, void());
    MOCK_METHOD1(StartListening, std::error_code(std::error_code& ec));
    MOCK_METHOD0(StopListening, void());
    MOCK_METHOD1(StopListening, std::error_code(std::error_code& ec));
    MOCK_METHOD0(PrintRegisters, void());
    MOCK_METHOD2(SPIWrite, void(const void* buf, uint8_t len));
    MOCK_METHOD3(SPIWrite, std::error_code(const void* buf, uint8_t len, std::error_code& ec));
    MOCK_METHOD3(SPIWriteRead, void(const void* bufOut, void* bufIn, uint8_t len));
    MOCK_METHOD4(SPIWriteRead, std::error_code(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec));

    inline void SetupForNRFTest();
};

// Useful as an action, to return a value from Reg()
template <uint8_t... Vals>
std::error_code SPIWriteReadRegValue(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec)
{
    const uint8_t* cmd = reinterpret_cast<const uint8_t*>(bufOut);
    uint8_t* res = reinterpret_cast<uint8_t*>(bufIn);
    if (len > 0 && (cmd[0] & ~RF24::Commands::REGISTER_MASK) == 0)
    {
        // Command is R_REGISTER
        res[0] = GetDefaultVal(RF24::Regs::STATUS);
        // Get values into one array
        const uint8_t val[] = {Vals...};
        std::copy_n(reinterpret_cast<const uint8_t*>(&val), std::min<uint8_t>(len - 1, sizeof(val)), res + 1);
    }
    return ec;
}
// Useful as a default action (.Will(Invoke(SPIWriteReadDefaultRegValue)))
inline std::error_code SPIWriteReadDefaultRegValue(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec)
{
    const uint8_t* cmd = reinterpret_cast<const uint8_t*>(bufOut);
    uint8_t* res = reinterpret_cast<uint8_t*>(bufIn);
    if (len > 0 && (cmd[0] & ~RF24::Commands::REGISTER_MASK) == 0)
    {
        // Command is R_REGISTER
        res[0] = GetDefaultVal(RF24::Regs::STATUS);
        if (len == 2)
        {
            // 1 byte register
            res[1] = GetDefaultVal(RF24::Regs::RegAddr<1>{cmd[0]});
        }
        else
        {
            // Have to make local copy, because constexpr variable needs definition otherwise
            uint64_t val = RF24::RegVals::DefaultValues::RX_ADDR_P0;
            switch (cmd[0])
            {
            case RF24::Regs::RX_ADDR_P0.addr:
                // val = RF24::RegVals::DefaultValues::RX_ADDR_P0;
                break;
            case RF24::Regs::RX_ADDR_P1.addr:
                val = RF24::RegVals::DefaultValues::RX_ADDR_P1;
                break;
            case RF24::Regs::TX_ADDR.addr:
                val = RF24::RegVals::DefaultValues::TX_ADDR;
                break;
            default:
                ec = std::make_error_code(std::errc::invalid_argument);
                return ec;
            }
            std::copy_n(reinterpret_cast<const uint8_t*>(&val), std::min<uint8_t>(len - 1, 5), res + 1);
        }
    }
    return ec;
}
// Useful as a matcher Truly(IsReadRegisterCommand<reg>). Will be undefined behavior for length 0
template <uint8_t addr>
bool IsReadRegisterCommand(const void* buf)
{
    return reinterpret_cast<const uint8_t*>(buf)[0] == (RF24::Commands::R_REGISTER | addr);
}

// Useful as matcher
template <uint8_t addr, uint8_t... vals>
bool IsWriteRegisterCommand(const void* buf)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(buf);
    const std::initializer_list<uint8_t> i = {vals...};
    // i.size() == 0 to not increment bytes if vals is not given
    return bytes[0] == (RF24::Commands::W_REGISTER | addr)
        && (i.size() == 0 || std::equal(i.begin(), i.end(), bytes + 1));
}

inline void MockNRF::SetupForNRFTest()
{
    using namespace ::testing;
    EXPECT_CALL(*this, Write(_)).Times(AnyNumber());
    EXPECT_CALL(*this, Write(_, _)).Times(AnyNumber()).WillRepeatedly(Return(std::error_code{}));
    EXPECT_CALL(*this, SPIWrite(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, SPIWrite(_, _, _)).Times(AnyNumber());
    auto wrapEc = [](auto a, auto b, auto c) {
        std::error_code ec;
        SPIWriteReadDefaultRegValue(a, b, c, ec);
        if (ec)
        {
            throw std::system_error(ec);
        }
    };
    EXPECT_CALL(*this, SPIWriteRead(_, _, _)).Times(AnyNumber()).WillRepeatedly(Invoke(wrapEc));
    EXPECT_CALL(*this, SPIWriteRead(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Invoke(&SPIWriteReadDefaultRegValue));
    constexpr uint8_t fifoStatus = RF24::RegVals::FIFOStatus::RX_EMPTY | RF24::RegVals::FIFOStatus::TX_EMPTY;
    EXPECT_CALL(*this, SPIWriteRead(Truly(IsReadRegisterCommand<RF24::Regs::FIFO_STATUS.addr>), _, Gt(1), _))
        .Times(AnyNumber())
        .WillRepeatedly(Invoke(&SPIWriteReadRegValue<fifoStatus>));
    // To prevent segfaults of above with len 0
    EXPECT_CALL(*this, SPIWriteRead(_, _, 0, _)).Times(0);
    EXPECT_CALL(*this, PrintRegisters()).Times(AnyNumber());
    EXPECT_CALL(*this, OpenReadingPipe(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, StartListening()).Times(AnyNumber());
}

#endif
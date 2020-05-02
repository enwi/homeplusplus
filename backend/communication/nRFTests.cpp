#include "nRFTests.h"

#include <algorithm>
#include <array>
#include <system_error>

namespace
{
    constexpr std::array<RF24::Regs::RegAddr<1>, 21> g_regs = {RF24::Regs::CONFIG, RF24::Regs::EN_AA,
        RF24::Regs::EN_RXADDR, RF24::Regs::SETUP_AW, RF24::Regs::SETUP_RETR, RF24::Regs::RF_CH, RF24::Regs::RF_SETUP,
        RF24::Regs::STATUS,
        // read only: Observe TX cannot be changed
        // RF24::Regs::OBSERVE_TX,
        // read only
        // RF24::Regs::RPD,
        RF24::Regs::RX_ADDR_P2, RF24::Regs::RX_ADDR_P3, RF24::Regs::RX_ADDR_P4, RF24::Regs::RX_ADDR_P5,
        RF24::Regs::RX_PW_P0, RF24::Regs::RX_PW_P1, RF24::Regs::RX_PW_P2, RF24::Regs::RX_PW_P3, RF24::Regs::RX_PW_P4,
        RF24::Regs::RX_PW_P5, RF24::Regs::FIFO_STATUS, RF24::Regs::DYNPD, RF24::Regs::FEATURE};
    constexpr std::array<uint8_t, 21> g_defaults
        = {RF24::RegVals::DefaultValues::CONFIG, RF24::RegVals::DefaultValues::EN_AA,
            RF24::RegVals::DefaultValues::EN_RXADDR, RF24::RegVals::DefaultValues::SETUP_AW,
            RF24::RegVals::DefaultValues::SETUP_RETR, RF24::RegVals::DefaultValues::RF_CH,
            RF24::RegVals::DefaultValues::RF_SETUP, RF24::RegVals::DefaultValues::STATUS,
            // Observe TX cannot be changed
            // RF24::RegVals::DefaultValues::OBSERVE_TX,
            // read only
            // RF24::RegVals::DefaultValues::RPD,
            RF24::RegVals::DefaultValues::RX_ADDR_P2, RF24::RegVals::DefaultValues::RX_ADDR_P3,
            RF24::RegVals::DefaultValues::RX_ADDR_P4, RF24::RegVals::DefaultValues::RX_ADDR_P5,
            RF24::RegVals::DefaultValues::RX_PW_P0, RF24::RegVals::DefaultValues::RX_PW_P1,
            RF24::RegVals::DefaultValues::RX_PW_P2, RF24::RegVals::DefaultValues::RX_PW_P3,
            RF24::RegVals::DefaultValues::RX_PW_P4, RF24::RegVals::DefaultValues::RX_PW_P5,
            RF24::RegVals::DefaultValues::FIFO_STATUS, RF24::RegVals::DefaultValues::DYNPD,
            RF24::RegVals::DefaultValues::FEATURE};

    std::error_code ResetState(RF24::IRadio& nrf)
    {
        std::error_code ec;
        nrf.Write(RF24::Commands::FLUSH_RX, ec);
        if (ec)
        {
            return ec;
        }
        nrf.Write(RF24::Commands::FLUSH_TX, ec);
        if (ec)
        {
            return ec;
        }

        // P0 and P1 are bigger registers
        if (nrf.Reg(RF24::Regs::RX_ADDR_P0).Set(RF24::RegVals::DefaultValues::RX_ADDR_P0, ec))
        {
            return ec;
        }
        if (nrf.Reg(RF24::Regs::RX_ADDR_P1).Set(RF24::RegVals::DefaultValues::RX_ADDR_P1, ec))
        {
            return ec;
        }
        for (std::size_t i = 0; i < g_regs.size(); ++i)
        {
            if (nrf.Reg(g_regs[i]).Set(g_defaults[i], ec))
            {
                return ec;
            }
        }
        return ec;
    }
    std::error_code VerifyDefault(RF24::IRadio& nrf)
    {
        std::error_code ec;
        // Verify FIFOs empty
        uint8_t fifoStatus = nrf.Reg(RF24::Regs::FIFO_STATUS).Get(ec);
        if (ec)
        {
            return ec;
        }
        // TODO: add own error code
        std::error_code invalidValue = std::make_error_code(std::errc::protocol_error);
        if ((fifoStatus & RF24::RegVals::FIFOStatus::RX_EMPTY) == 0)
        {
            std::cerr << "VerifyDefault: RX fifo not empty\n";
            return invalidValue;
        }
        if ((fifoStatus & RF24::RegVals::FIFOStatus::TX_EMPTY) == 0)
        {
            std::cerr << "VerifyDefault: TX fifo not empty\n";
            return invalidValue;
        }
        uint64_t rx0 = nrf.Reg(RF24::Regs::RX_ADDR_P0).Get(ec);
        if (ec)
        {
            return ec;
        }
        if (rx0 != RF24::RegVals::DefaultValues::RX_ADDR_P0)
        {
            std::cerr << "VerifyDefault: RX_ADDR_P0, expected " << std::hex << RF24::RegVals::DefaultValues::RX_ADDR_P0
                      << ", got " << std::hex << rx0 << std::dec << '\n';
            return invalidValue;
        }
        uint64_t rx1 = nrf.Reg(RF24::Regs::RX_ADDR_P1).Get(ec);
        if (ec)
        {
            return ec;
        }
        if (rx1 != RF24::RegVals::DefaultValues::RX_ADDR_P1)
        {
            std::cerr << "VerifyDefault: RX_ADDR_P1, expected " << std::hex << RF24::RegVals::DefaultValues::RX_ADDR_P1
                      << ", got " << std::hex << rx1 << std::dec << '\n';
            return invalidValue;
        }
        for (std::size_t i = 0; i < g_regs.size(); ++i)
        {
            uint8_t val = nrf.Reg(g_regs[i]).Get(ec);
            if (ec)
            {
                return ec;
            }
            if (val != g_defaults[i])
            {
                std::cerr << "VerifyDefault: register " << (int)g_regs[i].addr << ", expected " << std::hex
                          << (int)g_defaults[i] << ", got " << std::hex << (int)val << std::dec << '\n';
                return invalidValue;
            }
        }
        return ec;
    }

} // namespace
uint8_t GetDefaultVal(RF24::Regs::RegAddr<1> addr)
{
    // observe tx is not in list
    if (addr.addr == RF24::Regs::OBSERVE_TX.addr)
    {
        return RF24::RegVals::DefaultValues::OBSERVE_TX;
    }
    auto it = std::find_if(std::begin(g_regs), std::end(g_regs), [&](auto a) { return a.addr == addr.addr; });
    if (it == std::end(g_regs))
    {
        throw std::out_of_range("GetDefaultVal addr not found");
    }
    return g_defaults[it - std::begin(g_regs)];
}
std::error_code PerformTests(RF24::IRadio& nrf)
{
    std::error_code ec = ResetState(nrf);
    if (ec)
    {
        return ec;
    }
    ec = VerifyDefault(nrf);
    return ec;
}

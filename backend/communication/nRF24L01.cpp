/*
 * nRF24L01.c
 *
 *      Author: Erich Styger
 */

#include "nRF24L01.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "spi.h"

#if !defined(_MSC_VER) && !defined(__APPLE__)
GPIOPin::GPIOPin(int num, Mode mode) : num(num)
{
    {
        constexpr const char* export_str = "/sys/class/gpio/export";
        std::ofstream exportgpio(
            export_str); // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
        // export gpio, check if successful
        if (!(exportgpio << num))
        {
            std::cout << "Error: Could not open export file" << '\n';
            throw std::runtime_error("gpio pin: export");
        }
        std::cout << "Successfully exported gpio" << num << '\n';
    }
    SetMode(mode);
}

GPIOPin::GPIOPin(GPIOPin&& other) noexcept : num(other.num)
{
    // Set to -1 so other does not close it
    other.num = -1;
}

GPIOPin& GPIOPin::operator=(GPIOPin&& other) noexcept
{
    // If this is open, other's destructor will close it
    std::swap(num, other.num);
    return *this;
}

void GPIOPin::operator=(bool val)
{
    RF24::detail::ErrorThrower([&](auto& ec) { this->Set(val, ec); });
}

GPIOPin::operator bool()
{
    // TODO: implement GPIOPin read
    throw std::logic_error("please implement GPIOPin::operator bool().");
}

std::error_code GPIOPin::Set(bool val, std::error_code& ec)
{
    const std::string setval_str = std::string("/sys/class/gpio/gpio") + std::to_string(num) + "/value";
    std::ofstream setvalgpio(setval_str); // open value file for gpio
    if (!setvalgpio)
    {
        ec = std::error_code(errno, std::generic_category());
        return ec;
    }
    if (!(setvalgpio << val))
    {
        ec = std::error_code(errno, std::generic_category());
        std::cout << " OPERATION FAILED: Unable to set the value of GPIO" << num << " ." << std::endl;
        return ec;
    }
    ec = std::error_code{};
    return ec;
}

bool GPIOPin::Get(std::error_code& ec)
{
    ec = std::make_error_code(std::errc::operation_not_supported);
    return false;
}

GPIOPin::~GPIOPin()
{
    // Num < 0: moved from GPIO, do not close
    if (num >= 0)
    {
        constexpr const char* unexport_str = "/sys/class/gpio/unexport";
        std::ofstream unexportgpio(unexport_str); // Open unexport file
        if (!(unexportgpio << num))
        {
            std::cout << "Error: Could not open unexport file" << '\n';
            // Do not throw exception in destructor!
        }
        std::cout << "Successfully unexported gpio" << num << '\n';
    }
}

void GPIOPin::SetMode(Mode mode)
{
    const std::string setDirStr = "/sys/class/gpio/gpio" + std::to_string(num) + "/direction";
    std::ofstream setDirFile(setDirStr); // open direction file for gpio
    const char* modeStr = mode == Mode::input ? "in" : (mode == Mode::output ? "out" : "");
    if (!(setDirFile << modeStr))
    {
        std::cout << "Could not open gpio direction file for " << num << '\n';
        throw std::runtime_error("gpio pin: set mode");
    }
}
#else
GPIOPin::GPIOPin(int num, Mode mode) {}
GPIOPin::GPIOPin(GPIOPin&& other) noexcept {}
GPIOPin& GPIOPin::operator=(GPIOPin&& other) noexcept
{
    return *this;
}
void GPIOPin::operator=(bool val) {}
GPIOPin::operator bool()
{
    return false;
}
std::error_code GPIOPin::Set(bool val, std::error_code& ec)
{
    return ec;
}
bool GPIOPin::Get(std::error_code& ec)
{
    return false;
}
GPIOPin::~GPIOPin() {}
void GPIOPin::SetMode(Mode mode) {}
#endif

constexpr uint8_t RF24::NRF24L01::maxLen;

#if !defined(_MSC_VER) && !defined(__APPLE__)
RF24::NRF24L01::NRF24L01(SPI&& spi, int ce, int csn)
    : spi(std::move(spi)), ce(ce, GPIOPin::Mode::output), csn(csn, GPIOPin::Mode::output)
{
    ce = false; /* CE low: do not send or receive data */
    csn = true; /* CSN high: not sending commands to the device */
    WaitMs(5);
}

uint8_t RF24::NRF24L01::GetStatus()
{
    return detail::ErrorThrower([this](auto& ec) { return this->GetStatus(ec); });
}

uint8_t RF24::NRF24L01::GetStatus(std::error_code& ec)
{
    return WriteRead(Commands::NOP, ec);
}

uint8_t RF24::NRF24L01::WriteRead(uint8_t val)
{
    return detail::ErrorThrower([&](auto& ec) { return this->WriteRead(val, ec); });
}

uint8_t RF24::NRF24L01::WriteRead(uint8_t val, std::error_code& ec)
{
    uint8_t res;
    if (useCsn)
    {
        if (csn.Set(false, ec))
        {
            return 0;
        }
    }
    ec = spi.transfer(reinterpret_cast<const char*>(&val), reinterpret_cast<char*>(&res), 1);
    if (useCsn)
    {
        if (csn.Set(true, ec))
        {
            return 0;
        }
    }
    WaitUs(5);
    return res;
}

void RF24::NRF24L01::Write(uint8_t val)
{
    detail::ErrorThrower([&](auto& ec) { this->WriteRead(val, ec); });
}

std::error_code RF24::NRF24L01::Write(uint8_t val, std::error_code& ec)
{
    WriteRead(val, ec);
    return ec;
}

bool RF24::NRF24L01::Transmit(const void* buf, uint8_t len)
{
    return detail::ErrorThrower([&](auto& ec) { return this->Transmit(buf, len, ec); });
}

bool RF24::NRF24L01::Transmit(const void* buf, uint8_t len, std::error_code& ec)
{
    int result = 0;
    bool tx_ok = false, tx_fail = false;
    unsigned int wrong_count = 0;
    // Begin the write
    // Transmitter power-up
    uint8_t configVal = (Reg(Regs::CONFIG).Get(ec) | RegVals::Config::PWR_UP) & ~RegVals::Config::PRIM_RX;
    if (ec)
    {
        return false;
    }
    if (Reg(Regs::CONFIG).Set(configVal, ec))
    {
        return false;
    }
    WaitUs(150);

    // Send the payload
    uint8_t status;

    const uint8_t data_len = std::min(len, maxLen);
    uint8_t txbuf[maxLen + 1];
    txbuf[0] = Commands::W_TX_PAYLOAD;
    memcpy(txbuf + 1, buf, data_len);
    do
    {
        SPIWrite(txbuf, data_len + 1, ec);
        if (ec)
        {
            return false;
        }
        WaitUs(4);

        // Put to transmit mode
        if (ce.Set(true, ec))
        {
            return false;
        }
        WaitUs(30);

        // ------------
        // At this point we could return from a non-blocking write, and then call
        // the rest after an interrupt

        // Instead, we are going to block here until we get TX_DS (transmission completed and ack'd)
        // or MAX_RT (maximum retries, transmission failed).  Also, we'll timeout in case the radio
        // is flaky and we get neither.

        // IN the end, the send should be blocking.  It comes back in 60ms worst case, or much faster
        // if I tighted up the retry logic.  (Default settings will be 1500us.
        // Monitor the send
        // uint8_t observe_tx;
        uint32_t sent_at = std::time(nullptr); // us since system reboot
        const uint32_t timeout = 50000; // us to wait for timeout
        do
        {
            status = GetStatus(ec);
            if (ec)
            {
                ce = false;
                return false;
            }
            // observe_tx = Reg(RF24_OBSERVE_TX);
        } while (
            !(status & (RegVals::Status::TX_DS | RegVals::Status::MAX_RT)) && (std::time(nullptr) - sent_at < timeout));
        if (ce.Set(false, ec))
        {
            return false;
        }

        // The part above is what you could recreate with your own interrupt handler,
        // and then call this when you got an interrupt
        // ------------

        // Call this when you get an interrupt
        // The status tells us three things
        // * The send was successful (TX_DS)
        // * The send failed, too many retries (MAX_RT)
        // * There is an ack packet waiting (RX_DR)

        status = GetStatus(ec);
        if (ec)
        {
            return false;
        }

        if (Reg(Regs::STATUS).Set(RegVals::Status::TX_DS | RegVals::Status::MAX_RT | RegVals::Status::RX_DR, ec))
        {
            return false;
        }

        tx_ok = status & RegVals::Status::TX_DS;
        tx_fail = status & RegVals::Status::MAX_RT;

        result = tx_ok;
        wrong_count++;
    } while (!(tx_ok || tx_fail) && wrong_count < 8);
    if (wrong_count >= 8)
    {
        std::cout << "Why does this happen?" << '\n';
    }
    // Yay, we are done.
    // Power down
    if (Reg(Regs::CONFIG).Set(~RegVals::Config::PWR_UP, ec))
    {
        return false;
    }
    // Flush buffers
    if (Write(Commands::FLUSH_TX, ec))
    {
        return false;
    }
    return result;
}

bool RF24::NRF24L01::Available(uint8_t* pipe)
{
    return detail::ErrorThrower([&](auto& ec) { return this->Available(pipe, ec); });
}

bool RF24::NRF24L01::Available(uint8_t* pipe, std::error_code& ec)
{
    // Get the status register which is sent with every SPI command
    const uint8_t status = GetStatus(ec);
    if (ec)
    {
        return false;
    }

    // Check if a packet was received
    const bool result = (status & RegVals::Status::RX_DR);

    // If pipe is not a nullptr, put the pipe information in it
    if (pipe != nullptr)
    {
        *pipe = (status & RegVals::Status::RX_P_NO) >> 1;
    }
    // Clear data received bit
    if (Reg(Regs::STATUS).Set(RegVals::Status::RX_DR | RegVals::Status::TX_DS, ec))
    {
        return false;
    }
    return result;
}

uint8_t RF24::NRF24L01::GetPayloadLength()
{
    return detail::ErrorThrower([&](auto& ec) { return this->GetPayloadLength(ec); });
}

uint8_t RF24::NRF24L01::GetPayloadLength(std::error_code& ec)
{
    const uint8_t tx[2] = {Commands::R_RX_PL_WID, Commands::NOP};
    uint8_t rx[2] = {};
    if (SPIWriteRead(tx, rx, 2, ec))
    {
        return 0;
    }
    return rx[1];
}

bool RF24::NRF24L01::Read(void* buf, uint8_t len)
{
    return detail::ErrorThrower([&](auto& ec) { return this->Read(buf, len, ec); });
}

bool RF24::NRF24L01::Read(void* buf, uint8_t len, std::error_code& ec)
{
    len = std::min(len, static_cast<uint8_t>(maxLen));
    std::cout << (int)Reg(Regs::FIFO_STATUS).Get(ec) << " ";
    if (ec)
    {
        return false;
    }
    std::cout << (int)GetStatus(ec) << " ";
    if (ec)
    {
        return false;
    }
    std::cout << (int)Reg(Regs::CONFIG).Get(ec) << '\n';
    if (ec)
    {
        return false;
    }
    uint8_t txbuf[maxLen + 1];
    uint8_t rxbuf[maxLen + 1];
    txbuf[0] = Commands::R_RX_PAYLOAD;
    memset(txbuf + 1, Commands::NOP, len);
    std::cout << "len:" << (int)len << '\n';
    for (uint8_t i : txbuf)
    {
        std::cout << (int)i << '|';
    }
    std::cout << '\n';
    // Get the payload
    SPIWriteRead(txbuf, rxbuf, len + 1, ec);
    if (ec)
    {
        return false;
    }
    for (uint8_t i : rxbuf)
    {
        std::cout << (int)i << '|';
    }
    std::cout << '\n';
    memcpy(buf, rxbuf + 1, len);
    for (uint8_t* i = (uint8_t*)buf; i < (uint8_t*)buf + len; i++)
    {
        std::cout << (int)*i << '|';
    }
    std::cout << '\n';
    // Clear data received bit
    if (Reg(Regs::STATUS).Set(RegVals::Status::RX_DR | RegVals::Status::TX_DS, ec))
    {
        return false;
    }
    std::cout << (int)Reg(Regs::FIFO_STATUS).Get(ec) << " ";
    if (ec)
    {
        return false;
    }
    std::cout << (int)GetStatus(ec) << " ";
    if (ec)
    {
        return false;
    }
    std::cout << (int)Reg(Regs::CONFIG).Get(ec) << '\n';
    if (ec)
    {
        return false;
    }

    bool result = Reg(Regs::FIFO_STATUS).Get(ec) & RegVals::FIFOStatus::RX_EMPTY;
    if (ec)
    {
        return false;
    }
    return result;
}

void RF24::NRF24L01::OpenReadingPipe(uint8_t pipe, uint64_t addr)
{
    detail::ErrorThrower([&](auto& ec) { this->OpenReadingPipe(pipe, addr, ec); });
}

std::error_code RF24::NRF24L01::OpenReadingPipe(uint8_t pipe, uint64_t addr, std::error_code& ec)
{
    ec = std::error_code{};
    if (pipe == 0)
    {
        if (Reg(Regs::RX_ADDR_P0).Set(addr, ec))
        {
            return ec;
        }
        pipe0Addr = addr;
    }
    else if (pipe == 1)
    {
        Reg(Regs::RX_ADDR_P1).Set(addr, ec);
    }
    else if (pipe == 2)
    {
        Reg(Regs::RX_ADDR_P2).Set(static_cast<uint8_t>(addr & 0xFF), ec);
    }
    else if (pipe == 3)
    {
        Reg(Regs::RX_ADDR_P3).Set(static_cast<uint8_t>(addr & 0xFF), ec);
    }
    else if (pipe == 4)
    {
        Reg(Regs::RX_ADDR_P4).Set(static_cast<uint8_t>(addr & 0xFF), ec);
    }
    else if (pipe == 5)
    {
        Reg(Regs::RX_ADDR_P5).Set(static_cast<uint8_t>(addr & 0xFF), ec);
    }
    if (ec)
    {
        return ec;
    }
    uint8_t enRXVal = Reg(Regs::EN_RXADDR).Get(ec) | (RegVals::EnRXAddr::ERX_P0 << pipe);
    if (ec)
    {
        return ec;
    }
    return Reg(Regs::EN_RXADDR).Set(enRXVal, ec);
}

void RF24::NRF24L01::CloseReadingPipe(uint8_t pipe)
{
    detail::ErrorThrower([&](auto& ec) { this->CloseReadingPipe(pipe, ec); });
}

std::error_code RF24::NRF24L01::CloseReadingPipe(uint8_t pipe, std::error_code& ec)
{
    uint8_t val = Reg(Regs::EN_RXADDR).Get(ec) & ~(RegVals::EnRXAddr::ERX_P0 << pipe);
    if (ec)
    {
        return ec;
    }
    return Reg(Regs::EN_RXADDR).Set(val, ec);
}

void RF24::NRF24L01::OpenWritingPipe(uint64_t addr)
{
    detail::ErrorThrower([&](auto& ec) { this->OpenWritingPipe(addr, ec); });
}

std::error_code RF24::NRF24L01::OpenWritingPipe(uint64_t addr, std::error_code& ec)
{
    bool pipe0Used = Reg(Regs::EN_RXADDR).Get(ec) & RegVals::EnRXAddr::ERX_P0;
    if (ec)
    {
        return ec;
    }
    if (pipe0Used)
    {
        // Save pipe 0 address
        pipe0Addr = Reg(Regs::RX_ADDR_P0).Get(ec);
        if (ec)
        {
            pipe0Addr = 0;
            return ec;
        }
    }
    else
    {
        pipe0Addr = 0;
    }

    if (Reg(Regs::TX_ADDR).Set(addr, ec))
    {
        return ec;
    }
    if (Reg(Regs::RX_ADDR_P0).Set(addr, ec))
    {
        return ec;
    }
    // I don't know if this is right
    uint8_t enRxAddr = Reg(Regs::EN_RXADDR).Get(ec);
    if (ec)
    {
        return ec;
    }
    return Reg(Regs::EN_RXADDR).Set(enRxAddr | RegVals::EnRXAddr::ERX_P0, ec);
}

void RF24::NRF24L01::StartListening()
{
    detail::ErrorThrower([&](auto& ec) { this->StartListening(ec); });
}

std::error_code RF24::NRF24L01::StartListening(std::error_code& ec)
{
    // Power up and set to primary rx mode
    uint8_t config = Reg(Regs::CONFIG).Get(ec);
    if (ec)
    {
        return ec;
    }
    if (Reg(Regs::CONFIG).Set(config | RegVals::Config::PWR_UP | RegVals::Config::PRIM_RX, ec))
    {
        return ec;
    }
    // Reset flags
    if (Reg(Regs::STATUS).Set(RegVals::Status::TX_DS | RegVals::Status::RX_DR | RegVals::Status::MAX_RT, ec))
    {
        return ec;
    }

    uint8_t enRxAddr = Reg(Regs::EN_RXADDR).Get(ec);
    if (ec)
    {
        return ec;
    }
    if (pipe0Addr != 0)
    {
        if (Reg(Regs::RX_ADDR_P0).Set(pipe0Addr, ec))
        {
            return ec;
        }
        if (Reg(Regs::EN_RXADDR).Set(enRxAddr | RegVals::EnRXAddr::ERX_P0, ec))
        {
            return ec;
        }
    }
    else
    {
        if (Reg(Regs::EN_RXADDR).Set(enRxAddr & ~RegVals::EnRXAddr::ERX_P0, ec))
        {
            return ec;
        }
    }
    if (ce.Set(true, ec))
    {
        return ec;
    }

    WaitUs(130);
    std::cout << "Start listening at: " << std::time(nullptr) << '\n';
    return ec;
}

void RF24::NRF24L01::StopListening()
{
    detail::ErrorThrower([&](auto& ec) { this->StopListening(ec); });
}

std::error_code RF24::NRF24L01::StopListening(std::error_code& ec)
{
    if (ce.Set(false, ec))
    {
        return ec;
    }
    WaitUs(130);
    std::cout << "Stop listening at: " << std::time(nullptr) << '\n';
    return ec;
}

void RF24::NRF24L01::PrintRegisters()
{
    std::cout << "The current register status:" << '\n';
    PrintRegister("CONFIG", Regs::CONFIG);
    PrintRegister("EN_AA", Regs::EN_AA);
    PrintRegister("EN_RXADDR", Regs::EN_RXADDR);
    PrintRegister("SETUP_AW", Regs::SETUP_AW);
    PrintRegister("SETUP_RETR", Regs::SETUP_RETR);
    PrintRegister("RF_CH", Regs::RF_CH);
    PrintRegister("RF_SETUP", Regs::RF_SETUP);
    PrintRegister("STATUS", Regs::STATUS);
    PrintRegister("OBSERVE_TX", Regs::OBSERVE_TX);
    PrintRegister("RPD", Regs::RPD);
    std::cout << '\n';
    PrintRegister("RX_ADDR_P0", Regs::RX_ADDR_P0);
    PrintRegister("RX_ADDR_P1", Regs::RX_ADDR_P1);
    PrintRegister("RX_ADDR_P2", Regs::RX_ADDR_P2);
    PrintRegister("RX_ADDR_P3", Regs::RX_ADDR_P3);
    PrintRegister("RX_ADDR_P4", Regs::RX_ADDR_P4);
    PrintRegister("RX_ADDR_P5", Regs::RX_ADDR_P5);
    PrintRegister("TX_ADDR", Regs::TX_ADDR);
    std::cout << '\n';
    PrintRegister("RX_PW_P0", Regs::RX_PW_P0);
    PrintRegister("RX_PW_P1", Regs::RX_PW_P1);
    PrintRegister("RX_PW_P2", Regs::RX_PW_P2);
    PrintRegister("RX_PW_P3", Regs::RX_PW_P3);
    PrintRegister("RX_PW_P4", Regs::RX_PW_P4);
    PrintRegister("RX_PW_P5", Regs::RX_PW_P5);
    std::cout << '\n';
    PrintRegister("FIFO_STATUS", Regs::FIFO_STATUS);
    PrintRegister("DYNPD", Regs::DYNPD);
    PrintRegister("FEATURE", Regs::FEATURE);
    std::cout << std::endl;
}

void RF24::NRF24L01::SPIWrite(const void* buf, uint8_t len)
{
    SPIWriteRead(buf, nullptr, len);
}

std::error_code RF24::NRF24L01::SPIWrite(const void* buf, uint8_t len, std::error_code& ec)
{
    return SPIWriteRead(buf, nullptr, len, ec);
}

void RF24::NRF24L01::SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len)
{
    detail::ErrorThrower([&](auto& ec) { this->SPIWriteRead(bufOut, bufIn, len, ec); });
}

std::error_code RF24::NRF24L01::SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec)
{
    if (useCsn)
    {
        if (csn.Set(false, ec))
        {
            return ec;
        }
    }
    ec = spi.transfer(reinterpret_cast<const char*>(bufOut), reinterpret_cast<char*>(bufIn), len);
    if (useCsn)
    {
        if (csn.Set(true, ec))
        {
            return ec;
        }
    }
    // Ensure 10us delay between spi commands
    WaitUs(10);
    return ec;
}

#else

RF24::NRF24L01::NRF24L01(SPI&& spi, int ce, int csn)
    : spi(std::move(spi)), ce(ce, GPIOPin::Mode::output), csn(csn, GPIOPin::Mode::output)
{}

uint8_t RF24::NRF24L01::GetStatus()
{
    return 0;
}
uint8_t RF24::NRF24L01::GetStatus(std::error_code& ec)
{
    return 0;
}
uint8_t RF24::NRF24L01::WriteRead(uint8_t val)
{
    return 0;
}
uint8_t RF24::NRF24L01::WriteRead(uint8_t val, std::error_code& ec)
{
    return 0;
}
void RF24::NRF24L01::Write(uint8_t val) {}
std::error_code RF24::NRF24L01::Write(uint8_t val, std::error_code& ec)
{
    return ec;
}
bool RF24::NRF24L01::Transmit(const void* buf, uint8_t len)
{
    return false;
}
bool RF24::NRF24L01::Transmit(const void* buf, uint8_t len, std::error_code& ec)
{
    return false;
}
bool RF24::NRF24L01::Available(uint8_t* pipe)
{
    return false;
}
bool RF24::NRF24L01::Available(uint8_t* pipe, std::error_code& ec)
{
    return false;
}
uint8_t RF24::NRF24L01::GetPayloadLength()
{
    return 0;
}
uint8_t RF24::NRF24L01::GetPayloadLength(std::error_code& ec)
{
    return 0;
}
bool RF24::NRF24L01::Read(void* buf, uint8_t len)
{
    return false;
}
bool RF24::NRF24L01::Read(void* buf, uint8_t len, std::error_code& ec)
{
    return false;
}
void RF24::NRF24L01::OpenReadingPipe(uint8_t pipe, uint64_t addr) {}
std::error_code RF24::NRF24L01::OpenReadingPipe(uint8_t pipe, uint64_t addr, std::error_code& ec)
{
    return ec;
}
void RF24::NRF24L01::CloseReadingPipe(uint8_t pipe) {}
std::error_code RF24::NRF24L01::CloseReadingPipe(uint8_t pipe, std::error_code& ec)
{
    return ec;
}
void RF24::NRF24L01::OpenWritingPipe(uint64_t addr) {}
std::error_code RF24::NRF24L01::OpenWritingPipe(uint64_t addr, std::error_code& ec)
{
    return ec;
}
void RF24::NRF24L01::StartListening() {}
std::error_code RF24::NRF24L01::StartListening(std::error_code& ec)
{
    return ec;
}
void RF24::NRF24L01::StopListening() {}
std::error_code RF24::NRF24L01::StopListening(std::error_code& ec)
{
    return ec;
}

void RF24::NRF24L01::PrintRegisters() {}
void RF24::NRF24L01::SPIWrite(const void* buf, uint8_t len) {}
std::error_code RF24::NRF24L01::SPIWrite(const void* buf, uint8_t len, std::error_code& ec)
{
    return ec;
}
void RF24::NRF24L01::SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len) {}
std::error_code RF24::NRF24L01::SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec)
{
    return ec;
}

#endif

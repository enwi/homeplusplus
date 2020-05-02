#include "spi.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

#include <string.h>

#if !defined(_MSC_VER) && !defined(__APPLE__)
#include <fcntl.h> // open and definitions
#include <sys/ioctl.h>

SPI::SPI(std::unique_ptr<FileControl> filecontrol, int dev, uint8_t mode, uint8_t bits, uint32_t speed)
    : fc(std::move(filecontrol))
{
    char device[] = "/dev/spidev0.0";
    if (dev)
    {
        device[11] += (dev / 10) % 10;
        device[13] += dev % 10;
    }

    std::cout << "[SPI] Using SPI device " << device << std::endl;

    spiDevFD = fc->open(device, O_RDWR);
    if (spiDevFD < 0)
    {
        std::error_code ec{errno, std::generic_category()};
        std::cout << "[SPI] Could not open SPI device " << device << std::endl;
        throw std::system_error(ec, "spi open: open device");
    }

    // set spi mode
    int ioctlState = fc->ioctl(spiDevFD, SPI_IOC_WR_MODE, &mode);
    if (ioctlState < 0)
    {
        std::error_code ec{errno, std::generic_category()};
        std::cout << "[SPI] Could not set SPIMode (WR) ... ioctl fail\n";
        throw std::system_error(ec, "spi open: set mode");
    }

    // set bits per word
    ioctlState = fc->ioctl(spiDevFD, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ioctlState < 0)
    {
        std::error_code ec{errno, std::generic_category()};
        std::cout << "[SPI] Could not set SPI bitsPerWord (WR) ... ioctl fail\n";
        throw std::system_error(ec, "spi open: set bitsPerWord");
    }

    // set lsb first
    int lsbFirst = 0;
    ioctlState = fc->ioctl(spiDevFD, SPI_IOC_WR_LSB_FIRST, &lsbFirst);
    if (ioctlState < 0)
    {
        std::error_code ec{errno, std::generic_category()};
        std::cout << "[SPI] Could not set SPI lsb first (WR) ... ioctl fail\n";
        throw std::system_error(ec, "spi open: set LSB first");
    }

    // set max speed
    ioctlState = fc->ioctl(spiDevFD, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ioctlState < 0)
    {
        std::error_code ec{errno, std::generic_category()};
        std::cout << "[SPI] Could not set SPI speed (WR) ... ioctl fail\n";
        throw std::system_error(ec, "spi open: set speed");
    }

    memset(&tr, 0, sizeof(tr));
    tr.speed_hz = speed;
    tr.bits_per_word = bits;
    tr.delay_usecs = 0;
    tr.cs_change = 0;
}

SPI::SPI(SPI&& other) noexcept : fc(std::move(other.fc)), spiDevFD(other.spiDevFD), tr(other.tr)
{
    // Set to -1 so it does not close file which is now used by this
    other.spiDevFD = -1;
}

SPI& SPI::operator=(SPI&& other) noexcept
{
    // Swap, if this file is open, it will be closed by other's destructor
    std::swap(spiDevFD, other.spiDevFD);
    std::swap(tr, other.tr);
    std::swap(fc, other.fc);
    return *this;
}

//! Destructor that deinitializes the spidev with the file descriptor \ref spiDevFD
SPI::~SPI()
{
    // Negative file is error or empty
    if (spiDevFD >= 0)
    {
        int closeState = fc->close(spiDevFD);
        if (closeState < 0)
        {
            std::error_code ec{errno, std::generic_category()};
            std::cout << "[SPI] Could not close SPI device: " << ec.message() << '\n';
        }
        else
        {
            std::cout << "[SPI] Successfully closed SPI device\n";
        }
    }
}

std::error_code SPI::transfer(const char* tx, char* rx, uint32_t len)
{
    // Check that spi is open
    if (spiDevFD < 1)
    {
        return std::make_error_code(std::errc::bad_file_descriptor);
    }
    // one spi transfer for each byte
    tr.tx_buf = (unsigned long)tx; // transmit from "data"
    tr.rx_buf = (unsigned long)rx; // receive into "data"
    tr.len = len;

    int retVal = fc->ioctl(spiDevFD, SPI_IOC_MESSAGE(1), &tr);
    if (retVal < 1)
    {
        std::error_code ec{errno, std::generic_category()};
        std::cout << "[SPI] Problem transmitting spi data..ioctl: " << strerror(ec.value()) << '\n';
        return ec;
    }
    return std::error_code{};
}

#else
SPI::SPI(std::unique_ptr<FileControl>, int, uint8_t, uint8_t, uint32_t) {}

// Move constructor
SPI::SPI(SPI&&) noexcept {}
// Move assignment
SPI& SPI::operator=(SPI&& other) noexcept
{
    return *this;
}

SPI::~SPI() {}

std::error_code transfer(const char* tx, char* rx, uint32_t len)
{
    return std::make_error_code(std::errc::not_supported);
}

#endif

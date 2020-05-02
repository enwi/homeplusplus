#ifndef SPI_H
#define SPI_H

#include <memory>
#include <system_error>

#include <stdint.h>

#include "../utility/FileControl.h"

#if !defined(_MSC_VER) && !defined(__APPLE__)
#include <linux/spi/spidev.h> // Needed for spi_ioc_transfer struct
#endif

class SPI
{
public:
    //! Constructor that initializes the spidev and all variables used by \ref transfer
    //! \param dev Specifies the spidev interface and chip select to be used.
    //! Standard value is 0. Also 10 is spidev1.0 or 25 spidev2.5
    //! \param mode Specifies the spi mode. Standard is 0.
    //! Possible modes are
    //! SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data
    //! (change) on falling edge SPI_MODE_1 (0,1) 	CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on
    //! falling edge, output data (change) on rising edge SPI_MODE_2 (1,0) 	CPOL = 1, CPHA = 0, Clock idle high,
    //! data is clocked in on falling edge, output data (change) on rising edge SPI_MODE_3 (1,1) 	CPOL = 1, CPHA
    //! = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge \param bits
    //! Specifies the bits per word. Standard is 8. \param speed Specifies the spi speed in Hz. Standard is
    //! 8MHz(8000000).
    explicit SPI(std::unique_ptr<FileControl> filecontrol, int dev = 0, uint8_t mode = 0, uint8_t bits = 8,
        uint32_t speed = 8000000);

    // Copy constructor deleted
    SPI(const SPI& other) = delete;
    // Copy assignment deleted
    SPI& operator=(const SPI& other) = delete;
    // Move constructor
    SPI(SPI&& other) noexcept;
    // Move assignment
    SPI& operator=(SPI&& other) noexcept;

    //! Destructor that deinitializes the spidev with the file descriptor \ref spiDevFD
    ~SPI();

    //! Function to transfer data via spi
    //! \param tx Buffer to be transmitted. If nullptr, do not transmit
    //! \param rx Buffer to store received data. If nullptr, do not receive
    //! \param len Size of tx
    //! \return error_code with the state of the transaction.
    std::error_code transfer(const char* tx, char* rx, uint32_t len);

private:
    std::unique_ptr<FileControl> fc;
    int spiDevFD = -1; //!< Variable to hold the file descriptor of the spidev. Set by constructor
#if !defined(_MSC_VER) && !defined(__APPLE__)
    struct spi_ioc_transfer tr; //!< Struct for spi transfers
#endif
};

#endif

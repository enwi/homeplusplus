#include "communication/spi.h"

#if !defined(_MSC_VER) && !defined(__APPLE__)

#include <memory>
#include <string>
#include <system_error> // errno
#include <vector>

#include <fcntl.h> // open and definitions
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/ioctl.h> // ioctl commands

#include "../SilenceCout.h"
#include "../mocks/MockFileControl.h"

TEST(spi, Constructor)
{
    using namespace ::testing;
    SilenceCout silence;

    std::unique_ptr<MockFileControl> mockfc1 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc1, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(-1));
    EXPECT_THROW(SPI(std::move(mockfc1), 11, 0, 8, 8000000), std::system_error);

    std::unique_ptr<MockFileControl> mockfc2 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc2, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc2, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(-1));
    EXPECT_THROW(SPI(std::move(mockfc2), 11, 0, 8, 8000000), std::system_error);

    std::unique_ptr<MockFileControl> mockfc3 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc3, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc3, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc3, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(-1));
    EXPECT_THROW(SPI(std::move(mockfc3), 11, 0, 8, 8000000), std::system_error);

    std::unique_ptr<MockFileControl> mockfc4 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc4, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc4, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc4, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc4, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(-1));
    EXPECT_THROW(SPI(std::move(mockfc4), 11, 0, 8, 8000000), std::system_error);

    std::unique_ptr<MockFileControl> mockfc5 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc5, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc5, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc5, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc5, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc5, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(-1));
    EXPECT_THROW(SPI(std::move(mockfc5), 11, 0, 8, 8000000), std::system_error);

    std::unique_ptr<MockFileControl> mockfc6 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc6, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc6, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc6, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc6, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc6, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc6, close(1)).Times(1).WillOnce(Return(1));
    EXPECT_NO_THROW(SPI(std::move(mockfc6), 11, 0, 8, 8000000));
}

TEST(spi, MoveConstructor)
{
    using namespace ::testing;
    SilenceCout silence;

    std::unique_ptr<MockFileControl> mockfc1 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc1, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, close(1)).Times(1).WillOnce(Return(1));
    SPI spi1(std::move(mockfc1), 11, 0, 8, 8000000);

    EXPECT_NO_THROW(SPI(std::move(spi1)));
}

TEST(spi, OperatorEquals)
{
    using namespace ::testing;
    SilenceCout silence;

    std::unique_ptr<MockFileControl> mockfc1 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc1, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, close(1)).Times(1).WillOnce(Return(1));
    SPI spi1(std::move(mockfc1), 11, 0, 8, 8000000);

    std::unique_ptr<MockFileControl> mockfc2 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc2, open(StrEq("/dev/spidev1.2"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc2, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc2, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc2, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc2, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc2, close(1)).Times(1).WillOnce(Return(1));
    SPI spi2(std::move(mockfc2), 12, 0, 8, 8000000);

    spi2 = std::move(spi1);
}

TEST(spi, Destructor)
{
    using namespace ::testing;
    SilenceCout silence;

    std::unique_ptr<MockFileControl> mockfc1 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc1, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, close(1)).Times(1).WillOnce(Return(-1));
    SPI(std::move(mockfc1), 11, 0, 8, 8000000);
}

TEST(spi, Transfer)
{
    using namespace ::testing;
    SilenceCout silence;

    const char* cc1;
    char* c1;
    std::unique_ptr<MockFileControl> mockfc1 = std::make_unique<MockFileControl>();
    EXPECT_CALL(*mockfc1, open(StrEq("/dev/spidev1.1"), 2)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MODE, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_BITS_PER_WORD, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_LSB_FIRST, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_WR_MAX_SPEED_HZ, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, ioctl(1, SPI_IOC_MESSAGE(1), Truly([&](const void* p) {
        const spi_ioc_transfer* test_tr = static_cast<const spi_ioc_transfer*>(p);
        return test_tr->tx_buf == (unsigned long)cc1 ? (test_tr->rx_buf == (unsigned long)c1 ? true : false) : false;
    })))
        .Times(2)
        .WillOnce(Return(-1))
        .WillOnce(Return(1));
    EXPECT_CALL(*mockfc1, close(1)).Times(1).WillOnce(Return(1));

    SPI spi1(std::move(mockfc1), 11, 0, 8, 8000000);
    SPI spi2 = std::move(spi1);

    EXPECT_EQ(std::make_error_code(std::errc::bad_file_descriptor), spi1.transfer(cc1, c1, 1));

    errno = 9;
    EXPECT_EQ(std::error_code(errno, std::generic_category()), spi2.transfer(cc1, c1, 1));
    EXPECT_EQ(std::error_code(), spi2.transfer(cc1, c1, 1));
}
#endif

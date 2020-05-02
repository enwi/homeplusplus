/*
 * nRF24L01.h
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <system_error>
#include <type_traits>

#include "spi.h"

#include "../config/Config.h"

// Just left over because maybe its needed
namespace UselessConstantsDontUse
{
    /* Bit Mnemonics */
    /* CONFIG Register Bits */
    constexpr uint8_t RF24_MASK_RX_DR
        = (1 << 6); /* Mask interrupt caused by RX_DR: 1: interrupt masked. 0: interrupt enabled */
    constexpr uint8_t RF24_MASK_TX_DS
        = (1 << 5); /* Mask interrupt caused by TX_DS: 1: interrupt masked. 0: interrupt enabled */
    constexpr uint8_t RF24_MASK_MAX_RT = (1 << 4); /* Mask interrupt caused by MAX_RT. 1: interrupt not reflected on IRQ
                                                      pin. 0: reflect MAX_RT as active low interrupt on IRQ pin */
    constexpr uint8_t RF24_EN_CRC = (1 << 3); /* Enable CRC. Forced high if on of the bits in EN_AA is high */
    constexpr uint8_t RF24_CRCO = (1 << 2); /* CRC encoding scheme, 0: 1 byte, 1: 2 bytes */
    constexpr uint8_t RF24_PWR_UP = (1 << 1); /* 1: Power up, 0: Power down */
    constexpr uint8_t RF24_PRIM_RX = (1 << 0); /* 1: PRX, 0: PTX */
    constexpr uint8_t RF24_PRIM_TX = (0); /* 0: PTX */

    constexpr uint8_t RF24_ENAA_P5 = 5;
    constexpr uint8_t RF24_ENAA_P4 = 4;
    constexpr uint8_t RF24_ENAA_P3 = 3;
    constexpr uint8_t RF24_ENAA_P2 = 2;
    constexpr uint8_t RF24_ENAA_P1 = 1;
    constexpr uint8_t RF24_ENAA_P0 = 0;
    constexpr uint8_t RF24_ERX_P5 = 5;
    constexpr uint8_t RF24_ERX_P4 = 4;
    constexpr uint8_t RF24_ERX_P3 = 3;
    constexpr uint8_t RF24_ERX_P2 = 2;
    constexpr uint8_t RF24_ERX_P1 = 1;
    constexpr uint8_t RF24_ERX_P0 = 0;
    constexpr uint8_t RF24_AW = 0;
    constexpr uint8_t RF24_ARD = 4;
    constexpr uint8_t RF24_ARC = 0;
    constexpr uint8_t RF24_PLL_LOCK = 4;
    constexpr uint8_t RF24_RF_DR_HIGH = 3;
    constexpr uint8_t RF24_RF_DR_LOW = 5;
    constexpr uint8_t RF24_RF_PWR = 1;
    constexpr uint8_t RF24_LNA_HCURR = 0;
    constexpr uint8_t RF24_RX_DR = 6;
    constexpr uint8_t RF24_TX_DS = 5;
    constexpr uint8_t RF24_MAX_RT = 4;
    constexpr uint8_t RF24_RX_P_NO = 1;
    constexpr uint8_t RF24_TX_FULL = 0;
    constexpr uint8_t RF24_PLOS_CNT = 4;
    constexpr uint8_t RF24_ARC_CNT = 0;
    constexpr uint8_t RF24_TX_REUSE = 6;
    constexpr uint8_t RF24_FIFO_FULL = 5;
    constexpr uint8_t RF24_TX_EMPTY = 4;
    constexpr uint8_t RF24_RX_FULL = 1;
    constexpr uint8_t RF24_RX_EMPTY = 0;
    constexpr uint8_t RF24_DPL_P5 = 5;
    constexpr uint8_t RF24_DPL_P4 = 4;
    constexpr uint8_t RF24_DPL_P3 = 3;
    constexpr uint8_t RF24_DPL_P2 = 2;
    constexpr uint8_t RF24_DPL_P1 = 1;
    constexpr uint8_t RF24_DPL_P0 = 0;
    constexpr uint8_t RF24_EN_DPL = 2;
    constexpr uint8_t RF24_EN_ACK_PAY = 1;
    constexpr uint8_t RF24_EN_DYN_ACK = 0;
} // namespace UselessConstantsDontUse

class GPIOPin
{
public:
    enum class Mode
    {
        input,
        output
    };

public:
    GPIOPin(int num, Mode mode);
    GPIOPin(GPIOPin&& other) noexcept;
    GPIOPin& operator=(GPIOPin&& other) noexcept;
    void operator=(bool val);
    operator bool();
    std::error_code Set(bool val, std::error_code& ec);
    bool Get(std::error_code& ec);
    ~GPIOPin();
    void SetMode(Mode mode);

private:
    int num;
};

namespace RF24
{
    namespace Regs
    {
        template <std::size_t N>
        struct RegAddr
        {
            uint8_t addr;
        };
        /* Memory Map - register address defines */
        constexpr RegAddr<1> CONFIG{0x00}; /* CONFIG register */
        constexpr RegAddr<1> EN_AA{0x01}; /* EN_AA register */
        constexpr RegAddr<1> EN_RXADDR{0x02}; /* EN_RXADDR register */
        constexpr RegAddr<1> SETUP_AW{0x03}; /* SETUP_AW register */
        constexpr RegAddr<1> SETUP_RETR{0x04};
        constexpr RegAddr<1> RF_CH{0x05};
        constexpr RegAddr<1> RF_SETUP{0x06}; /* SETUP register */
        constexpr RegAddr<1> STATUS{0x07};
        constexpr RegAddr<1> OBSERVE_TX{0x08};
        constexpr RegAddr<1> RPD{0x09}; /* Mnemonic for nRF24L01+ */
        // constexpr RegAddr<1> CD {0x09};   /* Mnemonic from nRF24L01, new is RPD */
        constexpr RegAddr<5> RX_ADDR_P0{0x0A};
        constexpr RegAddr<5> RX_ADDR_P1{0x0B};
        constexpr RegAddr<1> RX_ADDR_P2{0x0C};
        constexpr RegAddr<1> RX_ADDR_P3{0x0D};
        constexpr RegAddr<1> RX_ADDR_P4{0x0E};
        constexpr RegAddr<1> RX_ADDR_P5{0x0F};
        constexpr RegAddr<5> TX_ADDR{0x10};
        constexpr RegAddr<1> RX_PW_P0{0x11};
        constexpr RegAddr<1> RX_PW_P1{0x12};
        constexpr RegAddr<1> RX_PW_P2{0x13};
        constexpr RegAddr<1> RX_PW_P3{0x14};
        constexpr RegAddr<1> RX_PW_P4{0x15};
        constexpr RegAddr<1> RX_PW_P5{0x16};
        constexpr RegAddr<1> FIFO_STATUS{0x17};
        constexpr RegAddr<1> DYNPD{0x1C};
        constexpr RegAddr<1> FEATURE{0x1D};

    } // namespace Regs

    namespace RegVals
    {
        namespace DefaultValues
        {
            constexpr uint8_t CONFIG = 0x08;
            constexpr uint8_t EN_AA = 0x3F;
            constexpr uint8_t EN_RXADDR = 0x03;
            constexpr uint8_t SETUP_AW = 0x03;
            constexpr uint8_t SETUP_RETR = 0x03;
            constexpr uint8_t RF_CH = 0x02;
            constexpr uint8_t RF_SETUP = 0x0F;
            constexpr uint8_t STATUS = 0x0E;
            constexpr uint8_t OBSERVE_TX = 0x00;
            constexpr uint8_t RPD = 0x00;
            constexpr uint64_t RX_ADDR_P0 = 0xE7E7E7E7E7;
            constexpr uint64_t RX_ADDR_P1 = 0xC2C2C2C2C2;
            constexpr uint8_t RX_ADDR_P2 = 0xC3;
            constexpr uint8_t RX_ADDR_P3 = 0xC4;
            constexpr uint8_t RX_ADDR_P4 = 0xC5;
            constexpr uint8_t RX_ADDR_P5 = 0xC6;
            constexpr uint64_t TX_ADDR = 0xE7E7E7E7E7;
            constexpr uint8_t RX_PW_P0 = 0x00;
            constexpr uint8_t RX_PW_P1 = 0x00;
            constexpr uint8_t RX_PW_P2 = 0x00;
            constexpr uint8_t RX_PW_P3 = 0x00;
            constexpr uint8_t RX_PW_P4 = 0x00;
            constexpr uint8_t RX_PW_P5 = 0x00;
            constexpr uint8_t FIFO_STATUS = 0x11;
            constexpr uint8_t DYNPD = 0x00;
            constexpr uint8_t FEATURE = 0x00;
        } // namespace DefaultValues
        namespace Config
        {
            /* CONFIG register bitwise definitions */
            constexpr uint8_t RESERVED = 0x80;
            constexpr uint8_t MASK_RX_DR = 0x40;
            constexpr uint8_t MASK_TX_DS = 0x20;
            constexpr uint8_t MASK_MAX_RT = 0x10;
            constexpr uint8_t EN_CRC = 0x08;
            constexpr uint8_t CRCO = 0x04;
            constexpr uint8_t PWR_UP = 0x02;
            constexpr uint8_t PRIM_RX = 0x01;
        } // namespace Config
        namespace EnAA
        {
            /* EN_AA register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
            constexpr uint8_t ENAA_ALL = 0x3F;
            constexpr uint8_t ENAA_P5 = 0x20;
            constexpr uint8_t ENAA_P4 = 0x10;
            constexpr uint8_t ENAA_P3 = 0x08;
            constexpr uint8_t ENAA_P2 = 0x04;
            constexpr uint8_t ENAA_P1 = 0x02;
            constexpr uint8_t ENAA_P0 = 0x01;
            constexpr uint8_t ENAA_NONE = 0x00;
        } // namespace EnAA
        namespace EnRXAddr
        {
            /* EN_RXADDR register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
            constexpr uint8_t ERX_ALL = 0x3F;
            constexpr uint8_t ERX_P5 = 0x20;
            constexpr uint8_t ERX_P4 = 0x10;
            constexpr uint8_t ERX_P3 = 0x08;
            constexpr uint8_t ERX_P2 = 0x04;
            constexpr uint8_t ERX_P1 = 0x02;
            constexpr uint8_t ERX_P0 = 0x01;
            constexpr uint8_t ERX_NONE = 0x00;
        } // namespace EnRXAddr
        namespace SetupAW
        {
            /* SETUP_AW register bitwise definitions */
            constexpr uint8_t RESERVED = 0xFC;
            constexpr uint8_t _5BYTES = 0x03;
            constexpr uint8_t _4BYTES = 0x02;
            constexpr uint8_t _3BYTES = 0x01;
            constexpr uint8_t ILLEGAL = 0x00;
        } // namespace SetupAW
        namespace SetupRetr
        {
            /* SETUP_RETR register bitwise definitions */
            constexpr uint8_t ARD = 0xF0;
            constexpr uint8_t ARD_4000 = 0xF0; /* 4400 us retry delay */
            constexpr uint8_t ARD_3750 = 0xE0; /* 3750 us retry delay */
            constexpr uint8_t ARD_3500 = 0xD0; /* 3500 us retry delay */
            constexpr uint8_t ARD_3250 = 0xC0; /* 3250 us retry delay */
            constexpr uint8_t ARD_3000 = 0xB0; /* 3000 us retry delay */
            constexpr uint8_t ARD_2750 = 0xA0; /* 2750 us retry delay */
            constexpr uint8_t ARD_2500 = 0x90; /* 2500 us retry delay */
            constexpr uint8_t ARD_2250 = 0x80; /* 2250 us retry delay */
            constexpr uint8_t ARD_2000 = 0x70; /* 2000 us retry delay */
            constexpr uint8_t ARD_1750 = 0x60; /* 1750 us retry delay */
            constexpr uint8_t ARD_1500 = 0x50; /* 1500 us retry delay */
            constexpr uint8_t ARD_1250 = 0x40; /* 1250 us retry delay */
            constexpr uint8_t ARD_1000 = 0x30; /* 1000 us retry delay */
            constexpr uint8_t ARD_750 = 0x20; /* 750 us retry delay */
            constexpr uint8_t ARD_500 = 0x10; /* 500 us retry delay */
            constexpr uint8_t ARD_250 = 0x00; /* 250 us retry delay */
            constexpr uint8_t ARC = 0x0F;
            constexpr uint8_t ARC_15 = 0x0F; /* 15 retry count */
            constexpr uint8_t ARC_14 = 0x0E; /* 14 retry count */
            constexpr uint8_t ARC_13 = 0x0D; /* 13 retry count */
            constexpr uint8_t ARC_12 = 0x0C; /* 12 retry count */
            constexpr uint8_t ARC_11 = 0x0B; /* 11 retry count */
            constexpr uint8_t ARC_10 = 0x0A; /* 10 retry count */
            constexpr uint8_t ARC_9 = 0x09; /* 9 retry count */
            constexpr uint8_t ARC_8 = 0x08; /* 8 retry count */
            constexpr uint8_t ARC_7 = 0x07; /* 7 retry count */
            constexpr uint8_t ARC_6 = 0x06; /* 6 retry count */
            constexpr uint8_t ARC_5 = 0x05; /* 5 retry count */
            constexpr uint8_t ARC_4 = 0x04; /* 4 retry count */
            constexpr uint8_t ARC_3 = 0x03; /* 3 retry count */
            constexpr uint8_t ARC_2 = 0x02; /* 2 retry count */
            constexpr uint8_t ARC_1 = 0x01; /* 1 retry count */
            constexpr uint8_t ARC_0 = 0x00; /* 0 retry count, retry disabled */
        } // namespace SetupRetr
        namespace RFCh
        {
            /* RF_CH register bitwise definitions */
            constexpr uint8_t RESERVED = 0x80;
        } // namespace RFCh
        namespace RFSetup
        {
            /* RF_SETUP register bitwise definitions */
            constexpr uint8_t RESERVED = 0xE0;
            constexpr uint8_t PLL_LOCK = 0x10;
            constexpr uint8_t RF_DR = 0x08;
            constexpr uint8_t RF_DR_250 = 0x20;
            constexpr uint8_t RF_DR_1000 = 0x00;
            constexpr uint8_t RF_DR_2000 = 0x08;
            constexpr uint8_t RF_PWR = 0x06;
            constexpr uint8_t RF_PWR_0 = 0x06;
            constexpr uint8_t RF_PWR_6 = 0x04;
            constexpr uint8_t RF_PWR_12 = 0x02;
            constexpr uint8_t RF_PWR_18 = 0x00;
            constexpr uint8_t LNA_HCURR = 0x01;
        } // namespace RFSetup
        namespace Status
        {
            /* STATUS register bit definitions */
            constexpr uint8_t RESERVED = 0x80; /* bit 1xxx xxxx: This bit is reserved */
            constexpr uint8_t RX_DR
                = 0x40; /* bit x1xx xxxx: Data ready RX FIFO interrupt. Asserted when new data arrives RX FIFO */
            constexpr uint8_t TX_DS
                = 0x20; /* bit xx1x xxxx: Data sent TX FIFO interrupt. Asserted when packet transmitted on TX. */
            constexpr uint8_t MAX_RT = 0x10; /* bit xxx1 xxxx: maximum number of TX retransmit interrupts */
            constexpr uint8_t RX_P_NO = 0x0E;
            constexpr uint8_t RX_P_NO_RX_FIFO_NOT_EMPTY = 0x0E;
            constexpr uint8_t RX_P_NO_UNUSED = 0x0C;
            constexpr uint8_t RX_P_NO_5 = 0x0A;
            constexpr uint8_t RX_P_NO_4 = 0x08;
            constexpr uint8_t RX_P_NO_3 = 0x06;
            constexpr uint8_t RX_P_NO_2 = 0x04;
            constexpr uint8_t RX_P_NO_1 = 0x02;
            constexpr uint8_t RX_P_NO_0 = 0x00; /* bit xxxx 111x: pipe number for payload */
            constexpr uint8_t TX_FULL = 0x01; /* bit xxxx xxx1: if bit set, then TX FIFO is full */
        } // namespace Status
        namespace ObserveTx
        {
            /* OBSERVE_TX register bitwise definitions */
            constexpr uint8_t PLOS_CNT = 0xF0;
            constexpr uint8_t ARC_CNT = 0x0F;
        } // namespace ObserveTx
        namespace Rpd
        {
            /* RPD register bitwise definitions for nRF24L01+ */
            constexpr uint8_t RESERVED = 0xFE;
            constexpr uint8_t RPD = 0x01;
        } // namespace Rpd
        namespace RXPwP0
        {
            /* RX_PW_P0 register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
        } // namespace RXPwP0
        namespace RXPwP1
        {
            /* RX_PW_P1 register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
        } // namespace RXPwP1
        namespace RXPwP2
        {
            /* RX_PW_P2 register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
        } // namespace RXPwP2
        namespace RXPwP3
        {
            /* RX_PW_P3 register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
        } // namespace RXPwP3
        namespace RXPwP4
        {
            /* RX_PW_P4 register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
        } // namespace RXPwP4
        namespace RXPwP5
        {
            /* RX_PW_P5 register bitwise definitions */
            constexpr uint8_t RESERVED = 0xC0;
        } // namespace RXPwP5
        namespace FIFOStatus
        {
            /* FIFO_STATUS register bitwise definitions */
            constexpr uint8_t RESERVED = 0x8C;
            constexpr uint8_t TX_REUSE = 0x40;
            constexpr uint8_t TX_FULL = 0x20;
            constexpr uint8_t TX_EMPTY = 0x10;
            constexpr uint8_t RX_FULL = 0x02;
            constexpr uint8_t RX_EMPTY = 0x01;
        } // namespace FIFOStatus
        namespace DynPD
        {
            /* DYNPD register bitwise definitions*/
            constexpr uint8_t RESERVED = 0xC0;
            constexpr uint8_t DPL_ALL = 0x3F;
            constexpr uint8_t DPL_P5 = 0x20;
            constexpr uint8_t DPL_P4 = 0x10;
            constexpr uint8_t DPL_P3 = 0x08;
            constexpr uint8_t DPL_P2 = 0x04;
            constexpr uint8_t DPL_P1 = 0x02;
            constexpr uint8_t DPL_P0 = 0x01;
        } // namespace DynPD
        namespace Feature
        {
            /* FEATURE register bitwise definitions */
            constexpr uint8_t RESERVED = 0xF8;
            constexpr uint8_t EN_DPL = 0x04;
            constexpr uint8_t EN_ACK_PAY = 0x02;
            constexpr uint8_t EN_DYN_ACK = 0x01;
        } // namespace Feature
    } // namespace RegVals

    namespace Commands
    {
        /* Command Name Mnemonics (Instructions) */
        constexpr uint8_t R_REGISTER = 0x00;
        constexpr uint8_t W_REGISTER = 0x20;
        constexpr uint8_t REGISTER_MASK = 0x1F;
        constexpr uint8_t ACTIVATE = 0x50;
        constexpr uint8_t R_RX_PL_WID = 0x60;
        constexpr uint8_t R_RX_PAYLOAD = 0x61;
        constexpr uint8_t W_TX_PAYLOAD = 0xA0;
        constexpr uint8_t FLUSH_TX = 0xE1;
        constexpr uint8_t FLUSH_RX = 0xE2;
        constexpr uint8_t REUSE_TX_PL = 0xE3;
        constexpr uint8_t NOP = 0xFF;
    } // namespace Commands

    namespace detail
    {
        // If not specialized, try next higher class
        template <std::size_t Nbytes>
        struct RegTraits : RegTraits<Nbytes + 1>
        {};
        template <>
        struct RegTraits<0>
        {};
        template <>
        struct RegTraits<1>
        {
            using type = uint8_t;
            using type_or_cref = type;
        };
        template <>
        struct RegTraits<2>
        {
            using type = uint16_t;
            using type_or_cref = type;
        };
        template <>
        struct RegTraits<4>
        {
            using type = uint32_t;
            using type_or_cref = type;
        };
        template <>
        struct RegTraits<8>
        {
            using type = uint64_t;
            using type_or_cref = const uint64_t&;
        };

        template <typename F, std::enable_if_t<!std::is_void<std::result_of_t<F(std::error_code&)>>::value>* = nullptr>
        inline auto ErrorThrower(F&& f)
        {
            std::error_code ec;
            auto&& e = f(ec);
            if (ec)
            {
                throw std::system_error(ec);
            }
            return e;
        }
        template <typename F, std::enable_if_t<std::is_void<std::result_of_t<F(std::error_code&)>>::value>* = nullptr>
        inline void ErrorThrower(F&& f)
        {
            std::error_code ec;
            f(ec);
            if (ec)
            {
                throw std::system_error(ec);
            }
        }
    } // namespace detail

    class IRadio
    {
    public:
        template <std::size_t Nbytes>
        class Register
        {
        public:
            using type = typename detail::RegTraits<Nbytes>::type;
            using type_or_cref = typename detail::RegTraits<Nbytes>::type_or_cref;

            std::error_code Set(type_or_cref val, std::error_code& ec)
            {
                // 1st bit of data is write register command, 2nd is written data
                uint8_t tx[Nbytes + 1];
                tx[0] = Commands::W_REGISTER | addr;
                memcpy(tx + 1, reinterpret_cast<const uint8_t*>(&val), Nbytes);
                parent->SPIWrite(tx, Nbytes + 1, ec);
                return ec;
            }
            type Get(std::error_code& ec)
            {
                // Fill TX buffer with read register command + NOP to read the result
                uint8_t tx[Nbytes + 1];
                tx[0] = Commands::R_REGISTER | addr;
                memset(tx + 1, Commands::NOP, Nbytes);
                uint8_t rx[Nbytes + 1] = {};
                if (parent->SPIWriteRead(tx, rx, Nbytes + 1, ec))
                {
                    return type{};
                }
                // Result is 2nd byte of receive buffer, first byte is status register
                type result{};
                memcpy(&result, rx + 1, Nbytes);
                return result;
            }
            // convenience functions
            void operator=(type_or_cref val)
            {
                detail::ErrorThrower([&](auto& ec) { this->Set(val, ec); });
            }
            void operator&=(type_or_cref val) { *this = static_cast<type>(*this) & val; }
            void operator|=(type_or_cref val) { *this = static_cast<type>(*this) | val; }
            operator type()
            {
                return detail::ErrorThrower([&](auto& ec) { return this->Get(ec); });
            }

        private:
            // Only parent can construct
            friend class IRadio;
            Register(IRadio& parent, uint8_t addr) : parent(&parent), addr(addr) {}

        private:
            IRadio* parent;
            uint8_t addr;
        };

    public:
        virtual ~IRadio() = default;

        virtual uint8_t GetStatus() = 0;
        virtual uint8_t GetStatus(std::error_code& ec) = 0;
        virtual uint8_t WriteRead(uint8_t val) = 0;
        virtual uint8_t WriteRead(uint8_t val, std::error_code& ec) = 0;
        virtual void Write(uint8_t val) = 0;
        virtual std::error_code Write(uint8_t val, std::error_code& ec) = 0;
        virtual bool Transmit(const void* buf, uint8_t len) = 0;
        virtual bool Transmit(const void* buf, uint8_t len, std::error_code& ec) = 0;
        virtual bool Available(uint8_t* pipe) = 0;
        virtual bool Available(uint8_t* pipe, std::error_code& ec) = 0;
        virtual uint8_t GetPayloadLength() = 0;
        virtual uint8_t GetPayloadLength(std::error_code& ec) = 0;
        virtual bool Read(void* buf, uint8_t len) = 0;
        virtual bool Read(void* buf, uint8_t len, std::error_code& ec) = 0;
        template <std::size_t N>
        Register<N> Reg(Regs::RegAddr<N> addr)
        {
            return Register<N>(*this, addr.addr);
        }
        virtual void OpenReadingPipe(uint8_t pipe, uint64_t addr) = 0;
        virtual std::error_code OpenReadingPipe(uint8_t pipe, uint64_t addr, std::error_code& ec) = 0;
        virtual void CloseReadingPipe(uint8_t pipe) = 0;
        virtual std::error_code CloseReadingPipe(uint8_t pipe, std::error_code& ec) = 0;
        virtual void OpenWritingPipe(uint64_t addr) = 0;
        virtual std::error_code OpenWritingPipe(uint64_t addr, std::error_code& ec) = 0;
        virtual void StartListening() = 0;
        virtual std::error_code StartListening(std::error_code& ec) = 0;
        virtual void StopListening() = 0;
        virtual std::error_code StopListening(std::error_code& ec) = 0;

        virtual void PrintRegisters() = 0;
        virtual void SPIWrite(const void* buf, uint8_t len) = 0;
        virtual std::error_code SPIWrite(const void* buf, uint8_t len, std::error_code& ec) = 0;
        virtual void SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len) = 0;
        virtual std::error_code SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec) = 0;
    };

    class NullRadio : public IRadio
    {
    public:
        uint8_t GetStatus() override { return RegVals::DefaultValues::STATUS; }
        uint8_t GetStatus(std::error_code&) override { return RegVals::DefaultValues::STATUS; }
        uint8_t WriteRead(uint8_t) override { return 0; }
        uint8_t WriteRead(uint8_t, std::error_code&) override { return 0; }
        void Write(uint8_t) override {}
        std::error_code Write(uint8_t, std::error_code& ec) override { return ec; }
        bool Transmit(const void*, uint8_t) override { return false; }
        bool Transmit(const void*, uint8_t, std::error_code&) override { return false; }
        bool Available(uint8_t*) override { return false; }
        bool Available(uint8_t*, std::error_code&) override { return false; }
        uint8_t GetPayloadLength() override { return 0; }
        uint8_t GetPayloadLength(std::error_code&) override { return 0; }
        bool Read(void*, uint8_t) override { return false; }
        bool Read(void*, uint8_t, std::error_code&) override { return false; }
        void OpenReadingPipe(uint8_t, uint64_t) override {}
        std::error_code OpenReadingPipe(uint8_t, uint64_t, std::error_code& ec) override { return ec; }
        void CloseReadingPipe(uint8_t) override {}
        std::error_code CloseReadingPipe(uint8_t, std::error_code& ec) override { return ec; }
        void OpenWritingPipe(uint64_t) override {}
        std::error_code OpenWritingPipe(uint64_t, std::error_code& ec) override { return ec; }
        void StartListening() override {}
        std::error_code StartListening(std::error_code& ec) override { return ec; }
        void StopListening() override {}
        std::error_code StopListening(std::error_code& ec) override { return ec; }

        void PrintRegisters() override {}
        void SPIWrite(const void*, uint8_t) override {}
        std::error_code SPIWrite(const void*, uint8_t, std::error_code& ec) override { return ec; }
        void SPIWriteRead(const void*, void*, uint8_t) override {}
        std::error_code SPIWriteRead(const void*, void*, uint8_t, std::error_code& ec) override { return ec; }
    };

    class NRF24L01 : public IRadio
    {
    public:
        NRF24L01(SPI&& spi, int ce, int csn);

        uint8_t GetStatus() override;
        uint8_t GetStatus(std::error_code& ec) override;
        uint8_t WriteRead(uint8_t val) override;
        uint8_t WriteRead(uint8_t val, std::error_code& ec) override;
        void Write(uint8_t val) override;
        std::error_code Write(uint8_t val, std::error_code& ec) override;
        bool Transmit(const void* buf, uint8_t len) override;
        bool Transmit(const void* buf, uint8_t len, std::error_code& ec) override;
        bool Available(uint8_t* pipe) override;
        bool Available(uint8_t* pipe, std::error_code& ec) override;
        uint8_t GetPayloadLength() override;
        uint8_t GetPayloadLength(std::error_code& ec) override;
        bool Read(void* buf, uint8_t len) override;
        bool Read(void* buf, uint8_t len, std::error_code& ec) override;
        void OpenReadingPipe(uint8_t pipe, uint64_t addr) override;
        std::error_code OpenReadingPipe(uint8_t pipe, uint64_t addr, std::error_code& ec) override;
        void CloseReadingPipe(uint8_t pipe) override;
        std::error_code CloseReadingPipe(uint8_t pipe, std::error_code& ec) override;
        void OpenWritingPipe(uint64_t addr) override;
        std::error_code OpenWritingPipe(uint64_t addr, std::error_code& ec) override;
        void StartListening() override;
        std::error_code StartListening(std::error_code& ec) override;
        void StopListening() override;
        std::error_code StopListening(std::error_code& ec) override;

        void PrintRegisters() override;
        template <std::size_t N>
        void PrintRegister(const char* name, Regs::RegAddr<N> reg)
        {
            std::cout << std::setw(11) << name << std::setw(1) << " = 0x" << std::hex << std::setw(2 * N)
                      << std::setfill('0') << static_cast<int64_t>(Reg<N>(reg)) << std::setw(1) << std::dec
                      << std::setfill(' ');
        }
        void SPIWrite(const void* buf, uint8_t len) override;
        std::error_code SPIWrite(const void* buf, uint8_t len, std::error_code& ec) override;
        void SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len) override;
        std::error_code SPIWriteRead(const void* bufOut, void* bufIn, uint8_t len, std::error_code& ec) override;

    private:
        inline void WaitUs(int64_t x)
        {
#ifndef _MSC_VER
            usleep(x);
#endif
        }
        inline void WaitMs(int64_t x)
        {
#ifndef _MSC_VER
            usleep(x * 1000);
#endif
        }

    private:
        SPI spi;
        GPIOPin ce;
        GPIOPin csn;
        uint64_t pipe0Addr;
        static constexpr bool useCsn = configuration::USE_ORANGE_PI;
        static constexpr uint8_t maxLen = 32;
    };
} // namespace RF24

// Just in case someone forgets:
//	Pin definitions
//	constexpr const char* CE = configuration::USE_ORANGE_PI ? "3" : "22";  // rpi 22, opi  3
//	constexpr const char* CSN = configuration::USE_ORANGE_PI ? "13" : "8";  // rpi  8, opi 13
//	constexpr int SPI_DEVICE = configuration::USE_ORANGE_PI ? 10 : 0;    // rpi 0 for spidev0.0, opi 10 for
// spidev1.0 	constexpr uint8_t SCKL = 11; 	constexpr uint8_t MOSI = 10; 	constexpr uint8_t MISO = 9;
// Settings for SPI 	constexpr int SPI_SPEED = 8000000;

#endif /* NRF24L01_H_ */

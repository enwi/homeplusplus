/*
 * nRF24L01.h
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_

 //#include <pigpio.h>
#include <stdint.h>
#include <unistd.h>
#include <string>

/* Memory Map - register address defines */
constexpr uint8_t RF24_CONFIG = 0x00; /* CONFIG register */
constexpr uint8_t RF24_EN_AA = 0x01; /* EN_AA register */
constexpr uint8_t RF24_EN_RXADDR = 0x02; /* EN_RXADDR register */
constexpr uint8_t RF24_SETUP_AW = 0x03; /* SETUP_AW register */
constexpr uint8_t RF24_SETUP_RETR = 0x04;
constexpr uint8_t RF24_RF_CH = 0x05;
constexpr uint8_t RF24_RF_SETUP = 0x06; /* SETUP register */
constexpr uint8_t RF24_STATUS = 0x07;
constexpr uint8_t RF24_OBSERVE_TX = 0x08;
constexpr uint8_t RF24_RPD = 0x09;    /* Mnemonic for nRF24L01+ */
//constexpr uint8_t CD = 0x09;   /* Mnemonic from nRF24L01, new is RPD */
constexpr uint8_t RF24_RX_ADDR_P0 = 0x0A;
constexpr uint8_t RF24_RX_ADDR_P1 = 0x0B;
constexpr uint8_t RF24_RX_ADDR_P2 = 0x0C;
constexpr uint8_t RF24_RX_ADDR_P3 = 0x0D;
constexpr uint8_t RF24_RX_ADDR_P4 = 0x0E;
constexpr uint8_t RF24_RX_ADDR_P5 = 0x0F;
constexpr uint8_t RF24_TX_ADDR = 0x10;
constexpr uint8_t RF24_RX_PW_P0 = 0x11;
constexpr uint8_t RF24_RX_PW_P1 = 0x12;
constexpr uint8_t RF24_RX_PW_P2 = 0x13;
constexpr uint8_t RF24_RX_PW_P3 = 0x14;
constexpr uint8_t RF24_RX_PW_P4 = 0x15;
constexpr uint8_t RF24_RX_PW_P5 = 0x16;
constexpr uint8_t RF24_FIFO_STATUS = 0x17;
constexpr uint8_t RF24_DYNPD = 0x1C;
constexpr uint8_t RF24_FEATURE = 0x1D;

/* Bit Mnemonics */
/* CONFIG Register Bits */
constexpr uint8_t RF24_MASK_RX_DR = (1 << 6);  /* Mask interrupt caused by RX_DR: 1: interrupt masked. 0: interrupt enabled */
constexpr uint8_t RF24_MASK_TX_DS = (1 << 5);  /* Mask interrupt caused by TX_DS: 1: interrupt masked. 0: interrupt enabled */
constexpr uint8_t RF24_MASK_MAX_RT = (1 << 4);  /* Mask interrupt caused by MAX_RT. 1: interrupt not reflected on IRQ pin. 0: reflect MAX_RT as active low interrupt on IRQ pin */
constexpr uint8_t RF24_EN_CRC = (1 << 3);  /* Enable CRC. Forced high if on of the bits in EN_AA is high */
constexpr uint8_t RF24_CRCO = (1 << 2);  /* CRC encoding scheme, 0: 1 byte, 1: 2 bytes */
constexpr uint8_t RF24_PWR_UP = (1 << 1);  /* 1: Power up, 0: Power down */
constexpr uint8_t RF24_PRIM_RX = (1 << 0);  /* 1: PRX, 0: PTX */
constexpr uint8_t RF24_PRIM_TX = (0);     /* 0: PTX */

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

/* Command Name Mnemonics (Instructions) */
constexpr uint8_t RF24_R_REGISTER = 0x00;
constexpr uint8_t RF24_W_REGISTER = 0x20;
constexpr uint8_t RF24_REGISTER_MASK = 0x1F;
constexpr uint8_t RF24_ACTIVATE = 0x50;
constexpr uint8_t RF24_R_RX_PL_WID = 0x60;
constexpr uint8_t RF24_R_RX_PAYLOAD = 0x61;
constexpr uint8_t RF24_W_TX_PAYLOAD = 0xA0;
constexpr uint8_t RF24_FLUSH_TX = 0xE1;
constexpr uint8_t RF24_FLUSH_RX = 0xE2;
constexpr uint8_t RF24_REUSE_TX_PL = 0xE3;
constexpr uint8_t RF24_NOP = 0xFF;

constexpr uint8_t RF24_CONFIG_DEFAULT_VAL = 0x08;
constexpr uint8_t RF24_EN_AA_DEFAULT_VAL = 0x3F;
constexpr uint8_t RF24_EN_RXADDR_DEFAULT_VAL = 0x03;
constexpr uint8_t RF24_SETUP_AW_DEFAULT_VAL = 0x03;
constexpr uint8_t RF24_SETUP_RETR_DEFAULT_VAL = 0x03;
constexpr uint8_t RF24_RF_CH_DEFAULT_VAL = 0x02;
constexpr uint8_t RF24_RF_SETUP_DEFAULT_VAL = 0x0F;
constexpr uint8_t RF24_STATUS_DEFAULT_VAL = 0x0E;
constexpr uint8_t RF24_OBSERVE_TX_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_CD_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_RX_ADDR_P0_B0_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_RX_ADDR_P0_B1_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_RX_ADDR_P0_B2_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_RX_ADDR_P0_B3_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_RX_ADDR_P0_B4_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_RX_ADDR_P1_B0_DEFAULT_VAL = 0xC2;
constexpr uint8_t RF24_RX_ADDR_P1_B1_DEFAULT_VAL = 0xC2;
constexpr uint8_t RF24_RX_ADDR_P1_B2_DEFAULT_VAL = 0xC2;
constexpr uint8_t RF24_RX_ADDR_P1_B3_DEFAULT_VAL = 0xC2;
constexpr uint8_t RF24_RX_ADDR_P1_B4_DEFAULT_VAL = 0xC2;
constexpr uint8_t RF24_RX_ADDR_P2_DEFAULT_VAL = 0xC3;
constexpr uint8_t RF24_RX_ADDR_P3_DEFAULT_VAL = 0xC4;
constexpr uint8_t RF24_RX_ADDR_P4_DEFAULT_VAL = 0xC5;
constexpr uint8_t RF24_RX_ADDR_P5_DEFAULT_VAL = 0xC6;
constexpr uint8_t RF24_TX_ADDR_B0_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_TX_ADDR_B1_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_TX_ADDR_B2_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_TX_ADDR_B3_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_TX_ADDR_B4_DEFAULT_VAL = 0xE7;
constexpr uint8_t RF24_RX_PW_P0_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_RX_PW_P1_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_RX_PW_P2_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_RX_PW_P3_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_RX_PW_P4_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_RX_PW_P5_DEFAULT_VAL = 0x00;
constexpr uint8_t RF24_FIFO_STATUS_DEFAULT_VAL = 0x11;

/* CONFIG register bitwise definitions */
constexpr uint8_t RF24_CONFIG_RESERVED = 0x80;
constexpr uint8_t RF24_CONFIG_MASK_RX_DR = 0x40;
constexpr uint8_t RF24_CONFIG_MASK_TX_DS = 0x20;
constexpr uint8_t RF24_CONFIG_MASK_MAX_RT = 0x10;
constexpr uint8_t RF24_CONFIG_EN_CRC = 0x08;
constexpr uint8_t RF24_CONFIG_CRCO = 0x04;
constexpr uint8_t RF24_CONFIG_PWR_UP = 0x02;
constexpr uint8_t RF24_CONFIG_PRIM_RX = 0x01;

/* EN_AA register bitwise definitions */
constexpr uint8_t RF24_EN_AA_RESERVED = 0xC0;
constexpr uint8_t RF24_EN_AA_ENAA_ALL = 0x3F;
constexpr uint8_t RF24_EN_AA_ENAA_P5 = 0x20;
constexpr uint8_t RF24_EN_AA_ENAA_P4 = 0x10;
constexpr uint8_t RF24_EN_AA_ENAA_P3 = 0x08;
constexpr uint8_t RF24_EN_AA_ENAA_P2 = 0x04;
constexpr uint8_t RF24_EN_AA_ENAA_P1 = 0x02;
constexpr uint8_t RF24_EN_AA_ENAA_P0 = 0x01;
constexpr uint8_t RF24_EN_AA_ENAA_NONE = 0x00;

/* EN_RXADDR register bitwise definitions */
constexpr uint8_t RF24_EN_RXADDR_RESERVED = 0xC0;
constexpr uint8_t RF24_EN_RXADDR_ERX_ALL = 0x3F;
constexpr uint8_t RF24_EN_RXADDR_ERX_P5 = 0x20;
constexpr uint8_t RF24_EN_RXADDR_ERX_P4 = 0x10;
constexpr uint8_t RF24_EN_RXADDR_ERX_P3 = 0x08;
constexpr uint8_t RF24_EN_RXADDR_ERX_P2 = 0x04;
constexpr uint8_t RF24_EN_RXADDR_ERX_P1 = 0x02;
constexpr uint8_t RF24_EN_RXADDR_ERX_P0 = 0x01;
constexpr uint8_t RF24_EN_RXADDR_ERX_NONE = 0x00;

/* SETUP_AW register bitwise definitions */
constexpr uint8_t RF24_SETUP_AW_RESERVED = 0xFC;
//constexpr uint8_t RF24_SETUP_AW = 0x03; //Already defined above
constexpr uint8_t RF24_SETUP_AW_5BYTES = 0x03;
constexpr uint8_t RF24_SETUP_AW_4BYTES = 0x02;
constexpr uint8_t RF24_SETUP_AW_3BYTES = 0x01;
constexpr uint8_t RF24_SETUP_AW_ILLEGAL = 0x00;

/* SETUP_RETR register bitwise definitions */
constexpr uint8_t RF24_SETUP_RETR_ARD = 0xF0;
constexpr uint8_t RF24_SETUP_RETR_ARD_4000 = 0xF0; /* 4400 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_3750 = 0xE0; /* 3750 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_3500 = 0xD0; /* 3500 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_3250 = 0xC0; /* 3250 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_3000 = 0xB0; /* 3000 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_2750 = 0xA0; /* 2750 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_2500 = 0x90; /* 2500 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_2250 = 0x80; /* 2250 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_2000 = 0x70; /* 2000 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_1750 = 0x60; /* 1750 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_1500 = 0x50; /* 1500 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_1250 = 0x40; /* 1250 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_1000 = 0x30; /* 1000 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_750 = 0x20; /* 750 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_500 = 0x10; /* 500 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARD_250 = 0x00; /* 250 us retry delay */
constexpr uint8_t RF24_SETUP_RETR_ARC = 0x0F;
constexpr uint8_t RF24_SETUP_RETR_ARC_15 = 0x0F; /* 15 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_14 = 0x0E; /* 14 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_13 = 0x0D; /* 13 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_12 = 0x0C; /* 12 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_11 = 0x0B; /* 11 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_10 = 0x0A; /* 10 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_9 = 0x09; /* 9 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_8 = 0x08; /* 8 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_7 = 0x07; /* 7 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_6 = 0x06; /* 6 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_5 = 0x05; /* 5 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_4 = 0x04; /* 4 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_3 = 0x03; /* 3 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_2 = 0x02; /* 2 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_1 = 0x01; /* 1 retry count */
constexpr uint8_t RF24_SETUP_RETR_ARC_0 = 0x00; /* 0 retry count, retry disabled */

/* RF_CH register bitwise definitions */
constexpr uint8_t RF24_RF_CH_RESERVED = 0x80;

/* RF_SETUP register bitwise definitions */
constexpr uint8_t RF24_RF_SETUP_RESERVED = 0xE0;
constexpr uint8_t RF24_RF_SETUP_PLL_LOCK = 0x10;
constexpr uint8_t RF24_RF_SETUP_RF_DR = 0x08;
constexpr uint8_t RF24_RF_SETUP_RF_DR_250 = 0x20;
constexpr uint8_t RF24_RF_SETUP_RF_DR_1000 = 0x00;
constexpr uint8_t RF24_RF_SETUP_RF_DR_2000 = 0x08;
constexpr uint8_t RF24_RF_SETUP_RF_PWR = 0x06;
constexpr uint8_t RF24_RF_SETUP_RF_PWR_0 = 0x06;
constexpr uint8_t RF24_RF_SETUP_RF_PWR_6 = 0x04;
constexpr uint8_t RF24_RF_SETUP_RF_PWR_12 = 0x02;
constexpr uint8_t RF24_RF_SETUP_RF_PWR_18 = 0x00;
constexpr uint8_t RF24_RF_SETUP_LNA_HCURR = 0x01;

/* STATUS register bit definitions */
constexpr uint8_t RF24_STATUS_RESERVED = 0x80;   /* bit 1xxx xxxx: This bit is reserved */
constexpr uint8_t RF24_STATUS_RX_DR = 0x40;   /* bit x1xx xxxx: Data ready RX FIFO interrupt. Asserted when new data arrives RX FIFO */
constexpr uint8_t RF24_STATUS_TX_DS = 0x20;   /* bit xx1x xxxx: Data sent TX FIFO interrupt. Asserted when packet transmitted on TX. */
constexpr uint8_t RF24_STATUS_MAX_RT = 0x10;   /* bit xxx1 xxxx: maximum number of TX retransmit interrupts */
constexpr uint8_t RF24_STATUS_RX_P_NO = 0x0E;
constexpr uint8_t RF24_STATUS_RX_P_NO_RX_FIFO_NOT_EMPTY = 0x0E;
constexpr uint8_t RF24_STATUS_RX_P_NO_UNUSED = 0x0C;
constexpr uint8_t RF24_STATUS_RX_P_NO_5 = 0x0A;
constexpr uint8_t RF24_STATUS_RX_P_NO_4 = 0x08;
constexpr uint8_t RF24_STATUS_RX_P_NO_3 = 0x06;
constexpr uint8_t RF24_STATUS_RX_P_NO_2 = 0x04;
constexpr uint8_t RF24_STATUS_RX_P_NO_1 = 0x02;
constexpr uint8_t RF24_STATUS_RX_P_NO_0 = 0x00;   /* bit xxxx 111x: pipe number for payload */
constexpr uint8_t RF24_STATUS_TX_FULL = 0x01;   /* bit xxxx xxx1: if bit set, then TX FIFO is full */

/* OBSERVE_TX register bitwise definitions */
constexpr uint8_t RF24_OBSERVE_TX_PLOS_CNT = 0xF0;
constexpr uint8_t RF24_OBSERVE_TX_ARC_CNT = 0x0F;

/* CD register bitwise definitions for nRF24L01 */
//constexpr uint8_t RF24_CD_RESERVED = 0xFE;
//constexpr uint8_t RF24_CD_CD = 0x01;

/* RPD register bitwise definitions for nRF24L01+ */
constexpr uint8_t RF24_RPD_RESERVED = 0xFE;
constexpr uint8_t RF24_RPD_RPD = 0x01;

/* RX_PW_P0 register bitwise definitions */
constexpr uint8_t RF24_RX_PW_P0_RESERVED = 0xC0;

/* RX_PW_P1 register bitwise definitions */
constexpr uint8_t RF24_RX_PW_P1_RESERVED = 0xC0;

/* RX_PW_P2 register bitwise definitions */
constexpr uint8_t RF24_RX_PW_P2_RESERVED = 0xC0;

/* RX_PW_P3 register bitwise definitions */
constexpr uint8_t RF24_RX_PW_P3_RESERVED = 0xC0;

/* RX_PW_P4 register bitwise definitions */
constexpr uint8_t RF24_RX_PW_P4_RESERVED = 0xC0;

/* RX_PW_P5 register bitwise definitions */
constexpr uint8_t RF24_RX_PW_P5_RESERVED = 0xC0;

/* FIFO_STATUS register bitwise definitions */
constexpr uint8_t RF24_FIFO_STATUS_RESERVED = 0x8C;
constexpr uint8_t RF24_FIFO_STATUS_TX_REUSE = 0x40;
constexpr uint8_t RF24_FIFO_STATUS_TX_FULL = 0x20;
constexpr uint8_t RF24_FIFO_STATUS_TX_EMPTY = 0x10;
constexpr uint8_t RF24_FIFO_STATUS_RX_FULL = 0x02;
constexpr uint8_t RF24_FIFO_STATUS_RX_EMPTY = 0x01;

/* DYNPD register bitwise definitions*/
constexpr uint8_t RF24_DYNPD_RESERVED = 0xC0;
constexpr uint8_t RF24_DYNPD_DPL_ALL = 0x3F;
constexpr uint8_t RF24_DYNPD_DPL_P5 = 0x20;
constexpr uint8_t RF24_DYNPD_DPL_P4 = 0x10;
constexpr uint8_t RF24_DYNPD_DPL_P3 = 0x08;
constexpr uint8_t RF24_DYNPD_DPL_P2 = 0x04;
constexpr uint8_t RF24_DYNPD_DPL_P1 = 0x02;
constexpr uint8_t RF24_DYNPD_DPL_P0 = 0x01;

/* FEATURE register bitwise definitions */
constexpr uint8_t RF24_FEATURE_RESERVED = 0xF8;
constexpr uint8_t RF24_FEATURE_EN_DPL = 0x04;
constexpr uint8_t RF24_FEATURE_EN_ACK_PAY = 0x02;
constexpr uint8_t RF24_FEATURE_EN_DYN_ACK = 0x01;


#ifndef _MSC_VER
uint8_t SPI_WriteRead(uint8_t val);
void SPI_WriteReadBuffer(const void *bufOut, void *bufIn, uint8_t bufSize);
void SPI_WriteBuffer(const void *bufOut, uint8_t bufSize);


void RF_WriteRegister(uint8_t reg, uint8_t val);
uint8_t RF_ReadRegister(uint8_t reg);
uint8_t RF_ReadRegisterData(uint8_t reg, void *buf, uint8_t bufSize);
void RF_WriteRegisterData(uint8_t reg, const void *buf, uint8_t bufSize);

uint8_t RF_GetStatus(void);

uint8_t RF_WriteRead(uint8_t val);
void RF_Write(uint8_t val);

void RF_TxPayload(const uint8_t *payload, uint8_t payloadSize);
void RF_RxPayload(uint8_t *payload, uint8_t payloadSize);

void RF_ResetStatusIRQ(uint8_t flags);

void RF_Init();
void RF_Terminate();

bool RF_Available(uint8_t* pipe = 0);
uint8_t RF_GetPayloadLen();

void RF_OpenReadingPipe(uint8_t pipe, uint64_t addr);
void RF_CloseReadingPipe(uint8_t pipe);

void RF_OpenWritingPipe(uint64_t addr);

void RF_StartListening();
void RF_StopListening();

bool RF_Read(void* buf, uint8_t len);
//0: no ack; 1: ack; -1: ???
int RF_Write(const void* buf, uint8_t len);

void DBG_PrintRegister(const char* name, uint8_t reg);
void DBG_PrintDataRegister(const char* name, uint8_t reg, uint8_t size);
void DBG_PrintRegisters();

int exportGPIO(const char* gpio);
int unexportGPIO(const char* gpio);
int setModesGPIO();
int setValGPIO(const char* gpio, int val);
int SpiOpenPort(const char* spi_device);
int SpiClosePort(int spi_handle);
int SpiWriteAndRead(int spi_handle, const char *in, char *out, int length);
#else

inline uint8_t SPI_WriteRead(uint8_t val) { return 0; }
inline void SPI_WriteReadBuffer(const void *bufOut, void *bufIn, uint8_t bufSize) {}
inline void SPI_WriteBuffer(const void *bufOut, uint8_t bufSize) {}


inline void RF_WriteRegister(uint8_t reg, uint8_t val) {}
inline uint8_t RF_ReadRegister(uint8_t reg) { return 0; }
inline uint8_t RF_ReadRegisterData(uint8_t reg, void *buf, uint8_t bufSize) { return 0; }
inline void RF_WriteRegisterData(uint8_t reg, const void *buf, uint8_t bufSize) {}

inline uint8_t RF_GetStatus(void) { return 0; }

inline uint8_t RF_WriteRead(uint8_t val) { return 0; }
inline void RF_Write(uint8_t val) {}

inline void RF_TxPayload(const void *payload, uint8_t payloadSize) {}
inline void RF_RxPayload(void *payload, uint8_t payloadSize) {}
inline bool RF_DataIsReady(void) { return false; }

inline void RF_ResetStatusIRQ(uint8_t flags) {}

inline void RF_Init(void) {}
inline void RF_Terminate() {}

inline bool RF_Available(uint8_t* pipe = 0) { return false; }
inline uint8_t RF_GetPayloadLen() { return 0; }

inline void RF_OpenReadingPipe(uint8_t pipe, uint64_t addr) {}
inline void RF_CloseReadingPipe(uint8_t pipe) {}

inline void RF_OpenWritingPipe(uint64_t addr) {}

inline void RF_StartListening() {}
inline void RF_StopListening() {}

inline bool RF_Read(void* buf, uint8_t len) { return false; }
//0: no ack{} 1: ack{} -1: ???
inline int RF_Write(const void* buf, uint8_t len) { return 0; }

inline void DBG_PrintRegister(const char* name, uint8_t reg) {}
inline void DBG_PrintDataRegister(const char* name, uint8_t reg, uint8_t size) {}
inline void DBG_PrintRegisters() {}

#endif

extern int spiHandle;
extern uint64_t pipe0Address;

//Pin definitions
constexpr const char* CE = "22";
constexpr const char* CSN = "8";
constexpr const char* SPI_DEVICE = "/dev/spidev0.0";
//#define CE	"13"        //22
//#define CSN	"2"         //8
#define SCKL	11
#define MOSI	10 
#define MISO	9
//#define IRQ		2 No IRQ

//Settings for SPI
#define SPI_SPEED		8000000
#define SPI_CHANNEL     0
#define SPI_FLAGS       0x0

#ifndef _MSC_VER

/* Macros to hide low level functionality */
#define RF_WAIT_US(x)  usleep(x)  /* wait for the given number of micro-seconds */
#define RF_WAIT_MS(x)  usleep(x*1000)  /* wait for the given number of milli-seconds */
#define RF_CE_LOW()    setValGPIO(CE, 0)      /* put CE LOW */
#define RF_CE_HIGH()   setValGPIO(CE, 1)      /* put CE HIGH */
#define RF_CSN_LOW()   /*setValGPIO(CSN, 0)      put CSN LOW */
#define RF_CSN_HIGH()  /*setValGPIO(CSN, 1)      put CSN HIGH */

#else

#define RF_WAIT_US(x)
#define RF_WAIT_MS(x)
#define RF_CE_LOW()
#define RF_CE_HIGH()
#define RF_CSN_LOW()
#define RF_CSN_HIGH()

#endif

#endif /* NRF24L01_H_ */

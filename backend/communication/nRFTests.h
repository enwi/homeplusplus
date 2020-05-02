#ifndef NRF_TESTS_H
#define NRF_TESTS_H

#include "nRF24L01.h"

// Returns default value for register, except RX_ADDR_X and TX_ADDR
uint8_t GetDefaultVal(RF24::Regs::RegAddr<1> addr);

std::error_code PerformTests(RF24::IRadio& nrf);

#endif
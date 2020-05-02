/*
* nRF24L01.c
*
*      Author: Erich Styger
*/

#include "nRF24L01.h"
#include <cstring>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>			//Needed for SPI port
#include <stdio.h>
#include <stdlib.h>

#include <ctime>

//#include <stdlib.h>
//#include <fcntl.h>
//#include <unistd.h>

/*
// Definitions for direct GPIO register access
#define BCM2708_PERI_BASE   0x20000000 //Change to 0x3F000000 on RPi 2
#define GPIO_BASE           (BCM2708_PERI_BASE + 0x2000000) //GPIO controller

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)

int mem_fd;
void* gpio_map;
//I/O access
volatile unsigned int* gpio;

//GPIO setup macros. Use INP_GPIO(x) BEFORE OUT_GPIO(x) or SET_GPIO_ALT(x,y)!
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock
*/

int spiHandle;
uint64_t pipe0Address = 0;

uint8_t spi_txbuf[32 + 1]; //Buffers for sending SPI commands in one go
uint8_t spi_rxbuf[32 + 1];

//RF24 radio(CE, CSN);

/*!
* \brief Writes a byte and reads the value
* \param val Value to write. This value will be shifted out
* \return The value shifted in
*/
uint8_t SPI_WriteRead(uint8_t val) {
	uint8_t res;
	//int err = spiXfer(spiHandle, reinterpret_cast<char*>(&val), reinterpret_cast<char*>(&res), 1);
	int err = SpiWriteAndRead(spiHandle, reinterpret_cast<const char*>(&val), reinterpret_cast<char*>(&res), 1);
	if (err != 1)
	{
		std::cout << "Error in SPI_WriteRead: " << err << std::endl;
	}
	return res;
}

/*!
* \brief Writes a buffer to the SPI bus and the same time reads in the data
* \param bufInOut In-/Output buffer, filled with read data
* \param bufSize Size of input and output buffer
*/
void SPI_WriteReadBuffer(const void* bufOut, void* bufIn, uint8_t bufSize) {
	//int res = spiXfer(spiHandle, const_cast<char*>(reinterpret_cast<const char*>(bufOut)), reinterpret_cast<char*>(bufIn), bufSize);
	int res = SpiWriteAndRead(spiHandle, reinterpret_cast<const char*>(bufOut), reinterpret_cast<char*>(bufIn), bufSize);
	if (res != bufSize)
	{
		std::cout << "Error in SPI_WriteReadBuffer: " << res << std::endl;
	}
}

/*!
* \brief Writes multiple bytes to the SPI bus
* You have to call SPI.beginTransaction(SPI_SETTINGS) before this and SPI.endTransaction() after you finished.
* \param bufOut Buffer to write
* \param bufSize Size of buffer
*/
void SPI_WriteBuffer(const void* bufOut, uint8_t bufSize) {
	char wayne[bufSize];
	SpiWriteAndRead(spiHandle, const_cast<char*>(static_cast<const char*>(bufOut)), wayne, bufSize);
}

/*!
* \brief Write a register value to the transceiver
* \param reg Register to write
* \param val Value of the register to write
*/
void RF_WriteRegister(uint8_t reg, uint8_t val) {
	uint8_t* ptx = spi_txbuf;
	//1st bit of data is write register command, 2nd is written data
	*ptx++ = (RF24_W_REGISTER | reg);
	*ptx++ = val;
	SPI_WriteBuffer((void*)spi_txbuf, 2);

	RF_WAIT_US(10); /* insert a delay until next command */
}

/*!
* \brief Reads a byte value from a register
* \param reg Register to read
* \return Register value read
*/
uint8_t RF_ReadRegister(uint8_t reg) {

	uint8_t* ptx = spi_txbuf;
	//Fill TX buffer with read register command + NOP to read the result
	*ptx++ = (RF24_R_REGISTER | reg);
	*ptx++ = RF24_NOP;

	SPI_WriteReadBuffer((void*)spi_txbuf, (void*)spi_rxbuf, 2);

	RF_WAIT_US(10);
	//Result is 2nd byte of receive buffer, first byte is status register
	return spi_rxbuf[1];
}

/*!
* \brief Read multiple bytes from the bus.
* Modified to return the status register
* \param reg Register address
* \param buf Buffer where to write the data
* \param bufSize Buffer size in bytes
*/
uint8_t RF_ReadRegisterData(uint8_t reg, void *buf, uint8_t bufSize) {

	uint8_t* ptx = spi_txbuf;
	uint8_t* prx = spi_rxbuf;
	*ptx++ = (RF24_R_REGISTER | reg);
	memset((void*)ptx, RF24_NOP, bufSize);
	SPI_WriteReadBuffer((void*)spi_txbuf, (void*)spi_rxbuf, bufSize + 1);
	//1st bit of result is answer
	uint8_t status = *prx++;
	//The rest is register data
	memcpy((void*)buf, (void*)prx, bufSize);

	RF_WAIT_US(10);
	return status;
}

/*!
* \brief Write multiple bytes to the bus.
* \param reg Register address
* \param buf Buffer of what to write
* \param bufSize Buffer size in bytes
*/
void RF_WriteRegisterData(uint8_t reg, const void *buf, uint8_t bufSize) {

	uint8_t* ptx = spi_txbuf;
	//1st bit of data is write register command, rest is written data
	*ptx++ = (RF24_W_REGISTER | reg);
	memcpy((void*)ptx, (void*)buf, bufSize);
	SPI_WriteBuffer((void*)spi_txbuf, bufSize + 1);


	RF_WAIT_US(10);
}

/*!
* \brief Writes a byte and reads back one byte
* \param val Byte to write to the SPI shift register
* \return Byte read from the SPI shift register
*/
uint8_t RF_WriteRead(uint8_t val) {

	val = SPI_WriteRead(val);

	RF_WAIT_US(10);
	return val;
}

/*!
* \brief Writes a byte to the bus, without returning the byte read.
* \param val Byte to write.
*/

void RF_Write(uint8_t val) {
	(void)SPI_WriteRead(val);

	RF_WAIT_US(10);
}

/*!
* \brief Read and return the STATUS
* \return Status
*/
uint8_t RF_GetStatus(void) {
	return RF_WriteRead(RF24_NOP);
}

/*!
* \brief Reset the given mask of status bits
* \param flags Flags, one or more of RF24_STATUS_RX_DR, RF24_STATUS_TX_DS, RF24_STATUS_MAX_RT
*/
void RF_ResetStatusIRQ(uint8_t flags) {
	RF_WAIT_US(10);
	RF_CSN_LOW();
	RF_WAIT_US(10);
	RF_WriteRegister(RF24_STATUS, flags); /* reset all IRQ in status register */
	RF_WAIT_US(10);
	RF_CSN_HIGH();
	RF_WAIT_US(10);
}

/*!
* \brief Send the payload to the Tx FIFO and send it
* \param payload Buffer with payload to send
* \param payloadSize Size of payload buffer
*/
void RF_TxPayload(const uint8_t *payload, uint8_t payloadSize) {
	RF_Write(RF24_FLUSH_TX); /* flush old data */
	RF_WriteRegisterData(RF24_W_TX_PAYLOAD, payload, payloadSize); /* write payload */
	RF_CE_HIGH(); /* start transmission */
	RF_WAIT_US(15); /* keep signal high for 15 micro-seconds */
	RF_CE_LOW();  /* back to normal */
}

/*!
* \brief Receive the Rx payload from the FIFO and stores it in a buffer.
* \param payload Pointer to the payload buffer
* \param payloadSize Size of the payload buffer
*/
void RF_RxPayload(uint8_t *payload, uint8_t payloadSize) {
	RF_CE_LOW(); /* need to disable rx mode during reading RX data */
	RF_ReadRegisterData(RF24_R_RX_PAYLOAD, payload, payloadSize); /* rx payload */
	RF_CE_HIGH(); /* re-enable rx mode */
}

/*!
* \brief Initializes the transceiver.
*/
void RF_Init()
{
	//gpioInitialise();
	//gpioSetMode(CE, PI_OUTPUT);
	//gpioSetMode(CSN, PI_OUTPUT);
	exportGPIO(CE);
	exportGPIO(CSN);
	setModesGPIO();
	RF_CE_LOW();   /* CE high: do not send or receive data */
	RF_CSN_HIGH(); /* CSN low: not sending commands to the device */

	//spiHandle = spiOpen(SPI_CHANNEL, SPI_SPEED, SPI_FLAGS);
	spiHandle = SpiOpenPort(SPI_DEVICE);

	RF_WAIT_MS(5);
}

void RF_Terminate()
{
	//spiClose(spiHandle);
	//gpioTerminate();

	SpiClosePort(spiHandle);

	unexportGPIO(CE);
	unexportGPIO(CSN);
}

//Checks if data is available
bool RF_Available(uint8_t* pipe)
{
	uint8_t status;
	//Get the status register which is sent with every SPI command
	status = RF_GetStatus();

	//Check if a packet was received
	bool result = (status & RF24_STATUS_RX_DR);

	//If pipe is not a nullptr, put the pipe information in it
	if (pipe != 0)
	{
		*pipe = (status & RF24_STATUS_RX_P_NO) >> 1;
	}

	//Clear data received bit
	RF_WriteRegister(RF24_STATUS, RF24_RX_DR | RF24_TX_DS);


	return result;
}

uint8_t RF_GetPayloadLen()
{
	spi_txbuf[0] = RF24_R_RX_PL_WID;
	spi_txbuf[1] = RF24_NOP;
	SPI_WriteReadBuffer(spi_txbuf, spi_rxbuf, 2);
	return spi_rxbuf[1];
}

void RF_OpenReadingPipe(uint8_t pipe, uint64_t addr)
{
	if (pipe == 0)
	{
		RF_WriteRegisterData(RF24_RX_ADDR_P0, reinterpret_cast<const uint8_t*>(&addr), 5);
		pipe0Address = addr;
	}
	else if (pipe == 1)
	{
		RF_WriteRegisterData(RF24_RX_ADDR_P1, reinterpret_cast<const uint8_t*>(&addr), 5);
	}
	else if (pipe == 2)
	{
		RF_WriteRegister(RF24_RX_ADDR_P2, static_cast<uint8_t>(addr & 0xFF));
	}
	else if (pipe == 3)
	{
		RF_WriteRegister(RF24_RX_ADDR_P3, static_cast<uint8_t>(addr & 0xFF));
	}
	else if (pipe == 4)
	{
		RF_WriteRegister(RF24_RX_ADDR_P4, static_cast<uint8_t>(addr & 0xFF));
	}
	else if (pipe == 5)
	{
		RF_WriteRegister(RF24_RX_ADDR_P5, static_cast<uint8_t>(addr & 0xFF));
	}
	RF_WriteRegister(RF24_EN_RXADDR, RF_ReadRegister(RF24_EN_RXADDR) | (RF24_EN_RXADDR_ERX_P0 << pipe));
}

void RF_CloseReadingPipe(uint8_t pipe)
{
	RF_WriteRegister(RF24_EN_RXADDR, RF_ReadRegister(RF24_EN_RXADDR) & (~(RF24_EN_RXADDR_ERX_P0 << pipe)));
}

void RF_OpenWritingPipe(uint64_t addr)
{
	if (RF_ReadRegister(RF24_EN_RXADDR) & RF24_EN_RXADDR_ERX_P0)
	{
		//Save pipe 0 address
		RF_ReadRegisterData(RF24_RX_ADDR_P0, &pipe0Address, 5);
	}
	else
	{
		pipe0Address = 0;
	}

	RF_WriteRegisterData(RF24_TX_ADDR, &addr, 5);
	RF_WriteRegisterData(RF24_RX_ADDR_P0, &addr, 5);
	//I don't know if this is right
	RF_WriteRegister(RF24_EN_RXADDR, RF_ReadRegister(RF24_EN_RXADDR) | RF24_EN_RXADDR_ERX_P0);
}

void RF_StartListening()
{
	//Power up and set to primary rx mode
	RF_WriteRegister(RF24_CONFIG, RF_ReadRegister(RF24_CONFIG) | RF24_CONFIG_PWR_UP | RF24_CONFIG_PRIM_RX);
	//Reset flags
	RF_WriteRegister(RF24_STATUS, RF24_STATUS_TX_DS | RF24_STATUS_RX_DR | RF24_STATUS_MAX_RT);

	if (pipe0Address != 0)
	{
		RF_WriteRegisterData(RF24_RX_ADDR_P0, &pipe0Address, 5);
		RF_WriteRegister(RF24_EN_RXADDR, RF_ReadRegister(RF24_EN_RXADDR) | RF24_EN_RXADDR_ERX_P0);
	}
	else
	{
		RF_WriteRegister(RF24_EN_RXADDR, RF_ReadRegister(RF24_EN_RXADDR) & ~RF24_EN_RXADDR_ERX_P0);
	}
	RF_CE_HIGH();

	RF_WAIT_US(130);
	//std::cout << "Start listening at: " << gpioTick() << std::endl;
	std::cout << "Start listening at: " << std::time(0) << std::endl;
}

void RF_StopListening()
{
	RF_CE_LOW();
	RF_WAIT_US(130);
	//std::cout << "Stop listening at: " << gpioTick() << std::endl;
	std::cout << "Stop listening at: " << std::time(0) << std::endl;
}

bool RF_Read(void* buf, uint8_t len)
{
	len = std::min(len, static_cast<uint8_t>(len));
	std::cout << (int)RF_ReadRegister(RF24_FIFO_STATUS) << " " << (int)RF_GetStatus() << " " << (int)RF_ReadRegister(RF24_CONFIG) << std::endl;
	spi_txbuf[0] = RF24_R_RX_PAYLOAD;
	memset(spi_txbuf + 1, RF24_NOP, len);
	std::cout << "len:" << (int)len << std::endl;;
	for (uint8_t i : spi_txbuf)
	{
		std::cout << (int)i << '|';
	}
	std::cout << std::endl;
	//Get the payload
	SPI_WriteReadBuffer(spi_txbuf, spi_rxbuf, len + 1);
	for (uint8_t i : spi_rxbuf)
	{
		std::cout << (int)i << '|';
	}
	std::cout << std::endl;
	memcpy(buf, spi_rxbuf + 1, len);
	for (uint8_t *i = (uint8_t*)buf; i < (uint8_t*)buf + len; i++)
	{
		std::cout << (int)*i << '|';
	}
	std::cout << std::endl;
	//Clear data received bit
	RF_WriteRegister(RF24_STATUS, RF24_RX_DR | RF24_TX_DS);
	std::cout << (int)RF_ReadRegister(RF24_FIFO_STATUS) << " " << (int)RF_GetStatus() << " " << (int)RF_ReadRegister(RF24_CONFIG) << std::endl;
	return RF_ReadRegister(RF24_FIFO_STATUS) & RF24_FIFO_STATUS_RX_EMPTY;
}

int RF_Write(const void* buf, uint8_t len)
{
	int result = 0;
	bool tx_ok = false, tx_fail = false;


	unsigned int wrong_count = 0;
	// Begin the write
	// Transmitter power-up
	RF_WriteRegister(RF24_CONFIG, (RF_ReadRegister(RF24_CONFIG) | RF24_CONFIG_PWR_UP) & ~RF24_CONFIG_PRIM_RX);
	RF_WAIT_US(150);

	// Send the payload
	uint8_t status;

	uint8_t data_len = std::min(len, static_cast<uint8_t>(32));

	spi_txbuf[0] = RF24_W_TX_PAYLOAD;
	memcpy(spi_txbuf + 1, buf, data_len);
	do
	{
		SPI_WriteBuffer(spi_txbuf, data_len + 1);

		RF_WAIT_US(4);

		// Put to transmit mode
		RF_CE_HIGH();
		RF_WAIT_US(30);


		// ------------
		// At this point we could return from a non-blocking write, and then call
		// the rest after an interrupt

		// Instead, we are going to block here until we get TX_DS (transmission completed and ack'd)
		// or MAX_RT (maximum retries, transmission failed).  Also, we'll timeout in case the radio
		// is flaky and we get neither.

		// IN the end, the send should be blocking.  It comes back in 60ms worst case, or much faster
		// if I tighted up the retry logic.  (Default settings will be 1500us.
		// Monitor the send
		uint8_t observe_tx;
		uint32_t sent_at = std::time(0); //us since system reboot
		const uint32_t timeout = 50000; //us to wait for timeout
		do
		{
			status = RF_ReadRegisterData(RF24_OBSERVE_TX, &observe_tx, 1);
		} while (!(status & (RF24_STATUS_TX_DS | RF24_STATUS_MAX_RT)) && (std::time(0) - sent_at < timeout));
		RF_CE_LOW();

		// The part above is what you could recreate with your own interrupt handler,
		// and then call this when you got an interrupt
		// ------------

		// Call this when you get an interrupt
		// The status tells us three things
		// * The send was successful (TX_DS)
		// * The send failed, too many retries (MAX_RT)
		// * There is an ack packet waiting (RX_DR)

		status = RF_GetStatus();

		RF_WriteRegister(RF24_STATUS, RF24_STATUS_TX_DS | RF24_STATUS_MAX_RT | RF24_STATUS_RX_DR);

		tx_ok = status & RF24_STATUS_TX_DS;
		tx_fail = status & RF24_STATUS_MAX_RT;

		result = tx_ok;
		wrong_count++;
	} while (!(tx_ok || tx_fail) && wrong_count < 8);
	if (wrong_count >= 8)
	{
		std::cout << "Why does this happen?" << std::endl;
		result = -1;
	}
	// Yay, we are done.
	// Power down
	RF_WriteRegister(RF24_CONFIG, RF_ReadRegister(RF24_CONFIG) | ~RF24_CONFIG_PWR_UP);
	// Flush buffers
	RF_Write(RF24_FLUSH_TX);
	return result;
}

//DEBUG FUNCTIONS
void DBG_PrintRegister(const char* name, uint8_t reg)
{
	static char buf[32];
	buf[0] = '\0';
	sprintf(buf, "%11s = 0x%02X", name, RF_ReadRegister(reg));
	std::cout << buf;
}
void DBG_PrintDataRegister(const char* name, uint8_t reg, uint8_t size)
{
	static char buf[64];
	buf[0] = '\0';
	sprintf(buf, "%8s = 0x", name);
	uint8_t* val = new uint8_t[size];
	RF_ReadRegisterData(reg, val, size);
	for (uint8_t* i = val + size - 1; i >= val; i--)
	{
		sprintf(strchr(buf, '\0'), "%02X", *i);
	}
	delete[] val;
	std::cout << buf;
}
void DBG_PrintRegisters()
{
	std::cout << "The current register status:" << std::endl;
	DBG_PrintRegister("CONFIG", RF24_CONFIG);
	DBG_PrintRegister("EN_AA", RF24_EN_AA);
	DBG_PrintRegister("EN_RXADDR", RF24_EN_RXADDR);
	DBG_PrintRegister("SETUP_AW", RF24_SETUP_AW);
	DBG_PrintRegister("SETUP_RETR", RF24_SETUP_RETR);
	DBG_PrintRegister("RF_CH", RF24_RF_CH);
	DBG_PrintRegister("RF_SETUP", RF24_RF_SETUP);
	DBG_PrintRegister("STATUS", RF24_STATUS);
	DBG_PrintRegister("OBSERVE_TX", RF24_OBSERVE_TX);
	DBG_PrintRegister("RPD", RF24_RPD);
	std::cout << std::endl;
	DBG_PrintDataRegister("RX_ADDR_P0", RF24_RX_ADDR_P0, 5);
	DBG_PrintDataRegister("RX_ADDR_P1", RF24_RX_ADDR_P1, 5);
	DBG_PrintRegister("RX_ADDR_P2", RF24_RX_ADDR_P2);
	DBG_PrintRegister("RX_ADDR_P3", RF24_RX_ADDR_P3);
	DBG_PrintRegister("RX_ADDR_P4", RF24_RX_ADDR_P4);
	DBG_PrintRegister("RX_ADDR_P5", RF24_RX_ADDR_P5);
	DBG_PrintDataRegister("TX_ADDR", RF24_TX_ADDR, 5);
	std::cout << std::endl;
	DBG_PrintRegister("RX_PW_P0", RF24_RX_PW_P0);
	DBG_PrintRegister("RX_PW_P1", RF24_RX_PW_P1);
	DBG_PrintRegister("RX_PW_P2", RF24_RX_PW_P2);
	DBG_PrintRegister("RX_PW_P3", RF24_RX_PW_P3);
	DBG_PrintRegister("RX_PW_P4", RF24_RX_PW_P4);
	DBG_PrintRegister("RX_PW_P5", RF24_RX_PW_P5);
	std::cout << std::endl;
	DBG_PrintRegister("FIFO_STATUS", RF24_FIFO_STATUS);
	DBG_PrintRegister("DYNPD", RF24_DYNPD);
	DBG_PrintRegister("FEATURE", RF24_FEATURE);
	std::cout << std::endl;
}

int exportGPIO(const char* gpio)
{
	const char* export_str = "/sys/class/gpio/export";
	std::ofstream exportgpio(export_str); // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
	if (exportgpio < 0)
	{
		std::cout << "Error: Could not open export file" << std::endl;
		return -1;
	}

	exportgpio << gpio;    // export gpio
	exportgpio.close();    // close export file
	return 0;
}

int unexportGPIO(const char* gpio)
{
	const char* unexport_str = "/sys/class/gpio/unexport";
	std::ofstream unexportgpio(unexport_str); // Open unexport file
	if (unexportgpio < 0)
	{
		std::cout << "Error: Could not open unexport file" << std::endl;
		return -1;
	}

	unexportgpio << gpio;    // export gpio
	unexportgpio.close(); //close unexport file
	return 0;
}

int setModesGPIO()
{
	std::string setdir_str22 = std::string("/sys/class/gpio/gpio") + CE + "/direction";
	std::ofstream setdirgpio22(setdir_str22.c_str()); // open direction file for gpio
	if (setdirgpio22 < 0)
	{
		std::cout << "Could not open gpio direction file for 22(CE)" << std::endl;
		return -1;
	}
	setdirgpio22 << "out";  // write direction to direction file
	setdirgpio22.close();   // close direction file

	std::string setdir_str08 = std::string("/sys/class/gpio/gpio") + CSN + "/direction";
	std::ofstream setdirgpio08(setdir_str08.c_str()); // open direction file for gpio
	if (setdirgpio08 < 0)
	{
		std::cout << "Could not open gpio direction file for 22(CE)" << std::endl;
		return -1;
	}
	setdirgpio08 << "out";  // write direction to direction file
	setdirgpio08.close();   // close direction file
	return 0;
}

int setValGPIO(const char* gpio, int val)
{

	const std::string setval_str = std::string("/sys/class/gpio/gpio") + gpio + "/value";
	std::ofstream setvalgpio(setval_str.c_str()); // open value file for gpio
	if (setvalgpio < 0) {
		std::cout << " OPERATION FAILED: Unable to set the value of GPIO" << gpio << " ." << std::endl;
		return -1;
	}

	setvalgpio << val;//write value to value file
	setvalgpio.close();// close value file
	return 0;
}

unsigned char spi_mode;
unsigned char spi_bitsPerWord;
unsigned int spi_speed;


//***********************************
//***********************************
//********** SPI OPEN PORT **********
//***********************************
//***********************************
//spi_device	0=CS0, 1=CS1
int SpiOpenPort(const char* spi_device)
{
	//----- SET SPI MODE -----
	//SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
	//SPI_MODE_1 (0,1) 	CPOL = 0, CPHA = 1, Clock idle low, data is clocked in on falling edge, output data (change) on rising edge
	//SPI_MODE_2 (1,0) 	CPOL = 1, CPHA = 0, Clock idle high, data is clocked in on falling edge, output data (change) on rising edge
	//SPI_MODE_3 (1,1) 	CPOL = 1, CPHA = 1, Clock idle high, data is clocked in on rising, edge output data (change) on falling edge
	spi_mode = SPI_MODE_0;

	//----- SET BITS PER WORD -----
	spi_bitsPerWord = 8;

	//----- SET SPI BUS SPEED -----
	//spi_speed = 10;		//1000000 = 1MHz (1uS per bit) 
	spi_speed = 4000000;		//8000000 = 8MHz (1uS per bit) 
	int lsbFirst = 1; //LSB first


	int spi_handle = open(spi_device, O_RDWR);

	if (spi_handle < 0)
	{
		std::cout << "Error - Could not open SPI device\n";
		return -1;
	}

	int status_value = ioctl(spi_handle, SPI_IOC_WR_MODE, &spi_mode);
	if (status_value < 0)
	{
		std::cout << "Could not set SPIMode (WR)...ioctl fail\n";
		return -1;
	}

	status_value = ioctl(spi_handle, SPI_IOC_RD_MODE, &spi_mode);
	if (status_value < 0)
	{
		std::cout << "Could not set SPIMode (RD)...ioctl fail\n";
		return -1;
	}

	status_value = ioctl(spi_handle, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
	if (status_value < 0)
	{
		std::cout << "Could not set SPI bitsPerWord (WR)...ioctl fail\n";
		return -1;
	}

	status_value = ioctl(spi_handle, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
	if (status_value < 0)
	{
		std::cout << "Could not set SPI bitsPerWord(RD)...ioctl fail\n";
		return -1;
	}

	status_value = ioctl(spi_handle, SPI_IOC_WR_LSB_FIRST, &lsbFirst);

	status_value = ioctl(spi_handle, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	if (status_value < 0)
	{
		std::cout << "Could not set SPI speed (WR)...ioctl fail\n";
		return -1;
	}

	status_value = ioctl(spi_handle, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
	if (status_value < 0)
	{
		std::cout << "Could not set SPI speed (RD)...ioctl fail\n";
		return -1;
	}
	return spi_handle;
}



//************************************
//************************************
//********** SPI CLOSE PORT **********
//************************************
//************************************
int SpiClosePort(int spi_handle)
{
	int status_value = close(spi_handle);
	if (status_value < 0)
	{
		std::cout << "Error - Could not close SPI device\n";
		return -1;
	}
	return(status_value);
}

//*******************************************
//*******************************************
//********** SPI WRITE & READ DATA **********
//*******************************************
//*******************************************
//data		Bytes to write.  Contents is overwritten with bytes read.
int SpiWriteAndRead(int spi_handle, const char *in, char *out, int length)
{
	spi_ioc_transfer spi[2] = {};

	spi[0].delay_usecs = 1;
	spi[0].cs_change = 0;
	//one spi transfer for each byte
	spi[1].tx_buf = (unsigned long)(in); // transmit from "data"
	spi[1].rx_buf = (unsigned long)(out); // receive into "data"
	spi[1].len = length;
	spi[1].delay_usecs = 1;
	spi[1].speed_hz = spi_speed;
	spi[1].bits_per_word = spi_bitsPerWord;
	spi[1].cs_change = 1;

	int retVal = ioctl(spi_handle, SPI_IOC_MESSAGE(2), spi);

	if (retVal < 0)
	{
		std::cout << "Error - Problem transmitting spi data..ioctl" << strerror(errno) << '\n';
		return -1;
	}

	return length;
}

/*int SpiWriteAndRead (int spi_device, char *in, char *out, int length)
{
	struct spi_ioc_transfer spi[length];
	int i = 0;
	int retVal = -1;
	int *spi_cs_fd;

	if (spi_device)
		spi_cs_fd = &spi_cs1_fd;
	else
		spi_cs_fd = &spi_cs0_fd;

	memset(&spi[i], 0, sizeof (spi[i]));
	spi.tx_buf = (unsigned long)in; // transmit from "data"
	spi.rx_buf = (unsigned long)out; // receive into "data"
	spi.len = length ;
	spi.delay_usecs = 0 ;
	spi.speed_hz = spi_speed ;
	spi.bits_per_word = spi_bitsPerWord ;
	spi.cs_change = 0;

	retVal = ioctl(*spi_cs_fd, SPI_IOC_MESSAGE(length), &spi) ;

	if(retVal < 0)
	{
		std::cout << "Error - Problem transmitting spi data..ioctl\n";
		return -1;
	}

	return length;
}*/
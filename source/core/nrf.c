#include "nrf.h"
#include "spi.h"
#include <stdio.h>
#include <time.h>

#define SPI_CLK_SPEED 10000000
#define NRF_SPI_KEY 0x7258AE90

#define PIN_CE 19
#define PIN_CSN 26

// Data
static char nrf_isInitFlag = 0;

// Util Functions
static void nrf_sleep(long usec);

// Setup and initialize the nRF24L01 interface
int nrf_init()
{
	//already initialized?
	if(nrf_isInitFlag == 1) return 0;
	
	//check dependencies
	if(!spi_isInit()) {
		fprintf(stderr, "nrf_init: SPI dependency is not initialized\n");
		nrf_close();
		return 1;
	}
	
	//setup select pins
	spi_setGPIODir(PIN_CE, 0x00);
	spi_setGPIODir(PIN_CSN, 0x00);
	spi_writeGPIO(PIN_CE, 0);
	spi_writeGPIO(PIN_CSN, 1);
	nrf_sleep(50*1000);
	
	nrf_isInitFlag = 1;
	return 0;
}

// Checks if the nRF24L01 interface is initialized
char nrf_isInit()
{
	return nrf_isInitFlag;
}

// Enables the nRF24L01 chip
void nrf_enable()
{
	spi_writeGPIO(PIN_CE, 1);
}

// Disables the nRF24L01 chip
void nrf_disable()
{
	spi_writeGPIO(PIN_CE, 0);
}

// Write one byte into the given register
void nrf_configRegister(uint8_t reg, uint8_t value) 
{
	spi_obtainLock(NRF_SPI_KEY, 1);
	spi_writeGPIO(PIN_CSN, 0);
	
	unsigned char buf[2];
	buf[0] = W_REGISTER | (REGISTER_MASK & reg);
	buf[1] = value;
	spi_transfer(buf, 2);
  
	spi_writeGPIO(PIN_CSN, 1);
	spi_unlock(NRF_SPI_KEY);
}

// Reads an array of bytes from the given registers
void nrf_readRegister(uint8_t reg, uint8_t* value, uint8_t len) 
{
	int i;
	for(i=0; i<len; i++) value[i] = 0x00;
		
	spi_obtainLock(NRF_SPI_KEY, 1);
	spi_writeGPIO(PIN_CSN, 0);
	
	unsigned char buf = R_REGISTER | (REGISTER_MASK & reg);
	spi_transfer(&buf, 1);
	spi_transfer(value, len);
  
	spi_writeGPIO(PIN_CSN, 1);
	spi_unlock(NRF_SPI_KEY);
}

// Writes an array of bytes into the given registers
void nrf_writeRegister(uint8_t reg, uint8_t* value, uint8_t len) 
{
	spi_obtainLock(NRF_SPI_KEY, 1);
	spi_writeGPIO(PIN_CSN, 0);
	
	unsigned char buf = W_REGISTER | (REGISTER_MASK & reg);
	spi_transfer(&buf, 1);
	spi_transfer(value, len);
  
	spi_writeGPIO(PIN_CSN, 1);
	spi_unlock(NRF_SPI_KEY);
}

// Flush RX and TX FIFO
void nrf_flushRxTx()
{
	unsigned char buf;
	spi_obtainLock(NRF_SPI_KEY, 1);
	
	spi_writeGPIO(PIN_CSN, 0);
	buf = FLUSH_RX;
	spi_transfer(&buf, 1);
	spi_writeGPIO(PIN_CSN, 1);
	
	spi_writeGPIO(PIN_CSN, 0);
	buf = FLUSH_TX;
	spi_transfer(&buf, 1);
	spi_writeGPIO(PIN_CSN, 1);
	
	spi_unlock(NRF_SPI_KEY);
}

// Read the status register
uint8_t nrf_status()
{
	spi_obtainLock(NRF_SPI_KEY, 1);
	spi_writeGPIO(PIN_CSN, 0);

	unsigned char buf = R_REGISTER | (REGISTER_MASK & STATUS);
	spi_transfer(&buf, 1);

	spi_writeGPIO(PIN_CSN, 1);
	spi_unlock(NRF_SPI_KEY);

	return buf;
}

// Read the rx payload
void nrf_readRxPayload(uint8_t* value, uint8_t len)
{
	int i;
	for(i=0; i<len; i++) value[i] = 0x00;
		
	spi_obtainLock(NRF_SPI_KEY, 1);
	spi_writeGPIO(PIN_CSN, 0);
	
	unsigned char buf = R_RX_PAYLOAD;
	spi_transfer(&buf, 1);
	spi_transfer(value, len);
  
	spi_writeGPIO(PIN_CSN, 1);
	spi_unlock(NRF_SPI_KEY);
}

// Write the rx payload
void nrf_writeTxPayload(uint8_t* value, uint8_t len)
{
	spi_obtainLock(NRF_SPI_KEY, 1);
	spi_writeGPIO(PIN_CSN, 0);
	
	unsigned char buf = W_TX_PAYLOAD;
	spi_transfer(&buf, 1);
	spi_transfer(value, len);
  
	spi_writeGPIO(PIN_CSN, 1);
	spi_unlock(NRF_SPI_KEY);
}

// Closes the nRF24L01 interface
int nrf_close()
{
	//power down 
	if(nrf_isInitFlag == 1) nrf_disable();
		
	nrf_isInitFlag = 0;
	return 0;
}

// Micro second sleep function
static void nrf_sleep(long usec) {
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
	nanosleep(&ts, &ts);
}

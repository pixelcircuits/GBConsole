#ifndef SPI_H
#define SPI_H
#include <stdint.h>

// Setup and initialize the SPI interface
int spi_init(uint32_t clockSpeedHz);

// Checks if the SPI interface is initialized
uint8_t spi_isInit();

// Sets the direction of the given pin (1=input, 0=output)
void spi_setGPIODir(uint8_t pin, uint8_t dir);

// Sets the weak pullup or pulldown on the given pin (0=off, 1=down, 2=up)
void spi_setGPIOPud(uint8_t pin, uint8_t pud);

// Writes the output value of the given pin (1=high, 0=low)
void spi_writeGPIO(uint8_t pin, uint8_t val);

// Reads the output value of the given pin (1=high, 0=low)
uint8_t spi_readGPIO(uint8_t pin);

// Locks the SPI interface from use in other threads
void spi_obtainLock(uint32_t key, uint8_t disableCS);

// Unlocks the SPI interface for use in other threads
void spi_unlock(uint32_t key);

// Writes (and reads) an number of bytes to SPI
void spi_transfer(uint8_t* buf, uint32_t len);

// Starts a long read operation by writing the given bytes to SPI
void spi_read_start(uint8_t* buf, uint32_t len);

// Reads a single byte from SPI (continuation for long read)
uint8_t spi_read_cont();

// Ends a long read operation by writing the given bytes to SPI
void spi_read_end(uint8_t* buf, uint32_t len);

// Closes the SPI interface
int spi_close();


#endif /* SPI_H */

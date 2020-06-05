#include "gba.h"
#include "gba_cart.h"
#include "gba_save.h"
#include "gba_flash.h"
#include "egpio.h"
#include "spi.h"

#define _1(x)   (x)
#define _0(x)   ((unsigned char)~(x))

// Helper functions
static void gba_flash_writeAtmel(char* buffer, unsigned int length);
static void gba_flash_writeOther(char* buffer, unsigned int length);
static void gba_flash_writeBus(int address, char data);

// Reads the Flash/SRAM of a connected GBA cartridge
void gba_flash_read(char* buffer, unsigned int length)
{
	//read as much as possible from bank 0
	unsigned int numReads = length;
	if(length > GBA_SAVE_SIZE_512K) numReads = GBA_SAVE_SIZE_512K;
	gba_flash_readAt(buffer, 0, numReads);
	
	//read more from bank 1 if needed
	if(length > GBA_SAVE_SIZE_512K) {
		buffer += GBA_SAVE_SIZE_512K;
		length -= GBA_SAVE_SIZE_512K;
		gba_flash_readAt(buffer, GBA_SAVE_SIZE_512K, length);
	}
}

// Reads Save data like Flash or SRAM from the given address
void gba_flash_readAt(char* buffer, unsigned int start, unsigned int length)
{
	int i;
	
	//ensure default state
	gba_cart_powerUp();
	
	//pull GBA_CS2 pin low while we read data
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
	
	//switch to bank 1 if starting past 512K
	if(start >= GBA_SAVE_SIZE_512K) {
		egpio_setPortDir(EX_GPIO_PORTC, 0x00);
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0xB0);
		gba_flash_writeBus(0x0000, 0x01);
		gba_cart_delay(500000); //5ms
	}
		
	//read the data
	egpio_setPortDir(EX_GPIO_PORTC, 0xFF);
	unsigned int offset = start;
	if(start >= GBA_SAVE_SIZE_512K) offset -= GBA_SAVE_SIZE_512K;
	for(i = 0; i < length; i++) {
		
		//set the address
		unsigned int address = offset + i;
		egpio_writePortAB((char) address, (char) (address >> 8));
	
		//pull RD pin low while we read data
		spi_setSelectPin(GBA_SPI_RD, 0x00);
		buffer[i] = egpio_readPort(EX_GPIO_PORTC);
		spi_setSelectPin(GBA_SPI_RD, 0x01);
	}
		
	//switch back to bank 0 if starting past 512K
	if(start >= GBA_SAVE_SIZE_512K) {
		egpio_setPortDir(EX_GPIO_PORTC, 0x00);
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0xB0);
		gba_flash_writeBus(0x0000, 0x00);
		gba_cart_delay(500000); //5ms
	}
		
	//pull RD and GBA_CS2 back to high
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
}

// Writes to the Flash memory of a connected GBA cartridge
void gba_flash_write(char* buffer, unsigned int length)
{
	char manufacturerId, deviceId;
	char flashManufacturer = gba_flash_checkManufacturer(&manufacturerId, &deviceId);
	
	//ensure default state
	gba_cart_powerUp();
	
	//write flash according to manufacturer
	if(flashManufacturer == GBA_FLASH_MANUFACTURER_ATMEL) {
		gba_flash_writeAtmel(buffer, length);
	} else if(flashManufacturer == GBA_FLASH_MANUFACTURER_OTHER) {
		gba_flash_writeOther(buffer, length);
	}
	
	//pull GBA_CS2, RD and WR back to high
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
}

// Reads the manufacturer code of the flash chip
char gba_flash_checkManufacturer(char* manufacturerId, char* deviceId)
{	
	//ensure default state
	gba_cart_powerUp();
	
	//pull GBA_CS2 pin low while we write data
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
	
	//write the bus cycles for software id entry
	gba_flash_writeBus(0x5555, 0xAA);
	gba_flash_writeBus(0x2AAA, 0x55);
	gba_flash_writeBus(0x5555, 0x90);
	gba_cart_delay(500000); //5ms
	
	//read manufacturer id
	egpio_setPortDir(EX_GPIO_PORTC, 0xFF);
	egpio_writePortAB(0x00, 0x00);
	spi_setSelectPin(GBA_SPI_RD, 0x00);
	manufacturerId[0] = egpio_readPort(EX_GPIO_PORTC);
	spi_setSelectPin(GBA_SPI_RD, 0x01);
		
	//read device id
	egpio_writePortAB(0x01, 0x00);
	spi_setSelectPin(GBA_SPI_RD, 0x00);
	deviceId[0] = egpio_readPort(EX_GPIO_PORTC);
	spi_setSelectPin(GBA_SPI_RD, 0x01);
	
	//write the bus cycles for software id exit
	egpio_setPortDir(EX_GPIO_PORTC, 0x00);
	gba_flash_writeBus(0x5555, 0xAA);
	gba_flash_writeBus(0x2AAA, 0x55);
	gba_flash_writeBus(0x5555, 0xF0);
	gba_cart_delay(500000); //5ms
	
	//pull GBA_CS2 back to high
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
	
	//return the manufacturer
	if(manufacturerId[0] == 0x1F) return GBA_FLASH_MANUFACTURER_ATMEL;
	if(manufacturerId[0] == 0xBF) return GBA_FLASH_MANUFACTURER_OTHER;
	if(manufacturerId[0] == 0xC2) return GBA_FLASH_MANUFACTURER_OTHER;
	if(manufacturerId[0] == 0x32) return GBA_FLASH_MANUFACTURER_OTHER;
	if(manufacturerId[0] == 0x62) return GBA_FLASH_MANUFACTURER_OTHER;
	return GBA_FLASH_MANUFACTURER_UNKNOWN;
}

// Writes to the Flash memory of a connected GBA cartridge (atmel manufacturer)
static void gba_flash_writeAtmel(char* buffer, unsigned int length) {
	int i, j;
	
	//pull GBA_CS2 pin low while we write data
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
	
	for(i = 0; i < length; i += 128) {
		
		//write the bus cycles for sector program
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0xA0);
		for (j = 0; j < 128; j++) {
			gba_flash_writeBus(i + j, buffer[i]);
		}
		gba_cart_delay(2000000); //20ms
	}
}

// Writes to the Flash memory of a connected GBA cartridge (other manufacturer)
static void gba_flash_writeOther(char* buffer, unsigned int length) {
	int i;
	
	//pull GBA_CS2 pin low while we write data
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
	
	//cap the num data to read if we need to bank switch later
	unsigned int numWrites = length;
	if(numWrites > GBA_SAVE_SIZE_512K) numWrites = GBA_SAVE_SIZE_512K;
	for(i = 0; i < numWrites; i += 0x1000) {
		
		//write the bus cycles for sector erase
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0x80);
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(i, 0x30);
		gba_cart_delay(10000000); //25ms (100ms to be safe)
	}
	for(i = 0; i < numWrites; i++) {
		
		//write the bus cycles for byte program
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0xA0);
		gba_flash_writeBus(i, buffer[i]);
		gba_cart_delay(700); //7us (20us between each WR pulse)
	}
	
	//bank switch if there's still more to write
	if(length > GBA_SAVE_SIZE_512K) {
		
		//switch to bank 1
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0xB0);
		gba_flash_writeBus(0x0000, 0x01);
		gba_cart_delay(500000); //5ms
		
		//write the rest of the data
		numWrites = length - GBA_SAVE_SIZE_512K;
		for(i = 0; i < numWrites; i += 0x1000) {
			
			//write the bus cycles for sector erase
			gba_flash_writeBus(0x5555, 0xAA);
			gba_flash_writeBus(0x2AAA, 0x55);
			gba_flash_writeBus(0x5555, 0x80);
			gba_flash_writeBus(0x5555, 0xAA);
			gba_flash_writeBus(0x2AAA, 0x55);
			gba_flash_writeBus(i, 0x30);
			gba_cart_delay(10000000); //25ms (100ms to be safe)
		}
		for(i = 0; i < numWrites; i++) {
			
			//write the bus cycles for byte program
			gba_flash_writeBus(0x5555, 0xAA);
			gba_flash_writeBus(0x2AAA, 0x55);
			gba_flash_writeBus(0x5555, 0xA0);
			gba_flash_writeBus(i, buffer[GBA_SAVE_SIZE_512K + i]);
			gba_cart_delay(700); //7us (20us between each WR pulse)
		}
		
		//switch back to bank 0
		gba_flash_writeBus(0x5555, 0xAA);
		gba_flash_writeBus(0x2AAA, 0x55);
		gba_flash_writeBus(0x5555, 0xB0);
		gba_flash_writeBus(0x0000, 0x00);
		gba_cart_delay(500000); //5ms
	}
}

// Write a bus cycle to flash
static void gba_flash_writeBus(int address, char data) {
	egpio_writePortAB((char) address, (char) (address >> 8));
	egpio_writePort(EX_GPIO_PORTC, data);
		
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS) & _0(GBA_WR + GBA_CS2 + GBA_CLK + GBA_PWR));
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
}

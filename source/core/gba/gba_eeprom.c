#include "gba.h"
#include "gba_cart.h"
#include "gba_save.h"
#include "gba_eeprom.h"
#include "egpio.h"
#include "spi.h"

#define EEPROM_READ 0xC0
#define EEPROM_WRITE 0x80

#define _1(x)   (x)
#define _0(x)   ((unsigned char)~(x))

// Helper functions
static void gba_eeprom_writeByte(char byte, char includeStop);
static char gba_eeprom_readByte(char ignoreStart);

// Reads the EEPROM of a connected GBA cartridge
void gba_eeprom_read(char* buffer, unsigned int length)
{
	int i, j;
	
	//ensure default state
	gba_cart_powerUp();
	
	//determine if 4K or 64K and start loop
	int index = 0;
	int numReads = 64;
	if(length > GBA_SAVE_SIZE_4K) numReads = 1024;
	for(j = 0; j < numReads && index < length; j++) {
		
		//setup for EEPROM write
		egpio_setPortDir(EX_GPIO_PORTA, 0x00);
		egpio_writePort(EX_GPIO_PORTC, 0x80);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_WR + GBA_CS2) & _0(GBA_CS + GBA_CLK + GBA_PWR));
		
		//write the read command to EEPROM
		if(length > GBA_SAVE_SIZE_4K) {
			gba_eeprom_writeByte(EEPROM_READ | (char)((j >> 8) & 0x03), 0);
			gba_eeprom_writeByte((char) j, 1);
		} else {
			gba_eeprom_writeByte(EEPROM_READ | (char) j, 1);
		}
		
		//switch back to defaults
		egpio_writePort(EX_GPIO_PORTC, 0x00);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
		
		//setup for EEPROM read
		egpio_setPortDir(EX_GPIO_PORTA, 0x01);
		egpio_writePort(EX_GPIO_PORTC, 0x80);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_WR + GBA_CS2) & _0(GBA_CS + GBA_CLK + GBA_PWR));
		
		//clock in the 64 bits of data
		buffer[index++] = gba_eeprom_readByte(1);
		for(i = 1; i < 8; i++) {
			buffer[index++] = gba_eeprom_readByte(0);
		}
		
		//back to defaults
		egpio_writePort(EX_GPIO_PORTC, 0x00);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
	}
}

// Writes to the EEPROM of a connected GBA cartridge
void gba_eeprom_write(char* buffer, unsigned int length)
{
	int i, j;
	
	//ensure default state
	gba_cart_powerUp();
	
	//determine if 4K or 64K and start loop
	int index = 0;
	int numWrites = 64;
	if(length > GBA_SAVE_SIZE_4K) numWrites = 1024;
	for(j = 0; j < numWrites; j++) {
		
		//setup for EEPROM write
		egpio_setPortDir(EX_GPIO_PORTA, 0x00);
		egpio_writePort(EX_GPIO_PORTC, 0x80);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_WR + GBA_CS2) & _0(GBA_CS + GBA_CLK + GBA_PWR));
		
		//write the write command to EEPROM
		if(length > GBA_SAVE_SIZE_4K) {
			gba_eeprom_writeByte(EEPROM_WRITE | (char)((j >> 8) & 0x03), 0);
			gba_eeprom_writeByte((char) j, 0);
		} else {
			gba_eeprom_writeByte(EEPROM_WRITE | (char) j, 0);
		}
		
		//clock out the 64 bits of data
		for(i = 0; i < 7; i++) {
			gba_eeprom_writeByte(buffer[index++], 0);
		}
		gba_eeprom_writeByte(buffer[index++], 1);
		
		//back to defaults
		egpio_writePort(EX_GPIO_PORTC, 0x00);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
		
		//must wait before next write
		gba_cart_delay(700000); //7ms
	}
}

// Clocks a bit out to the EEPROM
static void gba_eeprom_writeByte(char byte, char includeStop) {
	int i;
	for(i=0; i<8; i++) {
		egpio_writePort(EX_GPIO_PORTA, (byte >> (7-i)) & 0x01);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS2) & _0(GBA_CS + GBA_WR + GBA_CLK + GBA_PWR));
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_WR + GBA_CS2) & _0(GBA_CS + GBA_CLK + GBA_PWR));
	}
	if(includeStop) {
		egpio_writePort(EX_GPIO_PORTA, 0x00);
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS2) & _0(GBA_CS + GBA_WR + GBA_CLK + GBA_PWR));
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_WR + GBA_CS2) & _0(GBA_CS + GBA_CLK + GBA_PWR));
	}
}

// Clocks a bit in form the EEPROM
static char gba_eeprom_readByte(char ignoreStart) {
	int i;
	char byte = 0x00;
	if(ignoreStart) {
		for(i=0; i<4; i++) {
			spi_writeGPIO(GBA_GPIO_RD, 0x00);
			gba_cart_delay(200); //600ns
			spi_writeGPIO(GBA_GPIO_RD, 0x01);
			gba_cart_delay(200); //600ns
		}
	}
	for(i=0; i<8; i++) {
		spi_writeGPIO(GBA_GPIO_RD, 0x00);
		gba_cart_delay(200); //600ns
		byte |= egpio_readPort(EX_GPIO_PORTA) << (7-i);
		spi_writeGPIO(GBA_GPIO_RD, 0x01);
		gba_cart_delay(200); //600ns
	}
	return byte;
}

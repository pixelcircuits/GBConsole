#include "gba.h"
#include "gba_cart.h"
#include "gba_save.h"
#include "gba_sram.h"
#include "egpio.h"
#include "spi.h"

#define _1(x)   (x)
#define _0(x)   ((unsigned char)~(x))

// Reads the SRAM of a connected GBA cartridge
void gba_sram_read(char* buffer, unsigned int length)
{
	gba_sram_readAt(buffer, 0, length);
}

// Reads the SRAM from the given address of a connected GBA cartridge
void gba_sram_readAt(char* buffer, unsigned int start, unsigned int length)
{
	int i;
	
	//ensure default state
	gba_cart_powerUp();
	
	//pull GBA_CS2 pin low while we read data
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
		
	//read the data
	egpio_setPortDir(EX_GPIO_PORTC, 0xFF);
	if(start >= GBA_SAVE_SIZE_512K) start -= GBA_SAVE_SIZE_512K;
	if(length > GBA_SAVE_SIZE_512K) length = GBA_SAVE_SIZE_512K;
	for(i = 0; i < length; i++) {
		
		//set the address
		unsigned int address = start + i;
		egpio_writePortAB((char) address, (char) (address >> 8));
	
		//pull RD pin low while we read data
		spi_setSelectPin(GBA_SPI_RD, 0x00);
		buffer[i] = egpio_readPort(EX_GPIO_PORTC);
		spi_setSelectPin(GBA_SPI_RD, 0x01);
	}
		
	//pull RD and GBA_CS2 back to high
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
}

// Writes to the SRAM of a connected GBA cartridge
void gba_sram_write(char* buffer, unsigned int length)
{
	int i;
	
	//ensure default state
	gba_cart_powerUp();
	
	//pull GBA_CS2 pin low while we write data
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
	for(i = 0; i < length; i++) {
		
		//set the address and data
		egpio_writePortAB((char) i, (char) (i >> 8));
		egpio_writePort(EX_GPIO_PORTC, buffer[i]);
	
		//pull WR pin low to write data
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS) & _0(GBA_WR + GBA_CS2 + GBA_CLK + GBA_PWR));
		gba_cart_delay(100); //1us
		egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR) & _0(GBA_CS2 + GBA_CLK + GBA_PWR));
	}
		
	//pull WR and GBA_CS2 back to high
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
}

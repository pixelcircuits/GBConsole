#include "gbc.h"
#include "gbc_cart.h"
#include "gbc_rom.h"
#include "egpio.h"
#include "spi.h"

#define INV(x)   ((unsigned char)~(x))

// Read the ROM of a connected GB cartridge at the given start and length
void gbc_rom_readAt(char* buffer, unsigned int start, unsigned int length)
{
	int i;
	
	//determine ROM vs RAM
	if(start < GBC_32K) {
		if((start + length) > GBC_32K) length = GBC_32K - start;
	} else {
		if(start > GBC_64K) start = GBC_64K;
		if((start + length) > GBC_64K) length = GBC_64K - start;
	}
	
	//ensure default state
	gbc_cart_powerUp();
	egpio_setPortDir(EX_GPIO_PORTC, 0xFF);
	
	//pull GBC_SPI_RD pin (and CSRAM if appropriate) low while we read data
	spi_setSelectPin(GBC_SPI_RD, 0x00);
	if(start >= GBC_32K) egpio_writePort(EX_GPIO_PORTD, INV(GBC_CSRAM | GBC_PWR));
	
	//read ROM data
	for(i = 0; i < length; i++) {
		
		//write address and read data
		char addr0 = (char) (start + i);
		char addr1 = (char) ((start + i) >> 8);
		if(i==0 || addr0 == 0) egpio_writePortAB(addr0, addr1);
		else egpio_writePort(EX_GPIO_PORTA, addr0);
		buffer[i] = egpio_readPort(EX_GPIO_PORTC);
	}
	
	//pull RD and CSRAM back to high
	egpio_writePort(EX_GPIO_PORTD, INV(GBC_PWR));
	spi_setSelectPin(GBC_SPI_RD, 0x01);
}

// Wrties to the RAM of a connected GB cartridge at the given start and length
void gbc_rom_writeAt(char* buffer, unsigned int start, unsigned int length)
{
	int i;
	
	//cleanse data
	if(start < GBC_32K) return;
	if(start > GBC_64K) start = GBC_64K;
	if((start + length) > GBC_64K) length = GBC_64K - start;
	
	//ensure default state
	gbc_cart_powerUp();
	
	//pull CSRAM pin low while we write data
	egpio_writePort(EX_GPIO_PORTD, INV(GBC_CSRAM | GBC_PWR));
	
	//write RAM data
	for(i = 0; i < length; i++) {
		
		//write address and data
		char addr0 = (char) (start + i);
		char addr1 = (char) ((start + i) >> 8);
		if(i==0 || addr0 == 0) egpio_writePortAB(addr0, addr1);
		else egpio_writePort(EX_GPIO_PORTA, addr0);
		egpio_writePort(EX_GPIO_PORTC, buffer[i]);
		
		//toggle the WR line
		egpio_writePort(EX_GPIO_PORTD, INV(GBC_CSRAM | GBC_WR | GBC_PWR));
		egpio_writePort(EX_GPIO_PORTD, INV(GBC_CSRAM | GBC_PWR));
	}
	
	//pull WR and CSRAM back to high
	egpio_writePort(EX_GPIO_PORTD, INV(GBC_PWR));
}

// Writes to ROM for bank switching
void gbc_rom_writeByte(char byte, unsigned int address)
{
	if(address > GBC_64K) address = GBC_64K;
	char addr0 = (char) address;
	char addr1 = (char) (address >> 8);
	
	//ensure default state
	gbc_cart_powerUp();
	
	//set data
	egpio_writePortAll(addr0, addr1, byte, INV(GBC_PWR));
	
	//pull WR low to write
	egpio_writePort(EX_GPIO_PORTD, INV(GBC_WR | GBC_PWR));
	egpio_writePort(EX_GPIO_PORTD, INV(GBC_PWR));
}


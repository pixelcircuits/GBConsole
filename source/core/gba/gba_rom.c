#include "gba.h"
#include "gba_cart.h"
#include "gba_rom.h"
#include "egpio.h"
#include "spi.h"

#define INV(x)   ((unsigned char)~(x))

// Read the ROM of a connected GBA cartridge at the given start and length
void gba_rom_readAt(char* buffer, unsigned int start, unsigned int length)
{
	int i;
	char addr0 = (char) (start / 2);
	char addr1 = (char) ((start / 2) >> 8);
	char addr2 = (char) ((start / 2) >> 16);
	
	//ensure default state
	gba_cart_powerUp();
	
	//pull GBA_CS low to latch the address
	egpio_writePortAll(addr0, addr1, addr2, INV(GBA_PWR));
	egpio_writePort(EX_GPIO_PORTD, INV(GBA_CS | GBA_PWR));
	
	//set data pins as inputs now
	egpio_setPortDirAll(0xFF, 0xFF, 0x00, INV(GBA_CS | GBA_WR | GBA_CS2 | GBA_PWR));

	//read all ROM data
	egpio_continuousReadAB_start();
	for(i = 0; i < length; i += 2) {
		
		//pull RD pin low while we read data
		spi_setSelectPin(GBA_SPI_RD, 0x00);
		egpio_continuousReadAB_cont(buffer+i);
		spi_setSelectPin(GBA_SPI_RD, 0x01);
	}
	egpio_continuousReadAB_end();
	
	//pull GBA_CS back to high
	egpio_writePort(EX_GPIO_PORTD, INV(GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
}

// Try to figure out the connected GBA cartridge ROM size
int gba_rom_determineSize()
{
	//note: based on the assumption that the rom chips internal counter rolls back to zero (the header)
	int i, match;
	char buffer[200];
	
	//read header
	char header[192];
	gba_rom_readAt(header, 0x00, 192);
	
	//4MB?
	match = 1;
	gba_rom_readAt(buffer, GBA_ROM_SIZE_4MB - 8, 200);
	for(i = 0; i < 192; i++) {
		if(buffer[i + 8] != header[i]) {
			match = 0;
			break;
		}
	}
	if(match == 1) return GBA_ROM_SIZE_4MB;
	
	//8MB?
	match = 1;
	gba_rom_readAt(buffer, GBA_ROM_SIZE_8MB - 8, 200);
	for(i = 0; i < 192; i++) {
		if(buffer[i + 8] != header[i]) {
			match = 0;
			break;
		}
	}
	if(match == 1) return GBA_ROM_SIZE_8MB;
	
	//16MB?
	match = 1;
	gba_rom_readAt(buffer, GBA_ROM_SIZE_16MB - 8, 200);
	for(i = 0; i < 192; i++) {
		if(buffer[i + 8] != header[i]) {
			match = 0;
			break;
		}
	}
	if(match == 1) return GBA_ROM_SIZE_16MB;
	
	return GBA_ROM_SIZE_32MB;
}

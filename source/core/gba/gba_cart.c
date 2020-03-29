#include "gba.h"
#include "gba_cart.h"
#include "egpio.h"
#include "spi.h"

#define GBA_PWR_ON_DELAY 1000000
#define INV(x)   ((unsigned char)~(x))

// Data
static char gba_power = 0;

// Powers up the cartridge slot to the null state
void gba_cart_powerUp()
{
	//switch on power while setting pins to default state all pins
	egpio_setPortDirAll(0x00, 0x00, 0x00, INV(GBA_CS | GBA_WR | GBA_CS2 | GBA_PWR));
	egpio_writePortAll(0x00, 0x00, 0x00, INV(GBA_PWR));
	spi_setSelectPin(GBA_SPI_RD, 0x01);
	
	//wait for things to power up
	if(gba_power == 0) {
		gba_cart_delay(GBA_PWR_ON_DELAY);
		gba_power = 1;
	}
}

// Powers down the cartridge slot to an all ground state
void gba_cart_powerDown()
{
	//disconnect pins
	egpio_setPortPullupAll(0x00, 0x00, 0x00, GBA_DTSW);
	egpio_setPortDirAll(0xFF, 0xFF, 0xFF, INV(GBA_PWR));
	spi_disableSelectPin(GBA_SPI_RD);
	
	//switch off power
	egpio_writePortAll(0x00, 0x00, 0x00, GBA_PWR);
	
	//gnd all pins
	spi_setSelectPin(GBA_SPI_RD, 0x00);
	egpio_setPortDirAll(0x00, 0x00, 0x00, GBA_DTSW);
	spi_enableSelectPin(GBA_SPI_RD);
	gba_power = 0;
}

// Util function to delay for the given period
void gba_cart_delay(int t) {
	volatile int i=0;
	for(;i<t;i++);
}

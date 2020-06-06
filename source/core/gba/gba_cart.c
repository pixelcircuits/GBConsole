#include "gba.h"
#include "gba_cart.h"
#include "egpio.h"
#include "spi.h"

#define GBA_PWR_ON_DELAY 1000000
#define _1(x)   (x)
#define _0(x)   ((unsigned char)~(x))

// Data
static char gba_power = 0;

// Powers up the cartridge slot to the null state
void gba_cart_powerUp()
{
	//switch on power while setting pins to default state all pins
	egpio_setPortDirAll(0x00, 0x00, 0x00, _1(GBA_IRQ + GBA_DTSW) | _0(GBA_CS + GBA_WR + GBA_CS2 + GBA_CLK + GBA_PWR)); //default: 1
	egpio_writePortAll(0x00, 0x00, 0x00, _1(GBA_CS + GBA_WR + GBA_CS2) & _0(GBA_CLK + GBA_PWR)); //default: 0
	spi_setGPIODir(GBA_GPIO_RD, 0x00);
	spi_writeGPIO(GBA_GPIO_RD, 0x01);
	
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
	egpio_setPortDirAll(0xFF, 0xFF, 0xFF, _1(GBA_CS + GBA_WR + GBA_CS2 + GBA_CLK + GBA_IRQ + GBA_DTSW) | _0(GBA_PWR)); //default: 1
	spi_setGPIODir(GBA_GPIO_RD, 0x01);
	
	//switch off power
	egpio_writePort(EX_GPIO_PORTD, _1(GBA_PWR));
	
	//gnd all pins
	egpio_writePortAll(0x00, 0x00, 0x00, _1(GBA_PWR) & _0(GBA_CS + GBA_WR + GBA_CS2 + GBA_CLK)); //default: 0
	spi_writeGPIO(GBA_GPIO_RD, 0x00);
	egpio_setPortDirAll(0x00, 0x00, 0x00, _1(GBA_IRQ + GBA_DTSW) | _0(GBA_CS + GBA_WR + GBA_CS2 + GBA_CLK + GBA_PWR)); //default: 1
	spi_setGPIODir(GBA_GPIO_RD, 0x00);
	gba_power = 0;
}

// Util function to delay for the given period
void gba_cart_delay(int t) {
	volatile int i=0;
	for(;i<t;i++);
}

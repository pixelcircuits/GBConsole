#include "gbc.h"
#include "gbc_cart.h"
#include "egpio.h"
#include "spi.h"

#define GBC_PWR_ON_DELAY 1000000
#define _1(x)   (x)
#define _0(x)   ((unsigned char)~(x))

// Data
static char gbc_power = 0;

// Powers up the cartridge slot to the null state
void gbc_cart_powerUp()
{
	//switch on power while setting pins to default state all pins
	egpio_setPortDirAll(0x00, 0x00, 0x00, _1(GBC_AUD + GBC_DTSW) | _0(GBC_CSRAM + GBC_WR + GBC_RST + GBC_CLK + GBC_PWR)); //default: 1
	egpio_writePortAll(0x00, 0x00, 0x00, _1(GBC_CSRAM + GBC_WR + GBC_RST) & _0(GBC_CLK + GBC_PWR)); //default: 0
	spi_setSelectPin(GBC_SPI_RD, 0x01);
	
	//wait for things to power up
	if(gbc_power == 0) {
		gbc_cart_delay(GBC_PWR_ON_DELAY);
		gbc_power = 1;
	}
}

// Powers down the cartridge slot to an all ground state
void gbc_cart_powerDown()
{
	//disconnect pins
	egpio_setPortPullupAll(0x00, 0x00, 0x00, GBC_DTSW);
	egpio_setPortDirAll(0xFF, 0xFF, 0xFF, _1(GBC_CSRAM + GBC_WR + GBC_RST + GBC_CLK + GBC_AUD + GBC_DTSW) | _0(GBC_PWR)); //default: 1
	spi_disableSelectPin(GBC_SPI_RD);
	
	//switch off power
	egpio_writePort(EX_GPIO_PORTD, _1(GBC_PWR));
	
	//gnd all pins
	egpio_writePortAll(0x00, 0x00, 0x00, _1(GBC_PWR) & _0(GBC_CSRAM + GBC_WR + GBC_RST + GBC_CLK)); //default: 0
	spi_setSelectPin(GBC_SPI_RD, 0x00);
	egpio_setPortDirAll(0x00, 0x00, 0x00, _1(GBC_AUD + GBC_DTSW) | _0(GBC_CSRAM + GBC_WR + GBC_RST + GBC_CLK + GBC_PWR)); //default: 1
	spi_enableSelectPin(GBC_SPI_RD);
	gbc_power = 0;
}

// Delay function
void gbc_cart_delay(int t) {
	volatile int i=0;
	for(;i<t;i++);
}

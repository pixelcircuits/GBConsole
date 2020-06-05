#include "egpio.h"
#include "spi.h"
#include <stdio.h>

#define SPI_CLK_SPEED 10000000

#define CHIPA_READ   0x49
#define CHIPA_WRITE  0x48
#define CHIPB_READ   0x43
#define CHIPB_WRITE  0x42

#define CHIP_REG_CONFIG  0x0A
#define CHIP_REG_DIR     0x00
#define CHIP_REG_DIR2    0x01
#define CHIP_REG_GPIO    0x12
#define CHIP_REG_GPIO2   0x13
#define CHIP_REG_GPPU    0x0C
#define CHIP_REG_GPPU2   0x0D

#define CHIP_CONFIG_SEQOP  0x20
#define CHIP_CONFIG_HAEN   0x08

// Data
static char egpio_isInitFlag = 0;

// Setup and initialize the expanded gpio
int egpio_init() 
{
	//already initialized?
	if(egpio_isInitFlag == 1) return 0;
	
	//check dependencies
	if(!spi_isInit()) {
		fprintf(stderr, "egpio_init: SPI dependency is not initialized\n");
		egpio_close();
		return 1;
	}
	
	//config chip A and B
	unsigned char buffer[3];
	buffer[0] = CHIPA_WRITE; buffer[1] = CHIP_REG_CONFIG; buffer[2] = CHIP_CONFIG_SEQOP | CHIP_CONFIG_HAEN;
	spi_transfer(buffer, 3);
	buffer[0] = CHIPB_WRITE; buffer[1] = CHIP_REG_CONFIG; buffer[2] = CHIP_CONFIG_SEQOP | CHIP_CONFIG_HAEN;
	spi_transfer(buffer, 3);
	
	//set defaults
	egpio_setPortDirAll(0xFF, 0xFF, 0xFF, 0xFF);
	egpio_setPortPullupAll(0x00, 0x00, 0x00, 0x00);
	
	egpio_isInitFlag = 1;
	return 0;
}

// Checks if the expanded GPIO interface is initialized
char egpio_isInit()
{
	return egpio_isInitFlag;
}

// Sets the direction on the given ports pins (1=input, 0=output)
void egpio_setPortDir(uint8_t port, uint8_t dir)
{
	unsigned char chip = CHIPA_WRITE;
	if(port == EX_GPIO_PORTC || port == EX_GPIO_PORTD) chip = CHIPB_WRITE;
	unsigned char reg = CHIP_REG_DIR;
	if(port == EX_GPIO_PORTB || port == EX_GPIO_PORTD) reg = CHIP_REG_DIR2;
	unsigned char buffer[3] = { chip, reg, dir };
	spi_transfer(buffer, 3);
}

// Sets the direction on all port pins (1=input, 0=output)
void egpio_setPortDirAll(uint8_t dirA, uint8_t dirB, uint8_t dirC, uint8_t dirD)
{
	unsigned char buffer[4];
	buffer[0] = CHIPA_WRITE; buffer[1] = CHIP_REG_DIR; buffer[2] = dirA; buffer[3] = dirB;
	spi_transfer(buffer, 4);
	buffer[0] = CHIPB_WRITE; buffer[1] = CHIP_REG_DIR; buffer[2] = dirC; buffer[3] = dirD;
	spi_transfer(buffer, 4);
}

// Sets the direction on port A and B (1=input, 0=output)
void egpio_setPortDirAB(uint8_t dirA, uint8_t dirB)
{
	unsigned char buffer[4];
	buffer[0] = CHIPA_WRITE; buffer[1] = CHIP_REG_DIR; buffer[2] = dirA; buffer[3] = dirB;
	spi_transfer(buffer, 4);
	
}

// Sets the direction on port C and D (1=input, 0=output)
void egpio_setPortDirCD(uint8_t dirC, uint8_t dirD)
{
	unsigned char buffer[4];
	buffer[0] = CHIPB_WRITE; buffer[1] = CHIP_REG_DIR; buffer[2] = dirC; buffer[3] = dirD;
	spi_transfer(buffer, 4);
}

// Sets the weak pullup on the given ports pins (1=on, 0=off)
void egpio_setPortPullup(uint8_t port, uint8_t pullup)
{
	unsigned char chip = CHIPA_WRITE;
	if(port == EX_GPIO_PORTC || port == EX_GPIO_PORTD) chip = CHIPB_WRITE;
	unsigned char reg = CHIP_REG_GPPU;
	if(port == EX_GPIO_PORTB || port == EX_GPIO_PORTD) reg = CHIP_REG_GPPU2;
	unsigned char buffer[3] = { chip, reg, pullup };
	spi_transfer(buffer, 3);
}

// Sets the weak pullup on all port pins (1=on, 0=off)
void egpio_setPortPullupAll(uint8_t pullupA, uint8_t pullupB, uint8_t pullupC, uint8_t pullupD)
{
	unsigned char buffer[4];
	buffer[0] = CHIPA_WRITE; buffer[1] = CHIP_REG_GPPU; buffer[2] = pullupA; buffer[3] = pullupB;
	spi_transfer(buffer, 4);
	buffer[0] = CHIPB_WRITE; buffer[1] = CHIP_REG_GPPU; buffer[2] = pullupC; buffer[3] = pullupD;
	spi_transfer(buffer, 4);
}

// Writes the output on the given ports pins (1=high, 0=low)
void egpio_writePort(uint8_t port, uint8_t val)
{
	unsigned char chip = CHIPA_WRITE;
	if(port == EX_GPIO_PORTC || port == EX_GPIO_PORTD) chip = CHIPB_WRITE;
	unsigned char reg = CHIP_REG_GPIO;
	if(port == EX_GPIO_PORTB || port == EX_GPIO_PORTD) reg = CHIP_REG_GPIO2;
	unsigned char buffer[3] = { chip, reg, val };
	spi_transfer(buffer, 3);
}

// Writes the output on all port pins (1=high, 0=low)
void egpio_writePortAll(uint8_t valA, uint8_t valB, uint8_t valC, uint8_t valD)
{
	unsigned char buffer[4];
	buffer[0] = CHIPA_WRITE; buffer[1] = CHIP_REG_GPIO; buffer[2] = valA; buffer[3] = valB;
	spi_transfer(buffer, 4);
	buffer[0] = CHIPB_WRITE; buffer[1] = CHIP_REG_GPIO; buffer[2] = valC; buffer[3] = valD;
	spi_transfer(buffer, 4);
}

// Writes the output on port A and B (1=high, 0=low)
void egpio_writePortAB(uint8_t valA, uint8_t valB)
{
	unsigned char buffer[4];
	buffer[0] = CHIPA_WRITE; buffer[1] = CHIP_REG_GPIO; buffer[2] = valA; buffer[3] = valB;
	spi_transfer(buffer, 4);
}

// Writes the output on port C and D (1=high, 0=low)
void egpio_writePortCD(uint8_t valC, uint8_t valD)
{
	unsigned char buffer[4];
	buffer[0] = CHIPB_WRITE; buffer[1] = CHIP_REG_GPIO; buffer[2] = valC; buffer[3] = valD;
	spi_transfer(buffer, 4);
}

// Reads the values on the given ports pins (1=high, 0=low)
char egpio_readPort(uint8_t port)
{
	unsigned char chip = CHIPA_READ;
	if(port == EX_GPIO_PORTC || port == EX_GPIO_PORTD) chip = CHIPB_READ;
	unsigned char reg = CHIP_REG_GPIO;
	if(port == EX_GPIO_PORTB || port == EX_GPIO_PORTD) reg = CHIP_REG_GPIO2;
	unsigned char buffer[3] = { chip, reg, 0x00 };
	spi_transfer(buffer, 3);
	return buffer[2];
}

// Start a continuous read operation on ports A and B
void egpio_continuousReadAB_start()
{
	unsigned char buffer[2] = { CHIPA_READ, CHIP_REG_GPIO };
	spi_read_start(buffer, 2);
}

// Continues a continuous read operation on ports A and B
void egpio_continuousReadAB_cont(char* buff)
{
	buff[0] = spi_read_cont();
	buff[1] = spi_read_cont();
}

// Start a continuous read operation on ports A and B
void egpio_continuousReadAB_end()
{
	spi_read_end(0, 0);
}

// Closes the expanded GPIO interface
int egpio_close() 
{
	egpio_isInitFlag = 0;
	return 0;
}

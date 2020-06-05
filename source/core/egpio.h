#ifndef EGPIO_H
#define EGPIO_H
#include <stdint.h>

#define EX_GPIO_PORTA  0x00
#define EX_GPIO_PORTB  0x01
#define EX_GPIO_PORTC  0x02
#define EX_GPIO_PORTD  0x03

// Setup and initialize the expanded GPIO
int egpio_init();

// Checks if the expanded GPIO interface is initialized
char egpio_isInit();

// Sets the direction on the given ports pins (1=input, 0=output)
void egpio_setPortDir(uint8_t port, uint8_t dir);

// Sets the direction on all port pins (1=input, 0=output)
void egpio_setPortDirAll(uint8_t dirA, uint8_t dirB, uint8_t dirC, uint8_t dirD);

// Sets the direction on port A and B (1=input, 0=output)
void egpio_setPortDirAB(uint8_t dirA, uint8_t dirB);

// Sets the direction on port C and D (1=input, 0=output)
void egpio_setPortDirCD(uint8_t dirC, uint8_t dirD);

// Sets the weak pullup on the given ports pins (1=on, 0=off)
void egpio_setPortPullup(uint8_t port, uint8_t pullup);

// Sets the weak pullup on all port pins (1=on, 0=off)
void egpio_setPortPullupAll(uint8_t pullupA, uint8_t pullupB, uint8_t pullupC, uint8_t pullupD);

// Writes the output on the given ports pins (1=high, 0=low)
void egpio_writePort(uint8_t port, uint8_t val);

// Writes the output on all port pins (1=high, 0=low)
void egpio_writePortAll(uint8_t valA, uint8_t valB, uint8_t valC, uint8_t valD);

// Writes the output on port A and B (1=high, 0=low)
void egpio_writePortAB(uint8_t valA, uint8_t valB);

// Writes the output on port C and D (1=high, 0=low)
void egpio_writePortCD(uint8_t valC, uint8_t valD);

// Reads the values on the given ports pins (1=high, 0=low)
char egpio_readPort(uint8_t port);

// Start a continuous read operation on ports A and B
void egpio_continuousReadAB_start();

// Continues a continuous read operation on ports A and B
void egpio_continuousReadAB_cont(char* buff);

// Start a continuous read operation on ports A and B
void egpio_continuousReadAB_end();

// Closes the expanded GPIO interface
int egpio_close();


#endif /* EGPIO_H */

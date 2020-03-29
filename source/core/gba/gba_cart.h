#ifndef GBA_CART_H
#define GBA_CART_H

#define GBA_CS 0x01
#define GBA_RD 0x02
#define GBA_WR 0x04
#define GBA_CS2 0x08
#define GBA_DTSW 0x40
#define GBA_PWR 0x80
#define GBA_SPI_RD 25

// Powers up the cartridge slot to the null state
void gba_cart_powerUp();

// Powers down the cartridge slot to an all ground state
void gba_cart_powerDown();

// Util function to delay for the given period
void gba_cart_delay(int t);

#endif /* GBA_CART_H */

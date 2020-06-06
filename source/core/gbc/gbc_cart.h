#ifndef GBC_CART_H
#define GBC_CART_H

#define GBC_CSRAM 0x01
#define GBC_RD 0x02
#define GBC_WR 0x04
#define GBC_RST 0x08
#define GBC_CLK 0x10
#define GBC_AUD 0x20
#define GBC_DTSW 0x40
#define GBC_PWR 0x80
#define GBC_GPIO_RD 25

// Powers up the cartridge slot to the null state
void gbc_cart_powerUp();

// Powers down the cartridge slot to an all ground state
void gbc_cart_powerDown();

// Util function to delay for the given period
void gbc_cart_delay(int t);

#endif /* GBC_CART_H */

#ifndef GBC_MBC3_H
#define GBC_MBC3_H

// Read ROM for MBC3 Memory Controller
unsigned int gbc_mbc3_readROM(char* buffer, unsigned int length);

// Read RAM for MBC3 Memory Controller
unsigned int gbc_mbc3_readRAM(char* buffer, unsigned int length);

// Write RAM for MBC3 Memory Controller
unsigned int gbc_mbc3_writeRAM(char* buffer, unsigned int length);

#endif /* GBC_MBC3_H */

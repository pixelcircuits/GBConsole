#ifndef GBC_MBC5_H
#define GBC_MBC5_H

// Read ROM for MBC5 Memory Controller
unsigned int gbc_mbc5_readROM(char* buffer, unsigned int length);

// Read RAM for MBC5 Memory Controller
unsigned int gbc_mbc5_readRAM(char* buffer, unsigned int length);

// Write RAM for MBC5 Memory Controller
unsigned int gbc_mbc5_writeRAM(char* buffer, unsigned int length);

#endif /* GBC_MBC5_H */

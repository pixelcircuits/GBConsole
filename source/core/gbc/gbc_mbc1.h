#ifndef GBC_MBC1_H
#define GBC_MBC1_H

// Read ROM for MBC1 Memory Controller
unsigned int gbc_mbc1_readROM(char* buffer, unsigned int length);

// Read RAM for MBC1 Memory Controller
unsigned int gbc_mbc1_readRAM(char* buffer, unsigned int length);

// Write RAM for MBC1 Memory Controller
unsigned int gbc_mbc1_writeRAM(char* buffer, unsigned int length);

#endif /* GBC_MBC1_H */

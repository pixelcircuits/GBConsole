#ifndef GBC_ROM_H
#define GBC_ROM_H

#define GBC_2K 2048
#define GBC_8K 8192
#define GBC_16K 16384
#define GBC_32K 32768
#define GBC_64K 65536
#define GBC_128K 131072
#define GBC_256K 262144
#define GBC_512K 524288
#define GBC_1M 1048576
#define GBC_2M 2097152
#define GBC_4M 4194304
#define GBC_8M 8388608
#define GBC_1p1M 1179648
#define GBC_1p2M 1310720
#define GBC_1p5M 1572864

// Read the ROM of a connected GB cartridge at the given start and length
void gbc_rom_readAt(char* buffer, unsigned int start, unsigned int length);

// Wrties to the RAM of a connected GB cartridge at the given start and length
void gbc_rom_writeAt(char* buffer, unsigned int start, unsigned int length);

// Writes to ROM for bank switching
void gbc_rom_writeByte(char byte, unsigned int address);

#endif /* GBC_ROM_H */

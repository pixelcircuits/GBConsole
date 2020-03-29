#ifndef GBA_ROM_H
#define GBA_ROM_H

#define GBA_ROM_SIZE_4MB 4194304
#define GBA_ROM_SIZE_8MB 8388608
#define GBA_ROM_SIZE_16MB 16777216
#define GBA_ROM_SIZE_32MB 33554432

// Read the ROM of a connected GBA cartridge at the given start and length
void gba_rom_readAt(char* buffer, unsigned int start, unsigned int length);

// Try to figure out the connected GBA cartridge ROM size
int gba_rom_determineSize();

#endif /* GBA_ROM_H */

#ifndef GBA_SRAM_H
#define GBA_SRAM_H

// Reads the SRAM of a connected GBA cartridge
void gba_sram_read(char* buffer, unsigned int length);

// Reads the SRAM from the given address of a connected GBA cartridge
void gba_sram_readAt(char* buffer, unsigned int start, unsigned int length);

// Writes to the SRAM of a connected GBA cartridge
void gba_sram_write(char* buffer, unsigned int length);

#endif /* GBA_SRAM_H */

#ifndef GBA_FLASH_H
#define GBA_FLASH_H

#define GBA_FLASH_MANUFACTURER_ATMEL 0x00
#define GBA_FLASH_MANUFACTURER_OTHER 0x01
#define GBA_FLASH_MANUFACTURER_UNKNOWN 0x03

// Reads the Flash of a connected GBA cartridge
void gba_flash_read(char* buffer, unsigned int length);

// Reads the Flash from the given address of a connected GBA cartridge
void gba_flash_readAt(char* buffer, unsigned int start, unsigned int length);

// Writes to the Flash memory of a connected GBA cartridge
void gba_flash_write(char* buffer, unsigned int length);

// Reads the manufacturer code of the Flash chip
char gba_flash_checkManufacturer(char* manufacturerId, char* deviceId);

#endif /* GBA_FLASH_H */

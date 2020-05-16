#ifndef GBA_H
#define GBA_H

#define GBA_SAVE_TYPE_UNKNOWN 0
#define GBA_SAVE_TYPE_EEPROM_4K 1
#define GBA_SAVE_TYPE_EEPROM_64K 2
#define GBA_SAVE_TYPE_SRAM_256K 3
#define GBA_SAVE_TYPE_SRAM_512K 4
#define GBA_SAVE_TYPE_FLASH_512K 5
#define GBA_SAVE_TYPE_FLASH_1M 6

#define GBA_ERROR_NO_CARTRIDGE -1
#define GBA_ERROR_CARTRIDGE_CHANGED -2
#define GBA_ERROR_CARTRIDGE_NOT_LOADED -3

// Setup and initialize the GBA utils
int gba_init();

// Checks if the GBA utils are initialized
char gba_isInit();

// Loads the header and basic data of the connected GBA cartridge
char gba_loadHeader();

// Clears all loaded data for the connected GBA cartridge
void gba_loadClear();

// Checks if a GBA cartridge is currently connected and loaded
char gba_isLoaded();

// Gets the game title of the loaded GBA cartridge
char* gba_getGameTitle();

// Gets the game code of the loaded GBA cartridge
char* gba_getGameCode();

// Gets the maker code of the loaded GBA cartridge
char* gba_getMakerCode();

// Gets the ROM size of the loaded GBA cartridge
unsigned int gba_getROMSize();

// Gets the Save type of the loaded GBA cartridge
unsigned int gba_getSaveType();

// Gets the Save size of the loaded GBA cartridge
unsigned int gba_getSaveSize();

// Read the ROM of a connected GBA cartridge and returns the size
int gba_readROM(char* buffer, unsigned int length);

// Read the Save Data of a connected GBA cartridge and returns the size
int gba_readSave(char* buffer, unsigned int length);

// Write the Save Data to a connected GBA cartridge
int gba_writeSave(char* buffer, unsigned int length);

// Dumps the first 400 bytes of the connected GBA cartridge
void gba_dumpHeader(char* data);

// Cleans up the GBA utils
int gba_close();

#endif /* GBA_H */

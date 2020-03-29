#ifndef GBC_H
#define GBC_H

#define GBC_FLAG_CGB_UNSUPPORTED 0
#define GBC_FLAG_CGB_COMPATIBLE 1
#define GBC_FLAG_CGB_ONLY 2

#define GBC_MEM_CTRL_NONE 0
#define GBC_MEM_CTRL_MBC1 1
#define GBC_MEM_CTRL_MBC2 2
#define GBC_MEM_CTRL_MBC3 3
#define GBC_MEM_CTRL_MBC5 4

#define GBC_ERROR_NO_CARTRIDGE -1
#define GBC_ERROR_CARTRIDGE_CHANGED -2
#define GBC_ERROR_CARTRIDGE_NOT_LOADED -3

// Setup and initialize the GB utils
int gbc_init();

// Checks if the GB utils are initialized
char gbc_isInit();

// Loads the header and basic data of the connected GB cartridge
char gbc_loadHeader();

// Clears all loaded data for the connected GB cartridge
void gbc_loadClear();

// Checks if a GB cartridge is currently connected and loaded
char gbc_isLoaded();

// Gets the game title of the loaded GB cartridge
char* gbc_getGameTitle();

// Gets the game sha1 hash for the first 1k of rom of the loaded GB cartridge
char* gbc_getGameSHA1();

// Gets the CGB flag of the loaded GB cartridge
char gbc_getCGBFlag();

// Gets the memory controller type of the loaded GB cartridge
char gbc_getMemoryController();

// Gets the ROM size of the loaded GB cartridge
unsigned int gbc_getROMSize();

// Gets the Save size of the loaded GB cartridge
unsigned int gbc_getSaveSize();

// Read the ROM of a connected GB cartridge and returns the size
int gbc_readROM(char* buffer, unsigned int length);

// Read the Save Data of a connected GB cartridge and returns the size
int gbc_readSave(char* buffer, unsigned int length);

// Write the Save Data to a connected GB cartridge
int gbc_writeSave(char* buffer, unsigned int length);

// Cleans up the GB utils
int gbc_close();

#endif /* GBC_H */

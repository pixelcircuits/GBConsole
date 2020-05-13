#ifndef GBX_H
#define GBX_H

#define GBX_CARTRIDGE_TYPE_NONE 0
#define GBX_CARTRIDGE_TYPE_GB 1
#define GBX_CARTRIDGE_TYPE_GBC 2
#define GBX_CARTRIDGE_TYPE_GBA 3

#define GBX_ERROR_NO_CARTRIDGE -1
#define GBX_ERROR_CARTRIDGE_CHANGED -2
#define GBX_ERROR_CARTRIDGE_NOT_LOADED -3

// Setup and initialize the GBx utils
int gbx_init();

// Checks if the GBx utils are initialized
int gbx_isInit();

// Loads the header and basic data of the connected GBx cartridge
char gbx_loadHeader();

// Checks if a GBx cartridge is currently connected and loaded
char gbx_isLoaded();

// Prints the info of the connected GBx cartridge
void gbx_printInfo();

// Gets the type of GBx cartridge currently connected
char gbx_getCartridgeType();

// Gets the game title of the connected GBx cartridge
char* gbx_getGameTitle();

// Gets the game identifier of the connected GBx cartridge
char* gbx_getGameIdentifier();

// Gets the ROM size of the connected GBx cartridge
unsigned int gbx_getROMSize();

// Gets the Save size of the connected GBx cartridge
unsigned int gbx_getSaveSize();

// Gets the estimated amount of time to read GBx cartridge ROM (millis)
unsigned int gbx_timeToReadROM();

// Gets the estimated amount of time to read GBx cartridge Save (millis)
unsigned int gbx_timeToReadSave();

// Gets the estimated amount of time to write GBx cartridge Save (millis)
unsigned int gbx_timeToWriteSave();

// Read the ROM of the connected GBx cartridge
int gbx_readROM(char* data);

// Read the Save Data of the connected GBx cartridge
int gbx_readSave(char* data);

// Write the Save Data to the connected GBx cartridge
int gbx_writeSave(char* data);

// Checks the state of the cartridge detector switch
char gbx_checkDetectorSwitch();

// Dumps the first 200 bytes of the connected GBx cartridge
void gbx_dumpHeader(char* data);

// Cleans up the GBx utils
int gbx_close();

#endif /* GBX_H */

#include "gbx.h"
#include "gbc/gbc.h"
#include "gba/gba.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "egpio.h"
#include "spi.h"

#define GBX_DTSW 0x40
#define GBX_SPI_KEY 0xC42C4865

// Data
static char gbx_isInitFlag = 0;

// Helper functions
static char gbx_isGB();
static char gbx_isLoaded_noLock();

// Setup and initialize the GBx utils
int gbx_init()
{
	//already initialized?
	if(gbx_isInitFlag == 1) return 0;
	
	//init and check dependencies
	gbc_init();
	gba_init();
	if(!gbc_isInit() || !gba_isInit()) {
		if(!gbc_isInit()) fprintf(stderr, "gbx_init: GBC Utils dependency failed to initialize\n");
		if(!gba_isInit()) fprintf(stderr, "gbx_init: GBA Utils dependency failed to initialize\n");
		gbx_close();
		return 1;
	}
	
	gbx_isInitFlag = 1;
	return 0;
}

// Checks if the GBx utils are initialized
int gbx_isInit()
{
	return gbx_isInitFlag;
}

// Loads the header and basic data of the connected GBx cartridge
char gbx_loadHeader()
{
	char result = 0;
	spi_obtainLock(GBX_SPI_KEY, 0);
	
	if(gbx_isGB()) {
		gba_loadClear();
		result = gbc_loadHeader();
	} else {
		gbc_loadClear();
		result = gba_loadHeader();
	}
	
	spi_unlock(GBX_SPI_KEY);
	return result;
}

// Checks if a GBx cartridge is currently connected and loaded
char gbx_isLoaded()
{
	char result = 0;
	spi_obtainLock(GBX_SPI_KEY, 0);
	result = gbx_isLoaded_noLock();
	spi_unlock(GBX_SPI_KEY);
	return result;
}

// Prints the info of the connected GBx cartridge
void gbx_printInfo()
{
	if(gba_getROMSize() > 0) {
		printf("Game Title: %s\n", gba_getGameTitle());
		printf("Game Code: %s\n", gba_getGameCode());
		printf("Maker Code: %s\n", gba_getMakerCode());
		printf("ROM Size: %d\n", gba_getROMSize());
		if(gba_getSaveType() == GBA_SAVE_TYPE_EEPROM_4K) printf("Save Type: EEPROM_4K\n");
		if(gba_getSaveType() == GBA_SAVE_TYPE_EEPROM_64K) printf("Save Type: EEPROM_64K\n");
		if(gba_getSaveType() == GBA_SAVE_TYPE_SRAM_256K) printf("Save Type: SRAM_256K\n");
		if(gba_getSaveType() == GBA_SAVE_TYPE_SRAM_512K) printf("Save Type: SRAM_512K\n");
		if(gba_getSaveType() == GBA_SAVE_TYPE_FLASH_512K) printf("Save Type: FLASH_512K\n");
		if(gba_getSaveType() == GBA_SAVE_TYPE_FLASH_1M) printf("Save Type: FLASH_1M\n");
		if(gba_getSaveType() == GBA_SAVE_TYPE_UNKNOWN) printf("Save Type: unknown\n");
	} else if(gbc_getROMSize() > 0) {
		printf("Game Title: %s\n", gbc_getGameTitle());
		printf("SHA1 [First 1024]: %s\n", gbc_getGameSHA1());
		if(gbc_getCGBFlag() == GBC_FLAG_CGB_UNSUPPORTED) printf("CGB Flag: Original GB Game\n");
		if(gbc_getCGBFlag() == GBC_FLAG_CGB_COMPATIBLE) printf("CGB Flag: GBC Compatible Features\n");
		if(gbc_getCGBFlag() == GBC_FLAG_CGB_ONLY) printf("CGB Flag: GBC Only\n");
		if(gbc_getMemoryController() == GBC_MEM_CTRL_NONE) printf("Mem Ctrl: None\n");
		if(gbc_getMemoryController() == GBC_MEM_CTRL_MBC1) printf("Mem Ctrl: MBC1\n");
		if(gbc_getMemoryController() == GBC_MEM_CTRL_MBC2) printf("Mem Ctrl: MBC2\n");
		if(gbc_getMemoryController() == GBC_MEM_CTRL_MBC3) printf("Mem Ctrl: MBC3\n");
		if(gbc_getMemoryController() == GBC_MEM_CTRL_MBC5) printf("Mem Ctrl: MBC5\n");
		printf("ROM Size: %d\n", gbc_getROMSize());
		printf("Save Size: %d\n", gbc_getSaveSize());
	} else {
		printf("No Cartridge...\n");
	}
}

// Gets the type of GBx cartridge currently connected
char gbx_getCartridgeType()
{
	if(gbc_getROMSize() > 0) {
		if(gbc_getCGBFlag() == GBC_FLAG_CGB_UNSUPPORTED) return GBX_CARTRIDGE_TYPE_GB;
		else return GBX_CARTRIDGE_TYPE_GBC;
	} else if(gba_getROMSize() > 0) {
		return GBX_CARTRIDGE_TYPE_GBA;
	}
	return GBX_CARTRIDGE_TYPE_NONE;
}

// Gets the game title of the connected GBx cartridge
char* gbx_getGameTitle()
{
	if(gba_getROMSize() > 0) {
		return gba_getGameTitle();
	}
	return gbc_getGameTitle();
}

// Gets the game identifier of the connected GBx cartridge
char* gbx_getGameIdentifier()
{
	if(gba_getROMSize() > 0) {
		return gba_getGameCode();
	}
	return gbc_getGameSHA1();
}

// Gets the ROM size of the connected GBx cartridge
unsigned int gbx_getROMSize()
{
	if(gba_getROMSize() > 0) {
		return gba_getROMSize();
	}
	return gbc_getROMSize();
}

// Gets the Save size of the connected GBx cartridge
unsigned int gbx_getSaveSize()
{
	if(gba_getROMSize() > 0) {
		return gba_getSaveSize();
	}
	return gbc_getSaveSize();
}

// Gets the estimated amount of time to read GBx cartridge ROM (millis)
unsigned int gbx_timeToReadROM()
{
	if(gba_getROMSize() > 0) {
		return ((gba_getROMSize()/100)*1405)/10000; //0.0014050436
	}
	return ((gbc_getROMSize()/100)*10255)/10000; //0.01025515666
}

// Gets the estimated amount of time to read GBx cartridge Save (millis)
unsigned int gbx_timeToReadSave()
{
	if(gba_getROMSize() > 0) {
		if(gba_getSaveType()==GBA_SAVE_TYPE_EEPROM_4K) return 53;
		if(gba_getSaveType()==GBA_SAVE_TYPE_EEPROM_64K) return 946;
		if(gba_getSaveType()==GBA_SAVE_TYPE_SRAM_256K) return 361;
		if(gba_getSaveType()==GBA_SAVE_TYPE_SRAM_512K) return 722;
		if(gba_getSaveType()==GBA_SAVE_TYPE_FLASH_512K) return 720;
		if(gba_getSaveType()==GBA_SAVE_TYPE_FLASH_1M) return 1449;
		return 1500;
	}
	return ((gbc_getSaveSize()/100)*21314)/10000; //0.02131436
}

// Gets the estimated amount of time to write GBx cartridge Save (millis)
unsigned int gbx_timeToWriteSave()
{
	if(gba_getROMSize() > 0) {
		if(gba_getSaveType()==GBA_SAVE_TYPE_EEPROM_4K) return 541;
		if(gba_getSaveType()==GBA_SAVE_TYPE_EEPROM_64K) return 13289;
		if(gba_getSaveType()==GBA_SAVE_TYPE_SRAM_256K) return 734;
		if(gba_getSaveType()==GBA_SAVE_TYPE_SRAM_512K) return 1468;
		if(gba_getSaveType()==GBA_SAVE_TYPE_FLASH_512K) return 8638;
		if(gba_getSaveType()==GBA_SAVE_TYPE_FLASH_1M) return 16861;
		return 15000;
	}
	return ((gbc_getSaveSize()/100)*22158)/10000; //0.02215844
}

// Read the ROM of the connected GBx cartridge
int gbx_readROM(char* data)
{
	spi_obtainLock(GBX_SPI_KEY, 0);
	
	int result = GBX_ERROR_NO_CARTRIDGE;
	if(gba_getROMSize() > 0) {
		result = gba_readROM(data, gba_getROMSize());
	}
	else if(gbc_getROMSize() > 0) {
		result = gbc_readROM(data, gbc_getROMSize());
	}
	
	spi_unlock(GBX_SPI_KEY);
	return result;
}

// Read the Save Data of the connected GBx cartridge
int gbx_readSave(char* data)
{
	spi_obtainLock(GBX_SPI_KEY, 0);
	
	int result = GBX_ERROR_NO_CARTRIDGE;
	if(gba_getSaveSize() > 0) {
		result = gba_readSave(data, gba_getSaveSize());
	}
	else if(gbc_getSaveSize() > 0) {
		result = gbc_readSave(data, gbc_getSaveSize());
	}
	
	spi_unlock(GBX_SPI_KEY);
	return result;
}

// Write the Save Data to the connected GBx cartridge
int gbx_writeSave(char* data)
{
	spi_obtainLock(GBX_SPI_KEY, 0);

	//double check that the believed loaded cartridge is correct
	if(gbx_getROMSize() == 0) {
		spi_unlock(GBX_SPI_KEY);
		return GBX_ERROR_CARTRIDGE_NOT_LOADED;
	}
	if(gbx_isLoaded_noLock() == 0) {
		gbx_loadHeader();
		
		spi_unlock(GBX_SPI_KEY);
		if(gbx_getROMSize() > 0) return GBX_ERROR_CARTRIDGE_CHANGED;
		return GBX_ERROR_NO_CARTRIDGE;
	}

	//write data
	int result = GBX_ERROR_NO_CARTRIDGE;
	if(gba_getSaveSize() > 0) {
		result = gba_writeSave(data, gba_getSaveSize());
	}
	else if(gbc_getSaveSize() > 0) {
		result = gbc_writeSave(data, gbc_getSaveSize());
	}
	
	spi_unlock(GBX_SPI_KEY);
	return result;
}

// Cleans up the GBx utils
int gbx_close()
{
	//close dependencies
	gbc_close();
	gba_close();
		
	gbx_isInitFlag = 0;
	return 0;
}

// Checks for the GB cartridge detector switch
static char gbx_isGB() {
	char state = egpio_readPort(EX_GPIO_PORTD);
	if((state & GBX_DTSW) == GBX_DTSW) {
		return 0;
	}
	return 1;
}

// Checks if a GB cartridge is currently connected and loaded
static char gbx_isLoaded_noLock() {
	if(gbc_getROMSize() > 0) {
		if(gbc_isLoaded() == 1) return 1;
	} else if(gba_getROMSize() > 0) {
		if(gba_isLoaded() == 1) return 1;
	}
	return 0;
}

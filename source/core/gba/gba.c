#include "gba.h"
#include "gba_cart.h"
#include "gba_rom.h"
#include "gba_save.h"
#include "gba_sram.h"
#include "gba_flash.h"
#include "gba_eeprom.h"
#include <stdio.h>
#include "egpio.h"

//TODOs:
//-Atmel flash read/write is untested

// Constants
static const char gba_nintendoLogo[] = { 
	                        0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A, 
	0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19, 
	0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 
	0x82, 0xE3, 0xCE, 0xBF, 0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0, 
	0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27, 
	0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 
	0x40, 0xA7, 0x0E, 0xFD, 0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85, 
	0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF, 
	0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 
	0x87, 0xF0, 0x3C, 0xAF, 0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07
};

// Data
static char gba_isInitFlag = 0;
static char gba_loaded = 0;
static char gba_gameTitle[13];
static char gba_gameCode[5];
static char gba_makerCode[3];
static unsigned int gba_romSize;
static unsigned int gba_saveType;
static unsigned int gba_saveSize;

// Helper functions
static char gba_verifyLoaded();
static void gba_clearData();

// Setup and initialize the GBA utils
int gba_init()
{
	//already initialized?
	if(gba_isInitFlag == 1) return 0;
	
	//check dependencies
	if(!egpio_isInit()) {
		fprintf(stderr, "gba_init: eGPIO dependency is not initialized\n");
		gba_close();
		return 1;
	}
	
	//configure pins
	gba_cart_powerDown();
	
	//init data
	gba_clearData();
	
	gba_isInitFlag = 1;
	return 0;
}

// Checks if the GBA utils are initialized
char gba_isInit()
{
	return gba_isInitFlag;
}

// Gets the header of a connected GBA cartridge
char gba_loadHeader()
{
	int i;
	
	//read header
	char header[192];
	gba_rom_readAt(header, 0x00, 192);
	
	//verify header checksum
	unsigned char chk=0;
	for(i=0xA0; i<0xBC; i++) chk = chk - header[i];
	chk = chk - 0x19;
	if(header[0xBD] != chk) {
		//power down the cart slot
		gba_cart_powerDown();
		gba_clearData();
		return gba_loaded;
	}
	
	//extract general data from header
	for(i = 0; i < 12; i++) gba_gameTitle[i] = header[0xA0 + i];
	for(i = 0; i < 4; i++) gba_gameCode[i] = header[0xAC + i];
	for(i = 0; i < 2; i++) gba_makerCode[i] = header[0xB0 + i];
	
	//determine rom size
	gba_romSize = gba_rom_determineSize();
	
	//determine save type and size
	gba_saveType = gba_save_determineType();
	if(gba_saveType == GBA_SAVE_TYPE_EEPROM_4K) gba_saveSize = GBA_SAVE_SIZE_4K;
	else if(gba_saveType == GBA_SAVE_TYPE_EEPROM_64K) gba_saveSize = GBA_SAVE_SIZE_64K;
	else if(gba_saveType == GBA_SAVE_TYPE_SRAM_256K) gba_saveSize = GBA_SAVE_SIZE_256K;
	else if(gba_saveType == GBA_SAVE_TYPE_SRAM_512K) gba_saveSize = GBA_SAVE_SIZE_512K;
	else if(gba_saveType == GBA_SAVE_TYPE_FLASH_512K) gba_saveSize = GBA_SAVE_SIZE_512K;
	else if(gba_saveType == GBA_SAVE_TYPE_FLASH_1M) gba_saveSize = GBA_SAVE_SIZE_1M;
	else gba_saveSize = 0;
	
	//power down the cart slot
	gba_cart_powerDown();
	
	gba_loaded = 1;
	return gba_loaded;
}

// Clears all loaded data for the connected GBA cartridge
void gba_loadClear()
{
	gba_clearData();
}

// Checks if a GBA cartridge is currently connected and loaded
char gba_isLoaded()
{
	gba_verifyLoaded();	
	
	//power down the cart slot
	gba_cart_powerDown();
	return gba_loaded;
}

// Gets the game title of the loaded GBA cartridge
char* gba_getGameTitle()
{
	return gba_gameTitle;
}

// Gets the game code of the loaded GBA cartridge
char* gba_getGameCode()
{
	return gba_gameCode;
}

// Gets the maker code of the loaded GBA cartridge
char* gba_getMakerCode()
{
	return gba_makerCode;
}

// Gets the ROM size of the loaded GBA cartridge
unsigned int gba_getROMSize()
{
	return gba_romSize;
}

// Gets the Save type of the loaded GBA cartridge
unsigned int gba_getSaveType()
{
	return gba_saveType;
}

// Gets the Save size of the loaded GBA cartridge
unsigned int gba_getSaveSize()
{
	return gba_saveSize;
}

// Read the ROM of a connected GBA cartridge and returns the length
int gba_readROM(char* buffer, unsigned int length)
{
	if(gba_verifyLoaded() == 0) gba_loadHeader();
	if(gba_loaded == 0) {
		//power down the cart slot
		gba_cart_powerDown();
		return GBA_ERROR_NO_CARTRIDGE;
	}
	
	if(length > gba_romSize) length = gba_romSize;
	if(length > 0) {
		gba_rom_readAt(buffer, 0x00, length);
	}
	
	//power down the cart slot
	gba_cart_powerDown();
	return length;
}

// Read the Save Data of a connected GBA cartridge and returns the size
int gba_readSave(char* buffer, unsigned int length)
{
	if(gba_verifyLoaded() == 0) gba_loadHeader();
	if(gba_loaded == 0) {
		//power down the cart slot
		gba_cart_powerDown();
		return GBA_ERROR_NO_CARTRIDGE;
	}
	
	if(gba_saveType == GBA_SAVE_TYPE_UNKNOWN) {
		//power down the cart slot
		gba_cart_powerDown();
		return 0;
	}
	
	if(length > gba_saveSize) length = gba_saveSize;
	if(length > 0) {
		if(gba_saveType == GBA_SAVE_TYPE_SRAM_256K || gba_saveType == GBA_SAVE_TYPE_SRAM_512K) gba_sram_read(buffer, length);
		if(gba_saveType == GBA_SAVE_TYPE_FLASH_512K || gba_saveType == GBA_SAVE_TYPE_FLASH_1M) gba_flash_read(buffer, length);
		if(gba_saveType == GBA_SAVE_TYPE_EEPROM_4K || gba_saveType == GBA_SAVE_TYPE_EEPROM_64K) gba_eeprom_read(buffer, length);
	}
	
	//power down the cart slot
	gba_cart_powerDown();
	return length;
}

// Write the Save Data to a connected GBA cartridge
int gba_writeSave(char* buffer, unsigned int length)
{
	if(gba_loaded == 0) return GBA_ERROR_CARTRIDGE_NOT_LOADED;
	if(gba_verifyLoaded() == 0) {
		gba_loadHeader();
		if(gba_loaded == 1) return GBA_ERROR_CARTRIDGE_CHANGED;
		if(gba_loaded == 0) return GBA_ERROR_NO_CARTRIDGE;
	}	
	
	if(gba_saveType == GBA_SAVE_TYPE_UNKNOWN) {
		//power down the cart slot
		gba_cart_powerDown();
		return 0;
	}
	
	if(length > gba_saveSize) length = gba_saveSize;
	if(length > 0) {
		if(gba_saveType == GBA_SAVE_TYPE_SRAM_256K || gba_saveType == GBA_SAVE_TYPE_SRAM_512K) gba_sram_write(buffer, length);
		if(gba_saveType == GBA_SAVE_TYPE_FLASH_512K || gba_saveType == GBA_SAVE_TYPE_FLASH_1M) gba_flash_write(buffer, length);
		if(gba_saveType == GBA_SAVE_TYPE_EEPROM_4K || gba_saveType == GBA_SAVE_TYPE_EEPROM_64K) gba_eeprom_write(buffer, length);
	}
	
	//power down the cart slot
	gba_cart_powerDown();
	return length;
}

// Dumps the first 400 bytes of the connected GBA cartridge
void gba_dumpHeader(char* data)
{
	gba_rom_readAt(data, 0x00, 400);
	
	//power down the cart slot
	gba_cart_powerDown();
}

// Cleans up the GBA utils
int gba_close()
{
	gba_isInitFlag = 0;
	return 0;
}

// Checks that the currently connected GBA cartridge matches what is loaded
static char gba_verifyLoaded() {
	int i;
	
	//return early if not loaded at all
	if(gba_loaded == 0) return 0;
	
	//read header
	char header[192];
	gba_rom_readAt(header, 0x00, 192);
	
	//verify header checksum
	unsigned char chk=0;
	for(i=0xA0; i<0xBC; i++) chk = chk - header[i];
	chk = chk - 0x19;
	if(header[0xBD] != chk) {
		gba_clearData();
		return gba_loaded;
	}
	
	//check that the game title, code and maker code match expected
	char verified = 1;
	for(i = 0; i < 12; i++) {
		if(gba_gameTitle[i] != header[0xA0 + i]) {
			verified = 0;
			break;
		}
	} 
	for(i = 0; i < 4; i++) {
		if(gba_gameCode[i] != header[0xAC + i]) {
			verified = 0;
			break;
		}
	} 
	for(i = 0; i < 2; i++) {
		if(gba_makerCode[i] != header[0xB0 + i]) {
			verified = 0;
			break;
		}
	}
	if(verified == 0) {
		gba_clearData();
		return gba_loaded;
	}
	
	//all good
	return gba_loaded;
}

// Clears loaded data
static void gba_clearData() {
	int i;
	
	gba_loaded = 0;
	for(i = 0; i < 13; i++) gba_gameTitle[i] = 0;
	for(i = 0; i < 5; i++) gba_gameCode[i] = 0;
	for(i = 0; i < 3; i++) gba_makerCode[i] = 0;
	gba_saveType = 0;
	gba_saveSize = 0;
	gba_romSize = 0;
}

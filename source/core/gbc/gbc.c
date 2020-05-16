#include "gbc.h"
#include "gbc_cart.h"
#include "gbc_rom.h"
#include "gbc_mbc1.h"
#include "gbc_mbc2.h"
#include "gbc_mbc3.h"
#include "gbc_mbc5.h"
#include <openssl/sha.h>
#include <stdio.h>
#include "egpio.h"

#define GBC_CT_ROMONLY                 0x00
#define GBC_CT_MBC1                    0x01
#define GBC_CT_MBC1_RAM                0x02
#define GBC_CT_MBC1_RAM_BAT            0x03
#define GBC_CT_MBC2                    0x05
#define GBC_CT_MBC2_BAT                0x06
#define GBC_CT_ROM_RAM                 0x08
#define GBC_CT_ROM_RAM_BAT             0x09
#define GBC_CT_MMM01                   0x0B
#define GBC_CT_MMM01_RAM               0x0C
#define GBC_CT_MMM01_RAM_BAT           0x0D
#define GBC_CT_MBC3_TMR_BAT            0x0F
#define GBC_CT_MBC3_TMR_RAM_BAT        0x10
#define GBC_CT_MBC3                    0x11
#define GBC_CT_MBC3_RAM                0x12
#define GBC_CT_MBC3_RAM_BAT            0x13
#define GBC_CT_MBC5                    0x19
#define GBC_CT_MBC5_RAM                0x1A
#define GBC_CT_MBC5_RAM_BAT            0x1B
#define GBC_CT_MBC5_RMBL               0x1C
#define GBC_CT_MBC5_RMBL_RAM           0x1D
#define GBC_CT_MBC5_RMBL_RAM_BAT       0x1E
#define GBC_CT_MBC6                    0x20
#define GBC_CT_MBC7_SNSR_RMBL_RAM_BAT  0x22
#define GBC_CT_HuC3                    0xFE
#define GBC_CT_HuC1_RAM_BAT            0xFF 

//TODOs:
//-Support MBC6, MBC7, MMM01, HuC3, HuC1

// Constants
static const char gbc_nintendoLogo[] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

// Data
static char gbc_isInitFlag = 0;
static char gbc_loaded = 0;
static char gbc_gameTitle[17];
static char gbc_gameSHA1[41];
static char gbc_flagCGB;
static char gbc_memController;
static unsigned int gbc_romSize;
static unsigned int gbc_saveSize;

// Helper functions
static unsigned int gbc_readROMNoMemCtrl(char* buffer, unsigned int length);
static unsigned int gbc_readRAMNoMemCtrl(char* buffer, unsigned int length);
static unsigned int gbc_writeRAMNoMemCtrl(char* buffer, unsigned int length);
static char gbc_verifyLoaded();
static void gbc_clearData();

// Util functions
static void gbc_cleanString(char* str);

// Setup and initialize the GB utils
int gbc_init()
{
	//already initialized?
	if(gbc_isInitFlag == 1) return 0;
	
	//check dependencies
	if(!egpio_isInit()) {
		fprintf(stderr, "gbc_init: eGPIO dependency is not initialized\n");
		gbc_close();
		return 1;
	}
	
	//configure pins
	gbc_cart_powerDown();
	
	//init data
	gbc_clearData();
	
	gbc_isInitFlag = 1;
	return 0;
}

// Checks if the GB utils are initialized
char gbc_isInit()
{
	return gbc_isInitFlag;
}

// Gets the header of a connected GB cartridge
char gbc_loadHeader()
{
	int i;
	
	//wake up cartridge with a few reads (some seem to need it)
	char data[1024];
	gbc_rom_readAt(data, 0x00, 4);
	
	//read header
	gbc_rom_readAt(data, 0x00, 1024);
	char* header = &(data[0x100]);
	
	//verify nintendo logo
	char verified = 1;
	for(i=0; i<48; i++) {
		if(header[i + 4] != gbc_nintendoLogo[i]) {
			verified = 0;
			break;
		}
	} 
	if(verified == 0) {
		//power down the cart slot
		gbc_cart_powerDown();
		gbc_clearData();
		return gbc_loaded;
	} 
	
	//extract general data from header
	for(i = 0; i < 16; i++) gbc_gameTitle[i] = header[0x34 + i];
	
	//check for CGB flag
	if((gbc_gameTitle[15] & 0x80) == 0x80) {
		if((gbc_gameTitle[15] & 0xC0) == 0xC0) gbc_flagCGB = GBC_FLAG_CGB_ONLY;
		else gbc_flagCGB = GBC_FLAG_CGB_COMPATIBLE;
		gbc_gameTitle[15] = 0;
	} else {
		gbc_flagCGB = GBC_FLAG_CGB_UNSUPPORTED;
	}
	
	//clean game title
	gbc_cleanString(gbc_gameTitle);
	
	//determine memory controller
	if(header[0x47]==GBC_CT_MBC1 || header[0x47]==GBC_CT_MBC1_RAM || header[0x47]==GBC_CT_MBC1_RAM_BAT) {
		gbc_memController = GBC_MEM_CTRL_MBC1;
		
	} else if(header[0x47]==GBC_CT_MBC2 || header[0x47]==GBC_CT_MBC2_BAT) {
		gbc_memController = GBC_MEM_CTRL_MBC2;
		
	} else if(header[0x47]==GBC_CT_MBC3_TMR_BAT || header[0x47]==GBC_CT_MBC3_TMR_RAM_BAT || header[0x47]==GBC_CT_MBC3 
			|| header[0x47]==GBC_CT_MBC3_RAM || header[0x47]==GBC_CT_MBC3_RAM_BAT) {
		gbc_memController = GBC_MEM_CTRL_MBC3;
		
	} else if(header[0x47]==GBC_CT_MBC5 || header[0x47]==GBC_CT_MBC5_RAM || header[0x47]==GBC_CT_MBC5_RAM_BAT 
			|| header[0x47]==GBC_CT_MBC5_RMBL || header[0x47]==GBC_CT_MBC5_RMBL_RAM || header[0x47]==GBC_CT_MBC5_RMBL_RAM_BAT) {
		gbc_memController = GBC_MEM_CTRL_MBC5;
		
	} else if(header[0x47]==GBC_CT_MMM01 || header[0x47]==GBC_CT_MMM01_RAM || header[0x47]==GBC_CT_MMM01_RAM_BAT) {
		gbc_memController = GBC_MEM_CTRL_NONE; //not yet supported
		
	} else if(header[0x47]==GBC_CT_MBC6) {
		gbc_memController = GBC_MEM_CTRL_NONE; //not yet supported
		
	} else if(header[0x47]==GBC_CT_MBC7_SNSR_RMBL_RAM_BAT) {
		gbc_memController = GBC_MEM_CTRL_NONE; //not yet supported
		
	} else if(header[0x47]==GBC_CT_HuC1_RAM_BAT) {
		gbc_memController = GBC_MEM_CTRL_NONE; //not yet supported
		
	} else if(header[0x47]==GBC_CT_HuC3) {
		gbc_memController = GBC_MEM_CTRL_NONE; //not yet supported
		
	} else {
		gbc_memController = GBC_MEM_CTRL_NONE;
	}
	
	//determine rom size
	if(header[0x48] == 0x01) gbc_romSize = GBC_64K;
	else if(header[0x48] == 0x02) gbc_romSize = GBC_128K;
	else if(header[0x48] == 0x03) gbc_romSize = GBC_256K;
	else if(header[0x48] == 0x04) gbc_romSize = GBC_512K;
	else if(header[0x48] == 0x05) gbc_romSize = GBC_1M;
	else if(header[0x48] == 0x06) gbc_romSize = GBC_2M;
	else if(header[0x48] == 0x07) gbc_romSize = GBC_4M;
	else if(header[0x48] == 0x08) gbc_romSize = GBC_8M;
	else if(header[0x48] == 0x52) gbc_romSize = GBC_1p1M;
	else if(header[0x48] == 0x53) gbc_romSize = GBC_1p2M;
	else if(header[0x48] == 0x54) gbc_romSize = GBC_1p5M;
	else gbc_romSize = GBC_32K;
  
	//determine save size
	if(header[0x47]==GBC_CT_MBC1_RAM_BAT || header[0x47]==GBC_CT_ROM_RAM_BAT || header[0x47]==GBC_CT_MMM01_RAM_BAT 
			|| header[0x47]==GBC_CT_MBC3_TMR_RAM_BAT || header[0x47]==GBC_CT_MBC3_RAM_BAT || header[0x47]==GBC_CT_MBC5_RAM_BAT 
			|| header[0x47]==GBC_CT_MBC5_RMBL_RAM_BAT || header[0x47]==GBC_CT_MBC7_SNSR_RMBL_RAM_BAT || header[0x47]==GBC_CT_HuC1_RAM_BAT) {
		if(header[0x49] == 0x01) gbc_saveSize = GBC_2K;
		else if(header[0x49] == 0x02) gbc_saveSize = GBC_8K;
		else if(header[0x49] == 0x03) gbc_saveSize = GBC_32K;
		else if(header[0x49] == 0x04) gbc_saveSize = GBC_128K;
		else if(header[0x49] == 0x05) gbc_saveSize = GBC_64K;
		else gbc_saveSize = 0;
	} else {
		gbc_saveSize = 0;
	}
	
	//calculate sha1 hash of first 1k bytes
	unsigned char sha1_1k[20];
	SHA1((unsigned char*)data, 1024, sha1_1k);
	for(i=0; i<20; i++) sprintf(&(gbc_gameSHA1[i*2]), "%02X", sha1_1k[i]);
	gbc_gameSHA1[40] = 0;
	
	//power down the cart slot
	gbc_cart_powerDown();
	
	gbc_loaded = 1;
	return gbc_loaded;
}

// Clears all loaded data for the connected GB cartridge
void gbc_loadClear()
{
	gbc_clearData();
}

// Checks if a GB cartridge is currently connected and loaded
char gbc_isLoaded()
{
	gbc_verifyLoaded();
	
	//power down the cart slot
	gbc_cart_powerDown();
	return gbc_loaded;
}

// Gets the game title of the loaded GB cartridge
char* gbc_getGameTitle()
{
	return gbc_gameTitle;
}

// Gets the game sha1 hash for the first 1k of rom of the loaded GB cartridge
char* gbc_getGameSHA1()
{
	return gbc_gameSHA1;
}

// Gets the CGB flag of the loaded GB cartridge
char gbc_getCGBFlag()
{	
	return gbc_flagCGB;
}

// Gets the memory controller type of the loaded GB cartridge
char gbc_getMemoryController()
{
	return gbc_memController;
}

// Gets the ROM size of the loaded GB cartridge
unsigned int gbc_getROMSize()
{
	return gbc_romSize;
}

// Gets the Save size of the loaded GB cartridge
unsigned int gbc_getSaveSize()
{
	return gbc_saveSize;
}

// Read the ROM of a connected GB cartridge and returns the length
int gbc_readROM(char* buffer, unsigned int length)
{
	if(gbc_verifyLoaded() == 0) gbc_loadHeader();
	if(gbc_loaded == 0) {
		//power down the cart slot
		gbc_cart_powerDown();
		return GBC_ERROR_NO_CARTRIDGE;
	}
	
	if(length > gbc_romSize) length = gbc_romSize;
	if(length > 0) {
		if(gbc_memController == GBC_MEM_CTRL_MBC1) length = gbc_mbc1_readROM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC2) length = gbc_mbc2_readROM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC3) length = gbc_mbc3_readROM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC5) length = gbc_mbc5_readROM(buffer, length);
		else length = gbc_readROMNoMemCtrl(buffer, length);
	}
	
	//power down the cart slot
	gbc_cart_powerDown();
	return length;
}

// Read the Save Data of a connected GB cartridge and returns the size
int gbc_readSave(char* buffer, unsigned int length)
{
	if(gbc_verifyLoaded() == 0) gbc_loadHeader();
	if(gbc_loaded == 0) {
		//power down the cart slot
		gbc_cart_powerDown();
		return GBC_ERROR_NO_CARTRIDGE;
	}
	
	if(length > gbc_saveSize) length = gbc_saveSize;
	if(length > 0) {
		if(gbc_memController == GBC_MEM_CTRL_MBC1) length = gbc_mbc1_readRAM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC3) length = gbc_mbc3_readRAM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC5) length = gbc_mbc5_readRAM(buffer, length);
		else length = gbc_readRAMNoMemCtrl(buffer, length);
	}
	
	//power down the cart slot
	gbc_cart_powerDown();
	return length;
}

// Write the Save Data to a connected GB cartridge
int gbc_writeSave(char* buffer, unsigned int length)
{
	if(gbc_loaded == 0) return GBC_ERROR_CARTRIDGE_NOT_LOADED;
	if(gbc_verifyLoaded() == 0) {
		gbc_loadHeader();
		if(gbc_loaded == 1) return GBC_ERROR_CARTRIDGE_CHANGED;
		if(gbc_loaded == 0) return GBC_ERROR_NO_CARTRIDGE;
	}	
	
	if(length > gbc_saveSize) length = gbc_saveSize;
	if(length > 0) {
		if(gbc_memController == GBC_MEM_CTRL_MBC1) length = gbc_mbc1_writeRAM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC3) length = gbc_mbc3_writeRAM(buffer, length);
		else if(gbc_memController == GBC_MEM_CTRL_MBC5) length = gbc_mbc5_writeRAM(buffer, length);
		else length = gbc_writeRAMNoMemCtrl(buffer, length);
	}
	
	//power down the cart slot
	gbc_cart_powerDown();
	return length;
}

// Dumps the first 400 bytes of the connected GB cartridge
void gbc_dumpHeader(char* data)
{
	gbc_rom_readAt(data, 0x00, 400);
	
	//power down the cart slot
	gbc_cart_powerDown();
}

// Cleans up the GB utils
int gbc_close()
{
	gbc_isInitFlag = 0;
	return 0;
}

// Read ROM for No Memory Controller
static unsigned int gbc_readROMNoMemCtrl(char* buffer, unsigned int length) {
	if(length > GBC_32K) length = GBC_32K;
	gbc_rom_readAt(buffer, 0x00, length);
	return length;
}

// Read RAM for No Memory Controller
static unsigned int gbc_readRAMNoMemCtrl(char* buffer, unsigned int length) {
	if(length > GBC_32K) length = GBC_32K;
	gbc_rom_readAt(buffer, GBC_32K, length);
	return length;
}

// Write RAM for No Memory Controller
static unsigned int gbc_writeRAMNoMemCtrl(char* buffer, unsigned int length) {
	if(length > GBC_32K) length = GBC_32K;
	gbc_rom_writeAt(buffer, GBC_32K, length);
	return length;
}

// Checks that the currently connected GB cartridge matches what is loaded
static char gbc_verifyLoaded() {
	int i;
	
	//return early if not loaded at all
	if(gbc_loaded == 0) return 0;
	
	//read header
	char header[80];
	gbc_rom_readAt(header, 0x100, 80);
	
	//verify nintendo logo
	char verified = 1;
	for(i=0; i<48; i++) {
		if(header[i + 4] != gbc_nintendoLogo[i]) {
			verified = 0;
			break;
		}
	} 
	if(verified == 0) {
		gbc_clearData();
		return gbc_loaded;
	} 
	
	//extract general data from header
	char temp_gameTitle[17];
	for(i = 0; i < 16; i++) temp_gameTitle[i] = header[0x34 + i];
	
	//check for CGB flag
	char temp_flagCGB;
	if((temp_gameTitle[15] & 0x80) == 0x80) {
		if((temp_gameTitle[15] & 0xC0) == 0xC0) temp_flagCGB = GBC_FLAG_CGB_ONLY;
		else temp_flagCGB = GBC_FLAG_CGB_COMPATIBLE;
		temp_gameTitle[15] = 0;
	} else {
		temp_flagCGB = GBC_FLAG_CGB_UNSUPPORTED;
	}
	
	//clean game title
	gbc_cleanString(temp_gameTitle);
	
	//check that the game title and CGB flag match expected
	for(i = 0; i < 16; i++) {
		if(gbc_gameTitle[i] != temp_gameTitle[i]) {
			verified = 0;
			break;
		}
	} 
	if(gbc_flagCGB != temp_flagCGB) {
		verified = 0;
	}
	if(verified == 0) {
		gbc_clearData();
		return gbc_loaded;
	}
	
	//all good
	return gbc_loaded;
}

// Clears loaded data
static void gbc_clearData() {
	int i;
	
	gbc_loaded = 0;
	for(i = 0; i < 17; i++) gbc_gameTitle[i] = 0;
	gbc_flagCGB = GBC_FLAG_CGB_UNSUPPORTED;
	gbc_memController = GBC_MEM_CTRL_NONE;
	gbc_saveSize = 0;
	gbc_romSize = 0;
}

// Removes all invalid characters
static void gbc_cleanString(char* str) {
	int i = 0;
	while(str[i]) {
		if(str[i] < 0x20) {
			str[i] = str[i+1];
		} else {
			if(str[i] == 0x20) str[i] = '_';
			i++;
		}
	}
	
	i--;
	while(str[i]) {
		if(str[i] == '_') str[i] = 0;
		else break;
		i--;
	}
}

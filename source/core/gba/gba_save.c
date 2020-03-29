#include "gba.h"
#include "gba_sram.h"
#include "gba_flash.h"
#include "gba_eeprom.h"
#include "gba_save.h"

// Leniency percentages for adhering to save type check assumptions
#define LENIENCY_EEPROM_CHECK 97
#define LENIENCY_FLASH_OR_SRAM_CHECK 95
#define LENIENCY_SRAM_CHECK 95
#define LENIENCY_FLASH_CHECK 95

// Helper functions
static char gba_save_checkHasFlashOrSRAM();
static int gba_save_checkSRAM();
static int gba_save_checkFlash();
static int gba_save_checkEEPROM();

// Util functions
static char gba_save_countBits(char byte);
static char gba_save_countDiff(char byte0, char byte1);

// Try to figure out the connected GBA cartridge Save type and size
int gba_save_determineType()
{
	int i;
	
	//has eeprom?
	int eeprom = gba_save_checkEEPROM();
	if(eeprom > 0) return eeprom;
	
	//has sram/flash?
	if(gba_save_checkHasFlashOrSRAM() == 1) {
	
		//has sram?
		int sram = gba_save_checkSRAM();
		if(sram > 0) return sram;
	
		//has flash?
		int flash = gba_save_checkFlash();
		if(flash > 0) return flash;
	}
	
	return GBA_SAVE_TYPE_UNKNOWN;
}

// Checks if the connected GBA cartridge contains an SRAM or Flash chip
static char gba_save_checkHasFlashOrSRAM() {
	//note: based on the assumption that if no sram or flash is connected we will just get zeros
	int i, j, margin;
	
	//check for the existence of data
	char buffer[64];
	for(i = 0; i < 128; i++) {
		gba_flash_readAt(buffer, i * (GBA_SAVE_SIZE_512K / 128), 64);
		int numBits = 0;
		for(j = 0; j < 64; j++) {
			numBits += gba_save_countBits(buffer[j]);
		}
		margin = ((64 * 8) * (100 - LENIENCY_FLASH_OR_SRAM_CHECK)) / 100;
		if(numBits > margin) return 1;
	}
	
	return 0;
}

// Checks if the connected GBA cartridge contains an SRAM chip and returns its size
static int gba_save_checkSRAM() {
	//note: based on the assumption that we can successfully write and read a byte
	//note: size is based on the assumption that sram read past size will start repeating
	int i, margin;
	
	//read first byte
	char buffer[64];
	gba_sram_readAt(buffer, 0, 1);
	
	//try to write the inverse
	buffer[1] = ~buffer[0];
	gba_sram_write(buffer + 1, 1);
	
	//read first byte again and check if it changed
	gba_sram_readAt(buffer + 2, 0, 1);
	if(buffer[2] == buffer[0]) return GBA_SAVE_TYPE_UNKNOWN;
	
	//correct the byte we wrote over and check for successfull write
	gba_sram_write(buffer, 1);
	if(buffer[2] != buffer[1]) return GBA_SAVE_TYPE_UNKNOWN;
	
	//determine the size
	char start[64];
	int diffBits = 0;
	gba_sram_readAt(start, 0, 64);
	gba_sram_readAt(buffer, GBA_SAVE_SIZE_256K, 64);
	for(i = 0; i < 64; i++) diffBits += gba_save_countDiff(start[i], buffer[i]);
	gba_sram_readAt(start, GBA_SAVE_SIZE_256K - 64, 64);
	gba_sram_readAt(buffer, GBA_SAVE_SIZE_512K - 64, 64);
	for(i = 0; i < 64; i++) diffBits += gba_save_countDiff(start[i], buffer[i]);
	margin = ((128 * 8) * (100 - LENIENCY_SRAM_CHECK)) / 100;
	if(diffBits < margin) return GBA_SAVE_TYPE_SRAM_256K;
	
	return GBA_SAVE_TYPE_SRAM_512K;
}

// Checks if the connected GBA cartridge contains a Flash chip and returns its size
static int gba_save_checkFlash() {
	//note: based on the assumption of known manufacturer and device id sizes
	//note: size is based on the assumption that flash read past size will start repeating
	int i, margin;
	
	//get manufacturer and device to check
	char manufacturerId, deviceId;
	char flashManufacturer = gba_flash_checkManufacturer(&manufacturerId, &deviceId);
	if(manufacturerId == 0xBF && deviceId == 0xD4) return GBA_SAVE_TYPE_FLASH_512K;
	if(manufacturerId == 0x1F && deviceId == 0x3D) return GBA_SAVE_TYPE_FLASH_512K;
	if(manufacturerId == 0xC2 && deviceId == 0x1C) return GBA_SAVE_TYPE_FLASH_512K;
	if(manufacturerId == 0x32 && deviceId == 0x1B) return GBA_SAVE_TYPE_FLASH_512K;
	if(manufacturerId == 0xC2 && deviceId == 0x09) return GBA_SAVE_TYPE_FLASH_1M;
	if(manufacturerId == 0x62 && deviceId == 0x13) return GBA_SAVE_TYPE_FLASH_1M;
	
	//backup method to determine size (check start and end of sections for repeat)
	char start[64];
	char buffer[64];
	int diffBits = 0;
	gba_flash_readAt(start, 0, 64);
	gba_flash_readAt(buffer, GBA_SAVE_SIZE_512K, 64);
	for(i = 0; i < 64; i++) diffBits += gba_save_countDiff(start[i], buffer[i]);
	gba_flash_readAt(start, GBA_SAVE_SIZE_512K - 64, 64);
	gba_flash_readAt(buffer, GBA_SAVE_SIZE_1M - 64, 64);
	for(i = 0; i < 64; i++) diffBits += gba_save_countDiff(start[i], buffer[i]);
	margin = ((128 * 8) * (100 - LENIENCY_FLASH_CHECK)) / 100;
	if(diffBits < margin) return GBA_SAVE_TYPE_FLASH_512K;
	
	return GBA_SAVE_TYPE_FLASH_1M;
}

// Checks if the connected GBA cartridge contains an EEPROM chip and returns its size
static int gba_save_checkEEPROM() {
	//note: based on the assumption that a cartridge without eeprom will return all 0s or 1s when trying to be read as such
	//note: size is based on assumption that a 4k read like a 64k just repeats the same eight bytes
	int i, j, margin;
	
	//get data to analyze
	int bufferSize = GBA_SAVE_SIZE_4K + 8;
	char buffer[GBA_SAVE_SIZE_4K + 8];
	gba_eeprom_read(buffer, bufferSize);
	
	//has eeprom? (not all 0x00 or 0xFF with a bit of leniency)
	int numBits = 0;
	for(i = 0; i < bufferSize; i++) {
		numBits += gba_save_countBits(buffer[i]);
	}
	margin = ((bufferSize * 8) * LENIENCY_EEPROM_CHECK) / 100;
	if(numBits > margin || numBits < (bufferSize * 8) - margin) return 0;
		
	//actually 4k?
	int diffBits = 0;
	for(i=8; i < bufferSize; i += 8) {
		for(j = 0; j < 8; j++) {
			diffBits += gba_save_countDiff(buffer[i + j], buffer[(i + j) - 8]);
		}
	}
	margin = (((bufferSize - 8) * 8) * (100 - LENIENCY_EEPROM_CHECK)) / 100;
	if(diffBits < margin) return GBA_SAVE_TYPE_EEPROM_4K;
	
	return GBA_SAVE_TYPE_EEPROM_64K;
}

// Counts the number of 1s in the given byte
static char gba_save_countBits(char byte) {
	char i, count = 0;
	for(i = 0; i < 8; i++)
		if((byte >> i) & 0x01 == 0x01) count++;
	return count;
}

// Counts the number of differences between the two given bytes
static char gba_save_countDiff(char byte0, char byte1) {
	char i, count = 0;
	for(i = 0; i < 8; i++) {
		char mask = 0x01 << i;
		if((byte0 & mask) != (byte1 & mask)) count++;
	}
	return count;
}

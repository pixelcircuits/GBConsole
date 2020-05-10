#include "gbc.h"
#include "gbc_rom.h"
#include "gbc_mbc5.h"

// Helper functions
static unsigned int gbc_mbc5_rwRAM(char* buffer, unsigned int length, char isWrite);

// Read ROM for MBC5 Memory Controller
unsigned int gbc_mbc5_readROM(char* buffer, unsigned int length)
{
	int i;
	unsigned int numBanks = length / GBC_16K;
	
	//read bank 0
	gbc_rom_readAt(buffer, 0x00, GBC_16K);
	
	//read the remaining banks
	for(i = 1; i < numBanks; i++) {
		//set bank
		char bankNum0 = (char) i;
		char bankNum1 = (char) (i >> 8);
		gbc_rom_writeByte(bankNum1, 0x3000);
		gbc_rom_writeByte(bankNum0, 0x2000);
		
		//read more data
		unsigned int index = i * GBC_16K;
		gbc_rom_readAt(buffer + index, GBC_16K, GBC_16K);
	}
	
	//set back to bank 1
	gbc_rom_writeByte(0x00, 0x3000);
	gbc_rom_writeByte(0x01, 0x2000);
	
	return length;
}

// Read RAM for MBC5 Memory Controller
unsigned int gbc_mbc5_readRAM(char* buffer, unsigned int length)
{
	return gbc_mbc5_rwRAM(buffer, length, 0);
}

// Write RAM for MBC5 Memory Controller
unsigned int gbc_mbc5_writeRAM(char* buffer, unsigned int length)
{
	return gbc_mbc5_rwRAM(buffer, length, 1);
}

// Read RAM for MBC5 Memory Controller
static unsigned int gbc_mbc5_rwRAM(char* buffer, unsigned int length, char isWrite) {
	int i;
	unsigned int numBanks = length / GBC_8K;
	
	//enable RAM
	gbc_rom_writeByte(0x0A, 0x0100);
	
	//read/write the banks
	for(i = 0; i < numBanks; i++) {
		//set bank
		char bankNum0 = (char) (i & 0x0F);
		gbc_rom_writeByte(bankNum0, 0x4000);
		
		//read more data
		unsigned int index = i * GBC_8K;
		if(isWrite) gbc_rom_writeAt(buffer + index, 0xA000, GBC_8K);
		else gbc_rom_readAt(buffer + index, 0xA000, GBC_8K);
	}
	
	//set back to bank 0 and disable RAM
	gbc_rom_writeByte(0x00, 0x4000);
	gbc_rom_writeByte(0x00, 0x0100);
	
	return length;
}

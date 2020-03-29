#include "gbc.h"
#include "gbc_rom.h"
#include "gbc_mbc2.h"

// Read ROM for MBC2 Memory Controller
unsigned int gbc_mbc2_readROM(char* buffer, unsigned int length)
{
	int i;
	unsigned int numBanks = length / GBC_16K;
	
	//read bank 0
	gbc_rom_readAt(buffer, 0x00, GBC_16K);
	
	//read the remaining banks
	for(i = 1; i < numBanks; i++) {
		//set bank
		char bankNum0 = (char) (i & 0x0F);
		gbc_rom_writeByte(bankNum0, 0x2100);
		
		//read more data
		unsigned int index = i * GBC_16K;
		gbc_rom_readAt(buffer + index, GBC_16K, GBC_16K);
	}
	
	//set back to bank 1
	gbc_rom_writeByte(0x01, 0x2100);
	
	return length;
}

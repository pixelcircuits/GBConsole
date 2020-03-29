#ifndef GBA_EEPROM_H
#define GBA_EEPROM_H

// Reads the EEPROM of a connected GBA cartridge
void gba_eeprom_read(char* buffer, unsigned int length);

// Writes to the EEPROM of a connected GBA cartridge
void gba_eeprom_write(char* buffer, unsigned int length);

#endif /* GBA_EEPROM_H */

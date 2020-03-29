#ifndef GBA_SAVE_H
#define GBA_SAVE_H

#define GBA_SAVE_SIZE_4K 512
#define GBA_SAVE_SIZE_64K 8192
#define GBA_SAVE_SIZE_256K 32768
#define GBA_SAVE_SIZE_512K 65536
#define GBA_SAVE_SIZE_1M 131072

// Try to figure out the connected GBA cartridge Save type and size
int gba_save_determineType();

#endif /* GBA_SAVE_H */

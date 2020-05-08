//-----------------------------------------------------------------------------------------
// Title:	Game Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CGameManager.h"
#include "CSettingsManager.h"
#include <gbx.h>
#include <usb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define FILE_SCAN_BUFFER_SIZE 512
#define FILE_SCAN_BUFFER_READ(buff0, buff1, index)   ((index) >= FILE_SCAN_BUFFER_SIZE*2 ? 0 : ((index) < FILE_SCAN_BUFFER_SIZE ? buff0[(index)] : buff1[(index)-FILE_SCAN_BUFFER_SIZE]))

#define MAX_FILENAME_SIZE 256
#define MAX_ROMS 1024

//data constants
static const char* gm_romLocation = "/home/pi/RetroPie/roms";
static const char* gm_romPathGB = "/home/pi/RetroPie/roms/gb/";
static const char* gm_romPathGBC = "/home/pi/RetroPie/roms/gbc/";
static const char* gm_romPathGBA = "/home/pi/RetroPie/roms/gba/";

static const char* gm_saveBackupPathGB = "data/backups/gb/";
static const char* gm_saveBackupPathGBC = "data/backups/gbc/";
static const char* gm_saveBackupPathGBA = "data/backups/gba/";

static const char* gm_boxartImgPathGB = "/home/pi/libretro/gb/Named_Boxarts/";
static const char* gm_boxartImgPathGBC = "/home/pi/libretro/gbc/Named_Boxarts/";
static const char* gm_boxartImgPathGBA = "/home/pi/libretro/gba/Named_Boxarts/";
static const char* gm_snapImgPathGB = "/home/pi/libretro/gb/Named_Snaps/";
static const char* gm_snapImgPathGBC = "/home/pi/libretro/gbc/Named_Snaps/";
static const char* gm_snapImgPathGBA = "/home/pi/libretro/gba/Named_Snaps/";
static const char* gm_titleImgPathGB = "/home/pi/libretro/gb/Named_Titles/";
static const char* gm_titleImgPathGBC = "/home/pi/libretro/gbc/Named_Titles/";
static const char* gm_titleImgPathGBA = "/home/pi/libretro/gba/Named_Titles/";

static const char* gm_boxartImgDefaultGB = "data/img/box_gb.png";
static const char* gm_boxartImgDefaultGBC = "data/img/box_gbc.png";
static const char* gm_boxartImgDefaultGBA = "data/img/box_gba.png";
static const char* gm_snapImgDefaultGB = "data/img/screen_gb.png";
static const char* gm_snapImgDefaultGBC = "data/img/screen_gbc.png";
static const char* gm_snapImgDefaultGBA = "data/img/screen_gba.png";
static const char* gm_titleImgDefaultGB = "data/img/screen_gb.png";
static const char* gm_titleImgDefaultGBC = "data/img/screen_gbc.png";
static const char* gm_titleImgDefaultGBA = "data/img/screen_gba.png";

static const char* gm_romExGB = ".gb";
static const char* gm_romExGBC = ".gbc";
static const char* gm_romExGBA = ".gba";

static const char* gm_saveExGB = ".srm";
static const char* gm_saveExGBC = ".srm";
static const char* gm_saveExGBA = ".sav";

static const char* gm_stateExAll = ".state";

static const char* gm_listGB = "data/GameBoy.json";
static const char* gm_listGBC = "data/GameBoyColor.json";
static const char* gm_listGBA = "data/GameBoyAdvance.json";

static const char* gm_emulatorsPath = "/opt/retropie/libretrocores/";
static const char* gm_emulatorRetroarch = "/opt/retropie/emulators/retroarch/bin/retroarch";
static const char* gm_emulationRetroarchConfig = "data/retroarch/retroarch.cfg";

static const char* const gm_emulatorsGB[] = { "lr-gambatte", "lr-mgba", 0 };
static const char* const gm_emulatorExecGB[] = { "gambatte_libretro.so", "mgba_libretro.so", 0 };
static const char* const gm_emulatorSaveExGB[] = { ".srm", ".srm", 0 };
static const char* const gm_emulatorsGBA[] = { "lr-gpsp", "lr-mgba", 0 };
static const char* const gm_emulatorExecGBA[] = { "gpsp_libretro.so", "mgba_libretro.so", 0 };
static const char* const gm_emulatorSaveExGBA[] = { ".sav", ".srm", 0 };

static const char* gm_emulatorSettingGB = "game.gb.emulator";
static const char* gm_emulatorSettingGBA = "game.gba.emulator";

static const char* gm_biosGBA = "gba_bios.bin";
static const char* gm_biosPathGBA = "/home/pi/RetroPie/BIOS/";

static const char* gm_usbBackupFolder = "GBConsole_backup";
static const char* gm_usbBackupPathGB = "GBConsole_backup/gb/";
static const char* gm_usbBackupPathGBC = "GBConsole_backup/gbc/";
static const char* gm_usbBackupPathGBA = "GBConsole_backup/gba/";

//helper functions
static int gm_addRomsInDir(const char* dirPath, const char* fileExt, char** filenames, int count);
static bool gm_searchFileForDetails(const char* filePath, const char* identifier, char* dName, char* dDetails);
static bool gm_searchFileForDetails(const char* filePath, const char* fileExt, char** catalogFilenames, char** catalogNames, int catalogSize);
static char* gm_searchForImage(const char* dirPath, const char* name, const char* filename, const char* defaultImg);
static char* gm_strClone(const char* str);
static void gm_strSimplify(char* buffer, const char* str);
static void gm_strReplace(char* str, char find, char to);
static void gm_strRemove(char* str, char find);
static void gm_strFileSanitize(char* str);
static void gm_strToUpper(char* str);
static bool gm_strStartsWith(const char* str, const char* find);
static int gm_strLevenshtein(const char* s, const char* t);
static bool gm_fileExists(const char* filename);
static bool gm_directoryExists(const char* dirname);
static void gm_ensureDirectory(const char* dirname);
static void gm_renameFile(const char* from, const char* to);
static int gm_compareCatalogElements(const void* elem1, const void* elem2);

//! Main constructor
CGameManager::CGameManager(CSettingsManager* settingsManager)
{
	cartType = CARTRIDGE_TYPE_NONE;
	cartCatalogIndex = -1;
	cartName = 0;
	cartFilename = 0;
	cartImgBoxart = 0;
	cartImgSnap = 0;
	cartImgTitle = 0;
	
	catalogSize = 0;
	catalogNames = 0;
	catalogFilenames = 0;
	catalogImgBoxarts = 0;
	
	//make sure backup directories are created
	gm_ensureDirectory(gm_saveBackupPathGB);
	gm_ensureDirectory(gm_saveBackupPathGBC);
	gm_ensureDirectory(gm_saveBackupPathGBA);
	
	//find emulators
	numEmulatorsGB = 0;
	availableEmulatorsGB = 0;
	numEmulatorsGBA = 0;
	availableEmulatorsGBA = 0;
	findAvailableEmulators();
	
	//load settings
	stmgr = settingsManager;
	initSettings();
	
	//update BIOS
	updateBIOS();
	
	//init catalog
	loadCatalog();
}

//! Destructor
CGameManager::~CGameManager()
{
	//free resources
	for(int i=0; i<numEmulatorsGB; i++) delete[] availableEmulatorsGB[i];
	delete[] availableEmulatorsGB;
	availableEmulatorsGB = 0;
	numEmulatorsGB = 0;
	for(int i=0; i<numEmulatorsGBA; i++) delete[] availableEmulatorsGBA[i];
	delete[] availableEmulatorsGBA;
	availableEmulatorsGBA = 0;
	numEmulatorsGBA = 0;
	
	delete[] cartName;
	delete[] cartFilename;
	delete[] cartImgBoxart;
	delete[] cartImgSnap;
	delete[] cartImgTitle;
	cartName = 0;
	cartFilename = 0;
	cartImgBoxart = 0;
	cartImgSnap = 0;
	cartImgTitle = 0;
	
	for(int i=0; i<catalogSize; i++) {
		delete[] catalogNames[i];
		delete[] catalogFilenames[i];
		delete[] catalogImgBoxarts[i];
	}
	delete[] catalogNames;
	delete[] catalogFilenames;
	delete[] catalogImgBoxarts;
	catalogNames = 0;
	catalogFilenames = 0;
	catalogImgBoxarts = 0;
	catalogSize = 0;
}
	
//! Loads info of connected cartridge
void CGameManager::loadCartridge()
{
	//clear old data
	delete[] cartName;
	delete[] cartFilename;
	delete[] cartImgBoxart;
	delete[] cartImgSnap;
	delete[] cartImgTitle;
	cartName = 0;
	cartFilename = 0;
	cartImgBoxart = 0;
	cartImgSnap = 0;
	cartImgTitle = 0;
	cartCatalogIndex = -1;
	cartType = CARTRIDGE_TYPE_NONE;
	
	//check current cartridge
	if(gbx_loadHeader() > 0) {
		//cart type
		if(gbx_getCartridgeType()==GBX_CARTRIDGE_TYPE_GB) cartType = CARTRIDGE_TYPE_GB;
		else if(gbx_getCartridgeType()==GBX_CARTRIDGE_TYPE_GBC) cartType = CARTRIDGE_TYPE_GBC;
		else if(gbx_getCartridgeType()==GBX_CARTRIDGE_TYPE_GBA) cartType = CARTRIDGE_TYPE_GBA;
		
		//find additional details
		char name[MAX_FILENAME_SIZE];
		char details[MAX_FILENAME_SIZE];
		name[0] = 0;
		details[0] = 0;
		if(cartType == CARTRIDGE_TYPE_GBA) {
			gm_searchFileForDetails(gm_listGBA, gbx_getGameIdentifier(), name, details);
		} else {
			if(gm_searchFileForDetails(gm_listGB, gbx_getGameIdentifier(), name, details)) cartType = CARTRIDGE_TYPE_GB;
			else if(gm_searchFileForDetails(gm_listGBC, gbx_getGameIdentifier(), name, details)) cartType = CARTRIDGE_TYPE_GBC;
		}
		
		//determine file name
		char filename[MAX_FILENAME_SIZE];
		if(name[0] != 0 && details[0] != 0) {
			sprintf(filename, "%s %s", name, details);
			gm_strFileSanitize(filename);
		} else {
			sprintf(filename, "%s", gbx_getGameTitle());
			gm_strFileSanitize(filename);
			gm_strReplace(filename, ' ', '_');
		}
		cartFilename = gm_strClone(filename);
		if(cartType == CARTRIDGE_TYPE_GB) sprintf(filename, "%s%s", cartFilename, gm_romExGB);
		else if(cartType == CARTRIDGE_TYPE_GBC) sprintf(filename, "%s%s", cartFilename, gm_romExGBC);
		else if(cartType == CARTRIDGE_TYPE_GBA) sprintf(filename, "%s%s", cartFilename, gm_romExGBA);
		
		//determine catalog index
		for(int i=0; i<catalogSize; i++) {
			if(strcmp(filename, catalogFilenames[i]) == 0) {
				cartCatalogIndex = i;
				break;
			}
		}
		
		//determine name
		if(name[0] != 0) cartName = gm_strClone(name);
		else cartName = gm_strClone(gbx_getGameTitle());
		
		//determine boxart img
		if(cartType == CARTRIDGE_TYPE_GB) {
			cartImgBoxart = gm_searchForImage(gm_boxartImgPathGB, cartName, filename, gm_boxartImgDefaultGB);
		} else if(cartType == CARTRIDGE_TYPE_GBC) {
			cartImgBoxart = gm_searchForImage(gm_boxartImgPathGBC, cartName, filename, gm_boxartImgDefaultGBC);
		} else if(cartType == CARTRIDGE_TYPE_GBA) {
			cartImgBoxart = gm_searchForImage(gm_boxartImgPathGBA, cartName, filename, gm_boxartImgDefaultGBA);
		}
		
		//determine snap img
		if(cartType == CARTRIDGE_TYPE_GB) {
			cartImgSnap = gm_searchForImage(gm_snapImgPathGB, cartName, filename, gm_snapImgDefaultGB);
		} else if(cartType == CARTRIDGE_TYPE_GBC) {
			cartImgSnap = gm_searchForImage(gm_snapImgPathGBC, cartName, filename, gm_snapImgDefaultGBC);
		} else if(cartType == CARTRIDGE_TYPE_GBA) {
			cartImgSnap = gm_searchForImage(gm_snapImgPathGBA, cartName, filename, gm_snapImgDefaultGBA);
		}
		
		//determine title img
		if(cartType == CARTRIDGE_TYPE_GB) {
			cartImgTitle = gm_searchForImage(gm_titleImgPathGB, cartName, filename, gm_titleImgDefaultGB);
		} else if(cartType == CARTRIDGE_TYPE_GBC) {
			cartImgTitle = gm_searchForImage(gm_titleImgPathGBC, cartName, filename, gm_titleImgDefaultGBC);
		} else if(cartType == CARTRIDGE_TYPE_GBA) {
			cartImgTitle = gm_searchForImage(gm_titleImgPathGBA, cartName, filename, gm_titleImgDefaultGBA);
		}
	}
}

//! Gets the type of cartridge currently connected
char CGameManager::getCartridgeType()
{
	return cartType;
}

//! Gets the name of the cartridge currently connected
const char* CGameManager::getCartridgeName()
{
	return cartName;
}

//! Gets the boxart image of the cartridge currently connected
const char* CGameManager::getCartridgeImgBoxart()
{
	return cartImgBoxart;
}

//! Gets the snap image of the cartridge currently connected
const char* CGameManager::getCartridgeImgSnap()
{
	return cartImgSnap;
}

//! Gets the title image of the cartridge currently connected
const char* CGameManager::getCartridgeImgTitle()
{
	return cartImgTitle;
}
	
//! Gets the catalog index of the cartridge currently connected
int CGameManager::getCartridgeCatalogIndex()
{
	return cartCatalogIndex;
}

//! Checks if the cartridge save data has changed since last sync
int CGameManager::syncCartridgeCheck()
{
	//check if cartridge valid else try to load new cartridge
	if(gbx_isLoaded() == 0) {
		loadCartridge();
		if(cartType != CARTRIDGE_TYPE_NONE) return SYNC_CHECK_ERROR_CARTRIDGE_CHANGED;
		return SYNC_CHECK_ERROR_NO_CARTRIDGE;
	}
	
	//check if not catalogued
	if(cartCatalogIndex == -1) return SYNC_CHECK_NOT_CATALOGUED;
	
	//check if there even is a save
	if(gbx_getSaveSize() == 0) return SYNC_CHECK_SAVE_UNCHANGED;
	
	//build file names
	char backupFilename[1024];
	if(cartType == CARTRIDGE_TYPE_GB) {
		sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGB, cartFilename, gm_saveExGB);
	} else if(cartType == CARTRIDGE_TYPE_GBC) {
		sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGBC, cartFilename, gm_saveExGBC);
	} else if(cartType == CARTRIDGE_TYPE_GBA) {
		sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGBA, cartFilename, gm_saveExGBA);
	}
	
	//check if backup file even exists
	if(!gm_fileExists(backupFilename)) return SYNC_CHECK_SAVE_CHANGED;
	
	//get last known save file
	char* lastKnownSave = 0;
	int lastKnownSaveSize = 0;
	FILE* file = fopen(backupFilename, "rb");
	if(file != NULL) {
		fseek(file, 0, SEEK_END);
		lastKnownSaveSize = ftell(file);
		rewind(file);
		lastKnownSave = new char[gbx_getSaveSize()];
		fread(lastKnownSave, lastKnownSaveSize, 1, file);
		fclose(file);
	}
	
	//get current cartridge save data
	char* currentSave = new char[gbx_getSaveSize()];
	int currentSaveSize = gbx_readSave(currentSave);
	
	//check if save has changed
	int result = SYNC_CHECK_ERROR_UNKNOWN;
	if(lastKnownSave!=0 && currentSave!=0 && lastKnownSaveSize>0 && currentSaveSize>0 && lastKnownSaveSize==currentSaveSize) {
		result = SYNC_CHECK_SAVE_UNCHANGED;
		for(int i=0; i<lastKnownSaveSize; i++) {
			if(lastKnownSave[i] != currentSave[i]) {
				result = SYNC_CHECK_SAVE_CHANGED;
				break;
			}
		}
	}
	
	//clear data and finish
	delete[] lastKnownSave;
	delete[] currentSave;
	return result;
}
	
//! Estimates the amount of time to sync the currently connected cartridge
int CGameManager::syncCartridgeEstimateTime(bool updateCartSave)
{
	if(cartType == CARTRIDGE_TYPE_NONE) return 0;
	int time = 0;	
	
	//build file names
	char romFilename[1024];
	char saveFilename[1024];
	if(cartType == CARTRIDGE_TYPE_GB) {
		sprintf(romFilename, "%s%s%s", gm_romPathGB, cartFilename, gm_romExGB);
		sprintf(saveFilename, "%s%s%s", gm_romPathGB, cartFilename, gm_saveExGB);
	} else if(cartType == CARTRIDGE_TYPE_GBC) {
		sprintf(romFilename, "%s%s%s", gm_romPathGBC, cartFilename, gm_romExGBC);
		sprintf(saveFilename, "%s%s%s", gm_romPathGBC, cartFilename, gm_saveExGBC);
	} else if(cartType == CARTRIDGE_TYPE_GBA) {
		sprintf(romFilename, "%s%s%s", gm_romPathGBA, cartFilename, gm_romExGBA);
		sprintf(saveFilename, "%s%s%s", gm_romPathGBA, cartFilename, gm_saveExGBA);
	}
	
	//get ROM if not already saved
	if(!gm_fileExists(romFilename)) {
		time += gbx_timeToReadROM();
		time += (int)(gbx_getROMSize()*0.00029023034);//write file
	}
	if(gbx_getSaveSize() > 0) {
		if(updateCartSave) {
			
			//update the cartridge save data
			if(gm_fileExists(saveFilename)) {
				time += (int)(gbx_getSaveSize()*0.00001238505);//read file
				time += (int)(gbx_getSaveSize()*0.00029023034);//write file
				time += gbx_timeToWriteSave();
			}
		} else {
			
			//update the system save data
			time += gbx_timeToReadSave();
			time += (int)(gbx_getSaveSize()*0.00029023034);//write file
			time += (int)(gbx_getSaveSize()*0.00029023034);//write file
		}
	}
	
	if(time < 0) time = 0;
	return time;
}

//! Syncs the currently connected cartridge to the catalog
bool CGameManager::syncCartridge(bool updateCartSave)
{
	//double check the cartridge hasn't changed
	if(cartType == CARTRIDGE_TYPE_NONE) return false;
	bool failure = false;
	
	//build file names
	char romFilename[1024];
	char catalogFilename[1024];
	char saveFilename[1024];
	char backupFilename[1024];
	if(cartType == CARTRIDGE_TYPE_GB) {
		sprintf(romFilename, "%s%s%s", gm_romPathGB, cartFilename, gm_romExGB);
		sprintf(catalogFilename, "%s%s", cartFilename, gm_romExGB);
		sprintf(saveFilename, "%s%s%s", gm_romPathGB, cartFilename, gm_saveExGB);
		sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGB, cartFilename, gm_saveExGB);
	} else if(cartType == CARTRIDGE_TYPE_GBC) {
		sprintf(romFilename, "%s%s%s", gm_romPathGBC, cartFilename, gm_romExGBC);
		sprintf(catalogFilename, "%s%s", cartFilename, gm_romExGBC);
		sprintf(saveFilename, "%s%s%s", gm_romPathGBC, cartFilename, gm_saveExGBC);
		sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGBC, cartFilename, gm_saveExGBC);
	} else if(cartType == CARTRIDGE_TYPE_GBA) {
		sprintf(romFilename, "%s%s%s", gm_romPathGBA, cartFilename, gm_romExGBA);
		sprintf(catalogFilename, "%s%s", cartFilename, gm_romExGBA);
		sprintf(saveFilename, "%s%s%s", gm_romPathGBA, cartFilename, gm_saveExGBA);
		sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGBA, cartFilename, gm_saveExGBA);
	}
	
	//resources
	FILE* romFile = NULL;
	FILE* saveFile = NULL;
	FILE* saveBackupFile = NULL;
	char* romData = NULL;
	char* saveData = NULL;
	
	//get ROM if not already saved
	if(!gm_fileExists(romFilename)) {
		romFile = fopen(romFilename, "w");
		if(romFile != NULL) {
			romData = new char[gbx_getROMSize()];
			if(gbx_readROM(romData) != gbx_getROMSize()) {
				delete[] romData;
				romData = NULL;
			}
		}
		if(romData == NULL) {
			if(romFile) fclose(romFile);
			if(saveFile) fclose(saveFile);
			if(saveBackupFile) fclose(saveBackupFile);
			if(romData) delete[] romData;
			if(saveData) delete[] saveData;
			return false; //fail
		}
	}
	
	//get Save data
	if(gbx_getSaveSize() > 0) {
		if(updateCartSave) {
			
			//update the system save data
			if(gm_fileExists(saveFilename)) {
				saveFile = fopen(saveFilename, "rb");
				saveBackupFile = fopen(backupFilename, "w");
				if(saveFile != NULL && saveBackupFile != NULL) {
					fseek(saveFile, 0, SEEK_END);
					long savefilelen = ftell(saveFile);
					rewind(saveFile);
					if(savefilelen == gbx_getSaveSize()) {
						saveData = new char[gbx_getSaveSize()];
						fread(saveData, savefilelen, 1, saveFile);
					}
				}
			}
		} else {
			
			//get the cartridge save data
			saveFile = fopen(saveFilename, "w");
			saveBackupFile = fopen(backupFilename, "w");
			if(saveFile != NULL && saveBackupFile != NULL) {
				saveData = new char[gbx_getSaveSize()];
				if(gbx_readSave(saveData) != gbx_getSaveSize()) {
					delete[] saveData;
					saveData = NULL;
				}
			}
		}
		if(saveData == NULL) {
			if(romFile) fclose(romFile);
			if(saveFile) fclose(saveFile);
			if(saveBackupFile) fclose(saveBackupFile);
			if(romData) delete[] romData;
			if(saveData) delete[] saveData;
			return false; //fail
		}
	}
	
	//save ROM data to file
	if(romData != NULL && romFile != NULL) {
		for(int i=0; i<gbx_getROMSize(); i++) fputc(romData[i], romFile);
		
		//update catalog
		cartCatalogIndex = addToCatalog(cartName, catalogFilename, cartImgBoxart);
	}
	
	//save Save data
	if(updateCartSave) {
		
		//to cartridge
		if(saveData != NULL && saveBackupFile != NULL) {
			if(gbx_writeSave(saveData) == gbx_getSaveSize()) {
				for(int i=0; i<gbx_getSaveSize(); i++) fputc(saveData[i], saveBackupFile);
			} else {
				if(romFile) fclose(romFile);
				if(saveFile) fclose(saveFile);
				if(saveBackupFile) fclose(saveBackupFile);
				if(romData) delete[] romData;
				if(saveData) delete[] saveData;
				return false; //fail
			}
		}
	} else {
		
		//to file
		if(saveData != NULL && saveFile != NULL && saveBackupFile != NULL) {
			for(int i=0; i<gbx_getSaveSize(); i++) fputc(saveData[i], saveFile);
			for(int i=0; i<gbx_getSaveSize(); i++) fputc(saveData[i], saveBackupFile);
		}
	}
	
	//clear resources
	if(romFile) fclose(romFile);
	if(saveFile) fclose(saveFile);
	if(saveBackupFile) fclose(saveBackupFile);
	if(romData) delete[] romData;
	if(saveData) delete[] saveData;
	
	return true;
}

//! Plays the game from the given index in catalog
void CGameManager::playGame(int index)
{
	//make sure index is valid
	if(index < catalogSize && index > -1) {

		//determine emulator
		int emulator = -1;
		if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGB)==0 || strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGBC)==0) {
			if(selectedEmulatorGB>0) {
				for(int i=0; gm_emulatorsGB[i]; i++) if(strcmp(gm_emulatorsGB[i], availableEmulatorsGB[selectedEmulatorGB])==0) { emulator = i; break; }
			} else if(numEmulatorsGB>1) {
				for(int i=0; gm_emulatorsGB[i]; i++) if(strcmp(gm_emulatorsGB[i], availableEmulatorsGB[1])==0) { emulator = i; break; }
				
				//prefer lr-gambatte
				for(int i=1; i<numEmulatorsGB; i++) {
					if(strcmp(availableEmulatorsGB[i], "lr-gambatte")==0) {
						for(int j=0; gm_emulatorsGB[j]; j++) if(strcmp(gm_emulatorsGB[j], "lr-gambatte")==0) { emulator = j; break; }
						break;
					}
				}
			}
		} else if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGBA)==0) {
			if(selectedEmulatorGBA>0) {
				for(int i=0; gm_emulatorsGBA[i]; i++) if(strcmp(gm_emulatorsGBA[i], availableEmulatorsGBA[selectedEmulatorGBA])==0) { emulator = i; break; }
			} else if(numEmulatorsGBA>1) {
				for(int i=0; gm_emulatorsGBA[i]; i++) if(strcmp(gm_emulatorsGBA[i], availableEmulatorsGBA[1])==0) { emulator = i; break; }
				
				char biosFile[1024];
				sprintf(biosFile, "%s%s", gm_biosPathGBA, gm_biosGBA);
				if(gm_fileExists(biosFile)) {
					
					//prefer lr-gpsp if bios file
					for(int i=1; i<numEmulatorsGB; i++) {
						if(strcmp(availableEmulatorsGB[i], "lr-gpsp")==0) {
							for(int j=0; gm_emulatorsGB[j]; j++) if(strcmp(gm_emulatorsGB[j], "lr-gpsp")==0) { emulator = j; break; }
							break;
						}
					}
				} else {
					
					//prefer lr-mgba if no bios file
					for(int i=1; i<numEmulatorsGB; i++) {
						if(strcmp(availableEmulatorsGB[i], "lr-mgba")==0) {
							for(int j=0; gm_emulatorsGB[j]; j++) if(strcmp(gm_emulatorsGB[j], "lr-mgba")==0) { emulator = j; break; }
							break;
						}
					}
				}
			}
		}
		if(emulator > -1) {
			
			//build file names and run command
			char filename[1024];
			char romFilename[1024];
			char saveFilename[1024];
			char emuSaveFilename[1024];
			strcpy(filename, catalogFilenames[index]);
			strrchr(filename, '.')[0] = 0;
			char runCommand[1024];
			if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGB)==0) {
				sprintf(romFilename, "%s%s%s", gm_romPathGB, filename, gm_romExGB);
				sprintf(saveFilename, "%s%s%s", gm_romPathGB, filename, gm_saveExGB);
				sprintf(emuSaveFilename, "%s%s%s", gm_romPathGB, filename, gm_emulatorSaveExGB[emulator]);
				sprintf(runCommand, "%s -L %s%s/%s --config %s  \"%s\"", gm_emulatorRetroarch, gm_emulatorsPath, gm_emulatorsGB[emulator], gm_emulatorExecGB[emulator], gm_emulationRetroarchConfig, romFilename);
			} else if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGBC)==0) {
				sprintf(romFilename, "%s%s%s", gm_romPathGBC, filename, gm_romExGBC);
				sprintf(saveFilename, "%s%s%s", gm_romPathGBC, filename, gm_saveExGBC);
				sprintf(emuSaveFilename, "%s%s%s", gm_romPathGBC, filename, gm_emulatorSaveExGB[emulator]);
				sprintf(runCommand, "%s -L %s%s/%s --config %s  \"%s\"", gm_emulatorRetroarch, gm_emulatorsPath, gm_emulatorsGB[emulator], gm_emulatorExecGB[emulator], gm_emulationRetroarchConfig, romFilename);
			} else if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGBA)==0) {
				sprintf(romFilename, "%s%s%s", gm_romPathGBA, filename, gm_romExGBA);
				sprintf(saveFilename, "%s%s%s", gm_romPathGBA, filename, gm_saveExGBA);
				sprintf(emuSaveFilename, "%s%s%s", gm_romPathGBA, filename, gm_emulatorSaveExGBA[emulator]);
				sprintf(runCommand, "%s -L %s%s/%s --config %s  \"%s\"", gm_emulatorRetroarch, gm_emulatorsPath, gm_emulatorsGBA[emulator], gm_emulatorExecGBA[emulator], gm_emulationRetroarchConfig, romFilename);
			}
			
			//match save file extension to what emulator expects
			gm_renameFile(saveFilename, emuSaveFilename);
			
			//run rom
			if(gm_fileExists(romFilename)) {
				system(runCommand);
			}
			
			//move save file extension to system standard
			gm_renameFile(emuSaveFilename, saveFilename);
		}
	}
}
	
//! Gets the size of the game catalog
int CGameManager::getCatalogSize()
{
	return catalogSize;
}

//! Gets the names of the games in the catalog
const char** CGameManager::getCatalogNames()
{
	return (const char**)catalogNames;
}

//! Gets the boxart images of the games in the catalog
const char** CGameManager::getCatalogImgBoxarts()
{
	return (const char**)catalogImgBoxarts;
}
	
//! Removes the given index from the catalog
void CGameManager::removeGame(int index)
{
	//make sure index is valid
	if(index < catalogSize && index > -1) {
	
		//build file names
		char filename[1024];
		char romFilename[1024];
		char saveFilename[1024];
		char stateFilename[1024];
		char backupFilename[1024];
		strcpy(filename, catalogFilenames[index]);
		strrchr(filename, '.')[0] = 0;
		if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGB)==0) {
			sprintf(romFilename, "%s%s%s", gm_romPathGB, filename, gm_romExGB);
			sprintf(saveFilename, "%s%s%s", gm_romPathGB, filename, gm_saveExGB);
			sprintf(stateFilename, "%s%s%s", gm_romPathGB, filename, gm_stateExAll);
			sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGB, filename, gm_saveExGB);
		} else if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGBC)==0) {
			sprintf(romFilename, "%s%s%s", gm_romPathGBC, filename, gm_romExGBC);
			sprintf(saveFilename, "%s%s%s", gm_romPathGBC, filename, gm_saveExGBC);
			sprintf(stateFilename, "%s%s%s", gm_romPathGBC, filename, gm_stateExAll);
			sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGBC, filename, gm_saveExGBC);
		} else if(strcmp(strrchr(catalogFilenames[index], '.'), gm_romExGBA)==0) {
			sprintf(romFilename, "%s%s%s", gm_romPathGBA, filename, gm_romExGBA);
			sprintf(saveFilename, "%s%s%s", gm_romPathGBA, filename, gm_saveExGBA);
			sprintf(stateFilename, "%s%s%s", gm_romPathGBA, filename, gm_stateExAll);
			sprintf(backupFilename, "%s%s%s", gm_saveBackupPathGBA, filename, gm_saveExGBA);
		}
		
		//delete rom and save files
		remove(romFilename);
		remove(saveFilename);
		remove(stateFilename);
		remove(backupFilename);

		char** oldCatalogNames = catalogNames;
		char** oldCatalogFilenames = catalogFilenames;
		char** oldCatalogImgBoxarts = catalogImgBoxarts;
		if(catalogSize > 1) {
			catalogNames = new char*[catalogSize-1];
			catalogFilenames = new char*[catalogSize-1];
			catalogImgBoxarts = new char*[catalogSize-1];
			for(int i=0; i<catalogSize-1; i++) {
				int copyIndex = i;
				if(i >= index) copyIndex = i+1;
				catalogNames[i] = oldCatalogNames[copyIndex];
				catalogFilenames[i] = oldCatalogFilenames[copyIndex];
				catalogImgBoxarts[i] = oldCatalogImgBoxarts[copyIndex];
			}
		} else {
			catalogNames = 0;
			catalogFilenames = 0;
			catalogImgBoxarts = 0;
		}
		delete[] oldCatalogNames;
		delete[] oldCatalogFilenames;
		delete[] oldCatalogImgBoxarts;
		catalogSize--;
	}
}
	
//! Gets the GameBoy emulator option at the given index	
const char* CGameManager::getGBEmulatorOption(int index)
{
	if(index > -1 && index < numEmulatorsGB) return availableEmulatorsGB[index];
	return 0;
}

//! Gets the number of GameBoy emulator options
int CGameManager::getNumGBEmulatorOptions()
{
	return numEmulatorsGB;
}

//! Gets the selected GameBoy emulator option
int CGameManager::getGBEmulator()
{
	return selectedEmulatorGB;
}

//! Sets the selected GameBoy emulator option
void CGameManager::setGBEmulator(int index)
{
	if(index < 0) index = numEmulatorsGB-1; 
	if(index > numEmulatorsGB-1) index = 0; 
	selectedEmulatorGB = index;
	stmgr->setPropertyString(gm_emulatorSettingGB, availableEmulatorsGB[index]);
}

//! Gets the GameBoyAdvance emulator option at the given index	
const char* CGameManager::getGBAEmulatorOption(int index)
{
	if(index > -1 && index < numEmulatorsGBA) return availableEmulatorsGBA[index];
	return 0;
}

//! Gets the number of GameBoyAdvance emulator options
int CGameManager::getNumGBAEmulatorOptions()
{
	return numEmulatorsGBA;
}

//! Gets the selected GameBoyAdvance emulator option
int CGameManager::getGBAEmulator()
{
	return selectedEmulatorGBA;
}

//! Sets the selected GameBoyAdvance emulator option
void CGameManager::setGBAEmulator(int index)
{
	if(index < 0) index = numEmulatorsGBA-1; 
	if(index > numEmulatorsGBA-1) index = 0; 
	selectedEmulatorGBA = index;
	stmgr->setPropertyString(gm_emulatorSettingGBA, availableEmulatorsGBA[index]);
}
	
//! Copies all game data to USB drive
void CGameManager::backupToUSB()
{
	if(usb_mount()) {
		char temp[1024];
		sprintf(temp, "sudo -u$USER mkdir -p -m=0777 \"%s/%s\"", usb_getPath(), gm_usbBackupFolder);
		system(temp);
		sprintf(temp, "sudo cp -r %s %s/%s", gm_romPathGB, usb_getPath(), gm_usbBackupFolder);
		system(temp);
		sprintf(temp, "sudo cp -r %s %s/%s", gm_romPathGBC, usb_getPath(), gm_usbBackupFolder);
		system(temp);
		sprintf(temp, "sudo cp -r %s %s/%s", gm_romPathGBA, usb_getPath(), gm_usbBackupFolder);
		system(temp);
		
		usb_unmount();
	}
}
	
//! Copies all game data from USB drive
void CGameManager::restoreFromUSB()
{
	if(usb_mount()) {
		char temp[1024];
		sprintf(temp, "sudo cp -r %s/%s %s", usb_getPath(), gm_usbBackupPathGB, gm_romLocation);
		system(temp);
		sprintf(temp, "sudo cp -r %s/%s %s", usb_getPath(), gm_usbBackupPathGBC, gm_romLocation);
		system(temp);
		sprintf(temp, "sudo cp -r %s/%s %s ", usb_getPath(), gm_usbBackupPathGBA, gm_romLocation);
		system(temp);
		
		loadCatalog();
		
		usb_unmount();
	}
}

//! Loads up the catalog list
void CGameManager::loadCatalog()
{
	//free resources
	for(int i=0; i<catalogSize; i++) {
		delete[] catalogNames[i];
		delete[] catalogFilenames[i];
		delete[] catalogImgBoxarts[i];
	}
	delete[] catalogNames;
	delete[] catalogFilenames;
	delete[] catalogImgBoxarts;
	catalogSize = 0;
	
	//determine rom files in all folders
	int numRoms = 0;
	char* filenames[MAX_ROMS];
	numRoms = gm_addRomsInDir(gm_romPathGB, gm_romExGB, filenames, numRoms);
	numRoms = gm_addRomsInDir(gm_romPathGBC, gm_romExGBC, filenames, numRoms);
	numRoms = gm_addRomsInDir(gm_romPathGBA, gm_romExGBA, filenames, numRoms);
	if(numRoms > 0) {
		
		//initialize catalog lists
		catalogSize = numRoms;
		catalogNames = new char*[numRoms];
		catalogFilenames = new char*[numRoms];
		catalogImgBoxarts = new char*[numRoms];
		for(int i=0; i<numRoms; i++) {
			catalogFilenames[i] = filenames[i];
			catalogNames[i] = 0;
			catalogImgBoxarts[i] = 0;
		}
		
		//try to fill names from master lists
		gm_searchFileForDetails(gm_listGB, gm_romExGB, catalogFilenames, catalogNames, catalogSize);
		gm_searchFileForDetails(gm_listGBC, gm_romExGBC, catalogFilenames, catalogNames, catalogSize);
		gm_searchFileForDetails(gm_listGBA, gm_romExGBA, catalogFilenames, catalogNames, catalogSize);
		
		//fill remaining names from the filename
		for(int i=0; i<catalogSize; i++) {
			if(catalogNames[i] == 0) {
				int len = strlen(catalogFilenames[i]) - strlen(strrchr(catalogFilenames[i], '.'));
				catalogNames[i] = new char[len+1];
				for(int j=0; j<len; j++) catalogNames[i][j] = catalogFilenames[i][j];
				catalogNames[i][len] = 0;
			}
		}
		
		//search for boxart
		for(int i=0; i<catalogSize; i++) {
			char* boxart = 0;
			if(strcmp(strrchr(catalogFilenames[i], '.'), gm_romExGB)==0) {
				boxart = gm_searchForImage(gm_boxartImgPathGB, catalogNames[i], catalogFilenames[i], gm_boxartImgDefaultGB);
			} else if(strcmp(strrchr(catalogFilenames[i], '.'), gm_romExGBC)==0) {
				boxart = gm_searchForImage(gm_boxartImgPathGBC, catalogNames[i], catalogFilenames[i], gm_boxartImgDefaultGBC);
			} else if(strcmp(strrchr(catalogFilenames[i], '.'), gm_romExGBA)==0) {
				boxart = gm_searchForImage(gm_boxartImgPathGBA, catalogNames[i], catalogFilenames[i], gm_boxartImgDefaultGBA);
			}
			catalogImgBoxarts[i] = boxart;
		}
		
		//sort
		sortCatalog();
	}
}

//! Sorts the catalog list
int CGameManager::addToCatalog(const char* name, const char* filename, const char* boxartImg)
{
	char** oldCatalogNames = catalogNames;
	char** oldCatalogFilenames = catalogFilenames;
	char** oldCatalogImgBoxarts = catalogImgBoxarts;
	catalogNames = new char*[catalogSize+1];
	catalogFilenames = new char*[catalogSize+1];
	catalogImgBoxarts = new char*[catalogSize+1];
	
	for(int i=0; i<catalogSize; i++) {
		catalogNames[i] = oldCatalogNames[i];
		catalogFilenames[i] = oldCatalogFilenames[i];
		catalogImgBoxarts[i] = oldCatalogImgBoxarts[i];
	}
	delete[] oldCatalogNames;
	delete[] oldCatalogFilenames;
	delete[] oldCatalogImgBoxarts;
	
	catalogNames[catalogSize] = gm_strClone(name);
	catalogFilenames[catalogSize] = gm_strClone(filename);
	catalogImgBoxarts[catalogSize] = gm_strClone(boxartImg);
	catalogSize++;
	
	//sort and find index
	sortCatalog();
	for(int i=0; i<catalogSize; i++) {
		if(strcmp(filename, catalogFilenames[i]) == 0) return i;
	}
	return -1;
}

//! Sorts the catalog list
void CGameManager::sortCatalog()
{
	if(catalogSize > 0) {
		//build list and sort
		char* list[catalogSize][3];
		for(int i=0; i<catalogSize; i++) {
			list[i][0] = catalogNames[i];
			list[i][1] = catalogFilenames[i];
			list[i][2] = catalogImgBoxarts[i];
		}
		qsort(list, catalogSize, sizeof(char*)*3, gm_compareCatalogElements);
		
		//update data
		for(int i=0; i<catalogSize; i++) {
			catalogNames[i] = list[i][0];
			catalogFilenames[i] = list[i][1];
			catalogImgBoxarts[i] = list[i][2];
		}
	}
}

//! Searches system for available emulators
void CGameManager::findAvailableEmulators()
{
	for(int i=0; i<numEmulatorsGB; i++) delete[] availableEmulatorsGB[i];
	delete[] availableEmulatorsGB;
	availableEmulatorsGB = 0;
	numEmulatorsGB = 0;
	selectedEmulatorGB = 0;
	for(int i=0; i<numEmulatorsGBA; i++) delete[] availableEmulatorsGBA[i];
	delete[] availableEmulatorsGBA;
	availableEmulatorsGBA = 0;
	numEmulatorsGBA = 0;
	selectedEmulatorGBA = 0;
	
	DIR* dir = opendir(gm_emulatorsPath);
	if(dir != NULL) {
		struct dirent *dp;
		
		//get counts
		while((dp = readdir(dir)) != NULL)	{
			if((dp->d_type & DT_DIR) == DT_DIR) {
				for(int j=0; gm_emulatorsGB[j]; j++) if(strcmp(dp->d_name, gm_emulatorsGB[j])==0) { numEmulatorsGB++; break; }
				for(int j=0; gm_emulatorsGBA[j]; j++) if(strcmp(dp->d_name, gm_emulatorsGBA[j])==0) { numEmulatorsGBA++; break; }
			}
		}
		
		//copy emulator names to list
		rewinddir(dir);
		availableEmulatorsGB = new char*[++numEmulatorsGB];
		availableEmulatorsGBA = new char*[++numEmulatorsGBA];
		int indexGBC = 0;
		int indexGBA = 0;
		availableEmulatorsGB[indexGBC++] = gm_strClone("auto");
		availableEmulatorsGBA[indexGBA++] = gm_strClone("auto");
		while((dp = readdir(dir)) != NULL)	{
			if((dp->d_type & DT_DIR) == DT_DIR) {
				for(int j=0; gm_emulatorsGB[j]; j++) if(strcmp(dp->d_name, gm_emulatorsGB[j])==0) { availableEmulatorsGB[indexGBC++] = gm_strClone(gm_emulatorsGB[j]); break; }
				for(int j=0; gm_emulatorsGBA[j]; j++) if(strcmp(dp->d_name, gm_emulatorsGBA[j])==0) { availableEmulatorsGBA[indexGBA++] = gm_strClone(gm_emulatorsGBA[j]); break; }
			}
		}
		closedir(dir);
	}
}

//! Initializes the settings
void CGameManager::initSettings()
{
	char emulatorGB[128];
	stmgr->getPropertyString(gm_emulatorSettingGB, "auto", emulatorGB, 128);
	stmgr->setPropertyString(gm_emulatorSettingGB, emulatorGB);
	for(int i=0; i<numEmulatorsGB; i++) {
		if(strcmp(emulatorGB, availableEmulatorsGB[i])==0) {
			selectedEmulatorGB = i;
			break;
		}
	}
	char emulatorGBA[128];
	stmgr->getPropertyString(gm_emulatorSettingGBA, "auto", emulatorGBA, 128);
	stmgr->setPropertyString(gm_emulatorSettingGBA, emulatorGBA);
	for(int i=0; i<numEmulatorsGBA; i++) {
		if(strcmp(emulatorGBA, availableEmulatorsGBA[i])==0) {
			selectedEmulatorGBA = i;
			break;
		}
	}
}

//! Makes updates to bios files
void CGameManager::updateBIOS()
{
	char command[1024];
	
	//try to mount USB drive
	usb_mount();

	//look for and update bios files
	char biosTarget[1024];
	sprintf(biosTarget, "%s%s", gm_biosPathGBA, gm_biosGBA);
	char biosUSB[1024];
	sprintf(biosUSB, "%s/%s", usb_getPath(), gm_biosGBA);
	char biosBoot[1024];
	sprintf(biosBoot, "%s%s", "/boot/", gm_biosGBA);
	if(gm_fileExists(biosUSB)) {
		sprintf(command, "sudo cp %s %s", biosUSB, biosTarget);
		system(command);
	} else if(gm_fileExists(biosBoot)) {
		sprintf(command, "sudo cp %s %s", biosBoot, biosTarget);
		system(command);
	}
	
	//leave USB unmounted
	usb_unmount();
}

//helper functions
static int gm_addRomsInDir(const char* dirPath, const char* fileExt, char** filenames, int count) {
	DIR* dir = opendir(dirPath);
	if(dir != NULL) {
		struct dirent *dp;
		while((dp = readdir(dir)) != NULL)	{
			if(count < (MAX_ROMS-1) && (dp->d_type & DT_REG) == DT_REG && strcmp(strrchr(dp->d_name, '.'), fileExt)==0) {
				filenames[count] = gm_strClone(dp->d_name);
				count++;
			}
		}
		closedir(dir);
	}
	return count;
}
static bool gm_searchFileForDetails(const char* filePath, const char* identifier, char* dName, char* dDetails) {
	char buffer[2][FILE_SCAN_BUFFER_SIZE];
	char* b0 = buffer[0];
	char* b1 = buffer[1];
	for(int i=0; i<FILE_SCAN_BUFFER_SIZE; i++) {
		b0[i] = 0;
		b1[i] = 0;
	}
	
	//open file and start scanning it
	FILE* fileptr = fopen(filePath, "rb");
	fread(b0, 1, FILE_SCAN_BUFFER_SIZE, fileptr);
	while(fread(b1, 1, FILE_SCAN_BUFFER_SIZE, fileptr) == FILE_SCAN_BUFFER_SIZE) {
		for(int i=0; i<FILE_SCAN_BUFFER_SIZE; i++) {
			
			//start of entry?
			if(FILE_SCAN_BUFFER_READ(b0, b1, i) == '{') {
				int start = i;
				int end = i;
				for(int j=0; i+j<FILE_SCAN_BUFFER_SIZE*2; j++) {
					if(FILE_SCAN_BUFFER_READ(b0, b1, i+j) == '}') {
						end = i+j;
						break;
					}
				}
				if(end > start) {
					//get data for entry
					char serial[MAX_FILENAME_SIZE];
					char sha1_1k[MAX_FILENAME_SIZE];
					char name[MAX_FILENAME_SIZE];
					char details[MAX_FILENAME_SIZE];
					for(int j=0; j<MAX_FILENAME_SIZE; j++) {
						serial[j] = 0;
						sha1_1k[j] = 0;
						name[j] = 0;
						details[j] = 0;
					}
					for(int j=0; start+j<end; j++) {
						//read "serial"?
						if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+0) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+1) == 's' 
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+2) == 'e' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+3) == 'r'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+4) == 'i' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+5) == 'a'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+6) == 'l' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+7) == '"'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+8) == ':') {
							for(int k=0; start+j+11+k<end && k<(MAX_FILENAME_SIZE-1); k++) {
								if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+11+k) == '"') break;
								serial[k] = FILE_SCAN_BUFFER_READ(b0, b1, start+j+11+k);
							}
							if(name[0] != 0 && details[0] != 0 && (serial[0] != 0 || sha1_1k[0] != 0)) break;
						}
						
						//read "sha1_1k"?
						if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+0) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+1) == 's' 
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+2) == 'h' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+3) == 'a'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+4) == '1' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+5) == '_'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+6) == '1' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+7) == 'k'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+8) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+9) == ':') {
							for(int k=0; start+j+12+k<end && k<(MAX_FILENAME_SIZE-1); k++) {
								if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+12+k) == '"') break;
								sha1_1k[k] = FILE_SCAN_BUFFER_READ(b0, b1, start+j+12+k);
							}
							if(name[0] != 0 && details[0] != 0 && (serial[0] != 0 || sha1_1k[0] != 0)) break;
						}
						
						//read "name"?
						if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+0) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+1) == 'n' 
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+2) == 'a' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+3) == 'm'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+4) == 'e' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+5) == '"'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+6) == ':') {
							for(int k=0; start+j+9+k<end && k<(MAX_FILENAME_SIZE-1); k++) {
								if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+9+k) == '"') break;
								name[k] = FILE_SCAN_BUFFER_READ(b0, b1, start+j+9+k);
							}
							if(name[0] != 0 && details[0] != 0 && (serial[0] != 0 || sha1_1k[0] != 0)) break;
						}
						
						//read "details"?
						if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+0) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+1) == 'd' 
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+2) == 'e' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+3) == 't'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+4) == 'a' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+5) == 'i'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+6) == 'l' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+7) == 's'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+8) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+9) == ':') {
							for(int k=0; start+j+12+k<end && k<(MAX_FILENAME_SIZE-1); k++) {
								if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+12+k) == '"') break;
								details[k] = FILE_SCAN_BUFFER_READ(b0, b1, start+j+12+k);
							}
							if(name[0] != 0 && details[0] != 0 && (serial[0] != 0 || sha1_1k[0] != 0)) break;
						}
					}
					
					//identified entry?
					if(strcmp(identifier, serial) == 0 || strcmp(identifier, sha1_1k) == 0) {
						strcpy(dName, name);
						strcpy(dDetails, details);
						
						//close file and return
						fclose(fileptr);
						return true;
					}
				}
				i = end+1;
			}
		}
		char* tmp = b0;
		b0 = b1;
		b1 = tmp;
	}
	
	//close file
	fclose(fileptr);
	return false;
}
static bool gm_searchFileForDetails(const char* filePath, const char* fileExt, char** catalogFilenames, char** catalogNames, int catalogSize) {
	char buffer[2][FILE_SCAN_BUFFER_SIZE];
	char* b0 = buffer[0];
	char* b1 = buffer[1];
	for(int i=0; i<FILE_SCAN_BUFFER_SIZE; i++) {
		b0[i] = 0;
		b1[i] = 0;
	}
	
	//open file and start scanning it
	bool foundMatch = false;
	FILE* fileptr = fopen(filePath, "rb");
	fread(b0, 1, FILE_SCAN_BUFFER_SIZE, fileptr);
	while(fread(b1, 1, FILE_SCAN_BUFFER_SIZE, fileptr) == FILE_SCAN_BUFFER_SIZE) {
		for(int i=0; i<FILE_SCAN_BUFFER_SIZE; i++) {
			
			//start of entry?
			if(FILE_SCAN_BUFFER_READ(b0, b1, i) == '{') {
				int start = i;
				int end = i;
				for(int j=0; i+j<FILE_SCAN_BUFFER_SIZE*2; j++) {
					if(FILE_SCAN_BUFFER_READ(b0, b1, i+j) == '}') {
						end = i+j;
						break;
					}
				}
				if(end > start) {
					//get data for entry
					char filename[MAX_FILENAME_SIZE];
					char name[MAX_FILENAME_SIZE];
					char details[MAX_FILENAME_SIZE];
					for(int j=0; j<MAX_FILENAME_SIZE; j++) {
						filename[j] = 0;
						name[j] = 0;
						details[j] = 0;
					}
					for(int j=0; start+j<end; j++) {
						//read "name"?
						if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+0) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+1) == 'n' 
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+2) == 'a' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+3) == 'm'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+4) == 'e' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+5) == '"'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+6) == ':') {
							for(int k=0; start+j+9+k<end && k<(MAX_FILENAME_SIZE-1); k++) {
								if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+9+k) == '"') break;
								name[k] = FILE_SCAN_BUFFER_READ(b0, b1, start+j+9+k);
							}
							if(name[0] != 0 && details[0] != 0) break;
						}
						
						//read "details"?
						if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+0) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+1) == 'd' 
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+2) == 'e' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+3) == 't'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+4) == 'a' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+5) == 'i'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+6) == 'l' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+7) == 's'
						&& FILE_SCAN_BUFFER_READ(b0, b1, start+j+8) == '"' && FILE_SCAN_BUFFER_READ(b0, b1, start+j+9) == ':') {
							for(int k=0; start+j+12+k<end && k<(MAX_FILENAME_SIZE-1); k++) {
								if(FILE_SCAN_BUFFER_READ(b0, b1, start+j+12+k) == '"') break;
								details[k] = FILE_SCAN_BUFFER_READ(b0, b1, start+j+12+k);
							}
							if(name[0] != 0 && details[0] != 0) break;
						}
					}
					
					//does name match anything in catalog?
					sprintf(filename, "%s %s%s", name, details, fileExt);
					for(int i=0; i<catalogSize; i++) {
						if(strcmp(filename, catalogFilenames[i]) == 0) {
							catalogNames[i] = gm_strClone(name);
							foundMatch = true;
						}
					}
				}
				i = end+1;
			}
		}
		char* tmp = b0;
		b0 = b1;
		b1 = tmp;
	}
	
	//close file
	fclose(fileptr);
	return foundMatch;
}
static char* gm_searchForImage(const char* dirPath, const char* name, const char* filename, const char* defaultImg) {
	char simpleName[MAX_FILENAME_SIZE];
	gm_strSimplify(simpleName, name);
	
	//search the appropriate folder
	DIR* dir = opendir(dirPath);
	if(dir != NULL) {
		
		//find potential matches
		char matches[50][MAX_FILENAME_SIZE];
		int numMatches = 0;
		struct dirent *dp;
		while((dp = readdir(dir)) != NULL)	{
			if((dp->d_type & DT_REG)==DT_REG && strlen(dp->d_name)<MAX_FILENAME_SIZE) {
				char simpleNameFile[MAX_FILENAME_SIZE];
				gm_strSimplify(simpleNameFile, dp->d_name);
				if(gm_strStartsWith(simpleNameFile, simpleName)) {
					strcpy(matches[numMatches], dp->d_name);
					numMatches++;
					if(numMatches == 50) break;
				}
			}
		}
		closedir(dir);
		
		//pick the best match
		if(numMatches > 0) {
			char* bestMatch = matches[0];
			int bestMatchDistance = gm_strLevenshtein(matches[0], filename);
			for(int i=1; i<numMatches; i++) {
				int distance = gm_strLevenshtein(matches[i], filename);
				if(distance < bestMatchDistance) {
					bestMatchDistance = distance;
					bestMatch = matches[i];
				}
				
			}
			
			//create string with full image path
			char* image = new char[strlen(dirPath)+strlen(bestMatch)+1];
			sprintf(image, "%s%s", dirPath, bestMatch);
			return image;
		}
	}
	
	//return the default instead
	if(defaultImg != 0) return gm_strClone(defaultImg);
	return 0;
}
static char* gm_strClone(const char* str) {
	char* clone = new char[strlen(str)+1];
	strcpy(clone, str);
	return clone;
}
static void gm_strSimplify(char* buffer, const char* str) {
	strcpy(buffer, str);
	
	//use _ instead of &
	gm_strReplace(buffer, '&', '_');
	
	//remove !@#$%^&*()-+'\/"?,|<>:
	gm_strRemove(buffer, ' ');
	gm_strRemove(buffer, '!');
	gm_strRemove(buffer, '@');
	gm_strRemove(buffer, '#');
	gm_strRemove(buffer, '$');
	gm_strRemove(buffer, '%');
	gm_strRemove(buffer, '^');
	gm_strRemove(buffer, '&');
	gm_strRemove(buffer, '*');
	gm_strRemove(buffer, '(');
	gm_strRemove(buffer, ')');
	gm_strRemove(buffer, '+');
	gm_strRemove(buffer, '-');
	gm_strRemove(buffer, '\'');
	gm_strRemove(buffer, '\\');
	gm_strRemove(buffer, '/');
	gm_strRemove(buffer, '"');
	gm_strRemove(buffer, '?');
	gm_strRemove(buffer, ',');
	gm_strRemove(buffer, '<');
	gm_strRemove(buffer, '>');
	gm_strRemove(buffer, '|');
	gm_strRemove(buffer, ':');
	
	//ignore case
	gm_strToUpper(buffer);
}
static void gm_strReplace(char* str, char find, char to) {
	for(int i=0; str[i]!=0; i++) if(str[i]==find) str[i]=to;
}
static void gm_strRemove(char* str, char find) {
	for(int i=0; str[i]!=0; i++) if(str[i]==find) for(int j=0; str[i+j]!=0; j++) str[i+j]=str[i+j+1];
}
static void gm_strFileSanitize(char* str) {
	gm_strRemove(str, '<');
	gm_strRemove(str, '>');
	gm_strRemove(str, '"');
	gm_strRemove(str, ':');
	gm_strRemove(str, '/');
	gm_strRemove(str, '\\');
	gm_strRemove(str, '|');
	gm_strRemove(str, '?');
	gm_strRemove(str, '*');
}
static void gm_strToUpper(char* str) {
	for(int i=0; str[i]!=0; i++) if(str[i]>=97&&str[i]<=122) str[i]-=32;
}
static bool gm_strStartsWith(const char* str, const char* find) {
	for(int i=0; str[i]!=0 && find[i]!=0; i++) {
		if(str[i] != find[i]) return false;
	}
	return true;
}
static int gm_strLevenshteinDist(int i, int j, const char* s, int ls, const char* t, int lt, int** d) {
	if (d[i][j] >= 0) return d[i][j];

	int x;
	if (i == ls) x = lt - j;
	else if (j == lt) x = ls - i;
	else if (s[i] == t[j]) x = gm_strLevenshteinDist(i + 1, j + 1, s, ls, t, lt, d);
	else {
		x = gm_strLevenshteinDist(i + 1, j + 1, s, ls, t, lt, d);

		int y;
		if ((y = gm_strLevenshteinDist(i, j + 1, s, ls, t, lt, d)) < x) x = y;
		if ((y = gm_strLevenshteinDist(i + 1, j, s, ls, t, lt, d)) < x) x = y;
		x++;
	}
	return d[i][j] = x;
	
}
static int gm_strLevenshtein(const char* s, const char* t) {
	int ls = strlen(s), lt = strlen(t);
	
	//create cache
	int** d = new int*[ls + 1];
	for(int i=0; i<=ls; i++) {
		d[i] = new int[lt + 1];
		for(int j=0; j<=lt; j++) d[i][j] = -1;
	}
 
	int dist = gm_strLevenshteinDist(0, 0, s, ls, t, lt, (int**)d);
	
	//delete cache
	for(int i=0; i<=ls; i++) delete[] d[i];
	delete[] d;
	
	return dist;
}
static bool gm_fileExists(const char* filename) {
    FILE *file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}
static bool gm_directoryExists(const char* dirname) {
	DIR* dir = opendir(dirname);
	if (dir) {
		closedir(dir);
		return 1;
	}
	return 0;
}
static void gm_ensureDirectory(const char* dirname) {
	if(dirname == 0 || dirname[0] == 0 || dirname[0] == '/' || gm_directoryExists(dirname)) {
		return;
	}
	
	//make sure parent dir exists
	char parent[MAX_FILENAME_SIZE];
	strcpy(parent, dirname);
	for(int i=strlen(parent)-2; i>=0 && parent[i] != '/'; i--) parent[i] = 0;
	gm_ensureDirectory(parent);
	
	//create directory
	char command[1024];
	sprintf(command, "sudo -u$USER mkdir -p -m=0777 \"%s\"", dirname);
	system(command);
}
static void gm_renameFile(const char* from, const char* to) {
	char command[1024];
	sprintf(command, "sudo mv \"%s\" \"%s\" > /dev/null 2>&1", from, to);
	system(command);
}
static int gm_compareCatalogElements(const void* elem1, const void* elem2) {
    char* name1 = ((char**)elem1)[0];
    char* filename1 = ((char**)elem1)[1];
    char* boxartImg1 = ((char**)elem1)[2];
	int cartType1 = CARTRIDGE_TYPE_GBA;
	if(strcmp(strrchr(filename1, '.'), gm_romExGB)==0) cartType1 = CARTRIDGE_TYPE_GB;
	else if(strcmp(strrchr(filename1, '.'), gm_romExGBC)==0) cartType1 = CARTRIDGE_TYPE_GBC;
	
    char* name2 = ((char**)elem2)[0];
    char* filename2 = ((char**)elem2)[1];
    char* boxartImg2 = ((char**)elem2)[2];
	int cartType2 = CARTRIDGE_TYPE_GBA;
	if(strcmp(strrchr(filename2, '.'), gm_romExGB)==0) cartType2 = CARTRIDGE_TYPE_GB;
	else if(strcmp(strrchr(filename2, '.'), gm_romExGBC)==0) cartType2 = CARTRIDGE_TYPE_GBC;
	
	//first prioritize cart type
	if(cartType1==CARTRIDGE_TYPE_GB) {
		if(cartType2==CARTRIDGE_TYPE_GBC || cartType2==CARTRIDGE_TYPE_GBA) return -1;
	} else if(cartType1==CARTRIDGE_TYPE_GBC) {
		if(cartType2==CARTRIDGE_TYPE_GBA) return -1;
		if(cartType2==CARTRIDGE_TYPE_GB) return 1;
	} else if(cartType1==CARTRIDGE_TYPE_GBA) {
		if(cartType2==CARTRIDGE_TYPE_GB || cartType2==CARTRIDGE_TYPE_GBC) return 1;
	}
	
	//otherwise go by filename
	return strcmp(filename1, filename2);
}

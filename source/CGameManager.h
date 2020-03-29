//-----------------------------------------------------------------------------------------
// Title:	Game Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#define CARTRIDGE_TYPE_NONE 0
#define CARTRIDGE_TYPE_GB 1
#define CARTRIDGE_TYPE_GBC 2
#define CARTRIDGE_TYPE_GBA 3

#define SYNC_CHECK_SAVE_CHANGED 2
#define SYNC_CHECK_SAVE_UNCHANGED 1
#define SYNC_CHECK_NOT_CATALOGUED 0
#define SYNC_CHECK_ERROR_NO_CARTRIDGE -1
#define SYNC_CHECK_ERROR_CARTRIDGE_CHANGED -2
#define SYNC_CHECK_ERROR_UNKNOWN -3

class CSettingsManager;

//! Manages Game Data
class CGameManager
{
public:
	//! Main Constructor
	CGameManager(CSettingsManager* settingsManager);

	//! Destructor
	~CGameManager();
	
	//! Loads info of connected cartridge
	void loadCartridge();
	
	//! Gets the type of cartridge currently connected
	char getCartridgeType();
	
	//! Gets the name of the cartridge currently connected
	const char* getCartridgeName();
	
	//! Gets the boxart image of the cartridge currently connected
	const char* getCartridgeImgBoxart();
	
	//! Gets the snap image of the cartridge currently connected
	const char* getCartridgeImgSnap();
	
	//! Gets the title image of the cartridge currently connected
	const char* getCartridgeImgTitle();
	
	//! Gets the catalog index of the cartridge currently connected
	int getCartridgeCatalogIndex();
	
	//! Checks if the cartridge save data has changed since last sync
	int syncCartridgeCheck();
	
	//! Estimates the amount of time to sync the currently connected cartridge
	int syncCartridgeEstimateTime(bool updateCartSave);
	
	//! Syncs the currently connected cartridge to the catalog
	bool syncCartridge(bool updateCartSave);
	
	//! Plays the game from the given index in catalog
	void playGame(int index);
	
	//! Gets the size of the game catalog
	int getCatalogSize();
	
	//! Gets the names of the games in the catalog
	const char** getCatalogNames();
	
	//! Gets the boxart images of the games in the catalog
	const char** getCatalogImgBoxarts();
	
	//! Removes the given index from the catalog
	void removeGame(int index);
	
	//! Gets the GameBoy emulator option at the given index	
	const char* getGBEmulatorOption(int index);
	
	//! Gets the number of GameBoy emulator options
	int getNumGBEmulatorOptions();
	
	//! Gets the selected GameBoy emulator option
	int getGBEmulator();
	
	//! Sets the selected GameBoy emulator option
	void setGBEmulator(int index);
	
	//! Gets the GameBoyAdvance emulator option at the given index	
	const char* getGBAEmulatorOption(int index);
	
	//! Gets the number of GameBoyAdvance emulator options
	int getNumGBAEmulatorOptions();
	
	//! Gets the selected GameBoyAdvance emulator option
	int getGBAEmulator();
	
	//! Sets the selected GameBoyAdvance emulator option
	void setGBAEmulator(int index);
	
	//! Copies all game data to USB drive
	void backupToUSB();
	
	//! Copies all game data from USB drive
	void restoreFromUSB();
	
private:
	CSettingsManager* stmgr;
	char** availableEmulatorsGB;
	int numEmulatorsGB;
	int selectedEmulatorGB;
	char** availableEmulatorsGBA;
	int numEmulatorsGBA;
	int selectedEmulatorGBA;
	
	char cartType;
	int cartCatalogIndex;
	char* cartName;
	char* cartFilename;
	char* cartImgBoxart;
	char* cartImgSnap;
	char* cartImgTitle;
	
	int catalogSize;
	char** catalogNames;
	char** catalogFilenames;
	char** catalogImgBoxarts;
	
	//Util functions
	void loadCatalog();
	void sortCatalog();
	int addToCatalog(const char* name, const char* filename, const char* boxartImg);
	void findAvailableEmulators();
	void initSettings();
	void updateBIOS();
};

#endif

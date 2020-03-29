//-----------------------------------------------------------------------------------------
// Title:	Menu Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#define MENU_PAGE_STATE_NONE 0
#define MENU_PAGE_STATE_CARTRIDGE 1
#define MENU_PAGE_STATE_CATALOG 2
#define MENU_PAGE_STATE_SETTINGS 3

#define MENU_SELECTION_STATE_NONE 0
#define MENU_SELECTION_STATE_CARTRIDGE 1
#define MENU_SELECTION_STATE_CATALOG 2
#define MENU_SELECTION_STATE_SETTINGS 3
#define MENU_SELECTION_STATE_POWER 4

#define MENU_SELECTION_STATE_PLAY 5
#define MENU_SELECTION_STATE_SYNC 6
#define MENU_SELECTION_STATE_REFRESH 7

#define MENU_SELECTION_STATE_SETTINGS_RESOLUTION 8
#define MENU_SELECTION_STATE_SETTINGS_GB_EMULATOR 9
#define MENU_SELECTION_STATE_SETTINGS_GBA_EMULATOR 10
#define MENU_SELECTION_STATE_SETTINGS_BACKUP_USB 11
#define MENU_SELECTION_STATE_SETTINGS_RESTORE_USB 12
#define MENU_SELECTION_STATE_SETTINGS_CONNECT_BT 13

#define MENU_SELECTION_STATE_CAROUSEL 1000

#define MENU_NEXT_SELECTION_UP 0
#define MENU_NEXT_SELECTION_DOWN 1
#define MENU_NEXT_SELECTION_LEFT 2
#define MENU_NEXT_SELECTION_RIGHT 3

#define MENU_MAX_CATALOG_CAROUSEL 25
#define MENU_MAX_MODAL_BUTTONS 4

#define MENU_FRAME_RATE 30
#define MENU_PROGRESS_BAR_UPDATE_MILLIS 500

class CSettingsManager;
class CSceneManager;
class CSceneNode;
class CRectSceneNode;
class COutlineSceneNode;
class CTextSceneNode;
class CImageSceneNode;

//! Manages Menu GUI elements
class CMenuManager
{
public:
	//! Main Constructor
	CMenuManager(CSceneManager* smgr, CSettingsManager* stmgr);

	//! Destructor
	~CMenuManager();
	
	//! Loads up scene assets to render pages
	void loadSceneAssets();
	
	//! Clears all scene related data and vid assets
	void clearSceneAssets();
	
	//! Switches to a null page with the given optional text
	void setPageNull(const char* text);
	
	//! Switches to the cartridge page with no cartridge
	void setPageCartridgeEmpty(bool isLoading);

	//! Switches to the cartridge page
	void setPageCartridge(const char* title, const char* boxImg, const char* titleImg, const char* snapImg, bool isGBA, bool canPlay);
	
	//! Switches to the catalog page
	void setPageCatalog(const char** titles, const char** art, int count);
	
	//! Switches to the settings page
	void setPageSettings(bool canBackupUSB);
	
	//! Switches to the play game page
	void setPagePlayGame();
	
	//! Displays a modal dialog to the user and returns the selection
	int showModal(const char* text1, const char* text2, const char** buttonText, const char** buttonDesc, int numButtons);
	
	//! Displays the progress bar and starts the async update process
	void showProgressBar(const char* text, int estimatedDuration);
	
	//! Ends the progress bar process and returns menu to previous state
	void endProgressBar();
	
	//! Shows the given text in the bottom right corner until selection or page is changed
	void showLoadingText(const char* text);
	
	//! Sets the current page selection
	void setPageSelection(int selection, bool noAnimation);
	
	//! Gets the current page
	int getPage();
	
	//! Gets the current page selection
	int getPageSelection();
	
	//! Gets the next page selection in the given direction
	int getNextSelection(char dir);
	
	//! Gets the catalog index
	int getCatalogIndex();
	
	//! Renders the menu
	void render();
	
private:
	CSceneManager* smgr;
	CSettingsManager* stmgr;
	long time;
	int pageState;
	int selectionState;
	
	int carouselPage;
	int carouselIndex;
	const char** carouselTitles;
	const char** carouselArt;
	int carouselCount;
	
	int screenMargin;
	bool smallScreen;
	
	//General Scene Nodes
	CRectSceneNode* background;
	CRectSceneNode* shade;
	CRectSceneNode* modal;
	CTextSceneNode* modalText[2];
	CRectSceneNode* modalButton[MENU_MAX_MODAL_BUTTONS];
	CTextSceneNode* modalButtonText[MENU_MAX_MODAL_BUTTONS];
	CTextSceneNode* title;
	CTextSceneNode* subtitle;
	CTextSceneNode* infotext;
	CImageSceneNode* cartButton;
	CImageSceneNode* catButton;
	CImageSceneNode* settingsButton;
	CImageSceneNode* powerButton;
	CRectSceneNode* bottomLine;
	CImageSceneNode* dpadIcon;
	CTextSceneNode* dpadText;
	CImageSceneNode* abtnIcon;
	CTextSceneNode* abtnText;
	CImageSceneNode* bbtnIcon;
	CTextSceneNode* bbtnText;
	CTextSceneNode* loadingText;
	CRectSceneNode* loadingBar;
	COutlineSceneNode* cursor;
	
	//Cartridge Page Scene Nodes
	CTextSceneNode* gameTitle;
	CImageSceneNode* artBox;
	CImageSceneNode* artTitle;
	CImageSceneNode* artSnap;
	CImageSceneNode* playButton;
	CImageSceneNode* syncButton;
	CImageSceneNode* refreshButton;
	
	//Catalog Page Scene Nodes
	CTextSceneNode* catGameTitle;
	CImageSceneNode* carousel[MENU_MAX_CATALOG_CAROUSEL];
	CImageSceneNode* carouselNext;
	CImageSceneNode* carouselPrevious;
	CImageSceneNode* miniCarousel[MENU_MAX_CATALOG_CAROUSEL];
	CImageSceneNode* miniCarouselNext;
	CImageSceneNode* miniCarouselPrevious;
	CRectSceneNode* carouselCursor;
	
	//Settings Page Scene Nodes
	CTextSceneNode* stResolutionLabel;
	CTextSceneNode* stResolutionValue;
	CTextSceneNode* stGBEmulatorLabel;
	CTextSceneNode* stGBEmulatorValue;
	CTextSceneNode* stGBAEmulatorLabel;
	CTextSceneNode* stGBAEmulatorValue;
	CRectSceneNode* stSpacer;
	CImageSceneNode* stBtConnectButton;
	CImageSceneNode* stUSBBackupButton;
	CImageSceneNode* stUSBRestoreButton;
	
	//Animation data
	CSceneNode* cursorTarget;
	int carouselTargetX;
	unsigned char* showProgressBarSceneNodeLayers;
	bool endProgressBarThread;
	
	//Util functions
	void clearPageSpecificNodes();
	CSceneNode* setCarouselIndex(int index, bool updateCursor, bool* noAnimation);
	void positionCarousel(int x);
	void updateSettings();
};

#endif

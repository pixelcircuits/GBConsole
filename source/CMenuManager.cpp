//-----------------------------------------------------------------------------------------
// Title:	Menu Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CMenuManager.h"
#include "CSceneManager.h"
#include "CSettingsManager.h"
#include "CRectSceneNode.h"
#include "COutlineSceneNode.h"
#include "CImageSceneNode.h"
#include "CTextSceneNode.h"
#include "Vector.h"
#include <vid.h>
#include <inp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//data constants
static const char* mm_emulatorSettingGB = "game.gb.emulator";
static const char* mm_emulatorSettingGBA = "game.gba.emulator";
static const char* mm_resolutionSetting = "system.resolution";

//Progress Bar Thread
static void* mm_processProgressBar(void* args);

//! Main constructor
CMenuManager::CMenuManager(CSceneManager* smgr, CSettingsManager* stmgr)
{
	this->smgr = smgr;
	this->stmgr = stmgr;
	time = clock();
	
	//load assets
	loadSceneAssets();
}
	
//! Loads up scene assets to render pages
void CMenuManager::loadSceneAssets()
{
	//Set default data
	pageState = MENU_PAGE_STATE_NONE;
	selectionState = MENU_SELECTION_STATE_NONE;
	carouselPage = -1;
	carouselIndex = -1;
	carouselTitles = 0;
	carouselArt = 0;
	carouselCount = 0;
	
	screenMargin = 40;
	smallScreen = false;
	if(vid_getScreenWidth() < 1920) {
		screenMargin = 20;
		smallScreen = true;
	}
	
	//General Scene Nodes
	background = smgr->addRectSceneNode(COLOR_DARKGRAY, Vector(vid_getScreenWidth(),vid_getScreenHeight()), false);
	background->setLayer(1);
	shade = smgr->addRectSceneNode(COLOR_DARKGRAY, Vector(vid_getScreenWidth(),vid_getScreenHeight()), false);
	shade->setOpacity(200);
	modal = smgr->addRectSceneNode(COLOR_DARKGRAY, Vector(vid_getScreenWidth(),vid_getScreenHeight()), true);
	modalText[0] = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	modalText[1] = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	for(int i=0; i<MENU_MAX_MODAL_BUTTONS; i++) {
		modalButton[i] = smgr->addRectSceneNode(COLOR_GRAY, Vector(100,20), true);
		modalButtonText[i] = smgr->addTextSceneNode(0, COLOR_OFFWHITE, COLOR_GRAY, 24, false);
	}
	
	title = smgr->addImageSceneNode("data/img/title.png", Vector(356,60), false, false);
	title->setPosition(Vector(screenMargin,screenMargin));
	subtitle = smgr->addTextSceneNode(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	subtitle->setPosition(title->getPosition()+Vector(0,title->getSize().Y));
	infotext = smgr->addTextSceneNode(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	
	cartButton = smgr->addImageSceneNode("data/img/icon_cartridge.png", Vector(80,80), true, true);
	cartButton->setPosition(Vector((vid_getScreenWidth()/2)-100,screenMargin+10));
	cartButton->setNavPath(MENU_SELECTION_STATE_CARTRIDGE);
	cartButton->setNavAText("Open Cartridge");
	catButton = smgr->addImageSceneNode("data/img/icon_catalog.png", Vector(80,80), true, true);
	catButton->setPosition(Vector((vid_getScreenWidth()/2)+20,screenMargin+10));
	catButton->setNavPath(MENU_SELECTION_STATE_CATALOG);
	catButton->setNavAText("Open Catalog");
	
	powerButton = smgr->addImageSceneNode("data/img/icon_power.png", Vector(80,80), true, false);
	powerButton->setPosition(Vector(vid_getScreenWidth()-(powerButton->getSize().X+screenMargin),screenMargin+10));
	powerButton->setNavPath(MENU_SELECTION_STATE_POWER);
	powerButton->setNavAText("Power Off");
	settingsButton = smgr->addImageSceneNode("data/img/icon_settings.png", Vector(80,80), true, false);
	settingsButton->setPosition(powerButton->getPosition()-Vector(settingsButton->getSize().X+20,0));
	settingsButton->setNavPath(MENU_SELECTION_STATE_SETTINGS);
	settingsButton->setNavAText("Edit Settings");
	
	bottomLine = smgr->addRectSceneNode(COLOR_WHITE, Vector(vid_getScreenWidth()-screenMargin,2), false);
	bottomLine->setPosition(Vector(screenMargin/2, vid_getScreenHeight()-(40+screenMargin+1)));
	dpadIcon = smgr->addImageSceneNode("data/img/icon_dpad.png", Vector(32,32), false, false);
	dpadIcon->setPosition(Vector(screenMargin, vid_getScreenHeight()-(screenMargin+dpadIcon->getSize().Y)));
	dpadText = smgr->addTextSceneNode("Navigate", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	dpadText->setPosition(dpadIcon->getPosition() + Vector(dpadIcon->getSize().X+5, 2));
	
	abtnIcon = smgr->addImageSceneNode("data/img/icon_abtn.png", Vector(32,32), false, false);
	abtnIcon->setPosition(Vector(0,dpadIcon->getPosition().Y));
	abtnText = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	abtnText->setPosition(Vector(0,dpadText->getPosition().Y));
	
	bbtnIcon = smgr->addImageSceneNode("data/img/icon_bbtn.png", Vector(32,32), false, false);
	bbtnIcon->setPosition(Vector(0,dpadIcon->getPosition().Y));
	bbtnText = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	bbtnText->setPosition(Vector(0,dpadText->getPosition().Y));
	
	loadingText = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	loadingText->setPosition(Vector(0, dpadText->getPosition().Y));
	loadingBar = smgr->addRectSceneNode(COLOR_GRAY, Vector(600,20), true);
	loadingBar->setPosition(Vector(vid_getScreenWidth()-(loadingBar->getSize().X+screenMargin), loadingText->getPosition().Y+5));
	
	cursor = smgr->addOutlineSceneNode(COLOR_ACCENT, Vector(0,0));
	cursor->setLayer(7);
	
	//Cartridge Page Scene Nodes
	gameTitle = smgr->addTextSceneNode(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	artBox = smgr->addImageSceneNode(0, Vector(0,0), true, true);
	artTitle = smgr->addImageSceneNode(0, Vector(0,0), true, false);
	artSnap = smgr->addImageSceneNode(0, Vector(0,0), true, false);
	
	playButton = smgr->addImageSceneNode("data/img/icon_play.png", Vector(80,80), true, false);
	playButton->setNavPath(MENU_SELECTION_STATE_PLAY);
	playButton->setNavAText("Play Game");
	syncButton = smgr->addImageSceneNode("data/img/icon_sync.png", Vector(80,80), true, false);
	syncButton->setNavPath(MENU_SELECTION_STATE_SYNC);
	syncButton->setNavAText("Sync Cartridge");
	refreshButton = smgr->addImageSceneNode("data/img/icon_refresh.png", Vector(80,80), true, false);
	refreshButton->setNavPath(MENU_SELECTION_STATE_REFRESH);
	refreshButton->setNavAText("Refresh Cartridge");
	
	//Catalog Page Scene Nodes
	catGameTitle = smgr->addTextSceneNode(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) {
		carousel[i] = smgr->addImageSceneNode(0, Vector(0,0), true, true);
		carousel[i]->setNavAText("Play Game");
		carousel[i]->setNavBText("(hold) Delete Game");
		miniCarousel[i] = smgr->addImageSceneNode(0, Vector(0,0), true, true);
	}
	carouselNext = smgr->addImageSceneNode(0, Vector(0,0), true, true);
	miniCarouselNext = smgr->addImageSceneNode(0, Vector(0,0), true, true);
	carouselPrevious = smgr->addImageSceneNode(0, Vector(0,0), true, true);
	miniCarouselPrevious = smgr->addImageSceneNode(0, Vector(0,0), true, true);
	carouselCursor = smgr->addRectSceneNode(COLOR_ACCENT, Vector(30,5), false);
	
	//Settings Page Scene Nodes
	stResolutionLabel = smgr->addTextSceneNode("Resolution", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stResolutionLabel->setNavPath(MENU_SELECTION_STATE_SETTINGS_RESOLUTION);
	stResolutionLabel->setNavAText("Toggle System Resolution");
	stResolutionValue = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBEmulatorLabel = smgr->addTextSceneNode("GameBoy Emulator", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBEmulatorLabel->setNavPath(MENU_SELECTION_STATE_SETTINGS_GB_EMULATOR);
	stGBEmulatorLabel->setNavAText("Toggle GameBoy Emulator");
	stGBEmulatorValue = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBAEmulatorLabel = smgr->addTextSceneNode("GameBoy Advance Emulator", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBAEmulatorLabel->setNavPath(MENU_SELECTION_STATE_SETTINGS_GBA_EMULATOR);
	stGBAEmulatorLabel->setNavAText("Toggle GameBoy Advance Emulator");
	stGBAEmulatorValue = smgr->addTextSceneNode(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stSpacer = smgr->addRectSceneNode(COLOR_WHITE, Vector(2,100), false);
	stUSBRestoreButton = smgr->addImageSceneNode("data/img/icon_usbdown.png", Vector(80,80), true, false);
	stUSBRestoreButton->setNavPath(MENU_SELECTION_STATE_SETTINGS_RESTORE_USB);
	stUSBRestoreButton->setNavAText("Restore Game Data from USB");
	stUSBBackupButton = smgr->addImageSceneNode("data/img/icon_usbup.png", Vector(80,80), true, false);
	stUSBBackupButton->setNavPath(MENU_SELECTION_STATE_SETTINGS_RESTORE_USB);
	stUSBBackupButton->setNavAText("Backup Game Data to USB");
	stBtConnectButton = smgr->addImageSceneNode("data/img/icon_bluetooth.png", Vector(80,80), true, false);
	stBtConnectButton->setNavPath(MENU_SELECTION_STATE_SETTINGS_CONNECT_BT);
	stBtConnectButton->setNavAText("Pair Bluetooth Controller");
	
	//Animation data
	cursorTarget = 0;
	carouselTargetX = 0;
	showProgressBarSceneNodeLayers = 0;
	
	//Default page state
	clearPageSpecificNodes();
}

//! Clears all scene related data and vid assets
void CMenuManager::clearSceneAssets()
{
	delete[] showProgressBarSceneNodeLayers;
	smgr->clearScene();
}

//! Destructor
CMenuManager::~CMenuManager()
{
	clearSceneAssets();
}

//! Switches to a null page with the given optional text
void CMenuManager::setPageNull(const char* text)
{
	pageState = MENU_PAGE_STATE_NONE;
	clearPageSpecificNodes();
	
	//General Scene Nodes
	dpadIcon->setLayer(LAYER_HIDDEN);
	dpadText->setLayer(LAYER_HIDDEN);
	abtnIcon->setLayer(LAYER_HIDDEN);
	abtnText->setLayer(LAYER_HIDDEN);
	bbtnIcon->setLayer(LAYER_HIDDEN);
	bbtnText->setLayer(LAYER_HIDDEN);
	cursor->setLayer(LAYER_HIDDEN);
	
	//Null Scene Nodes
	if(text) {
		infotext->setText(text, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
		infotext->setPosition(Vector(vid_getScreenWidth()/2, vid_getScreenHeight()/2) - infotext->getSize()/2);
		infotext->setLayer(5);
		shade->setSize(infotext->getSize());
		shade->setPosition(infotext->getPosition());
		shade->setLayer(9);
	}
}

//! Switches to the cartridge page with no cartridge
void CMenuManager::setPageCartridgeEmpty(bool isLoading)
{
	pageState = MENU_PAGE_STATE_CARTRIDGE;
	clearPageSpecificNodes();
	
	cartButton->setImage("data/img/icon_cartridge.png", Vector(100,100), true);
	cartButton->setPosition(Vector((vid_getScreenWidth()/2)-110,screenMargin));
	cartButton->setNavPath(MENU_SELECTION_STATE_NONE);
	subtitle->setText("Cartridge", COLOR_LIGHTBLUE, COLOR_DARKGRAY, 36, false);
	subtitle->setLayer(5);
	cursor->setLayer(LAYER_HIDDEN);
	
	//Cartridge Page Scene Nodes	
	if(isLoading) {
		infotext->setText("Loading Cartridge", COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
		infotext->setPosition(Vector(vid_getScreenWidth()/2, vid_getScreenHeight()/2) - infotext->getSize()/2);
		infotext->setLayer(5);
		shade->setSize(infotext->getSize());
		shade->setPosition(infotext->getPosition());
		shade->setLayer(9);
		
		dpadIcon->setLayer(LAYER_HIDDEN);
		dpadText->setLayer(LAYER_HIDDEN);
		abtnIcon->setLayer(LAYER_HIDDEN);
		abtnText->setLayer(LAYER_HIDDEN);
		bbtnIcon->setLayer(LAYER_HIDDEN);
		bbtnText->setLayer(LAYER_HIDDEN);
		loadingText->setText("Loading...", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
		loadingText->setPosition(Vector(vid_getScreenWidth()-(loadingText->getSize().X+screenMargin), loadingText->getPosition().Y));
		loadingText->setLayer(5);
	
		this->render();
	} else {
		infotext->setText("No Cartridge", COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
		infotext->setPosition(Vector(vid_getScreenWidth()/2, vid_getScreenHeight()/2) - infotext->getSize()/2);
		infotext->setLayer(5);
		refreshButton->setPosition(Vector(vid_getScreenWidth()/2 - refreshButton->getSize().X/2, infotext->getPosition().Y+infotext->getSize().Y+20));
		refreshButton->setLayer(5);
	}
}

//! Switches to the cartridge page
void CMenuManager::setPageCartridge(const char* title, const char* boxImg, const char* titleImg, const char* snapImg, bool isGBA, bool canPlay)
{
	pageState = MENU_PAGE_STATE_CARTRIDGE;
	clearPageSpecificNodes();
	
	//General Scene Nodes
	cartButton->setImage("data/img/icon_cartridge.png", Vector(100,100), true);
	cartButton->setPosition(Vector((vid_getScreenWidth()/2)-110,screenMargin));
	cartButton->setNavPath(MENU_SELECTION_STATE_NONE);
	subtitle->setText("Cartridge", COLOR_LIGHTBLUE, COLOR_DARKGRAY, 36, false);
	subtitle->setLayer(5);
	cursor->setLayer(LAYER_HIDDEN);
	
	//Cartridge Page Scene Nodes
	int height = 500;
	if(smallScreen) height = 300;
	int screenhieght = height/2 - 10;
	int screenwidth = (float)screenhieght*1.5f;
	if(!isGBA) screenwidth = (float)screenhieght*1.1125f;
	artBox->setImage(boxImg, Vector(height, height), true);
	artBox->setPosition(Vector(vid_getScreenWidth()/2 - (height+screenwidth+20)/2, vid_getScreenHeight()/2 - height/2));
	artBox->setLayer(5);
	artTitle->setImage(titleImg, Vector(screenwidth,screenhieght), false);
	artTitle->setPosition(Vector((vid_getScreenWidth()/2 - (height+screenwidth+20)/2) + height+20, vid_getScreenHeight()/2 - height/2));
	artTitle->setLayer(5);
	artSnap->setImage(snapImg, Vector(screenwidth,screenhieght), false);
	artSnap->setPosition(Vector((vid_getScreenWidth()/2 - (height+screenwidth+20)/2) + height+20, vid_getScreenHeight()/2 + 10));
	artSnap->setLayer(5);
	gameTitle->setText(title, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	gameTitle->setPosition(Vector(vid_getScreenWidth()/2 - gameTitle->getSize().X/2, artBox->getPosition().Y-60));
	gameTitle->setLayer(5);
	
	playButton->setPosition(Vector(vid_getScreenWidth()/2 - (playButton->getSize().X+20+syncButton->getSize().X+10), vid_getScreenHeight()/2 + height/2 + 20));
	playButton->setLayer(5);
	if(canPlay) {
		playButton->setImage("data/img/icon_play.png", Vector(80,80), false);
		playButton->setNavPath(MENU_SELECTION_STATE_PLAY);
	} else {
		playButton->setImageLayered("data/img/icon_play.png", COLOR_DARKGRAY, 180, Vector(80,80), false);
		playButton->setNavPath(MENU_SELECTION_STATE_NONE);
	}
	syncButton->setPosition(Vector(vid_getScreenWidth()/2 - (syncButton->getSize().X+10), vid_getScreenHeight()/2 + height/2 + 20));
	syncButton->setLayer(5);
	refreshButton->setPosition(Vector(vid_getScreenWidth()/2 + (10+syncButton->getSize().X+20), vid_getScreenHeight()/2 + height/2 + 20));
	refreshButton->setLayer(5);
}

//! Switches to the catalog page
void CMenuManager::setPageCatalog(const char** titles, const char** art, int count)
{
	pageState = MENU_PAGE_STATE_CATALOG;
	clearPageSpecificNodes();
	
	//General Scene Nodes
	catButton->setImage("data/img/icon_catalog.png", Vector(100,100), true);
	catButton->setPosition(Vector((vid_getScreenWidth()/2)+10,screenMargin));
	catButton->setNavPath(MENU_SELECTION_STATE_NONE);
	subtitle->setText("Catalog", COLOR_LIGHTORANGE, COLOR_DARKGRAY, 36, false);
	subtitle->setLayer(5);
	cursor->setLayer(LAYER_HIDDEN);
	
	if(count > 0) {
		
		//Data
		carouselTitles = titles;
		carouselArt = art;
		carouselCount = count;
		carouselIndex = 0;
		
		//Cartridge Page Scene Nodes
		infotext->setText("Opening Catalog", COLOR_WHITE, COLOR_DARKGRAY, 36, false);
		infotext->setPosition(Vector(vid_getScreenWidth()/2, vid_getScreenHeight()/2) - infotext->getSize()/2);
		infotext->setLayer(5);
		setCarouselIndex(0, false, 0);
	} else {
	
		//Data
		carouselTitles = 0;
		carouselArt = 0;
		carouselCount = 0;
		carouselIndex = 0;
		
		//Cartridge Page Scene Nodes
		infotext->setText("Catalog is Empty", COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
		infotext->setPosition(Vector(vid_getScreenWidth()/2, vid_getScreenHeight()/2) - infotext->getSize()/2);
		infotext->setLayer(5);
	}
}
	
//! Switches to the settings page
void CMenuManager::setPageSettings(bool canBackupUSB)
{
	pageState = MENU_PAGE_STATE_SETTINGS;
	clearPageSpecificNodes();
	
	//General Scene Nodes
	subtitle->setText("Settings", COLOR_LIGHTGRAY, COLOR_DARKGRAY, 36, false);
	subtitle->setLayer(5);
	
	//Settings Page Scene Nodes
	stResolutionLabel->setLayer(5);
	stResolutionValue->setLayer(5);
	stGBEmulatorLabel->setLayer(5);
	stGBEmulatorValue->setLayer(5);
	stGBAEmulatorLabel->setLayer(5);
	stGBAEmulatorValue->setLayer(5);
	stSpacer->setLayer(5);
	stBtConnectButton->setLayer(5);
	stUSBBackupButton->setLayer(5);
	stUSBRestoreButton->setLayer(5);
	if(canBackupUSB) {
		stUSBBackupButton->setImage("data/img/icon_usbup.png", Vector(80,80), false);
		stUSBBackupButton->setNavPath(MENU_SELECTION_STATE_SETTINGS_BACKUP_USB);
		stUSBRestoreButton->setImage("data/img/icon_usbdown.png", Vector(80,80), false);
		stUSBRestoreButton->setNavPath(MENU_SELECTION_STATE_SETTINGS_RESTORE_USB);
	} else {
		stUSBBackupButton->setImageLayered("data/img/icon_usbup.png", COLOR_DARKGRAY, 180, Vector(80,80), false);
		stUSBBackupButton->setNavPath(MENU_SELECTION_STATE_NONE);
		stUSBRestoreButton->setImageLayered("data/img/icon_usbdown.png", COLOR_DARKGRAY, 180, Vector(80,80), false);
		stUSBRestoreButton->setNavPath(MENU_SELECTION_STATE_NONE);
	}
	updateSettings();
}

//! Displays a modal dialog to the user and returns the selection
int CMenuManager::showModal(const char* text1, const char* text2, const char** buttonText, const char** buttonDesc, int numButtons)
{
	int selection = -1;
	if(numButtons < 0) {
		selection = 0; //force selecting first button
		numButtons = -numButtons;
	}
	if(numButtons > MENU_MAX_MODAL_BUTTONS) numButtons = MENU_MAX_MODAL_BUTTONS;
	Vector cursorPos = cursor->getPosition();
	Vector cursorSize = cursor->getSize();
	int currSelection = selectionState;
	if(currSelection == MENU_SELECTION_STATE_CAROUSEL) currSelection = MENU_SELECTION_STATE_CAROUSEL + carouselIndex;
	const char* navAText = 0;
	for(int i=0; i<smgr->getSceneNodeCount(); i++) {
		if(smgr->getSceneNodes()[i]->getNavPath() == currSelection) {
			navAText = smgr->getSceneNodes()[i]->getNavAText();
			break;
		}
	}
	
	//record current state of scene nodes
	unsigned char* sceneNodeLayers = new unsigned char[smgr->getSceneNodeCount()];
	for(int i=0; i<smgr->getSceneNodeCount(); i++) sceneNodeLayers[i] = smgr->getSceneNodes()[i]->getLayer();
	
	//setup modal sizes
	int padding = 20;
	int buttonPadding = 10;
	int buttonSpacing = 10;
	int buttonHeight = 40;
	modal->setLayer(10);
	modalText[0]->setText(text1, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	modalText[0]->setLayer(11);
	modalText[1]->setText(text2, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	modalText[1]->setLayer(11);
	int buttonsLength = 0;
	for(int i=0; i<numButtons; i++) {
		modalButtonText[i]->setText(buttonText[i], COLOR_OFFWHITE, COLOR_GRAY, 24, false);
		modalButtonText[i]->setLayer(12);
		modalButton[i]->setSize(Vector(modalButtonText[i]->getSize().X + buttonPadding*2, buttonHeight));
		modalButton[i]->setLayer(11);
		buttonsLength += modalButton[i]->getSize().X;
		if(i<numButtons-1) buttonsLength += buttonSpacing;
	}
	int width = buttonsLength;
	if(width < modalText[0]->getSize().X) width = modalText[0]->getSize().X;
	if(width < modalText[1]->getSize().X) width = modalText[1]->getSize().X;
	width += padding*2;
	int height = 200;
	if(numButtons == 0) height = modalText[0]->getSize().Y + modalText[1]->getSize().Y + padding*2;
	
	//setup modal positions
	modal->setSize(Vector(width,height));
	modal->setPosition(Vector((vid_getScreenWidth()-width)/2,(vid_getScreenHeight()-height)/2));
	modalText[0]->setPosition(Vector((vid_getScreenWidth()-modalText[0]->getSize().X)/2,(vid_getScreenHeight()-height)/2 + padding));
	modalText[1]->setPosition(Vector((vid_getScreenWidth()-modalText[1]->getSize().X)/2,(vid_getScreenHeight()-height)/2 + modalText[0]->getSize().Y + padding));
	int buttonSpacingCalc = 0;
	for(int i=0; i<numButtons; i++) buttonSpacingCalc += modalButton[i]->getSize().X;
	buttonSpacingCalc = (width-buttonSpacingCalc)/(numButtons+1);
	int buttonSpacingTemp = 0;
	for(int i=0; i<numButtons; i++) {
		modalButton[i]->setPosition(modal->getPosition()+Vector((buttonSpacingCalc*(i+1))+buttonSpacingTemp, height-(buttonHeight+padding)));
		modalButtonText[i]->setPosition(modalButton[i]->getPosition()+Vector(buttonPadding,(buttonHeight-modalButtonText[i]->getSize().Y)/2));
		buttonSpacingTemp += modalButton[i]->getSize().X;
	}
	
	//position cursor and set initial selection
	if(numButtons > 0) {
		cursor->setPosition(modalButton[0]->getPosition());
		cursor->setSize(modalButton[0]->getSize());
		cursor->setLayer(14);
		abtnText->setText(buttonDesc[0], COLOR_WHITE, COLOR_DARKGRAY, 24, false);
		abtnText->setPosition(Vector(abtnIcon->getPosition().X+37, abtnText->getPosition().Y));
	} else {
		cursor->setLayer(LAYER_HIDDEN);
		abtnText->setText("Close", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
		abtnText->setPosition(Vector(abtnIcon->getPosition().X+37, abtnText->getPosition().Y));
	}
	
	//render shade
	shade->setSize(Vector(vid_getScreenWidth(),vid_getScreenHeight()-(150+90)));
	shade->setPosition(Vector(0,150));
	shade->setLayer(9);
	this->render();
	
	//hide everything except the changeable elements
	for(int i=0; i<smgr->getSceneNodeCount(); i++) smgr->getSceneNodes()[i]->setLayer(LAYER_HIDDEN);
	background->setSize(Vector(vid_getScreenWidth(),90));
	background->setPosition(Vector(0,vid_getScreenHeight()-90));
	background->setLayer(1);
	bottomLine->setLayer(5);
	dpadIcon->setLayer(5);
	dpadText->setLayer(5);
	abtnIcon->setLayer(5);
	abtnText->setLayer(5);
	modal->setLayer(10);
	modalText[0]->setLayer(11);
	modalText[1]->setLayer(11);
	for(int i=0; i<numButtons; i++) {
		modalButtonText[i]->setLayer(12);
		modalButton[i]->setLayer(11);
	}
	if(numButtons > 0) cursor->setLayer(14);
	
	//mini loop
	if(selection < 0) {
		selection = 0;
		while(true) {
			inp_updateButtonState();
			
			//move cursor?
			if(inp_getButtonState(INP_BTN_LF) == 1 && numButtons > 0 && selection > 0) {
				selection--;
				cursorTarget = modalButton[selection];
				abtnText->setText(buttonDesc[selection], COLOR_WHITE, COLOR_DARKGRAY, 24, false);
			}
			if(inp_getButtonState(INP_BTN_RT) == 1 && numButtons > 0 && selection < numButtons-1) {
				selection++;
				cursorTarget = modalButton[selection];
				abtnText->setText(buttonDesc[selection], COLOR_WHITE, COLOR_DARKGRAY, 24, false);
			}
			
			//exit?
			if(inp_getButtonState(INP_BTN_A) == 1) break;
			
			//cursorTarget
			this->render();
		}
	}
	
	//restore state of scene nodes
	cursorTarget = 0;
	cursor->setPosition(cursorPos);
	cursor->setSize(cursorSize);
	abtnText->setText(navAText, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	background->setSize(Vector(vid_getScreenWidth(),vid_getScreenHeight()));
	background->setPosition(Vector(0,0));
	for(int i=0; i<smgr->getSceneNodeCount(); i++) smgr->getSceneNodes()[i]->setLayer(sceneNodeLayers[i]);
	delete[] sceneNodeLayers;
	
	//return the user selection
	return selection;
}
	
//! Displays the progress bar and starts the async update process
void CMenuManager::showProgressBar(const char* text, int estimatedDuration)
{
	//record current state of scene nodes
	delete[] showProgressBarSceneNodeLayers;
	showProgressBarSceneNodeLayers = new unsigned char[smgr->getSceneNodeCount()];
	for(int i=0; i<smgr->getSceneNodeCount(); i++) showProgressBarSceneNodeLayers[i] = smgr->getSceneNodes()[i]->getLayer();
	
	//setup progress bar
	dpadIcon->setLayer(LAYER_HIDDEN);
	dpadText->setLayer(LAYER_HIDDEN);
	abtnIcon->setLayer(LAYER_HIDDEN);
	abtnText->setLayer(LAYER_HIDDEN);
	bbtnIcon->setLayer(LAYER_HIDDEN);
	bbtnText->setLayer(LAYER_HIDDEN);
	loadingText->setText(text, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	loadingText->setPosition(Vector(vid_getScreenWidth()-(loadingText->getSize().X+screenMargin+loadingBar->getSize().X+10), loadingText->getPosition().Y));
	loadingText->setLayer(5);
	loadingBar->setLayer(5);
	
	//render shade
	cursor->setLayer(LAYER_HIDDEN);
	shade->setSize(Vector(vid_getScreenWidth(),vid_getScreenHeight()-(150+90)));
	shade->setPosition(Vector(0,150));
	shade->setLayer(9);
	this->render();
	
	//hide everything except the changeable elements
	for(int i=0; i<smgr->getSceneNodeCount(); i++) smgr->getSceneNodes()[i]->setLayer(LAYER_HIDDEN);
	
	//start progress draw thread
	endProgressBarThread = false;
	int posX = loadingBar->getPosition().X;
	int posY = loadingBar->getPosition().Y;
	int sizeX = loadingBar->getSize().X;
	int sizeY = loadingBar->getSize().Y;
	void* params[6];
	params[0] = (void*)(&endProgressBarThread);
	params[1] = (void*)(&estimatedDuration);
	params[2] = (void*)(&posX);
	params[3] = (void*)(&posY);
	params[4] = (void*)(&sizeX);
	params[5] = (void*)(&sizeY);
	pthread_t progressBarThreadId; 
    pthread_create(&progressBarThreadId, NULL, mm_processProgressBar, (void*)params);
}

//! Ends the progress bar process and returns menu to previous state
void CMenuManager::endProgressBar()
{
	//end thread
	endProgressBarThread = true;
	
	//clean up state
	for(int i=0; i<smgr->getSceneNodeCount(); i++) smgr->getSceneNodes()[i]->setLayer(showProgressBarSceneNodeLayers[i]);
	delete[] showProgressBarSceneNodeLayers;
	showProgressBarSceneNodeLayers = 0;
}
	
//! Shows the given text in the bottom right corner until selection or page is changed
void CMenuManager::showLoadingText(const char* text)
{
	if(text > 0) {
		unsigned char cursorLayer = cursor->getLayer();
		unsigned char dpadIconLayer = dpadIcon->getLayer();
		unsigned char dpadTextLayer = dpadText->getLayer();
		unsigned char abtnIconLayer = abtnIcon->getLayer();
		unsigned char abtnTextLayer = abtnText->getLayer();
		unsigned char bbtnIconLayer = bbtnIcon->getLayer();
		unsigned char bbtnTextLayer = bbtnText->getLayer();
	
		cursor->setLayer(LAYER_HIDDEN);
		dpadIcon->setLayer(LAYER_HIDDEN);
		dpadText->setLayer(LAYER_HIDDEN);
		abtnIcon->setLayer(LAYER_HIDDEN);
		abtnText->setLayer(LAYER_HIDDEN);
		bbtnIcon->setLayer(LAYER_HIDDEN);
		bbtnText->setLayer(LAYER_HIDDEN);
		loadingText->setText(text, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
		loadingText->setPosition(Vector(vid_getScreenWidth()-(loadingText->getSize().X+screenMargin), loadingText->getPosition().Y));
		loadingText->setLayer(5);
		
		this->render();
		
		//restore old state
		cursor->setLayer(cursorLayer);
		dpadIcon->setLayer(dpadIconLayer);
		dpadText->setLayer(dpadTextLayer);
		abtnIcon->setLayer(abtnIconLayer);
		abtnText->setLayer(abtnTextLayer);
		bbtnIcon->setLayer(bbtnIconLayer);
		bbtnText->setLayer(bbtnTextLayer);
	}
	
	loadingText->setLayer(LAYER_HIDDEN);
	loadingBar->setLayer(LAYER_HIDDEN);
}

//! Sets the current page selection
void CMenuManager::setPageSelection(int selection, bool noAnimation)
{
	cursorTarget = 0;
	const char* aText = 0;
	const char* bText = 0;
	
	//special case for catalog page
	if(selection >= MENU_SELECTION_STATE_CAROUSEL && selection < MENU_SELECTION_STATE_CAROUSEL+1000) {
		cursorTarget = setCarouselIndex(selection - MENU_SELECTION_STATE_CAROUSEL, true, &noAnimation);
		selection = MENU_SELECTION_STATE_CAROUSEL;
		aText = carousel[0]->getNavAText();
		bText = carousel[0]->getNavBText();
	} else {
		
		//search for node with selection as nav
		for(int i=0; i<smgr->getSceneNodeCount(); i++) {
			if(smgr->getSceneNodes()[i]->getNavPath() == selection) {
				cursorTarget = smgr->getSceneNodes()[i];
				aText = smgr->getSceneNodes()[i]->getNavAText();
				bText = smgr->getSceneNodes()[i]->getNavBText();
				break;
			}
		}
	}
	
	//check if selection was valid
	if(cursorTarget) {
		selectionState = selection;
		cursor->setLayer(7);
		
		//dpad
		dpadIcon->setLayer(5);
		dpadText->setLayer(5);
	
		//a button text?
		if(aText) {
			abtnIcon->setPosition(Vector(dpadText->getPosition().X+dpadText->getSize().X+30, abtnIcon->getPosition().Y));
			abtnIcon->setLayer(5);
			abtnText->setText(aText, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
			abtnText->setPosition(Vector(abtnIcon->getPosition().X+37, abtnText->getPosition().Y));
			abtnText->setLayer(5);
		} else {
			abtnText->setLayer(LAYER_HIDDEN);
			abtnIcon->setLayer(LAYER_HIDDEN);
		}
		
		//b button text?
		if(bText) {
			bbtnIcon->setPosition(Vector(abtnText->getPosition().X+abtnText->getSize().X+30, bbtnIcon->getPosition().Y));
			bbtnIcon->setLayer(5);
			bbtnText->setText(bText, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
			bbtnText->setPosition(Vector(bbtnIcon->getPosition().X+37, bbtnText->getPosition().Y));
			bbtnText->setLayer(5);
		} else {
			bbtnText->setLayer(LAYER_HIDDEN);
			bbtnIcon->setLayer(LAYER_HIDDEN);
		}
		
		//misc
		loadingText->setLayer(LAYER_HIDDEN);
		loadingBar->setLayer(LAYER_HIDDEN);
		
		//skip animation?
		if(noAnimation) {
			cursor->setPosition(cursorTarget->getPosition());
			cursor->setSize(cursorTarget->getSize());
			cursorTarget = 0;
		}
	} else {
		selectionState = MENU_SELECTION_STATE_NONE;
		cursor->setLayer(LAYER_HIDDEN);
	} 
}

//! Gets the current page
int CMenuManager::getPage()
{
	return pageState;
}

//! Gets the current page selection
int CMenuManager::getPageSelection()
{
	return selectionState;
}
	
//! Gets the next page selection in the given direction
int CMenuManager::getNextSelection(char dir)
{
	int currSelection = selectionState;
	
	//special case for carousel
	if(currSelection == MENU_SELECTION_STATE_CAROUSEL) {
		if(dir == MENU_NEXT_SELECTION_RIGHT && carouselIndex < carouselCount-1) return MENU_SELECTION_STATE_CAROUSEL + (carouselIndex+1);
		if(dir == MENU_NEXT_SELECTION_LEFT && carouselIndex > 0) return MENU_SELECTION_STATE_CAROUSEL + (carouselIndex-1);
		if(dir == MENU_NEXT_SELECTION_RIGHT || dir == MENU_NEXT_SELECTION_LEFT) return MENU_SELECTION_STATE_NONE;
		currSelection = MENU_SELECTION_STATE_CAROUSEL + carouselIndex;
	}			
			
	//search for current selected node
	CSceneNode* selectedNode = 0;
	for(int i=0; i<smgr->getSceneNodeCount(); i++) {
		if(smgr->getSceneNodes()[i]->getNavPath() == currSelection) {
			selectedNode = smgr->getSceneNodes()[i];
			break;
		}
	}
	if(selectedNode > 0) {
		int upDownSearchRange = 1000;
		int leftRightSearchRange = 200;
		int range = upDownSearchRange;
		if(dir == MENU_NEXT_SELECTION_RIGHT || dir == MENU_NEXT_SELECTION_LEFT) range = leftRightSearchRange;
		
		//search for closest node in the given direction
		CSceneNode* closestNode = 0;
		int closestDist = 10000;
		for(int v=0; v<range; v++) {
			for(int i=0; i<smgr->getSceneNodeCount(); i++) {
				if(smgr->getSceneNodes()[i]->getNavPath() > MENU_SELECTION_STATE_NONE && smgr->getSceneNodes()[i]->getLayer() > LAYER_HIDDEN) {
					Vector testCenter = smgr->getSceneNodes()[i]->getPosition() + smgr->getSceneNodes()[i]->getSize()/2;
					Vector selectedCenter = selectedNode->getPosition() + selectedNode->getSize()/2;
					if((dir == MENU_NEXT_SELECTION_DOWN && selectedCenter.Y < testCenter.Y) || (dir == MENU_NEXT_SELECTION_UP && selectedCenter.Y > testCenter.Y)) {
						int dist = abs(selectedCenter.Y - testCenter.Y)*2 + abs(selectedCenter.X - testCenter.X);
						if((dist < closestDist) && (selectedCenter.X+v == testCenter.X || selectedCenter.X-v == testCenter.X)) {
							closestDist = dist;
							closestNode = smgr->getSceneNodes()[i];
						}
					}
					if((dir == MENU_NEXT_SELECTION_RIGHT && selectedCenter.X < testCenter.X) || (dir == MENU_NEXT_SELECTION_LEFT && selectedCenter.X > testCenter.X)) {
						int dist = abs(selectedCenter.X - testCenter.X)*2 + abs(selectedCenter.Y - testCenter.Y);
						if(dist < closestDist && (selectedCenter.Y+v == testCenter.Y || selectedCenter.Y-v == testCenter.Y)) {
							closestDist = dist;
							closestNode = smgr->getSceneNodes()[i];
						}
					}
				}
			}
		}
		if(closestNode > 0) {
			return closestNode->getNavPath();
		}
	}
	
	return MENU_SELECTION_STATE_NONE;
}
	
//! Gets the catalog index
int CMenuManager::getCatalogIndex()
{
	return carouselIndex;
}

//! Renders the menu
void CMenuManager::render()
{
	//sleep to maintain MENU_FRAME_RATE
	long currTime = clock();
	double diff = (double)(currTime - time) / CLOCKS_PER_SEC;
	if(diff < (1.0/MENU_FRAME_RATE)) {
		long usec = ((1.0/MENU_FRAME_RATE) - diff)*1000*1000;
		struct timespec ts;
		ts.tv_sec = usec / 1000000;
		ts.tv_nsec = (usec % 1000000) * 1000;
		nanosleep(&ts, &ts);
	}
	
	//settings page values
	if(pageState == MENU_PAGE_STATE_SETTINGS) updateSettings();
	
	//cursor animation
	if(cursorTarget) {
		cursor->setPosition((cursor->getPosition()+cursorTarget->getPosition()*3)/4);
		cursor->setSize((cursor->getSize()+cursorTarget->getSize()*3)/4);
		if(abs(cursor->getPosition().X - cursorTarget->getPosition().X) <= 1 
			&& abs(cursor->getPosition().Y - cursorTarget->getPosition().Y) <= 1
			&& abs(cursor->getSize().X - cursorTarget->getSize().X) <= 1 
			&& abs(cursor->getSize().Y - cursorTarget->getSize().Y) <= 1
		) {
			cursor->setPosition(cursorTarget->getPosition());
			cursor->setSize(cursorTarget->getSize());
			cursorTarget = 0;
		}
	}
	
	//carousel animation
	if(carouselTargetX != 0) {
		positionCarousel((carousel[0]->getPosition().X+carouselTargetX*2)/3);
		if(abs(carousel[0]->getPosition().X - carouselTargetX) <= 1) {
			positionCarousel(carouselTargetX);
			carouselTargetX = 0;
		}
		if(selectionState == MENU_SELECTION_STATE_CAROUSEL) {
			int pageStartIndex = carouselPage*MENU_MAX_CATALOG_CAROUSEL;
			int indexInPage = carouselIndex - pageStartIndex;
			cursor->setPosition(carousel[indexInPage]->getPosition());
		}
	}
	
	//draw all
	time = clock();
	smgr->drawAll();
}

//Util functions
void CMenuManager::clearPageSpecificNodes()
{
	//General Scene Nodes
	background->setSize(Vector(vid_getScreenWidth(),vid_getScreenHeight()));
	background->setPosition(Vector(0,0));
	shade->setLayer(LAYER_HIDDEN);
	modal->setLayer(LAYER_HIDDEN);
	modalText[0]->setText(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	modalText[0]->setLayer(LAYER_HIDDEN);
	modalText[1]->setText(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	modalText[1]->setLayer(LAYER_HIDDEN);
	for(int i=0; i<MENU_MAX_MODAL_BUTTONS; i++) {
		modalButton[i]->setLayer(LAYER_HIDDEN);
		modalButtonText[i]->setText(0, COLOR_OFFWHITE, COLOR_GRAY, 24, false);
		modalButtonText[i]->setLayer(LAYER_HIDDEN);
	}
	title->setLayer(5);
	subtitle->setText(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	subtitle->setLayer(LAYER_HIDDEN);
	infotext->setText(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	infotext->setLayer(LAYER_HIDDEN);
	cartButton->setImage("data/img/icon_cartridge.png", Vector(80,80), true);
	cartButton->setPosition(Vector((vid_getScreenWidth()/2)-100,screenMargin+10));
	cartButton->setNavPath(MENU_SELECTION_STATE_CARTRIDGE);
	cartButton->setLayer(5);
	catButton->setImage("data/img/icon_catalog.png", Vector(80,80), true);
	catButton->setPosition(Vector((vid_getScreenWidth()/2)+20,screenMargin+10));
	catButton->setNavPath(MENU_SELECTION_STATE_CATALOG);
	catButton->setLayer(5);
	settingsButton->setLayer(5);
	powerButton->setLayer(5);
	bottomLine->setLayer(5);
	dpadIcon->setLayer(5);
	dpadText->setLayer(5);
	loadingText->setLayer(LAYER_HIDDEN);
	loadingBar->setLayer(LAYER_HIDDEN);
	
	//Cartridge Page Scene Nodes
	gameTitle->setText(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	gameTitle->setLayer(LAYER_HIDDEN);
	artBox->setImage(0, Vector(0,0), true);
	artBox->setLayer(LAYER_HIDDEN);
	artTitle ->setImage(0, Vector(0,0), false);
	artTitle->setLayer(LAYER_HIDDEN);
	artSnap->setImage(0, Vector(0,0), false);
	artSnap->setLayer(LAYER_HIDDEN);
	playButton->setLayer(LAYER_HIDDEN);
	syncButton->setLayer(LAYER_HIDDEN);
	refreshButton->setLayer(LAYER_HIDDEN);
	
	//Catalog Page Scene Nodes
	catGameTitle->setText(0, COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
	catGameTitle->setLayer(LAYER_HIDDEN);
	for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) {
		carousel[i]->setImage(0, Vector(0,0), true);
		carousel[i]->setLayer(LAYER_HIDDEN);
		miniCarousel[i]->setImage(0, Vector(0,0), true);
		miniCarousel[i]->setLayer(LAYER_HIDDEN);
	}
	carouselNext->setImage(0, Vector(0,0), true);
	carouselNext->setLayer(LAYER_HIDDEN);
	miniCarouselNext->setImage(0, Vector(0,0), true);
	miniCarouselNext->setLayer(LAYER_HIDDEN);
	carouselPrevious->setImage(0, Vector(0,0), true);
	carouselPrevious->setLayer(LAYER_HIDDEN);
	miniCarouselPrevious->setImage(0, Vector(0,0), true);
	miniCarouselPrevious->setLayer(LAYER_HIDDEN);
	carouselCursor->setLayer(LAYER_HIDDEN);
	carouselCursor->setLayer(LAYER_HIDDEN);
	carouselPage = -1;
	carouselIndex = -1;
	
	//Settings Page Scene Nodes
	stResolutionLabel->setLayer(LAYER_HIDDEN);
	stResolutionValue->setText(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stResolutionValue->setLayer(LAYER_HIDDEN);
	stGBEmulatorLabel->setLayer(LAYER_HIDDEN);
	stGBEmulatorValue->setText(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBEmulatorValue->setLayer(LAYER_HIDDEN);
	stGBAEmulatorLabel->setLayer(LAYER_HIDDEN);
	stGBAEmulatorValue->setText(0, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBAEmulatorValue->setLayer(LAYER_HIDDEN);
	stSpacer->setLayer(LAYER_HIDDEN);
	stBtConnectButton->setLayer(LAYER_HIDDEN);
	stUSBBackupButton->setLayer(LAYER_HIDDEN);
	stUSBRestoreButton->setLayer(LAYER_HIDDEN);
}
CSceneNode* CMenuManager::setCarouselIndex(int index, bool updateCursor, bool* noAnimation)
{
	if(carouselCount > 0) {
		if(index < 0) index = 0;
		if(index >= carouselCount) index = carouselCount-1;
		
		//compile page related data
		int page = index/MENU_MAX_CATALOG_CAROUSEL;
		int pageStartIndex = page*MENU_MAX_CATALOG_CAROUSEL;
		int indexInPage = index - pageStartIndex;
		int pageSize = carouselCount - pageStartIndex;
		if(pageSize > MENU_MAX_CATALOG_CAROUSEL) pageSize = MENU_MAX_CATALOG_CAROUSEL;
		bool hasNext = (pageStartIndex+pageSize < carouselCount);
		bool hasPrevious = (pageStartIndex-1 >= 0);
		
		int height = 400;
		int spacing = 40;
		int miniHeight = 60;
		int miniSpacing = 6;
		if(smallScreen) {
			height = 250;
			miniHeight = 40;
		}
		
		//new page?
		if(carouselPage != page) {
			
			//show loading
			shade->setSize(Vector(vid_getScreenWidth(), vid_getScreenHeight()-(150+90)));
			shade->setPosition(Vector(0,150));
			shade->setLayer(9);
			cursor->setLayer(LAYER_HIDDEN);
			catGameTitle->setLayer(LAYER_HIDDEN);
			carouselCursor->setLayer(LAYER_HIDDEN);
			dpadIcon->setLayer(LAYER_HIDDEN);
			dpadText->setLayer(LAYER_HIDDEN);
			abtnIcon->setLayer(LAYER_HIDDEN);
			abtnText->setLayer(LAYER_HIDDEN);
			bbtnIcon->setLayer(LAYER_HIDDEN);
			bbtnText->setLayer(LAYER_HIDDEN);
			loadingText->setText("Loading...", COLOR_WHITE, COLOR_DARKGRAY, 24, false);
			loadingText->setPosition(Vector(vid_getScreenWidth()-(loadingText->getSize().X+screenMargin), loadingText->getPosition().Y));
			loadingText->setLayer(5);
			this->render();
			shade->setLayer(LAYER_HIDDEN);
			
			//load new images
			for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) {
				if(pageStartIndex+i < carouselCount) {
					carousel[i]->setImage(carouselArt[pageStartIndex+i], Vector(height,height), true);
					miniCarousel[i]->setImage(carouselArt[pageStartIndex+i], Vector(miniHeight,miniHeight), true);
				} else {
					carousel[i]->setImage(0, Vector(height,height), true);
					miniCarousel[i]->setImage(0, Vector(miniHeight,miniHeight), true);
				}
			}
			
			//next/previous images
			if(hasNext) {
				carouselNext->setImageLayered(carouselArt[pageStartIndex+pageSize], "data/img/icon_right_arrow_shadow.png", 190, Vector(height,height), true);
				miniCarouselNext->setImageLayered(carouselArt[pageStartIndex+pageSize], "data/img/icon_right_arrow_shadow.png", 190, Vector(miniHeight,miniHeight), true);
			} else {
				carouselNext->setImage(0, Vector(height,height), true);
				miniCarouselNext->setImage(0, Vector(miniHeight,miniHeight), true);
			}
			if(hasPrevious) {
				carouselPrevious->setImageLayered(carouselArt[pageStartIndex-1], "data/img/icon_left_arrow_shadow.png", 190, Vector(height,height), true);
				miniCarouselPrevious->setImageLayered(carouselArt[pageStartIndex-1], "data/img/icon_left_arrow_shadow.png", 190, Vector(miniHeight,miniHeight), true);
			} else {
				carouselPrevious->setImage(0, Vector(height,height), true);
				miniCarouselPrevious->setImage(0, Vector(miniHeight,miniHeight), true);
			}
		}
		
		//title
		infotext->setLayer(LAYER_HIDDEN);
		catGameTitle->setText(carouselTitles[index], COLOR_OFFWHITE, COLOR_DARKGRAY, 36, false);
		catGameTitle->setPosition(Vector(vid_getScreenWidth()/2 - catGameTitle->getSize().X/2, vid_getScreenHeight()/2 - (height/2 + catGameTitle->getSize().Y + 40)));
		catGameTitle->setLayer(5);
		
		//carousel positions
		int carouselOffset = (vid_getScreenWidth()/2 - height/2) - (height + spacing)*indexInPage;
		for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) carousel[i]->setLayer(LAYER_HIDDEN);
		carousel[indexInPage]->setLayer(5);
		for(int i=1; i<4; i++) {
			if(indexInPage-i >= 0) carousel[indexInPage-i]->setLayer(5);
			if(indexInPage+i < pageSize) carousel[indexInPage+i]->setLayer(5);
		}
		
		//carousel next/previous positions
		if(hasNext) carouselNext->setLayer(5);
		else carouselNext->setLayer(LAYER_HIDDEN);
		if(hasPrevious) carouselPrevious->setLayer(5);
		else carouselPrevious->setLayer(LAYER_HIDDEN);
		
		//mini carousel positions
		int pageAndNextPreviousSize = pageSize;
		if(hasNext) pageAndNextPreviousSize++;
		if(hasPrevious) pageAndNextPreviousSize++;
		int miniCarouselOffset = vid_getScreenWidth()/2 - (miniHeight*pageAndNextPreviousSize + miniSpacing*(pageAndNextPreviousSize-1))/2;
		if(hasPrevious) miniCarouselOffset += (miniHeight+miniSpacing);
		for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) {
			if(i < pageSize) {
				miniCarousel[i]->setPosition(Vector(miniCarouselOffset+((miniHeight+miniSpacing)*i), vid_getScreenHeight()/2 + height/2 + spacing*2));
				miniCarousel[i]->setLayer(5);
			} else {
				miniCarousel[i]->setLayer(LAYER_HIDDEN);
			}
		}
		
		//mini carousel next/previous positions
		if(hasNext) {
			miniCarouselNext->setPosition(Vector(miniCarouselOffset+((miniHeight+miniSpacing)*pageSize), vid_getScreenHeight()/2 + height/2 + spacing*2));
			miniCarouselNext->setLayer(5);
		} else {
			miniCarouselNext->setLayer(LAYER_HIDDEN);
		}
		if(hasPrevious) {
			miniCarouselPrevious->setPosition(Vector(miniCarouselOffset-(miniHeight+miniSpacing), vid_getScreenHeight()/2 + height/2 + spacing*2));
			miniCarouselPrevious->setLayer(5);
		} else {
			miniCarouselPrevious->setLayer(LAYER_HIDDEN);
		}
		
		//cursor
		if(updateCursor) {
			carouselCursor->setPosition(Vector(miniCarouselOffset + ((miniHeight+miniSpacing)*indexInPage) + (miniHeight-carouselCursor->getSize().X)/2, 
				vid_getScreenHeight()/2 + height/2 + spacing*2 - (carouselCursor->getSize().Y+5)));
			carouselCursor->setLayer(5);
		}
		
		//special end cases to use up more screen at the carousel ends
		if(carouselCount > 2 && ((index == 0) || (index == 1 && index > carouselIndex) 
				|| (index == carouselCount-1) || (index == carouselCount-2 && index < carouselIndex))) {
			if(index == 0) {
				carouselOffset -= (height+spacing);
				catGameTitle->setPosition(catGameTitle->getPosition()-Vector((height+spacing),0));
				if(catGameTitle->getPosition().X < screenMargin) 
					catGameTitle->setPosition(Vector(screenMargin, catGameTitle->getPosition().Y));
			}
			if(index == carouselCount-1) {
				carouselOffset += (height+spacing);
				catGameTitle->setPosition(catGameTitle->getPosition()+Vector((height+spacing),0));
				if(catGameTitle->getPosition().X > vid_getScreenWidth()-(screenMargin+catGameTitle->getSize().X)) 
					catGameTitle->setPosition(Vector(vid_getScreenWidth()-(screenMargin+catGameTitle->getSize().X), catGameTitle->getPosition().Y));
			}
			carouselTargetX = 0;
			positionCarousel(carouselOffset);
		} else {
		
			//animation
			if(noAnimation>0 && !(*noAnimation) && index != carouselIndex) {
				carouselTargetX = carouselOffset;
				if(page != carouselPage) {
					if(index > carouselIndex) positionCarousel(carouselOffset + (height+spacing));
					else positionCarousel(carouselOffset - (height+spacing));
				}
				*noAnimation = true;
			} else {
				carouselTargetX = 0;
				positionCarousel(carouselOffset);
			}
		}
		
		//adjust node nav paths
		for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) carousel[i]->setNavPath(MENU_SELECTION_STATE_NONE);
		carousel[indexInPage]->setNavPath(MENU_SELECTION_STATE_CAROUSEL + index);
		
		carouselPage = page;
		carouselIndex = index;
		return carousel[indexInPage];
	}
	return 0;
}
void CMenuManager::positionCarousel(int x)
{
	int height = 400;
	int spacing = 40;
	if(smallScreen) {
		height = 250;
	}
	
	//carousel positions
	for(int i=0; i<MENU_MAX_CATALOG_CAROUSEL; i++) {
		carousel[i]->setPosition(Vector(x+((height+spacing)*i), vid_getScreenHeight()/2 - height/2));
	}
	
	//carousel next/previous positions
	carouselNext->setPosition(Vector(x+((height+spacing)*MENU_MAX_CATALOG_CAROUSEL), vid_getScreenHeight()/2 - height/2));
	carouselPrevious->setPosition(Vector(x-(height+spacing), vid_getScreenHeight()/2 - height/2));
}
void CMenuManager::updateSettings()
{
	int xPadding = 30;
	int yPadding = 30;
	int ySpacing = stResolutionLabel->getSize().Y + yPadding;
	int numLines = 3;
	int linesOffset = -100;
	char buff[512];
	
	stResolutionLabel->setPosition(Vector(vid_getScreenWidth()/2 - (stResolutionLabel->getSize().X+xPadding), (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*0 + yPadding/2) + linesOffset));
	stmgr->getPropertyString(mm_resolutionSetting, "error", buff, 512);
	stResolutionValue->setText(buff, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stResolutionValue->setPosition(Vector(vid_getScreenWidth()/2 + xPadding, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*0 + yPadding/2) + linesOffset));
	
	stGBEmulatorLabel->setPosition(Vector(vid_getScreenWidth()/2 - (stGBEmulatorLabel->getSize().X+xPadding), (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*1 + yPadding/2) + linesOffset));
	stmgr->getPropertyString(mm_emulatorSettingGB, "error", buff, 512);
	stGBEmulatorValue->setText(buff, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBEmulatorValue->setPosition(Vector(vid_getScreenWidth()/2 + xPadding, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*1 + yPadding/2) + linesOffset));
	
	stGBAEmulatorLabel->setPosition(Vector(vid_getScreenWidth()/2 - (stGBAEmulatorLabel->getSize().X+xPadding), (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*2 + yPadding/2) + linesOffset));
	stmgr->getPropertyString(mm_emulatorSettingGBA, "error", buff, 512);
	stGBAEmulatorValue->setText(buff, COLOR_WHITE, COLOR_DARKGRAY, 24, false);
	stGBAEmulatorValue->setPosition(Vector(vid_getScreenWidth()/2 + xPadding, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*2 + yPadding/2) + linesOffset));
	
	stSpacer->setPosition(Vector(vid_getScreenWidth()/2 - 1, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (yPadding/2 - 10) + linesOffset));
	stSpacer->setSize(Vector(2, ySpacing*numLines));
	
	stBtConnectButton->setPosition(Vector(vid_getScreenWidth()/2 + 80, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*(numLines+1) + yPadding/2) + linesOffset));
	stUSBBackupButton->setPosition(Vector(vid_getScreenWidth()/2 - 260, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*(numLines+1) + yPadding/2) + linesOffset));
	stUSBRestoreButton->setPosition(Vector(vid_getScreenWidth()/2 - 160, (vid_getScreenHeight()/2 - (ySpacing*numLines)/2) + (ySpacing*(numLines+1) + yPadding/2) + linesOffset));
}

//Progress Bar Thread
static void* mm_processProgressBar(void* args)
{
	//get params
	void** params = (void**)args;
	bool* endFlag = (bool*)params[0];
	int duration = *((int*)params[1]);
	int posX = *((int*)params[2]);
	int posY = *((int*)params[3]);
	int sizeX = *((int*)params[4]);
	int sizeY = *((int*)params[5]);
	
	//loop
	int ticks = (duration / MENU_PROGRESS_BAR_UPDATE_MILLIS) + 1;
	for(int i=0; i<ticks; i++) {
		//sleep a bit before drawing
		if(ticks > 1) {
			struct timespec ts;
			ts.tv_sec = MENU_PROGRESS_BAR_UPDATE_MILLIS / 1000;
			ts.tv_nsec = (MENU_PROGRESS_BAR_UPDATE_MILLIS % 1000) * 1000000;
			nanosleep(&ts, &ts);
		}
		
		//check for thread end flag
		if(*endFlag) break;
		
		//draw more progress
		Color c = COLOR_WHITE;
		int width = (sizeX*(i+1))/ticks;
		vid_drawBox(posX, posY, width, sizeY, c.Red, c.Green, c.Blue, 255);
		vid_flush();
	}
	return 0;
}

//-----------------------------------------------------------------------------------------
// Title:	GameBoy Console Application
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CSettingsManager.h"
#include "CSceneManager.h"
#include "CMenuManager.h"
#include "CGameManager.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vid.h>
#include <bt.h>
#include <usb.h>
#include <spi.h>
#include <egpio.h>
#include <nrf.h>
#include <vkey.h>
#include <wgc.h>
#include <inp.h>
#include <gbx.h>

#define SPI_CLK_SPEED 10000000

#define BUTTON_AUTO_SCROLL_HOLD 10
#define BUTTON_DELETE_HOLD 30
#define BUTTON_POWER_HOLD 50

//functions
void core_close();

//program entry
int main(int argc, char** argv)
{
	//check for program skip flag
	if(remove(".skip") == 0) return 0;
	
	//init core modules
	if(vid_init() || bt_init() || usb_init() || spi_init(SPI_CLK_SPEED) || egpio_init() || nrf_init() || vkey_init() || wgc_init() || inp_init(1,1) || gbx_init()) {
		printf("Init failed. Are you running as root??\n");
		core_close();
		return 1;
	}
	wgc_startPolling();
	
	//create the managers
	CSettingsManager* settingsManager = new CSettingsManager();
	CSceneManager* sceneManager = new CSceneManager();
	CMenuManager* menuManager = new CMenuManager(sceneManager, settingsManager);
	CGameManager* gameManager = new CGameManager(settingsManager);	
	
	//start on the Cartridge Page if cartridge, otherwise Catalog Page
	menuManager->setPageCartridgeEmpty(true);
	menuManager->render();
	gameManager->loadCartridge();
	if(gameManager->getCartridgeType() != CARTRIDGE_TYPE_NONE) {
		menuManager->setPageCartridge(gameManager->getCartridgeName(), gameManager->getCartridgeImgBoxart(), gameManager->getCartridgeImgTitle(), 
			gameManager->getCartridgeImgSnap(), gameManager->getCartridgeType()==CARTRIDGE_TYPE_GBA, gameManager->getCartridgeCatalogIndex() >= 0);
		if(gameManager->getCartridgeCatalogIndex() >= 0) menuManager->setPageSelection(MENU_SELECTION_STATE_PLAY, true);
		else menuManager->setPageSelection(MENU_SELECTION_STATE_SYNC, true);
	} else if(gameManager->getCatalogSize() > 0) {
		menuManager->setPageCatalog(gameManager->getCatalogNames(), gameManager->getCatalogImgBoxarts(), gameManager->getCatalogSize());
		if(gameManager->getCatalogSize() > 0) menuManager->setPageSelection(MENU_SELECTION_STATE_CAROUSEL + 0, true);
		else menuManager->setPageSelection(MENU_SELECTION_STATE_CARTRIDGE, true);
	} else {
		menuManager->setPageCartridgeEmpty(false);
		menuManager->setPageSelection(MENU_SELECTION_STATE_REFRESH, true);
	}
	
	//main loop
	long clock = 0;
	bool xPressStarted = false;
	while(true) {
		inp_updateButtonState();
		xPressStarted |= (inp_getButtonState(INP_BTN_X) == 1);
		
		
		// Update Selection State
		int selectionState = MENU_SELECTION_STATE_NONE;
		if(inp_getButtonState(INP_BTN_UP) == 1) selectionState = menuManager->getNextSelection(MENU_NEXT_SELECTION_UP);
		if(inp_getButtonState(INP_BTN_DN) == 1) selectionState = menuManager->getNextSelection(MENU_NEXT_SELECTION_DOWN);
		if(inp_getButtonState(INP_BTN_LF) == 1) selectionState = menuManager->getNextSelection(MENU_NEXT_SELECTION_LEFT);
		if(inp_getButtonState(INP_BTN_RT) == 1) selectionState = menuManager->getNextSelection(MENU_NEXT_SELECTION_RIGHT);
		if(menuManager->getPageSelection() == MENU_SELECTION_STATE_CAROUSEL) {
			if(inp_getButtonState(INP_BTN_LF) > BUTTON_AUTO_SCROLL_HOLD && (clock%3) == 0) selectionState = menuManager->getNextSelection(MENU_NEXT_SELECTION_LEFT);
			if(inp_getButtonState(INP_BTN_RT) > BUTTON_AUTO_SCROLL_HOLD && (clock%3) == 0) selectionState = menuManager->getNextSelection(MENU_NEXT_SELECTION_RIGHT);
		}
		if(selectionState != MENU_SELECTION_STATE_NONE) {
			menuManager->setPageSelection(selectionState, false);
		}
		selectionState = menuManager->getPageSelection();
		
		
		// Exit?
		if(inp_getButtonState(INP_BTN_RT) > 0 && inp_getButtonState(INP_BTN_LF) > 0 
			&& inp_getButtonState(INP_BTN_UP) > 0 && inp_getButtonState(INP_BTN_DN) > 0 && inp_getButtonState(INP_BTN_A) == 1) break;
			
		
		// Cartridge Page?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_CARTRIDGE) {
			menuManager->setPageCartridgeEmpty(true);
			gameManager->loadCartridge();
			if(gameManager->getCartridgeType() == CARTRIDGE_TYPE_NONE) {
				menuManager->setPageCartridgeEmpty(false);
				menuManager->setPageSelection(MENU_SELECTION_STATE_REFRESH, true);
			} else {
				menuManager->setPageCartridge(gameManager->getCartridgeName(), gameManager->getCartridgeImgBoxart(), gameManager->getCartridgeImgTitle(), 
					gameManager->getCartridgeImgSnap(), gameManager->getCartridgeType()==CARTRIDGE_TYPE_GBA, gameManager->getCartridgeCatalogIndex() >= 0);
				if(gameManager->getCartridgeCatalogIndex() >= 0) menuManager->setPageSelection(MENU_SELECTION_STATE_PLAY, true);
				else menuManager->setPageSelection(MENU_SELECTION_STATE_SYNC, true);
			}
		}
		
		// Cartridge Refresh?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_REFRESH) {
			menuManager->setPageCartridgeEmpty(true);
			gameManager->loadCartridge();
			if(gameManager->getCartridgeType() == CARTRIDGE_TYPE_NONE) {
				menuManager->setPageCartridgeEmpty(false);
				menuManager->setPageSelection(MENU_SELECTION_STATE_REFRESH, true);
			} else {
				menuManager->setPageCartridge(gameManager->getCartridgeName(), gameManager->getCartridgeImgBoxart(), gameManager->getCartridgeImgTitle(), 
					gameManager->getCartridgeImgSnap(), gameManager->getCartridgeType()==CARTRIDGE_TYPE_GBA, gameManager->getCartridgeCatalogIndex() >= 0);
				if(gameManager->getCartridgeCatalogIndex() >= 0) menuManager->setPageSelection(MENU_SELECTION_STATE_PLAY, true);
				else menuManager->setPageSelection(MENU_SELECTION_STATE_SYNC, true);
			}
		}
		
		// Cartridge Sync?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_SYNC) {
			//check if sync is ready
			if(gameManager->getCartridgeCatalogIndex() > -1) menuManager->showLoadingText("Checking Cartridge...");
			int syncCheck = gameManager->syncCartridgeCheck();
			
			bool doSync = true;
			bool updateCartSave = false;
			if(syncCheck == SYNC_CHECK_SAVE_CHANGED) {
				const char* buttonText[] = { "Write to Cart", "Write to Sys", "Cancel" };
				const char* buttonDesc[] = { "Overwrite Cartridge Save Data", "Overwrite System Save Data", "Cancel Sync" };
				int selection = menuManager->showModal("Cartridge save data has changed since last sync.", "Overwrite cartridge or system save data?", buttonText, buttonDesc, 3);
				if(selection == 0) updateCartSave = true;
				if(selection == 2) doSync = false;
				
			} else if(syncCheck == SYNC_CHECK_SAVE_UNCHANGED) {
				const char* buttonText[] = { "Write to Cart", "Cancel" };
				const char* buttonDesc[] = { "Overwrite Cartridge Save Data", "Cancel Sync" };
				int selection = menuManager->showModal("Are you sure you want to overwrite", "the cartridge save data?", buttonText, buttonDesc, 2);
				if(selection == 0) updateCartSave = true;
				if(selection == 1) doSync = false;
				
			} else if(syncCheck == SYNC_CHECK_SAVE_NONE) {
				menuManager->showModal("Cartridge Has No Save Data:", "nothing to sync", 0, 0, 0);
				doSync = false;
				
			} else if(syncCheck == SYNC_CHECK_ERROR_UNKNOWN) {
				menuManager->showModal("Failed to Sync Cartridge:", "unknown error", 0, 0, 0);
				doSync = false;
				
			} else if(syncCheck == SYNC_CHECK_ERROR_NO_CARTRIDGE) {
				menuManager->showModal("Failed to Sync Cartridge:", "cartridge not connected", 0, 0, 0);
				menuManager->setPageCartridgeEmpty(false);
				menuManager->setPageSelection(MENU_SELECTION_STATE_REFRESH, true);
				doSync = false;
				
			} else if(syncCheck == SYNC_CHECK_ERROR_CARTRIDGE_CHANGED) {
				menuManager->showModal("Failed to Sync Cartridge:", "cartridge changed", 0, 0, 0);
				menuManager->setPageCartridge(gameManager->getCartridgeName(), gameManager->getCartridgeImgBoxart(), gameManager->getCartridgeImgTitle(), 
					gameManager->getCartridgeImgSnap(), gameManager->getCartridgeType()==CARTRIDGE_TYPE_GBA, gameManager->getCartridgeCatalogIndex() >= 0);
				if(gameManager->getCartridgeCatalogIndex() >= 0) menuManager->setPageSelection(MENU_SELECTION_STATE_PLAY, true);
				else menuManager->setPageSelection(MENU_SELECTION_STATE_SYNC, true);
				doSync = false;
			}
			
			//perform sync
			if(doSync) {
				int estimateMillis = gameManager->syncCartridgeEstimateTime(updateCartSave);
				menuManager->showProgressBar("Syncing", estimateMillis);
				gameManager->syncCartridge(updateCartSave);
				menuManager->endProgressBar();
				
				menuManager->setPageCartridge(gameManager->getCartridgeName(), gameManager->getCartridgeImgBoxart(), gameManager->getCartridgeImgTitle(), 
					gameManager->getCartridgeImgSnap(), gameManager->getCartridgeType()==CARTRIDGE_TYPE_GBA, gameManager->getCartridgeCatalogIndex() >= 0);
				if(gameManager->getCartridgeCatalogIndex() >= 0) menuManager->setPageSelection(MENU_SELECTION_STATE_PLAY, true);
				else menuManager->setPageSelection(MENU_SELECTION_STATE_SYNC, true);
			}
		}
		
		// Cartridge Play?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_PLAY) {
			menuManager->setPageNull("Loading");
			menuManager->render();
			menuManager->clearSceneAssets();
			vid_clear();
			gameManager->playGame(gameManager->getCartridgeCatalogIndex());
			
			vid_init();
			menuManager->loadSceneAssets();
			menuManager->setPageCartridge(gameManager->getCartridgeName(), gameManager->getCartridgeImgBoxart(), gameManager->getCartridgeImgTitle(), 
				gameManager->getCartridgeImgSnap(), gameManager->getCartridgeType()==CARTRIDGE_TYPE_GBA, gameManager->getCartridgeCatalogIndex() >= 0);
			if(gameManager->getCartridgeCatalogIndex() >= 0) menuManager->setPageSelection(MENU_SELECTION_STATE_PLAY, true);
			else menuManager->setPageSelection(MENU_SELECTION_STATE_SYNC, true);
		}
		
		
		// Catalog Page?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_CATALOG) {
			menuManager->setPageCatalog(gameManager->getCatalogNames(), gameManager->getCatalogImgBoxarts(), gameManager->getCatalogSize());
			if(gameManager->getCatalogSize() > 0) menuManager->setPageSelection(MENU_SELECTION_STATE_CAROUSEL + 0, true);
			else menuManager->setPageSelection(MENU_SELECTION_STATE_CARTRIDGE, true);
		}
		
		// Catalog Play?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_CAROUSEL) {
			int catalogIndex = menuManager->getCatalogIndex();
			menuManager->setPageNull("Loading");
			menuManager->render();
			menuManager->clearSceneAssets();
			vid_clear();
			gameManager->playGame(catalogIndex);
			
			vid_init();
			menuManager->loadSceneAssets();
			menuManager->setPageCatalog(gameManager->getCatalogNames(), gameManager->getCatalogImgBoxarts(), gameManager->getCatalogSize());
			menuManager->setPageSelection(MENU_SELECTION_STATE_CAROUSEL + catalogIndex, true);
		}
		
		// Catalog Delete?
		if(inp_getButtonState(INP_BTN_B) == BUTTON_DELETE_HOLD && selectionState == MENU_SELECTION_STATE_CAROUSEL) {
				const char* buttonText[] = { "Delete", "Cancel" };
				const char* buttonDesc[] = { "Delete Game", "Cancel Delete" };
				int selection = menuManager->showModal("Are you sure you want to delete", "this game and all save data?", buttonText, buttonDesc, 2);
				if(selection == 0) {
					int catalogIndex = menuManager->getCatalogIndex();
					gameManager->removeGame(catalogIndex);
					
					catalogIndex = catalogIndex - 1;
					if(catalogIndex < 0) catalogIndex = 0;
					menuManager->setPageCatalog(gameManager->getCatalogNames(), gameManager->getCatalogImgBoxarts(), gameManager->getCatalogSize());
					if(gameManager->getCatalogSize() > 0) menuManager->setPageSelection(MENU_SELECTION_STATE_CAROUSEL + catalogIndex, true);
					else menuManager->setPageSelection(MENU_SELECTION_STATE_CARTRIDGE, true);
				}
		}
		
		
		// Settings Page?
		if(inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_SETTINGS) {
			menuManager->setPageSettings(true);
			menuManager->setPageSelection(MENU_SELECTION_STATE_SETTINGS_RESOLUTION, true);
		}
		
		// Settings Resolution Change?
		if(selectionState == MENU_SELECTION_STATE_SETTINGS_RESOLUTION && inp_getButtonState(INP_BTN_A) == 1) {
			if(inp_getButtonState(INP_BTN_LF) == 1) settingsManager->setResolution(settingsManager->getResolution()-1);
			else settingsManager->setResolution(settingsManager->getResolution()+1);
		}
		
		// Settings GB Emulator Change?
		if(selectionState == MENU_SELECTION_STATE_SETTINGS_GB_EMULATOR && inp_getButtonState(INP_BTN_A) == 1) {
			if(inp_getButtonState(INP_BTN_LF) == 1) gameManager->setGBEmulator(gameManager->getGBEmulator()-1);
			else gameManager->setGBEmulator(gameManager->getGBEmulator()+1);
		}
		
		// Settings GBA Emulator Change?
		if(selectionState == MENU_SELECTION_STATE_SETTINGS_GBA_EMULATOR && inp_getButtonState(INP_BTN_A) == 1) {
			if(inp_getButtonState(INP_BTN_LF) == 1) gameManager->setGBAEmulator(gameManager->getGBAEmulator()-1);
			else gameManager->setGBAEmulator(gameManager->getGBAEmulator()+1);
		}
		
		// Settings Pair Bluetooth?
		if((selectionState == MENU_SELECTION_STATE_SETTINGS_CONNECT_BT && inp_getButtonState(INP_BTN_A) == 1) || (xPressStarted && inp_getButtonState(INP_BTN_X) == 0)) {
			xPressStarted = false;
			const char* buttonText[] = { "Cancel" };
			const char* buttonDesc[] = { "Cancel Bluetooth Pairing" };
			int selection = menuManager->showModal("Scanning for", "Bluetooth Controller...", buttonText, buttonDesc, -1);
			
			//remove paired devices
			bt_device** devices = bt_getDevices();
			for(int i=0; devices[i]; i++) {
				if(strstr(devices[i]->name, "8Bitdo SN30")==devices[i]->name) bt_removeDevice(devices[i]->mac);
			}
			
			//look for desired device
			bt_startPairTrustConnect("8Bitdo SN30");
			while(true) {
				inp_updateButtonState();
				if(inp_getButtonState(INP_BTN_A) == 1 || inp_getButtonState(INP_BTN_X) == 1) break;
				if(bt_checkPairTrustConnect() == BT_PTC_CONNECTED) {
					menuManager->showModal("Bluetooth Controller Paired", "Successfully!", 0, 0, 0);
					break;
				}
			}
			bt_stopPairTrustConnect();
		}
		
		// Settings USB Backup?
		if(selectionState == MENU_SELECTION_STATE_SETTINGS_BACKUP_USB && inp_getButtonState(INP_BTN_A) == 1) {
			if(usb_isAvailable()) {
				menuManager->setPageNull("Writing to USB...");
				menuManager->render();
				gameManager->backupToUSB();
				menuManager->setPageSettings(true);
				menuManager->setPageSelection(MENU_SELECTION_STATE_SETTINGS_BACKUP_USB, true);
			} else {
				menuManager->showModal("Failed to Backup to USB:", "no USB device detected", 0, 0, 0);
			}
		}
		
		// Settings USB Restore?
		if(selectionState == MENU_SELECTION_STATE_SETTINGS_RESTORE_USB && inp_getButtonState(INP_BTN_A) == 1) {
			if(usb_isAvailable()) {
				menuManager->setPageNull("Restoring from USB...");
				menuManager->render();
				gameManager->restoreFromUSB();
				menuManager->setPageSettings(true);
				menuManager->setPageSelection(MENU_SELECTION_STATE_SETTINGS_RESTORE_USB, true);
			} else {
				menuManager->showModal("Failed to Restore from USB:", "no USB device detected", 0, 0, 0);
			}
		}
		
		
		// Power?
		if((inp_getButtonState(INP_BTN_A) == 1 && selectionState == MENU_SELECTION_STATE_POWER) || inp_getButtonState(INP_BTN_X) == BUTTON_POWER_HOLD) {
			xPressStarted = false;
			const char* buttonText[] = { "Shutdown", "Cancel" };
			const char* buttonDesc[] = { "Shutdown System", "Cancel Shutdown" };
			int selection = menuManager->showModal("Are you sure you want to", "shutdown the system?", buttonText, buttonDesc, 2);
			if(selection == 0) {
				system("sudo shutdown now");
				break;
			}
		}
		
		
		menuManager->render();
		clock++;
	}
	
	
	//clean
	sceneManager->clearScene();
	
	//close down video
	core_close();
	return 0;
}

void core_close()
{
	gbx_close();
	inp_close();
	wgc_close();
	vkey_close();
	nrf_close();
	egpio_close();
	spi_close();
	usb_close();
	bt_close();
	vid_close();
}

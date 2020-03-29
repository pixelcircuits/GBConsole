#include "inp.h"
#include "vkey.h"
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <linux/input.h>

#define MAX_DEVICES 16
#define STATE_UPDATE_MILLIS 15000
#define DEVICE_UPDATE_MILLIS 8

#define TRACK_KEY_PRESS_Z          0
#define TRACK_KEY_PRESS_RIGHTSHIFT 1
#define TRACK_KEY_PRESS_ENTER      2
#define TRACK_KEY_PRESS_UP         3
#define TRACK_KEY_PRESS_DOWN       4
#define TRACK_KEY_PRESS_LEFT       5
#define TRACK_KEY_PRESS_RIGHT      6
#define TRACK_KEY_PRESS_X          7
#define TRACK_KEY_PRESS_Q          8
#define TRACK_KEY_PRESS_W          9
#define TRACK_KEY_PRESS_ESC        10
#define TRACK_KEY_PRESS_TOTAL      11

#define EIGHTBITDO_BTN_X 307
#define EIGHTBITDO_BTN_Y 308
#define EIGHTBITDO_BTN_A 304
#define EIGHTBITDO_BTN_B 305
#define EIGHTBITDO_BTN_L1 310
#define EIGHTBITDO_BTN_R1 311
#define EIGHTBITDO_BTN_SL 314
#define EIGHTBITDO_BTN_ST 315
#define EIGHTBITDO_BTN_UD 17
#define EIGHTBITDO_BTN_UD_UP -1
#define EIGHTBITDO_BTN_UD_DOWN 1
#define EIGHTBITDO_BTN_LR 16
#define EIGHTBITDO_BTN_LR_LEFT -1
#define EIGHTBITDO_BTN_LR_RIGHT 1

// Data
static char inp_isInitFlag = 0;
static char inp_deviceSize = 0;
static int inp_deviceFd[MAX_DEVICES];
static char* inp_deviceName[MAX_DEVICES];
static clock_t inp_lastDeviceUpdate = 0;
static int inp_keyPresses[TRACK_KEY_PRESS_TOTAL];
static unsigned char inp_buttonStates[10];
static char inp_inputType = INP_TYPE_UNKNOWN;
static int inp_hotkeyCount = 0;
static pthread_t inp_threadId = -1; 

// Helper Functions
static void inp_checkButtonInput();
static int inp_addDeviceToList(char* path);
static int inp_getDeviceFromList(char* path);
static void inp_removeDeviceFromList(int index);
static void inp_updateDeviceList();
static void inp_clearDeviceList();
static void inp_automateKeyboardHotkey(int code, int value);

// State polling thread
void* inp_thread_statePolling(void* args)
{
	while(inp_isInitFlag > 0) {
		inp_checkButtonInput();
		
		//sleep
		struct timespec ts;
		ts.tv_sec = DEVICE_UPDATE_MILLIS / 1000;
		ts.tv_nsec = (DEVICE_UPDATE_MILLIS % 1000) * 1000000;
		nanosleep(&ts, &ts);
	}
}

// Setup and initialize the Controller interface
int inp_init()
{
	//already initialized?
	if(inp_isInitFlag == 1) return 0;
	
	//data
	int i;
	for(i=0; i<TRACK_KEY_PRESS_TOTAL; i++) inp_keyPresses[i] = 0;
	
	//initial device search
	inp_updateDeviceList();
	
	//start thread
	inp_isInitFlag = 1;
    if(pthread_create(&inp_threadId, NULL, inp_thread_statePolling, NULL) > 0) {
		inp_close();
        return 1;
    }
	
	return 0;
}

// Checks if the Controller interface is initialized
char inp_isInit()
{
	return inp_isInitFlag;
}

// Updates the button holding state
void inp_updateButtonState()
{
	if(inp_keyPresses[TRACK_KEY_PRESS_X] > 0) {
		if(inp_buttonStates[INP_BTN_A] < 255) inp_buttonStates[INP_BTN_A]++;
	} else inp_buttonStates[INP_BTN_A] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_Z] > 0) {
		if(inp_buttonStates[INP_BTN_B] < 255) inp_buttonStates[INP_BTN_B]++;
	} else inp_buttonStates[INP_BTN_B] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_RIGHTSHIFT] > 0) {
		if(inp_buttonStates[INP_BTN_SL] < 255) inp_buttonStates[INP_BTN_SL]++;
	} else inp_buttonStates[INP_BTN_SL] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_ENTER] > 0) {
		if(inp_buttonStates[INP_BTN_ST] < 255) inp_buttonStates[INP_BTN_ST]++;
	} else inp_buttonStates[INP_BTN_ST] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_UP] > 0) {
		if(inp_buttonStates[INP_BTN_UP] < 255) inp_buttonStates[INP_BTN_UP]++;
	} else inp_buttonStates[INP_BTN_UP] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_DOWN] > 0) {
		if(inp_buttonStates[INP_BTN_DN] < 255) inp_buttonStates[INP_BTN_DN]++;
	} else inp_buttonStates[INP_BTN_DN] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_LEFT] > 0) {
		if(inp_buttonStates[INP_BTN_LF] < 255) inp_buttonStates[INP_BTN_LF]++;
	} else inp_buttonStates[INP_BTN_LF] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_RIGHT] > 0) {
		if(inp_buttonStates[INP_BTN_RT] < 255) inp_buttonStates[INP_BTN_RT]++;
	} else inp_buttonStates[INP_BTN_RT] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_Q] > 0) {
		if(inp_buttonStates[INP_BTN_L] < 255) inp_buttonStates[INP_BTN_L]++;
	} else inp_buttonStates[INP_BTN_L] = 0;
	
	if(inp_keyPresses[TRACK_KEY_PRESS_W] > 0) {
		if(inp_buttonStates[INP_BTN_R] < 255) inp_buttonStates[INP_BTN_R]++;
	} else inp_buttonStates[INP_BTN_R] = 0;
}

// Gets the holding state of the given button
unsigned char inp_getButtonState(char button)
{
	return inp_buttonStates[button];
}

// Gets the type of the last active input device
char inp_getInputType()
{
	return inp_inputType;
}

// Closes the Controller interface
int inp_close()
{
	inp_isInitFlag = 0;
	
	//wait for thread to stop
	if(inp_threadId > -1) pthread_join(inp_threadId, NULL);
	inp_threadId = -1;
	
	//close devices
	inp_clearDeviceList();
	
	return 0;
}

// Helper Functions
static void inp_checkButtonInput() {
	int i, n;
	
	//update list of input devices
	if((double)(clock() - inp_lastDeviceUpdate)/CLOCKS_PER_SEC > ((double)DEVICE_UPDATE_MILLIS)/1000.0) {
		inp_updateDeviceList();
	}
	
	//get input from devices
	for(i=0; i<inp_deviceSize; i++) {
		struct input_event ev[20];
		int rd = read(inp_deviceFd[i],ev,sizeof(ev));
		if(rd > 0) {
			int count = rd / sizeof(struct input_event);
			for(n=0; n<count; n++) {
				struct input_event *evp = &ev[n];
				if(evp->type == 1) {
					if(evp->code == KEY_Z || evp->code == EIGHTBITDO_BTN_B) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_Z]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_Z]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_Z] < 0) inp_keyPresses[TRACK_KEY_PRESS_Z] = 0;
					} else if(evp->code == KEY_RIGHTSHIFT || evp->code == EIGHTBITDO_BTN_SL) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_RIGHTSHIFT]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_RIGHTSHIFT]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_RIGHTSHIFT] < 0) inp_keyPresses[TRACK_KEY_PRESS_RIGHTSHIFT] = 0;
					} else if(evp->code == KEY_ENTER || evp->code == EIGHTBITDO_BTN_ST) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_ENTER]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_ENTER]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_ENTER] < 0) inp_keyPresses[TRACK_KEY_PRESS_ENTER] = 0;
					} else if(evp->code == KEY_UP) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_UP]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_UP]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_UP] < 0) inp_keyPresses[TRACK_KEY_PRESS_UP] = 0;
					} else if(evp->code == KEY_DOWN) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_DOWN]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_DOWN]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_DOWN] < 0) inp_keyPresses[TRACK_KEY_PRESS_DOWN] = 0;
					} else if(evp->code == KEY_LEFT) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_LEFT]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_LEFT]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_LEFT] < 0) inp_keyPresses[TRACK_KEY_PRESS_LEFT] = 0;
					} else if(evp->code == KEY_RIGHT) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_RIGHT]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_RIGHT]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_RIGHT] < 0) inp_keyPresses[TRACK_KEY_PRESS_RIGHT] = 0;
					} else if(evp->code == KEY_X || evp->code == EIGHTBITDO_BTN_A) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_X]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_X]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_X] < 0) inp_keyPresses[TRACK_KEY_PRESS_X] = 0;
					} else if(evp->code == KEY_Q || evp->code == EIGHTBITDO_BTN_L1) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_Q]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_Q]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_Q] < 0) inp_keyPresses[TRACK_KEY_PRESS_Q] = 0;
					} else if(evp->code == KEY_W || evp->code == EIGHTBITDO_BTN_R1) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_W]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_W]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_W] < 0) inp_keyPresses[TRACK_KEY_PRESS_W] = 0;
					} else if(evp->code == KEY_ESC) {
						if(evp->value == 0) inp_keyPresses[TRACK_KEY_PRESS_ESC]--;
						if(evp->value == 1) inp_keyPresses[TRACK_KEY_PRESS_ESC]++;
						if(inp_keyPresses[TRACK_KEY_PRESS_ESC] < 0) inp_keyPresses[TRACK_KEY_PRESS_ESC] = 0;
					}
					inp_automateKeyboardHotkey(evp->code, evp->value);
				} else if(evp->type == 3) {
					if(evp->code == EIGHTBITDO_BTN_UD) {
						if(evp->value == EIGHTBITDO_BTN_UD_UP) inp_keyPresses[TRACK_KEY_PRESS_UP]++;
						if(evp->value == EIGHTBITDO_BTN_UD_DOWN) inp_keyPresses[TRACK_KEY_PRESS_DOWN]++;
						if(evp->value == 0) {
							inp_keyPresses[TRACK_KEY_PRESS_UP]--;
							inp_keyPresses[TRACK_KEY_PRESS_DOWN]--;
						}
						if(inp_keyPresses[TRACK_KEY_PRESS_DOWN] < 0) inp_keyPresses[TRACK_KEY_PRESS_DOWN] = 0;
						if(inp_keyPresses[TRACK_KEY_PRESS_UP] < 0) inp_keyPresses[TRACK_KEY_PRESS_UP] = 0;
					} else if(evp->code == EIGHTBITDO_BTN_LR) {
						if(evp->value == EIGHTBITDO_BTN_LR_LEFT) inp_keyPresses[TRACK_KEY_PRESS_LEFT]++;
						if(evp->value == EIGHTBITDO_BTN_LR_RIGHT) inp_keyPresses[TRACK_KEY_PRESS_RIGHT]++;
						if(evp->value == 0) {
							inp_keyPresses[TRACK_KEY_PRESS_LEFT]--;
							inp_keyPresses[TRACK_KEY_PRESS_RIGHT]--;
						}
						if(inp_keyPresses[TRACK_KEY_PRESS_RIGHT] < 0) inp_keyPresses[TRACK_KEY_PRESS_RIGHT] = 0;
						if(inp_keyPresses[TRACK_KEY_PRESS_LEFT] < 0) inp_keyPresses[TRACK_KEY_PRESS_LEFT] = 0;
					}
				}
				
				//determine input device type
				if((evp->type == 1 && (evp->code == EIGHTBITDO_BTN_A || evp->code == EIGHTBITDO_BTN_B || evp->code == EIGHTBITDO_BTN_SL || evp->code == EIGHTBITDO_BTN_ST || evp->code == EIGHTBITDO_BTN_L1 || evp->code == EIGHTBITDO_BTN_R1))
						|| (evp->type == 3 && (evp->code == EIGHTBITDO_BTN_UD || evp->code == EIGHTBITDO_BTN_LR))) {
					inp_inputType = INP_TYPE_JOYSTICK;
				} else if(evp->type == 1 && (evp->code == KEY_RIGHTSHIFT || evp->code == KEY_ENTER || evp->code == KEY_UP || evp->code == KEY_DOWN || evp->code == KEY_LEFT || evp->code == KEY_RIGHT
						|| evp->code == KEY_Z || evp->code == KEY_X || evp->code == KEY_Q || evp->code == KEY_W || evp->code == KEY_ESC)) {
					inp_inputType = INP_TYPE_KEYBOARD;
				}
			}
		}
    }
}
static int inp_addDeviceToList(char* path) {
	if(inp_deviceSize < MAX_DEVICES-1) {
		int fd = open(path, O_RDONLY | O_NONBLOCK);
		if(fd > -1) {
			inp_deviceFd[inp_deviceSize] = fd;
			inp_deviceName[inp_deviceSize] = (char*)malloc((strlen(path)+1)*sizeof(char));
			strcpy(inp_deviceName[inp_deviceSize], path);
			inp_deviceSize++;
		}
	}
}
static int inp_getDeviceFromList(char* path) {
	int i;
	for(i=0; i<inp_deviceSize; i++) {
		if(strcmp(path, inp_deviceName[i])==0) return i;
	}
	return -1;
}
static void inp_removeDeviceFromList(int index) {
	int i;
	if(index >= 0 && index < inp_deviceSize) {
		free(inp_deviceName[index]);
		close(inp_deviceFd[index]);
	}
	for(i=index; i<inp_deviceSize-1; i++) {
		inp_deviceName[i] = inp_deviceName[i+1];
		inp_deviceFd[i] = inp_deviceFd[i+1];
	}
	inp_deviceSize--;
}
static void inp_updateDeviceList() {
	int i;
	char deviceFoundFlag[MAX_DEVICES];
	for(i=0; i<MAX_DEVICES; i++) {
		if(i < inp_deviceSize) deviceFoundFlag[i] = 0;
		else deviceFoundFlag[i] = 1;
	}
	
	//open device list
	int devicesFd = open("/proc/bus/input/devices", O_RDONLY | O_NONBLOCK);
	if(devicesFd > 0) {
		FILE* fp = fdopen(devicesFd, "r");
		char line[1024];
		while(fgets(line, 1024, fp)!=NULL) {
			if(strstr(line, "N: Name=")!=NULL) {
				char name[512];
				for(i=0; line[i+9]!='"' && line[i+9]!='\n'; i++) name[i] = line[i+9];
				name[i] = 0;
				
				//check for new device
				while(fgets(line, 1024, fp)!=NULL) {
					char* ev;
					if(strstr(line, "H: Handlers=")!=NULL) {
						if((ev=strstr(line, "event"))!=NULL && (strstr(line, "kbd")!=NULL || strstr(line, "js0")!=NULL || strstr(line, "js1")!=NULL)) {
							char event[64];
							for(i=0; ev[i]!=' ' && ev[i]!='\n'; i++) event[i] = ev[i];
							event[i] = 0;
							char fullPath[1024];
							sprintf(fullPath, "/dev/input/%s", event);
							
							int deviceIndex = inp_getDeviceFromList(fullPath);
							if(deviceIndex > -1) deviceFoundFlag[deviceIndex] = 1;
							else {
								//printf("found device %s: %s\n", name, fullPath); //new device
								inp_addDeviceToList(fullPath);
							}
						}
						break;
					}
				}
			}
		}
		fclose(fp);
	}
	
	//remove devices that no longer exist
	for(i=inp_deviceSize-1; i>=0; i--) {
		if(deviceFoundFlag[i]==0) {
			//printf("lost device %s\n", inp_deviceName[i]); //lost device
			inp_removeDeviceFromList(i);
		}
	}
	
	inp_lastDeviceUpdate = clock();
}
static void inp_clearDeviceList() {
	int i;
	for(i=0; i<inp_deviceSize; i++) {
		free(inp_deviceName[i]);
		close(inp_deviceFd[i]);
	}
	inp_deviceSize = 0;
}
static void inp_automateKeyboardHotkey(int code, int value) {
	if(code == KEY_ESC || code == KEY_F7 || code == KEY_F6 || code == KEY_F2 || code == KEY_F4 || code == KEY_SPACE || code == KEY_P || code == KEY_R) {
		if(value == 1) {
			inp_hotkeyCount++;
			vkey_setKeyState(KEY_HOME, 1);
		} else if(value == 0) {
			inp_hotkeyCount--;
			if(inp_hotkeyCount < 1) {
				inp_hotkeyCount = 0;
				vkey_setKeyState(KEY_HOME, 0);
			}
		}
	}
}

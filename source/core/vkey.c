#include "vkey.h"
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <linux/uinput.h>

// Constants
static const char* vkey_deviceId = "vkey-uinput";
static const int vkey_keyBits[] = { KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, 
	KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, 
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, 
	KEY_KP0, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7, KEY_KP8, KEY_KP9, 
	KEY_KPASTERISK, KEY_KPMINUS, KEY_KPPLUS, KEY_KPDOT, KEY_KPJPCOMMA, KEY_KPENTER, KEY_KPSLASH, KEY_KPEQUAL, KEY_KPPLUSMINUS, 
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, 
	KEY_ESC, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE, KEY_TAB, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER, KEY_LEFTCTRL, 
	KEY_RIGHTCTRL, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_LEFTSHIFT, KEY_RIGHTSHIFT, KEY_BACKSLASH, KEY_COMMA, 
	KEY_DOT, KEY_SLASH, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACE, KEY_CAPSLOCK, KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_HOME, 
	KEY_PAGEUP, KEY_PAGEDOWN, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_END, KEY_INSERT, KEY_DELETE
};

// Data
static int vkey_fd = -1;

// Helper functions
static void vkey_emit(int type, int code, int val);

// Setup and initialize the virtual keyboard
int vkey_init()
{
	int i;
	
	//already initialized?
	if(vkey_fd != -1) return 0;
	
	//open device
	if((vkey_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK) ) < 0) {
		fprintf(stderr, "vkey_init: Unable to open /dev/uinput: %s\n", strerror(errno));
		vkey_close();
		return 1;
	}

	//set phys
	ioctl(vkey_fd, UI_SET_PHYS, vkey_deviceId);

	//enable types and keys
	ioctl(vkey_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(vkey_fd, UI_SET_EVBIT, EV_ABS);
	for(i=0; i<102; i++) ioctl(vkey_fd, UI_SET_KEYBIT, vkey_keyBits[i]);
		
	//set axis absolute values (necessary for emulators)
	struct uinput_abs_setup abs_setup;
	memset(&abs_setup, 0, sizeof(abs_setup));
	abs_setup.code = ABS_X;
	abs_setup.absinfo.value = 0;
	abs_setup.absinfo.minimum = 255;
	abs_setup.absinfo.maximum = 0;
	abs_setup.absinfo.fuzz = 0;
	abs_setup.absinfo.flat = 0;
	abs_setup.absinfo.resolution = 0;
	ioctl(vkey_fd, UI_ABS_SETUP, &abs_setup);
	abs_setup.code = ABS_Y;
	ioctl(vkey_fd, UI_ABS_SETUP, &abs_setup);
	
	//setup deivce info
	struct uinput_setup usetup;
	memset(&usetup, 0, sizeof(usetup));
	strncpy(usetup.name, vkey_deviceId, sizeof(usetup.name) - 1);
	usetup.id.vendor  = 0x1;
	usetup.id.product = 0x1;
	usetup.id.version = 0x1;
	usetup.id.bustype = 0x3;
	usetup.ff_effects_max = FF_MAX_EFFECTS;
	ioctl(vkey_fd, UI_DEV_SETUP, &usetup);

	//create device
	ioctl(vkey_fd, UI_DEV_CREATE);

	//wait some time until the Window Manager can get the reference 
	//for the new virtual device to receive data from
	sleep(1);
	return 0;
}

// Checks if the Virtual Keyboard interface is initialized
char vkey_isInit()
{
	if(vkey_fd != -1) return 1;
	return 0;
}

// Gets the name of the virtual keyboard
const char* vkey_getDeviceName()
{
	return vkey_deviceId;
}

// Sets the state of the given key
void vkey_setKeyState(int code, char state)
{
	if(vkey_fd >= 0) {
		if(state > 0) state = 1;
		vkey_emit(EV_KEY, code, state);
		vkey_emit(EV_SYN, SYN_REPORT, 0);
	}
}

// Closes the virtual keyboard
int vkey_close()
{
	if(vkey_fd >= 0) {
		ioctl(vkey_fd, UI_DEV_DESTROY);
	    close(vkey_fd);
	    vkey_fd = -1;
	}
	
	return 0;
}

// Emits code value for the virtual device
static void vkey_emit(int type, int code, int val) {
	struct input_event ie;
	memset(&ie, 0, sizeof(ie));
	ie.type = type;
	ie.code = code;
	ie.value = val;
	write(vkey_fd, &ie, sizeof(ie));
}

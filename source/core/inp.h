#ifndef INP_H
#define INP_H

#define INP_BTN_A  0
#define INP_BTN_B  1
#define INP_BTN_RT 2
#define INP_BTN_LF 3
#define INP_BTN_UP 4
#define INP_BTN_DN 5
#define INP_BTN_SL 6
#define INP_BTN_ST 7
#define INP_BTN_L  8
#define INP_BTN_R  9

#define INP_TYPE_UNKNOWN 0
#define INP_TYPE_KEYBOARD 1
#define INP_TYPE_JOYSTICK 2

// Setup and initialize the Controller interface
int inp_init();

// Checks if the Controller interface is initialized
char inp_isInit();

// Updates the button holding state
void inp_updateButtonState();

// Gets the holding state of the given button
unsigned char inp_getButtonState(char button);

// Gets the type of the last active input device
char inp_getInputType();

// Closes the Controller interface
int inp_close();


#endif /* INP_H */

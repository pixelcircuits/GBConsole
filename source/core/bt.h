#ifndef BT_H
#define BT_H

#define BT_DEVICE_TYPE_UNKNOWN 0
#define BT_DEVICE_TYPE_AUDIOSOURCE 1
#define BT_DEVICE_TYPE_HID 2

#define BT_PTC_NOT_STARTED 0
#define BT_PTC_SEARCHING 1
#define BT_PTC_PAIRING 2
#define BT_PTC_TRUSTING 3
#define BT_PTC_CONNECTING 4
#define BT_PTC_CONNECTED 5

typedef struct {
   char mac[18]; 
   char name[256];
   int type;
   char paired;
   char trusted;
   char connected;
} bt_device;

// Setup and initialize the Bluetooth utils
int bt_init();

// Checks if the Bluetooth utils are initialized
char bt_isInit();

// Turns on Bluetooth discoverable
void bt_discoverableOn();

// Turns off Bluetooth discoverable
void bt_discoverableOff();

// Turns on Bluetooth scan
void bt_scanOn();

// Turns off Bluetooth scan
void bt_scanOff();

// Pairs the given Bluetooth device
char bt_pairDevice(const char* mac);

// Connects the given Bluetooth device
char bt_connectDevice(const char* mac);

// Trusts the given Bluetooth device
char bt_trustDevice(const char* mac);

// Unrusts the given Bluetooth device
char bt_untrustDevice(const char* mac);

// Removes the given Bluetooth device
char bt_removeDevice(const char* mac);

// Gets list of bluetooth devices
bt_device** bt_getDevices();

// Gets the details of the given device
void bt_getDevicesDetails(bt_device* device);

// Starts the background pair, trust and connect process
void bt_startPairTrustConnect(const char* name);

// Checks on the background pair, trust and connect process
char bt_checkPairTrustConnect();

// Stops the background pair, trust and connect process
void bt_stopPairTrustConnect();

// Cleans up the Bluetooth utils
int bt_close();

#endif /* BT_H */

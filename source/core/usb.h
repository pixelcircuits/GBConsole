#ifndef USB_H
#define USB_H

// Setup and initialize the USB Storage interface
int usb_init();

// Checks if the USB Storage interface is initialized
char usb_isInit();

// Gets the path of the USB Storage device
const char* usb_getPath();

// Gets if the USB Storage device is available
char usb_isAvailable();

// Gets if the USB Storage device is mounted
char usb_isMounted();

// Mounts the USB Storage device
char usb_mount();

// Unmounts the USB Storage device
void usb_unmount();

// Closes the USB Storage interface
int usb_close();


#endif /* USB_H */

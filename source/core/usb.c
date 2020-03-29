#include "usb.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Constants
static const char* usb_mountLocation = "/media/USBFlash";

// Data
static char usb_isInitFlag = 0;
static char usb_available = 0;
static char usb_mounted = 0;

// Helper Functions
static char usb_checkAvailable();

// Setup and initialize the USB Storage interface
int usb_init()
{
	usb_isInitFlag = 1;
	return 0;
}

// Checks if the USB Storage interface is initialized
char usb_isInit()
{
	return usb_isInitFlag;
}

// Gets the path of the USB Storage device
const char* usb_getPath()
{
	return usb_mountLocation;
}

// Gets if the USB Storage device is available
char usb_isAvailable()
{
	return usb_checkAvailable();
}

// Gets if the USB Storage device is mounted
char usb_isMounted()
{
	return usb_mounted;
}

// Mounts the USB Storage device
char usb_mount() 
{
	char command[1024];
	
	//make sure the mount directory exists
	sprintf(command, "sudo -u$USER mkdir -p -m=0777 \"%s\"", usb_mountLocation);
	system(command);
	
	//try to mount sda1
	sprintf(command, "sudo mount /dev/sda1 %s > /dev/null 2>&1", usb_mountLocation);
	system(command);
		
	//check if actually mounted
	usb_mounted = 0;
	FILE *fp = popen("sudo mount", "r");
	if (fp != NULL) {
		char buff[1024];
		while (fgets(buff, 1024, fp) != NULL) {
			if(strncmp(buff, "/dev/sda1", 9)==0) {
				if(strncmp(buff+13, usb_mountLocation, strlen(usb_mountLocation))==0) {
					usb_mounted = 1;
				}
			}
		}		
		pclose(fp);
	}
	return usb_mounted;
}

// Unmounts the USB Storage device
void usb_unmount() 
{
	char command[1024];
	sprintf(command, "sudo umount /dev/sda1 > /dev/null 2>&1");
	system(command);
	usb_mounted = 0;
}

// Closes the USB Storage interface
int usb_close()
{
	usb_unmount();
	
	usb_isInitFlag = 0;
	return 0;
}

// Helper function to check if USB storage device is connected 
static char usb_checkAvailable() {
	usb_available = 0;
	FILE *fp = popen("sudo test -e /dev/sda1 ; echo $?", "r");
	if (fp != NULL) {
		char buff[32];
		while (fgets(buff, 32, fp) != NULL) {
			if(strncmp(buff, "0", 1)==0) usb_available = 1;
		}		
		pclose(fp);
	}
	return usb_available;
}

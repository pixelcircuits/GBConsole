#include "bt.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_DEVICES 64
#define UPDATER_POLL_US 50000
#define PTC_POLL_MS 500

#define THREAD_STATUS_RUNNING 0
#define THREAD_STATUS_CLOSING 1
#define THREAD_STATUS_END 2

// Data
static char bt_isInitFlag = 0;
static bt_device* bt_deviceListInternal[MAX_DEVICES];
static bt_device* bt_deviceList[MAX_DEVICES];

// Processor "bluetoothctl"
static int bluetoohctl_infd = -1;
static int bluetoohctl_outfd = -1;
static pid_t bluetoohctl_pid = -1;
static char bluetoohctl_open();
static void bluetoohctl_close();

// Pair, trust and connect process
static void* bt_thread_ptcStatePolling(void* args);
static const char* bt_ptcName = 0;
static char bt_ptcState = BT_PTC_NOT_STARTED;
static pthread_t bt_ptcThreadId = -1;

// Updater thread
static char bt_processBluetoothUpdatesThreadStatus = THREAD_STATUS_END;
static char bt_holdBluetoothUpdates = 0;
static void* bt_processBluetoothUpdates(void* args);
static void bt_startBluetoothUpdatesThread();
static void bt_stopBluetoothUpdatesThread();

// Helper Functions
static void bt_fetchDetails(bt_device* device);
static char bt_executeCommand(const char* command, const char* success, const char* fail);
static bt_device* bt_addDeviceToList(bt_device** list, const char* mac);
static bt_device* bt_getDeviceFromList(bt_device** list, const char* mac);
static void bt_removeDeviceFromList(bt_device** list, const char* mac);
static void bt_clearDeviceList(bt_device** list);
static void bt_copyDeviceList(bt_device** original, bt_device** copy);
static void bt_usleep(long useconds);

// Setup and initialize the Bluetooth utils
int bt_init()
{
	//data
	int i;
	for(i=0; i<MAX_DEVICES; i++) {
		bt_deviceListInternal[i] = 0;
		bt_deviceList[i] = 0;
	}
	
	//start bluetoohctl process
	if(bluetoohctl_open() != 0) return 1;
	
	//start updater thread
	bt_startBluetoothUpdatesThread();
	
	bt_isInitFlag = 1;
	return 0;
}

// Checks if the Bluetooth utils are initialized
char bt_isInit()
{
	return bt_isInitFlag;
}

// Turns on Bluetooth discoverable
void bt_discoverableOn()
{
	if(bluetoohctl_infd > -1) {
		write(bluetoohctl_infd, "discoverable on\n", strlen("discoverable on\n"));
	}
}

// Turns off Bluetooth discoverable
void bt_discoverableOff()
{
	if(bluetoohctl_infd > -1) {
		write(bluetoohctl_infd, "discoverable off\n", strlen("discoverable off\n"));
	}
}

// Turns on Bluetooth scan
void bt_scanOn()
{
	if(bluetoohctl_infd > -1) {
		write(bluetoohctl_infd, "scan on\n", strlen("scan on\n"));
	}
}

// Turns off Bluetooth scan
void bt_scanOff()
{
	if(bluetoohctl_infd > -1) {
		write(bluetoohctl_infd, "scan off\n", strlen("scan off\n"));
	}
}

// Pairs the given Bluetooth device
char bt_pairDevice(const char* mac)
{
	char command[64];
	sprintf(command, "pair %s\n", mac);
	return bt_executeCommand(command, "Pairing successful", "Failed to pair");
}

// Connects the given Bluetooth device
char bt_connectDevice(const char* mac)
{
	char command[64];
	sprintf(command, "connect %s\n", mac);
	return bt_executeCommand(command, "Connection successful", "Failed to connect");
}

// Trusts the given Bluetooth device
char bt_trustDevice(const char* mac)
{
	char command[64];
	sprintf(command, "trust %s\n", mac);
	return bt_executeCommand(command, "trust succeeded", 0);
}

// Unrusts the given Bluetooth device
char bt_untrustDevice(const char* mac)
{
	char command[64];
	sprintf(command, "untrust %s\n", mac);
	return bt_executeCommand(command, "untrust succeeded", 0);
}

// Removes the given Bluetooth device
char bt_removeDevice(const char* mac)
{
	char command[64];
	sprintf(command, "remove %s\n", mac);
	return bt_executeCommand(command, "Device has been removed", 0);
}

// Gets list of bluetooth devices
bt_device** bt_getDevices()
{
	//make sure devices aren't currently being updated
	while(bt_holdBluetoothUpdates);
	bt_holdBluetoothUpdates = 1;
	
	bt_copyDeviceList(bt_deviceListInternal, bt_deviceList);
		
	bt_holdBluetoothUpdates = 0;
	return bt_deviceList;
}

// Gets the details of the given device
void bt_getDevicesDetails(bt_device* device)
{
	//make sure devices aren't currently being updated
	while(bt_holdBluetoothUpdates);
	bt_holdBluetoothUpdates = 1;
	
	bt_device* dev = bt_getDeviceFromList(bt_deviceListInternal, device->mac);
	if(dev != 0) memcpy(device, dev, sizeof(bt_device));
	
	bt_holdBluetoothUpdates = 0;
}

// Starts the background pair, trust and connect process
void bt_startPairTrustConnect(const char* name)
{
	bt_ptcName = name;
	if(bt_ptcThreadId == -1) {
		if(pthread_create(&bt_ptcThreadId, NULL, bt_thread_ptcStatePolling, NULL) > 0) {
			bt_ptcThreadId = -1;
		}
	}
}

// Checks on the background pair, trust and connect process
char bt_checkPairTrustConnect()
{
	return bt_ptcState;
}

// Stops the background pair, trust and connect process
void bt_stopPairTrustConnect()
{
	if(bt_ptcThreadId != -1) {
		pthread_t threadId = bt_ptcThreadId;
		bt_ptcThreadId = -1;
		pthread_join(threadId, NULL);
	}
	bt_ptcState = BT_PTC_NOT_STARTED;
}

// Cleans up the Bluetooth utils
int bt_close()
{
	//data
	bt_clearDeviceList(bt_deviceListInternal);
	bt_clearDeviceList(bt_deviceList);
	
	//stop background pair, trust connect thread
	bt_stopPairTrustConnect();
	
	//stop updater thread
	bt_stopBluetoothUpdatesThread();
	
	//close bluetoothctl process
	bluetoohctl_close();
	
	bt_isInitFlag = 0;
	return 0;
}

// Processor "bluetoothctl"
static char bluetoohctl_open() {
	pid_t pid;
	int p_stdin[2], p_stdout[2];
	if(pipe(p_stdin) != 0 || pipe(p_stdout) != 0) return -1;

	//start bluetoothctl
	pid = fork();
	if(pid < 0) return -1;
	else if(pid == 0) {
		dup2(p_stdout[0], STDIN_FILENO);
		dup2(p_stdin[1], STDOUT_FILENO);
		dup2(p_stdin[1], STDERR_FILENO);

		execl("/usr/bin/bluetoothctl", NULL);
		exit(1);
	}
	close(p_stdout[0]);
	close(p_stdin[1]);
	bluetoohctl_infd = p_stdout[1];
	bluetoohctl_outfd = p_stdin[0];
	bluetoohctl_pid = pid;

	return 0;
}
static void bluetoohctl_close() {
	int status;

	//gracefully shutdown bluetoohctl
	if(bluetoohctl_pid > -1) {
		if(bluetoohctl_infd) write(bluetoohctl_infd, "quit\n", strlen("quit\n"));
		else kill(bluetoohctl_pid, SIGKILL);
		waitpid(bluetoohctl_pid, &status, 0);
	}
	bluetoohctl_pid = -1;

	//close pipes
	if(bluetoohctl_infd > -1) close(bluetoohctl_infd);
	bluetoohctl_infd = -1;
	if(bluetoohctl_outfd > -1) close(bluetoohctl_outfd);
	bluetoohctl_outfd = -1;
}

// Pair, trust and connect process
static void* bt_thread_ptcStatePolling(void* args)
{
	bt_ptcState = BT_PTC_SEARCHING;
	bt_scanOn();
	while(bt_ptcThreadId != -1) {
		bt_usleep(PTC_POLL_MS * 1000);
		
		bt_device** devices = bt_getDevices();
		for(int i=0; devices[i]; i++) {
			if(devices[i]->paired==0 && strstr(devices[i]->name, bt_ptcName)==devices[i]->name) {
				bt_ptcState = BT_PTC_PAIRING;
				bt_pairDevice(devices[i]->mac);
				bt_ptcState = BT_PTC_TRUSTING;
				bt_trustDevice(devices[i]->mac);
				bt_ptcState = BT_PTC_CONNECTING;
				bt_connectDevice(devices[i]->mac);
				bt_ptcState = BT_PTC_CONNECTED;
				
				bt_ptcThreadId = -1;
				break;
			}
		}
	}
	bt_scanOff();
	return 0;
}

// Updater thread
static void* bt_processBluetoothUpdates(void* args) {
	int i, count, lineEnd, bufferIndex = 0;
	char mac[18]; mac[17] = 0;
	char buffer[2048];
	memset(buffer, 0, 2048*sizeof(char));
	
	if(bluetoohctl_outfd > -1) {
		while(1==1) {
			if(bt_processBluetoothUpdatesThreadStatus) break;
			
			//read data into the buffer
			ioctl(bluetoohctl_outfd, FIONREAD, &count);
			if(count > 0) {
				count = read(bluetoohctl_outfd, buffer+bufferIndex, 2048-bufferIndex);
				for(i=bufferIndex; i<bufferIndex+count; i++) if(buffer[i] == '\r') buffer[i] = ' ';
				bufferIndex += count;
			}
			
			//look for newline
			for(lineEnd=0; lineEnd<bufferIndex; lineEnd++) if(buffer[lineEnd] == '\n') break;
			if(lineEnd < bufferIndex) {
				char line[2048];
				for(i=0; i<lineEnd; i++) line[i] = buffer[i];
				line[lineEnd] = 0;
				
				//remove line from buffer
				for(i=0; i<2048; i++) {
					if(i<(2048-(lineEnd+1))) buffer[i] = buffer[i+(lineEnd+1)];
					else buffer[i] = 0;
				}
				bufferIndex -= (lineEnd+1);
		
				//look for keywords
				char* deviceStart;
				if((deviceStart=strstr(line, "Device")) != NULL) {
					deviceStart += strlen("Device ");
					for(i=0; deviceStart[i]!=' ' && deviceStart[i]!='\n'; i++) mac[i] = deviceStart[i];
			
					//make sure devices aren't currently being accessed
					while(bt_holdBluetoothUpdates);
					bt_holdBluetoothUpdates = 1;
					
					//new device
					if(strstr(line, "NEW") != NULL) {
						//printf("New Device - %s\n", mac);
						bt_device* device = bt_addDeviceToList(bt_deviceListInternal, mac);
						if(device!=0) bt_fetchDetails(device);
					}
					
					//device changed
					if(strstr(line, "CHG") != NULL) {
						//printf("Device Changed - %s\n", mac);
						bt_device* device = bt_getDeviceFromList(bt_deviceListInternal, mac);
						if(device!=0) {
							if(strstr(line, "Paired: yes") != NULL) device->paired = 1;
							if(strstr(line, "Paired: no") != NULL) device->paired = 0;
							if(strstr(line, "Trusted: yes") != NULL) device->trusted = 1;
							if(strstr(line, "Trusted: no") != NULL) device->trusted = 0;
							if(strstr(line, "Connected: yes") != NULL) device->connected = 1;
							if(strstr(line, "Connected: no") != NULL) device->connected = 0;
						}
					}
					
					//delete device
					if(strstr(line, "DEL") != NULL) {
						//printf("Device Deleted - %s\n", mac);
						bt_removeDeviceFromList(bt_deviceListInternal, mac);
					}
					
					bt_holdBluetoothUpdates = 0;
				}
			}
			
			bt_usleep(UPDATER_POLL_US);
		}
	}
	bt_processBluetoothUpdatesThreadStatus = THREAD_STATUS_END;
	return 0;
}
static void bt_startBluetoothUpdatesThread() {
	pthread_t threadId;
	bt_stopBluetoothUpdatesThread();
	
	bt_holdBluetoothUpdates = 0;
	bt_processBluetoothUpdatesThreadStatus = THREAD_STATUS_RUNNING;
    pthread_create(&threadId, NULL, bt_processBluetoothUpdates, NULL);
}
static void bt_stopBluetoothUpdatesThread() {
	if(bt_processBluetoothUpdatesThreadStatus == THREAD_STATUS_RUNNING) bt_processBluetoothUpdatesThreadStatus = THREAD_STATUS_CLOSING;
	while(bt_processBluetoothUpdatesThreadStatus != THREAD_STATUS_END) bt_usleep(10000);
}

// Helper Functions
static void bt_fetchDetails(bt_device* device) {
	device->type = BT_DEVICE_TYPE_UNKNOWN;
	device->name[0] = 0;
	device->paired = 0;
	device->trusted = 0;
	device->connected = 0;
   
	char command[128];
	sprintf(command, "echo 'info %s\\nquit\\n' | /usr/bin/bluetoothctl", device->mac);
	FILE *fp = popen(command, "r");
	if(fp != NULL) {
		char line[1024];
		while (fgets(line, 1024, fp) != NULL) {
			if(strstr(line, "Paired: yes") != NULL) device->paired = 1;
			if(strstr(line, "Trusted: yes") != NULL) device->trusted = 1;
			if(strstr(line, "Connected: yes") != NULL) device->connected = 1;
			if(strstr(line, "AudioSource") != NULL) device->type = BT_DEVICE_TYPE_AUDIOSOURCE;
			if(strstr(line, "Human Interface Device") != NULL) device->type = BT_DEVICE_TYPE_HID;
			if(strstr(line, "Name: ") != NULL) {
				int i;
				char* start = strstr(line, "Name: ") + strlen("Name: ");
				for(i=0; start[i]!='\n' && i<255; i++) device->name[i] = start[i];
				device->name[i] = 0;
			}
		}
		pclose(fp);
	}
}
static char bt_executeCommand(const char* command, const char* success, const char* fail) {
	int status, result, p_stdin[2], p_stdout[2];
	if(pipe(p_stdin) != 0 || pipe(p_stdout) != 0) return 0;

	//start bluetoothctl
	pid_t pid = fork();
	if(pid < 0) return 0;
	else if(pid == 0) {
		dup2(p_stdout[0], STDIN_FILENO);
		dup2(p_stdin[1], STDOUT_FILENO);
		dup2(p_stdin[1], STDERR_FILENO);

		execl("/usr/bin/bluetoothctl", NULL);
		exit(1);
	}
	close(p_stdout[0]);
	close(p_stdin[1]);

	//send command
	write(p_stdout[1], command, strlen(command));
	
	//wait for confirmation
	FILE* fp = fdopen(p_stdin[0], "r");
	while(1==1) {
		char line[1024];
		fgets(line, 1024, fp);
		
		if(strstr(line, success)!=NULL) {
			result = 1;
			break;
		} else if((fail!=0 && strstr(line, fail)!=NULL) || strstr(line, "not available")!=NULL) {
			result = 0;
			break;
		}
	}

	//gracefully shutdown bluetoohctl
	write(p_stdout[1], "quit\n", strlen("quit\n"));
	waitpid(pid, &status, 0);

	//close pipes
	fclose(fp);
	close(p_stdin[0]);
	
	return result;
}
static bt_device* bt_addDeviceToList(bt_device** list, const char* mac) {
	int i;
	for(i=0; list[i]!=0 && i<MAX_DEVICES; i++);
	if(i<(MAX_DEVICES-1)) {
		list[i] = (bt_device*)malloc(sizeof(bt_device));
		memset(list[i], 0, sizeof(bt_device));
		strcpy(list[i]->mac, mac);
		return list[i];
	}
	return 0;
}
static bt_device* bt_getDeviceFromList(bt_device** list, const char* mac) {
	int i;
	for(i=0; list[i]!=0 && i<MAX_DEVICES; i++) {
		if(strcmp(mac, list[i]->mac)==0) return list[i];
	}
	return 0;
}
static void bt_removeDeviceFromList(bt_device** list, const char* mac) {
	int i, j;
	for(i=0; list[i]!=0 && i<MAX_DEVICES; i++) {
		if(strcmp(mac, list[i]->mac)==0) {
			free(list[i]);
			for(j=i; j<(MAX_DEVICES-1); j++) list[j] = list[j+1];
			list[MAX_DEVICES-1] = 0;
		}
	}
}
static void bt_clearDeviceList(bt_device** list) {
	int i;
	for(i=0; i<MAX_DEVICES; i++) {
		if(list[i]) free(list[i]);
		list[i] = 0;
	}
}
static void bt_copyDeviceList(bt_device** original, bt_device** copy) {
	bt_clearDeviceList(copy);
	int i;
	for(i=0; original[i]!=0 && i<MAX_DEVICES; i++) {
		copy[i] = (bt_device*)malloc(sizeof(bt_device));
		memcpy(copy[i], original[i], sizeof(bt_device));
	}
}
static void bt_usleep(long useconds) {
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = useconds * 1000;
	nanosleep(&ts, &ts);
}
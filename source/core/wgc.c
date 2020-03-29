#include "wgc.h"
#include "nrf.h"
#include "vkey.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h> 

#define POLLING_US 15000
#define TIMEOUT_US 5000000

#define POLLING_ONGOING 10

#define WGC_KEY_B   KEY_Z
#define WGC_KEY_SL  KEY_RIGHTSHIFT
#define WGC_KEY_ST  KEY_ENTER
#define WGC_KEY_UP  KEY_UP
#define WGC_KEY_DN  KEY_DOWN
#define WGC_KEY_LF  KEY_LEFT
#define WGC_KEY_RT  KEY_RIGHT
#define WGC_KEY_A   KEY_X
#define WGC_KEY_L   KEY_Q
#define WGC_KEY_R   KEY_W

#define WGC_MASK_A  0x0001
#define WGC_MASK_B  0x0002
#define WGC_MASK_RT 0x0004
#define WGC_MASK_LF 0x0008
#define WGC_MASK_UP 0x0010
#define WGC_MASK_DN 0x0020
#define WGC_MASK_SL 0x0040
#define WGC_MASK_ST 0x0080
#define WGC_MASK_L  0x0100
#define WGC_MASK_R  0x0200

#define CTRL_UNKNOWN 0
#define CTRL_SEARCHING 1
#define CTRL_GBA 2
#define CTRL_GB 3

// Data
static char wgc_isInitFlag = 0;
static uint8_t wgc_isPolling = 0;
static uint8_t wgc_nrfChannel = 0;
static uint8_t wgc_nrfRx[6] = "wgbr";
static uint8_t wgc_nrfTx[6] = "wgbtx";
static uint8_t wgc_ctrlType = 0;
static uint32_t wgc_timeout = 0;
static pthread_t wgc_threadId = -1; 
static int wgc_state = 0x0000;

// Helper Functions
static void wgc_powerUpReceiver();
static void wgc_checkControllerType();
static void wgc_checkControllerData();
static void wgc_updateState(int data);

// Util Functions
static void wgc_sleep(long usec);

// Controller state polling thread
void* wgc_thread_statePolling(void* args)
{
	while(wgc_isInitFlag > 0) {
		if(wgc_isPolling == 1) {
			wgc_isPolling = POLLING_ONGOING;
			wgc_checkControllerData();
			wgc_isPolling = 1;
			
			wgc_timeout += POLLING_US;
			wgc_sleep(POLLING_US);
			
		} else {
			wgc_sleep(100*1000);
		}
	}
}

// Setup and initialize the Controller interface
int wgc_init()
{
	//already initialized?
	if(wgc_isInitFlag == 1) return 0;
	
	//check dependencies
	if(!nrf_isInit() || !vkey_isInit()) {
		if(!nrf_isInit()) fprintf(stderr, "wgc_init: nRF24L01 dependency is not initialized\n");
		if(!vkey_isInit()) fprintf(stderr, "wgc_init: Virtual Keyboard dependency is not initialized\n");
		wgc_close();
		return 1;
	}
	
	//default data
	wgc_isPolling = 0;
	wgc_ctrlType = 0;
	wgc_timeout = 0;
	
	//start polling thread
	wgc_isInitFlag = 1;
    if(pthread_create(&wgc_threadId, NULL, wgc_thread_statePolling, NULL) > 0) {
		wgc_close();
        return 1;
    }

	//set to channel 1 and address 1
	wgc_nrfChannel = 1;
	wgc_nrfRx[4] = 1;
	
	//configure nrf24l01
	nrf_configRegister(RF_CH, wgc_nrfChannel); //set RF channel
	nrf_configRegister(RX_PW_P0, 1); //set length of incoming payload
	nrf_configRegister(EN_AA, 0<<ENAA_P0); //disable auto acknowledgement
	nrf_configRegister(SETUP_RETR, 0); //disable auto retransmit delay
	nrf_writeRegister(RX_ADDR_P0, wgc_nrfRx, 5); //set RADDR and TADDR
	nrf_writeRegister(TX_ADDR, wgc_nrfTx, 5);
	nrf_configRegister(EN_RXADDR, (1<<ERX_P0)); //enable RX_ADDR_P0 address matching
	
	//clear any previous rx data
	nrf_configRegister(STATUS, (1<<RX_DR));
	nrf_flushRxTx();
	
	return 0;
}

// Checks if the Controller interface is initialized
char wgc_isInit()
{
	return wgc_isInitFlag;
}

// Stops the input collection
void wgc_stopPolling()
{
	while(wgc_isPolling == POLLING_ONGOING);
	wgc_isPolling = 0;
}

// Starts collecting controller input
void wgc_startPolling()
{
	wgc_isPolling = 1;
}

// Closes the Controller interface
int wgc_close()
{
	wgc_isInitFlag = 0;
	
	if(wgc_threadId > -1) pthread_join(wgc_threadId, NULL);
	wgc_threadId = -1;
	return 0;
}

// Powers up the nrf receiver
static void wgc_powerUpReceiver() {
	nrf_configRegister(CONFIG, (1<<EN_CRC) | (0<<CRCO) | (1<<PWR_UP) | (1<<PRIM_RX));
	wgc_sleep(5*1000);
	
	nrf_flushRxTx();
	nrf_enable();
}

// Checks if there is a controller available and what type it is
static void wgc_checkControllerType() {
	//start search?
	if(wgc_ctrlType == CTRL_UNKNOWN) {
		wgc_ctrlType = CTRL_SEARCHING;
		
		//make sure device is powered up
		wgc_powerUpReceiver();
		
		//set large payload to check what whats connected
		nrf_configRegister(RX_PW_P0, 9);
		
	}
	
	//check for payload
	if((wgc_ctrlType == CTRL_SEARCHING) && (nrf_status() & (1<<RX_DR))) {
		unsigned char payload[9];
		nrf_readRxPayload(payload, 9);
		
		//reset status register
		nrf_configRegister(STATUS, (1<<RX_DR));
		nrf_flushRxTx();
		
		//check GBA controller
		if(payload[1] != 0xBF) {
			wgc_timeout = 0;
			wgc_ctrlType = CTRL_GBA;
			nrf_configRegister(RX_PW_P0, 2);
			//printf("Found GBA controller\n");
		}
		
		//check GB controller
		else if(payload[1] == 0xBF) {
			wgc_timeout = 0;
			wgc_ctrlType = CTRL_GB;
			nrf_configRegister(RX_PW_P0, 1);
			//printf("Found GB controller\n");
		}
	}
}

// Updates the Controller interface
static void wgc_checkControllerData() {
	wgc_checkControllerType();
	if(wgc_ctrlType > CTRL_SEARCHING) {
		
		//timeout?
		if(wgc_timeout >= TIMEOUT_US) {
			wgc_ctrlType = CTRL_UNKNOWN;
			//printf("Controller timed out\n");
		}
			
		//check for payload
		else if(nrf_status() & (1<<RX_DR)) {
			unsigned char payload[2];
			wgc_timeout = 0;
		
			//update button state
			if(wgc_ctrlType == CTRL_GBA) {
				nrf_readRxPayload(payload, 2);
				wgc_updateState(((int)payload[0] << 8) + (int)payload[1]);
				
			} else if(wgc_ctrlType == CTRL_GB) {
				nrf_readRxPayload(payload, 1);
				wgc_updateState((int)payload[0]);
			}
			
			//reset status register
			nrf_configRegister(STATUS, (1<<RX_DR));
			nrf_flushRxTx();
		}
	} else {
		wgc_updateState(0x0000);
	}
}

// Update the current state of the controller
static void wgc_updateState(int data) {
	//check for escape sequence first
	if((data & 0x03C3) == 0x03C3 && (wgc_state & 0x03C3) != 0x03C3) {
		vkey_setKeyState(KEY_ESC, 1);
		//printf("press ESC\n");
	}
	if((data & 0x03C3) != 0x03C3 && (wgc_state & 0x03C3) == 0x03C3) {
		vkey_setKeyState(KEY_ESC, 0);
		//printf("release ESC\n");
	}
	
	//check for other keys and update state
	if((data & WGC_MASK_A) != (wgc_state & WGC_MASK_A)) {
		wgc_state = (wgc_state & ~WGC_MASK_A) | (data & WGC_MASK_A);
		if(data & WGC_MASK_A) {
			vkey_setKeyState(WGC_KEY_A, 1);
			//printf("press A\n");
		} else {
			vkey_setKeyState(WGC_KEY_A, 0);
			//printf("release A\n");
		}
	}
	if((data & WGC_MASK_B) != (wgc_state & WGC_MASK_B)) {
		wgc_state = (wgc_state & ~WGC_MASK_B) | (data & WGC_MASK_B);
		if(data & WGC_MASK_B) {
			vkey_setKeyState(WGC_KEY_B, 1);
			//printf("press B\n");
		} else {
			vkey_setKeyState(WGC_KEY_B, 0);
			//printf("release B\n");
		}
	}
	if((data & WGC_MASK_L) != (wgc_state & WGC_MASK_L)) {
		wgc_state = (wgc_state & ~WGC_MASK_L) | (data & WGC_MASK_L);
		if(data & WGC_MASK_L) {
			vkey_setKeyState(WGC_KEY_L, 1);
			//printf("press Left\n");
		} else {
			vkey_setKeyState(WGC_KEY_L, 0);
			//printf("release Left\n");
		}
	}
	if((data & WGC_MASK_R) != (wgc_state & WGC_MASK_R)) {
		wgc_state = (wgc_state & ~WGC_MASK_R) | (data & WGC_MASK_R);
		if(data & WGC_MASK_R) {
			vkey_setKeyState(WGC_KEY_R, 1);
			//printf("press Right\n");
		} else {
			vkey_setKeyState(WGC_KEY_R, 0);
			//printf("release Right\n");
		}
	}
	if((data & WGC_MASK_ST) != (wgc_state & WGC_MASK_ST)) {
		wgc_state = (wgc_state & ~WGC_MASK_ST) | (data & WGC_MASK_ST);
		if(data & WGC_MASK_ST) {
			vkey_setKeyState(WGC_KEY_ST, 1);
			//printf("press Start\n");
		} else {
			vkey_setKeyState(WGC_KEY_ST, 0);
			//printf("release Start\n");
		}
	}
	if((data & WGC_MASK_SL) != (wgc_state & WGC_MASK_SL)) {
		wgc_state = (wgc_state & ~WGC_MASK_SL) | (data & WGC_MASK_SL);
		if(data & WGC_MASK_SL) {
			vkey_setKeyState(WGC_KEY_SL, 1);
			//printf("press Select\n");
		} else {
			vkey_setKeyState(WGC_KEY_SL, 0);
			//printf("release Select\n");
		}
	}
	if((data & WGC_MASK_UP) != (wgc_state & WGC_MASK_UP)) {
		wgc_state = (wgc_state & ~WGC_MASK_UP) | (data & WGC_MASK_UP);
		if(data & WGC_MASK_UP) {
			vkey_setKeyState(WGC_KEY_UP, 1);
			//printf("press Up\n");
		} else {
			vkey_setKeyState(WGC_KEY_UP, 0);
			//printf("release Up\n");
		}
	}
	if((data & WGC_MASK_DN) != (wgc_state & WGC_MASK_DN)) {
		wgc_state = (wgc_state & ~WGC_MASK_DN) | (data & WGC_MASK_DN);
		if(data & WGC_MASK_DN) {
			vkey_setKeyState(WGC_KEY_DN, 1);
			//printf("press Down\n");
		} else {
			vkey_setKeyState(WGC_KEY_DN, 0);
			//printf("release Down\n");
		}
	}
	if((data & WGC_MASK_LF) != (wgc_state & WGC_MASK_LF)) {
		wgc_state = (wgc_state & ~WGC_MASK_LF) | (data & WGC_MASK_LF);
		if(data & WGC_MASK_LF) {
			vkey_setKeyState(WGC_KEY_LF, 1);
			//printf("press Left\n");
		} else {
			vkey_setKeyState(WGC_KEY_LF, 0);
			//printf("release Left\n");
		}
	}
	if((data & WGC_MASK_RT) != (wgc_state & WGC_MASK_RT)) {
		wgc_state = (wgc_state & ~WGC_MASK_RT) | (data & WGC_MASK_RT);
		if(data & WGC_MASK_RT) {
			vkey_setKeyState(WGC_KEY_RT, 1);
			//printf("press Right\n");
		} else {
			vkey_setKeyState(WGC_KEY_RT, 0);
			//printf("release Right\n");
		}
	}
}

// Micro second sleep function
static void wgc_sleep(long usec) {
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
	nanosleep(&ts, &ts);
}

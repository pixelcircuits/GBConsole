#ifndef WGC_H
#define WGC_H

// Setup and initialize the Controller interface
int wgc_init();

// Checks if the Controller interface is initialized
char wgc_isInit();

// Stops the input collection
void wgc_stopPolling();

// Starts collecting controller input
void wgc_startPolling();

// Closes the Controller interface
int wgc_close();


#endif /* WGC_H */

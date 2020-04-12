#ifndef VID_H
#define VID_H

typedef void VidTexture;

// Setup and initialize the Video interface
int vid_init();

// Checks if the Video interface is initialized
char vid_isInit();

// Gets the current width of the screen
int vid_getScreenWidth();

// Gets the current height of the screen
int vid_getScreenHeight();

// Gets the given textures width
int vid_getTextureWidth(VidTexture* t);

// Gets the given textures height
int vid_getTextureHeight(VidTexture* t);

// Saves the current screen as a bitmap
void vid_saveScreen(const char* file);

// Draws a box to the video buffer
void vid_drawBox(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b, unsigned char opaque);

// Draws a texture to the video buffer
void vid_drawTexture(VidTexture* t, int x, int y);

// Generates a texture from the given image file
VidTexture* vid_generateImageTexture(const char* filename, int w, int h, char smooth);

// Generates a texture from the given text
VidTexture* vid_generateTextTexture(const char* text, unsigned char rF, unsigned char gF, unsigned char bF, 
		unsigned char rB, unsigned char gB, unsigned char bB, char fontSize, char isBold);

// Generates a texture from the current state of the screen
VidTexture* vid_generateScreenTexture(int x, int y, int w, int h);

// Compsites the given image onto the given texture
void vid_compositeImageToTexture(VidTexture* t, const char* filename, unsigned char opaque, char smooth);

// Compsites the given color onto the given texture
void vid_compositeColorToTexture(VidTexture* t, unsigned char r, unsigned char g, unsigned char b, unsigned char opaque);

// Clears memory for the given texture
void vid_clearTexture(VidTexture* t);

// Flushes the video buffer to the screen
void vid_flush();

// Clears any cached elements (must reinitialize)
int vid_clear();

// Closes the Video interface
int vid_close();


#endif /* VID_H */

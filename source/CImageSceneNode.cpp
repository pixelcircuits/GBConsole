//-----------------------------------------------------------------------------------------
// Title:	Image Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CImageSceneNode.h"
#include <vid.h>

//! Main Constructor
CImageSceneNode::CImageSceneNode(CSceneManager* smgr)
	: CSceneNode(smgr), size(0,0), shadow(false), img(0)
{
}

//! Destructor
CImageSceneNode::~CImageSceneNode()
{
	vid_clearTexture(img);
}

//! Sets the image to use
void CImageSceneNode::setImage(const char* filename, Vector size, bool smooth)
{
	this->size = size;
	
	//load image
	vid_clearTexture(img);
	if(filename) img = vid_generateImageTexture(filename, size.X, size.Y, smooth);
	else img = 0;
}

//! Sets the image to use
void CImageSceneNode::setImageLayered(const char* filenameBase, const char* filenameTop, unsigned char topOpaque, Vector size, bool smooth)
{
	this->size = size;
	
	//load image
	vid_clearTexture(img);
	if(filenameBase && filenameTop) {
		img = vid_generateImageTexture(filenameBase, size.X, size.Y, smooth);
		vid_compositeImageToTexture(img, filenameTop, topOpaque, smooth);
	} else {
		img = 0;
	}
}

//! Sets the image to use
void CImageSceneNode::setImageLayered(const char* filename, Color color, unsigned char colorOpaque, Vector size, bool smooth)
{
	this->size = size;
	
	//load image
	vid_clearTexture(img);
	if(filename) {
		img = vid_generateImageTexture(filename, size.X, size.Y, smooth);
		vid_compositeColorToTexture(img, color.Red, color.Green, color.Blue, colorOpaque);
	} else {
		img = 0;
	}
}

//! Sets if image should have a shadow
void CImageSceneNode::setShadow(bool shadow)
{
	this->shadow = shadow;
}

//! Gets the size of the image
Vector CImageSceneNode::getSize() const
{
	return size;
}

//! Gets if image has a shadow
bool CImageSceneNode::getShadow() const
{
	return shadow;
}

//! Draws the node
void CImageSceneNode::render()
{
	if(img) vid_drawTexture(img, pos.X, pos.Y);
	if(shadow) {
		Color falloff1 = Color(30,30,30);
		vid_drawBox(pos.X-1, pos.Y-1, size.X+2, 1, falloff1.Red, falloff1.Green, falloff1.Blue, 255);
		vid_drawBox(pos.X-1, pos.Y+size.Y, size.X+2, 1, falloff1.Red, falloff1.Green, falloff1.Blue, 255);
		vid_drawBox(pos.X-1, pos.Y-1, 1, size.Y+2, falloff1.Red, falloff1.Green, falloff1.Blue, 255);
		vid_drawBox(pos.X+size.X, pos.Y-1, 1, size.Y+2, falloff1.Red, falloff1.Green, falloff1.Blue, 255);
		
		Color falloff2 = Color(40,40,40);
		vid_drawBox(pos.X, pos.Y-2, size.X, 1, falloff2.Red, falloff2.Green, falloff2.Blue, 255);
		vid_drawBox(pos.X, pos.Y+size.Y+1, size.X, 1, falloff2.Red, falloff2.Green, falloff2.Blue, 255);
		vid_drawBox(pos.X-2, pos.Y, 1, size.Y, falloff2.Red, falloff2.Green, falloff2.Blue, 255);
		vid_drawBox(pos.X+size.X+1, pos.Y, 1, size.Y, falloff2.Red, falloff2.Green, falloff2.Blue, 255);
	}
}

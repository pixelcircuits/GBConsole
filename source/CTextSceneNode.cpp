//-----------------------------------------------------------------------------------------
// Title:	Text Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CTextSceneNode.h"
#include <string.h>
#include <vid.h>

//! Main Constructor
CTextSceneNode::CTextSceneNode(CSceneManager* smgr)
	: CSceneNode(smgr), img(0), txt(0), shadow(false)
{
}

//! Destructor
CTextSceneNode::~CTextSceneNode()
{
	delete[] txt;
	vid_clearTexture(img);
}

//! Sets the image to use
void CTextSceneNode::setText(const char* text, Color textColor, Color backgroundColor, int fontSize, bool bold)
{
	if(txt==0 || text==0 || strcmp(text, txt)!=0) {
		
		//clear old data
		vid_clearTexture(img);
		img = 0;
		delete[] txt;
		txt = 0;
		
		//load image for text
		if(text) {
			txt = new char[strlen(text)+1];
			strcpy(txt, text);
			img = vid_generateTextTexture(text, textColor.Red, textColor.Green, textColor.Blue, 
				backgroundColor.Red, backgroundColor.Green, backgroundColor.Blue, fontSize, bold);
		}
	}
}

//! Sets if image should have a shadow
void CTextSceneNode::setShadow(bool shadow)
{
	this->shadow = shadow;
}
	
//! Gets the text
const char* CTextSceneNode::getText() const
{
	return txt;
}

//! Gets the size of the image
Vector CTextSceneNode::getSize() const
{
	if(img) return Vector(vid_getTextureWidth(img), vid_getTextureHeight(img));
	return Vector(0,0);
}

//! Gets if image has a shadow
bool CTextSceneNode::getShadow() const
{
	return shadow;
}

//! Draws the node
void CTextSceneNode::render()
{
	if(img) vid_drawTexture(img, pos.X, pos.Y);
	if(shadow) {
		Vector size = getSize();
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

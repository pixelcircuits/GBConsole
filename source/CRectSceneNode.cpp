//-----------------------------------------------------------------------------------------
// Title:	Rectangle Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CRectSceneNode.h"
#include <vid.h>

//! Main Constructor
CRectSceneNode::CRectSceneNode(CSceneManager* smgr)
	: CSceneNode(smgr), size(0,0), color(0,0,0), shadow(false), opacity(255)
{
}

//! Sets the color of the rect
void CRectSceneNode::setColor(Color color)
{
	this->color = color;
}

//! Sets if rect should have a shadow
void CRectSceneNode::setShadow(bool shadow)
{
	this->shadow = shadow;
}
	
//! Sets the rect opacity
void  CRectSceneNode::setOpacity(unsigned char opacity)
{
	this->opacity = opacity;
}

//! Sets the size of the rect
void CRectSceneNode::setSize(Vector size)
{
	this->size = size;
}

//! Gets the color of the rect
Color CRectSceneNode::getColor() const
{
	return color;
}

//! Gets if rect has a shadow
bool CRectSceneNode::getShadow() const
{
	return shadow;
}
	
//! Gets the rect opacity
unsigned char CRectSceneNode::getOpacity() const
{
	return opacity;
}

//! Gets the size of the rect
Vector CRectSceneNode::getSize() const
{
	return size;
}

//! Draws the node.
void CRectSceneNode::render()
{
	vid_drawBox(pos.X, pos.Y, size.X, size.Y, color.Red, color.Green, color.Blue, opacity);
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

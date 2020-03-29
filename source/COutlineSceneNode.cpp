//-----------------------------------------------------------------------------------------
// Title:	Outline Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "COutlineSceneNode.h"
#include <vid.h>

//! Main Constructor
COutlineSceneNode::COutlineSceneNode(CSceneManager* smgr)
	: CSceneNode(smgr), size(0,0), color(0,0,0)
{
}

//! Sets the color of the rect
void COutlineSceneNode::setColor(Color color)
{
	this->color = color;
}

//! Sets the size of the rect
void COutlineSceneNode::setSize(Vector size)
{
	this->size = size;
}

//! Gets the color of the rect
Color COutlineSceneNode::getColor() const
{
	return color;
}

//! Gets the size of the rect
Vector COutlineSceneNode::getSize() const
{
	return size;
}

//! Draws the node
void COutlineSceneNode::render()
{
	char width = 4;
	
	vid_drawBox(pos.X-width, pos.Y-width, size.X+(width*2), width, color.Red, color.Green, color.Blue, 255);
	vid_drawBox(pos.X-width, pos.Y+size.Y, size.X+(width*2), width, color.Red, color.Green, color.Blue, 255);
	vid_drawBox(pos.X-width, pos.Y-width, width, size.Y+(width*2), color.Red, color.Green, color.Blue, 255);
	vid_drawBox(pos.X+size.X, pos.Y-width, width, size.Y+(width*2), color.Red, color.Green, color.Blue, 255);
	
	vid_drawBox(pos.X-(width-1), pos.Y-(width+1), size.X+((width-1)*2), 1, color.Red, color.Green, color.Blue, 255);
	vid_drawBox(pos.X-(width-1), pos.Y+size.Y+width, size.X+((width-1)*2), 1, color.Red, color.Green, color.Blue, 255);
	vid_drawBox(pos.X-(width+1), pos.Y-(width-1), 1, size.Y+((width-1)*2), color.Red, color.Green, color.Blue, 255);
	vid_drawBox(pos.X+size.X+width, pos.Y-(width-1), 1, size.Y+((width-1)*2), color.Red, color.Green, color.Blue, 255);
}

//-----------------------------------------------------------------------------------------
// Title:	Rectangle Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef RECT_SCENE_NODE_H
#define RECT_SCENE_NODE_H

#include "CSceneNode.h"
#include "Color.h"
#include "Vector.h"

//! Node for a rectangle
class CRectSceneNode : public CSceneNode
{
public:
	//! Main Constructor
	CRectSceneNode(CSceneManager* smgr);

	//! Sets the color of the rect
	void setColor(Color color);
	
	//! Sets if rect should have a shadow
	void setShadow(bool shadow);
	
	//! Sets the rect opacity
	void setOpacity(unsigned char opacity);

	//! Sets the size of the rect
	void setSize(Vector size);
	
	//! Gets the color of the rect
	Color getColor() const;
	
	//! Gets if rect has a shadow
	bool getShadow() const;
	
	//! Gets the rect opacity
	unsigned char getOpacity() const;

	//! Gets the size of the rect
	Vector getSize() const;

	//! Draws the node
	virtual void render();
	
private:
	Vector size;
	Color color;
	bool shadow;
	unsigned char opacity;
};

#endif

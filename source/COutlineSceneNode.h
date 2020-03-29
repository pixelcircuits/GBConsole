//-----------------------------------------------------------------------------------------
// Title:	Outline Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef OUTLINE_SCENE_NODE_H
#define OUTLINE_SCENE_NODE_H

#include "CSceneNode.h"
#include "Color.h"
#include "Vector.h"

//! Node for an outline
class COutlineSceneNode : public CSceneNode
{
public:
	//! Main Constructor
	COutlineSceneNode(CSceneManager* smgr);

	//! Sets the color of the outline
	void setColor(Color color);

	//! Sets the size of the outline
	void setSize(Vector size);
	
	//! Gets the color of the outline
	Color getColor() const;

	//! Gets the size of the outline
	Vector getSize() const;

	//! Draws the node
	virtual void render();
	
private:
	Vector size;
	Color color;
};

#endif

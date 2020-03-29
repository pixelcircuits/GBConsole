//-----------------------------------------------------------------------------------------
// Title:	Text Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef TEXT_SCENE_NODE_H
#define TEXT_SCENE_NODE_H

#include "CSceneNode.h"
#include "Color.h"
#include "Vector.h"
#include <vid.h>

//! Node for a text
class CTextSceneNode : public CSceneNode
{
public:
	//! Main Constructor
	CTextSceneNode(CSceneManager* smgr);

	//! Destructor
	~CTextSceneNode();
	
	//! Sets the text to use
	void setText(const char* text, Color textColor, Color backgroundColor, int fontSize, bool bold);
	
	//! Sets if image should have a shadow
	void setShadow(bool shadow);
	
	//! Gets the text
	const char* getText() const;

	//! Gets the size of the text
	Vector getSize() const;
	
	//! Gets if image has a shadow
	bool getShadow() const;

	//! Draws the node.
	virtual void render();
	
private:
	VidTexture* img;
	char* txt;
	bool shadow;
};

#endif

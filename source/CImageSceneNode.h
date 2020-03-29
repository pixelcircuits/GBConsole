//-----------------------------------------------------------------------------------------
// Title:	Image Scene Node
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef IMG_SCENE_NODE_H
#define IMG_SCENE_NODE_H

#include "CSceneNode.h"
#include "Color.h"
#include "Vector.h"
#include <vid.h>

//! Node for an image.
class CImageSceneNode : public CSceneNode
{
public:
	//! Main Constructor
	CImageSceneNode(CSceneManager* smgr);

	//! Destructor
	~CImageSceneNode();

	//! Sets the image to use
	void setImage(const char* filename, Vector size, bool smooth);
	
	//! Sets the image to use
	void setImageLayered(const char* filenameBase, const char* filenameTop, unsigned char topOpaque, Vector size, bool smooth);
	
	//! Sets the image to use
	void setImageLayered(const char* filename, Color color, unsigned char colorOpaque, Vector size, bool smooth);
	
	//! Sets if image should have a shadow
	void setShadow(bool shadow);

	//! Gets the size of the image
	Vector getSize() const;
	
	//! Gets if image has a shadow
	bool getShadow() const;

	//! Draws the node
	virtual void render();
	
private:
	Vector size;
	bool shadow;
	VidTexture* img;
};

#endif

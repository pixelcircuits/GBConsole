//-----------------------------------------------------------------------------------------
// Title:	Scene Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "Color.h"
#include "Vector.h"

#define MAX_NUM_NODES 512

#define LAYER_HIDDEN 0

class CSceneNode;
class CRectSceneNode;
class COutlineSceneNode;
class CImageSceneNode;
class CTextSceneNode;

//! Class that handles the organization of a scene
class CSceneManager
{
public:
	//! Main constructor
	CSceneManager();

	//! Destructor
	~CSceneManager();

	//! Adds a rectangle node to the scene
	CRectSceneNode* addRectSceneNode(Color color, Vector size, bool shadow);

	//! Adds an outline node to the scene
	COutlineSceneNode* addOutlineSceneNode(Color color, Vector size);

	//! Adds an image node to the scene
	CImageSceneNode* addImageSceneNode(const char* filename, Vector size, bool shadow, bool smooth);

	//! Adds a text node to the scene
	CTextSceneNode* addTextSceneNode(const char* text, Color textColor, Color backgroundColor, int fontSize, bool bold);

	//! Gets a list of nodes in the current scene
	CSceneNode** getSceneNodes();
	
	//! Gets the number of nodes in the current scene
	unsigned int getSceneNodeCount();

	//! Removes a node from the scene
	void removeNode(CSceneNode* node);

	//! Clears all nodes from the scene
	void clearScene();

	//! Draws the scene
	void drawAll();
	
private:
	CSceneNode* nodeList[MAX_NUM_NODES];
	unsigned int nodeListSize;
};

#endif

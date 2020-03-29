//-----------------------------------------------------------------------------------------
// Title:	SceneNode
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "Vector.h"

class CSceneManager;

//! Interface for all nodes in a scene
class CSceneNode
{
public:
	//! Main Constructor
	CSceneNode(CSceneManager* smgr);

	//! Destructor
	virtual ~CSceneNode();

	//! Draws the node
	virtual void render();

	//! Removes the node from its assigned scene manager
	virtual void remove();

	//! Sets the position of the node
	virtual void setPosition(Vector pos);

	//! Sets the rendering layer of the node
	virtual void setLayer(unsigned char layer);

	//! Sets the navigation flag for the node
	virtual void setNavPath(int navPath);

	//! Sets the navigation a text for the node
	virtual void setNavAText(const char* text);

	//! Sets the navigation b text for the node
	virtual void setNavBText(const char* text);

	//! Gets the position of the node
	virtual Vector getPosition() const;

	//! Gets the size of the node
	virtual Vector getSize() const;

	//! Gets the rendering layer of the node
	virtual unsigned char getLayer() const;

	//! Gets the navigation flag for the node
	virtual int getNavPath() const;

	//! Gets the navigation a text for the node
	virtual const char* getNavAText() const;

	//! Gets the navigation b text for the node
	virtual const char* getNavBText() const;
	
protected:
	Vector pos;
	unsigned char layer;
	int navPath;
	const char* navAText;
	const char* navBText;
	CSceneManager* sceneManager;
};

#endif

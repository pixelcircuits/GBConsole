//-----------------------------------------------------------------------------------------
// Title:	SceneNode
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CSceneNode.h"
#include "CSceneManager.h"

//! Main Constructor
CSceneNode::CSceneNode(CSceneManager* smgr)
	: pos(0,0), layer(LAYER_HIDDEN), navPath(-1), navAText(0), navBText(0), sceneManager(smgr)
{
}

//! Destructor
CSceneNode::~CSceneNode()
{
	remove();
}

//! Draws the node
void CSceneNode::render()
{
}

//! Removes the node from its assigned scene manager
void CSceneNode::remove()
{
	sceneManager->removeNode(this);
}

//! Sets the position of the node
void CSceneNode::setPosition(Vector pos)
{
	this->pos = pos;
}

//! Sets the rendering layer of the node
void CSceneNode::setLayer(unsigned char layer)
{
	this->layer = layer;
}

//! Sets the navigation flag for the node
void CSceneNode::setNavPath(int navPath)
{
	this->navPath = navPath;
}

//! Sets the navigation a text for the node
void CSceneNode::setNavAText(const char* text)
{
	this->navAText = text;
}

//! Sets the navigation b text for the node
void CSceneNode::setNavBText(const char* text)
{
	this->navBText = text;
}

//! Gets the position of the node
Vector CSceneNode::getPosition() const
{
	return pos;
}

//! Gets the size of the node
Vector CSceneNode::getSize() const
{
	return Vector(0,0);
}

//! Gets the rendering layer of the node
unsigned char CSceneNode::getLayer() const
{
	return layer;
}

//! Gets the navigation flag for the node
int CSceneNode::getNavPath() const
{
	return navPath;
}

//! Gets the navigation a text for the node
const char* CSceneNode::getNavAText() const
{
	return navAText;
}

//! Gets the navigation b text for the node
const char* CSceneNode::getNavBText() const
{
	return navBText;
}

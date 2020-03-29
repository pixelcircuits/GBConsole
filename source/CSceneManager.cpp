//-----------------------------------------------------------------------------------------
// Title:	Scene Manager
// Program: GameBoy Console
// Authors: Stephen Monn
//-----------------------------------------------------------------------------------------
#include "CSceneManager.h"
#include "CRectSceneNode.h"
#include "COutlineSceneNode.h"
#include "CImageSceneNode.h"
#include "CTextSceneNode.h"
#include <vid.h>

//! Main constructor
CSceneManager::CSceneManager()
	: nodeListSize(0)
{
}

//! Destructor
CSceneManager::~CSceneManager()
{
	//clear memory
	clearScene();
}

//! Adds a rectangle node to the scene
CRectSceneNode* CSceneManager::addRectSceneNode(Color color, Vector size, bool shadow)
{
	//create the node
	CRectSceneNode* node = new CRectSceneNode(this);
	node->setPosition(Vector(0,0));
	node->setColor(color);
	node->setSize(size);
	node->setShadow(shadow);

	//add node to the list
	if(nodeListSize+1 < MAX_NUM_NODES)
	{
		nodeList[nodeListSize] = node;
		nodeListSize++;
	}

	//return 
	return node;
}

//! Adds an outline node to the scene
COutlineSceneNode* CSceneManager::addOutlineSceneNode(Color color, Vector size)
{
	//create the node
	COutlineSceneNode* node = new COutlineSceneNode(this);
	node->setPosition(Vector(0,0));
	node->setColor(color);
	node->setSize(size);

	//add node to the list
	if(nodeListSize+1 < MAX_NUM_NODES)
	{
		nodeList[nodeListSize] = node;
		nodeListSize++;
	}

	//return 
	return node;
}

//! Adds an image node to the scene
CImageSceneNode* CSceneManager::addImageSceneNode(const char* filename, Vector size, bool shadow, bool smooth)
{
	//create the node
	CImageSceneNode* node = new CImageSceneNode(this);
	node->setPosition(Vector(0,0));
	node->setImage(filename, size, smooth);
	node->setShadow(shadow);

	//add node to the list
	if(nodeListSize+1 < MAX_NUM_NODES)
	{
		nodeList[nodeListSize] = node;
		nodeListSize++;
	}

	//return 
	return node;
}

//! Adds a text node to the scene
CTextSceneNode* CSceneManager::addTextSceneNode(const char* text, Color textColor, Color backgroundColor, int fontSize, bool bold)
{
	//create the node
	CTextSceneNode* node = new CTextSceneNode(this);
	node->setPosition(Vector(0,0));
	node->setText(text, textColor, backgroundColor, fontSize, bold);

	//add node to the list
	if(nodeListSize+1 < MAX_NUM_NODES)
	{
		nodeList[nodeListSize] = node;
		nodeListSize++;
	}

	//return 
	return node;
}

//! Gets a list of nodes in the current scene
CSceneNode** CSceneManager::getSceneNodes()
{
	return nodeList;
}

//! Gets the number of nodes in the current scene
unsigned int CSceneManager::getSceneNodeCount()
{
	return nodeListSize;
}

//! Removes a node from the scene
void CSceneManager::removeNode(CSceneNode* node)
{
	for(unsigned int i=0; i<nodeListSize; i++)
	{
		//is this the node to remove
		if(nodeList[i] == node)
		{
			//bubble the rest of the elements down
			for(unsigned int j=i+1; j<nodeListSize; j++)
				nodeList[j-1] = nodeList[j];

			//sucessfully removed
			nodeListSize--;
			
			//clear the nodes memory
			delete node;

			//exit
			break;
		}
	}
}

//! Clears all nodes from the scene
void CSceneManager::clearScene()
{
	//free the memory
	for(signed int i=nodeListSize-1; i>=0; i--){
		nodeList[i]->remove();
	}

	//adjust the size
	nodeListSize = 0;
}

//! Draws the scene
void CSceneManager::drawAll()
{
	//find max layer
	unsigned char maxLayer = 0;
	for(int i=0; i<nodeListSize; i++) {
		if(nodeList[i]->getLayer() > maxLayer) maxLayer = nodeList[i]->getLayer();
	}
	
	//draw each node in layer order
	for(int layer=1; layer<maxLayer+1; layer++)
		for(int i=0; i<nodeListSize; i++)
			if(nodeList[i]->getLayer() == layer) nodeList[i]->render();
		
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//vid_shadeBox(0, 335, 1920, 550, 0,0,0);
	
	//set render box?
	//draw shade once
	//speed up rendering by combining textures?
	
	//push to screen
	vid_flush();
}

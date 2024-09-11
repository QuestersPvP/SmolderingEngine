#pragma once

// Standard Library
#include <vector>
#include <memory>

// Third party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Engine includes
#include "Engine/Public/Rendering/Utilities.h"

struct ObjectData
{
	int objectID = -1;
	int parentID = -1;
	std::string objectPath;
	std::string texturePath;
	glm::mat4 objectMatrix;
};

class Object
{
	/*
	* Object class functionality:
	* The object class holds the backend for things such as:
	* - Parent/Child relationships
	* - Update functions
	* - Location
	* - Rotation/Transform
	* 
	* Object class does NOT support:
	* - Rendering (e.g. no mesh etc.)
	* - Collision
	* - etc.
	* 
	* If you are looking to make something that will not be seen
	* within the game then this is a good option to choose.
	* If you are looking to add a Mesh, Texture, Collision, etc.
	* then your best option is to create a GameObject.
	*/

	/* Variables */
public:

protected:
	std::vector<Object*> childObjects;

private:
	int objectID;
	int parentObjectID;

	// TODO: WORK ON OBJECT CLASS MORE
	ObjectData objectData;

	/* Functions */

public:

	void AddChildObject(Object* inChild);
	void RemoveChildObject(Object* inChild);
	std::vector<Object*> GetChildObjects();
	bool HasChildObjects();

	ObjectData GetObjectData() { return objectData; };
	void SetObjectData(ObjectData inData) { objectData = inData; };

	//---
	void SetObjectID(int inObjectID); // TODO: REMOVE THIS
	const int GetObjectID();
	void SetObjectParentID(int inParentObjectID);
	const int GetParentObjectID();

	/* Overridable Fucntions */
public:
	virtual void SetModel(glm::mat4 inModel) = 0;
	virtual Model GetModel() = 0;
	virtual int GetUseTexture() = 0;
	virtual void SetUseTexture(int inUseTexture) = 0;
};

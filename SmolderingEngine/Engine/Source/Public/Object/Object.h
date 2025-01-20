#pragma once

// Standard Library
#include <vector>
#include <memory>

// Third party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Engine includes
#include "Engine/Source/Public/Rendering/Utilities.h"

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
protected:
	std::vector<Object*> childObjects;
	ObjectData objectData;

private:

	/* Functions */

public:
	Object() {};
	

	void AddChildObject(Object* inChild);
	void RemoveChildObject(Object* inChild);
	std::vector<Object*> GetChildObjects();
	bool HasChildObjects();

	ObjectData GetObjectData() { return objectData; };
	void SetObjectData(ObjectData inData) { objectData = inData; };

	// Getters + Setters
	const int GetObjectID() { return objectData.objectID; };
	const int GetParentObjectID() { return objectData.parentID; };

	/* Overridable Fucntions */
public:
	virtual void SetModel(glm::mat4 inModel) = 0;
	virtual Model& GetModel() = 0;
	virtual int GetUseTexture() = 0;
	virtual void SetUseTexture(int inUseTexture) = 0;
};

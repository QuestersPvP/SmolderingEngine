#pragma once

#include "Utilities/Includes/ApplicationIncludes.h"

/*
* The Object class is the base for any Object that will go in the scene.
* Hold basic Information:
*	- Position / Rotation
*	- Parent / Children
* 
* Preforms basic functionality:
*	- Update position of Object and Children
*/
class Object
{
	/* Functions */
public:
	// Initialize Object to default values
	Object();
	Object(glm::vec3 _position, glm::vec3 _rotation);

#pragma region Getter/Setters
	// Sets/Gets the parent of this Object
	void SetParent(Object* _parent) { parent = _parent; };
	Object* GetParent() { return parent; };

	// Sets/Gets the position of this Object
	void SetPosition(glm::vec3 _position) { position = _position; };
	glm::vec3 GetPosition() { return position; };

	// Sets/Gets the rotation of this Object
	void SetRotation(glm::vec3 _rotation) { rotation = _rotation; };
	glm::vec3 GetRotation() { return rotation; };
#pragma endregion

#pragma region Virtual Functions
	// Called every frame, updates Object's position and calls Update() on any child Objects.
	virtual void Update();
#pragma endregion

	/* Variables */
private:
	// The current parent of this object, defaults to nullptr
	Object* parent;

	// std::vector of child Objects
	std::vector<Object*> children;

	// Position of this object
	glm::vec3 position;

	// Rotation of this object
	glm::vec3 rotation;
};


#pragma once


// Engine Includes
#include "Object.h"

#include "Engine/Public/Rendering/Mesh.h"

class GameObject : public Object
{
	/* Variables */
public:
	Mesh objectMesh;

private:
	Model objectModel;


	/* Functions */
public:
	GameObject();

	// void ApplyLocalTransform(glm::vec3 inTransform);
	void ApplyLocalYRotation(float inAngle);
	// void ApplyGlobalTransform
	// void ApplyGlobalRotation


	/* Getters + Setters */
public:
	void SetModel(glm::mat4 inModel) override;
	Model GetModel() override;
	int GetUseTexture() override;
	void SetUseTexture(int inUseTexture) override;
};
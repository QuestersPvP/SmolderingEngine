#pragma once

// Standard Library
#include <iostream>

// Project Includes
#include "Object.h"

class GameObject : public Object
{
	/* Variables */
public:
	class Mesh* objectMesh;
	class MeshModel* objectMeshModel;

private:
	Model objectModel;


	/* Functions */
public:
	GameObject();
	GameObject(ObjectData _data, class Mesh* _mesh, class MeshModel* _meshModel);

	// TODO: RE-DO THESE PROPERLY
	void ApplyLocalTransform(glm::vec3 inTransform);
	void ApplyLocalYRotation(float inAngle);
	//void ApplyGlobalTransform(glm::vec3 inTransform);
	// void ApplyGlobalRotation


	/* Getters + Setters */
public:
	void SetModel(glm::mat4 inModel) override;
	Model GetModel() override;
	int GetUseTexture() override;
	void SetUseTexture(int inUseTexture) override;
};
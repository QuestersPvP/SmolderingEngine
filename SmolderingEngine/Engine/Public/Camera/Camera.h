#pragma once

// Engine includes
#include "Engine/Public/Object/Object.h"

class Camera : public Object
{
	/* Variables */
public:
	Model objectModel;
	UniformBufferObjectViewProjection uboViewProjection;

	/* Functions */
public:
	// TODO: Add more constructors 
	Camera();

	virtual void SetModel(glm::mat4 inModel) override;
	virtual Model GetModel() override;
	int GetUseTexture() override;
	void SetUseTexture(int inUseTexture) override;
};
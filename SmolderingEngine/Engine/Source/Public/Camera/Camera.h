#pragma once

// Engine includes
#include "Engine/Source/Public/Object/Object.h"

class Camera : public Object
{
	/* Variables */
public:
	Model objectModel;
	UniformBufferObjectViewProjection uboViewProjection;

	/* Functions */
public:
	Camera();
	Camera(float _fovAngle, float _width, float _height, float _nearPlane, float _farPlane);

	virtual void SetModel(glm::mat4 _model) override;
	virtual Model& GetModel() override;
	int GetUseTexture() override;
	void SetUseTexture(int _useTexture) override;
};
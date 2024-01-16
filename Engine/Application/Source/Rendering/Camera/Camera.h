#pragma once

#include "Utilities/Includes/ApplicationIncludes.h"

class Camera
{
public:
	enum CameraType { lookat, firstperson };
	CameraType type = CameraType::lookat;
	bool flipY = false;
	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();
	glm::vec4 viewPos = glm::vec4();
	float rotationSpeed = 1000.0f;
	float movementSpeed = 10000.0f;
	CameraMatrices matrices;

	void SetRotationSpeed(float _rotationSpeed);
	
	void SetPosition(glm::vec3 _position);
	void SetRotation(glm::vec3 _rotation);

	void SetPerspective(float _fov, float _aspectRatio, float _zNear, float _zFar);
	void UpdateAspectRatio(float _aspectRatio);

private:
	float fov;
	float zNear;
	float zFar;

	void UpdateViewMatrix();
};


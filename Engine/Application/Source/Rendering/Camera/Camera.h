#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
	enum CameraType { lookat, firstperson };
	CameraType type = CameraType::lookat;
	bool flipY = false;
	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();
	glm::vec4 viewPos = glm::vec4();
	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;
	struct
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} matrices;

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::vec3 rotation);
	void SetPerspective(float fov, float aspect, float znear, float zfar);
	void SetRotationSpeed(float rotationSpeed);

private:
	float fov;
	float znear;
	float zfar;

	void UpdateViewMatrix();
};


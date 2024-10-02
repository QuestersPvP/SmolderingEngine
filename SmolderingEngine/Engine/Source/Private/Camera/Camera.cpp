#include "Engine/Source/Public/Camera/Camera.h"

// Third Party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
	// Matrix creation								//FOV			     // Aspect ratio // near, far plane
	uboViewProjection.projection = glm::perspective(glm::radians(45.0f), 1280.f / 720.f, 0.1f, 1000.f);
											// where camera is				// where we are looking			// Y is up
	uboViewProjection.view = glm::lookAt(glm::vec3(0.0f, 20.0f, -50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// invert the Y (Vulkan treats Y as downwards, so if we invert it then Y is up.)
	uboViewProjection.projection[1][1] *= -1;
}

Camera::Camera(float _fovAngle, float _width, float _height, float _nearPlane, float _farPlane)
{
	// Matrix creation								
	uboViewProjection.projection = glm::perspective(glm::radians(_fovAngle), _width / _height, _nearPlane, _farPlane);
	// where camera is				// where we are looking			// Y is up
	uboViewProjection.view = glm::lookAt(glm::vec3(0.0f, 20.0f, -50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// invert the Y (Vulkan treats Y as downwards, so if we invert it then Y is up.)
	uboViewProjection.projection[1][1] *= -1;
}

void Camera::SetModel(glm::mat4 _model)
{
}

Model Camera::GetModel()
{
	return objectModel;
}

int Camera::GetUseTexture()
{
	return 0;
}

void Camera::SetUseTexture(int _useTexture)
{
}

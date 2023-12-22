#include "Camera.h"

void Camera::UpdateViewMatrix()
{
	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 translation = position;
	if (flipY) 
	{
		translation.y *= -1.0f;
	}
	transM = glm::translate(glm::mat4(1.0f), translation);

	if (type == CameraType::firstperson)
	{
		matrices.view = rotM * transM;
	}
	else
	{
		matrices.view = transM * rotM;
	}

	viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

	//updated = true;
}

void Camera::SetRotationSpeed(float _rotationSpeed)
{
	rotationSpeed = _rotationSpeed;
}

void Camera::SetPosition(glm::vec3 _position)
{
	position = _position;
	UpdateViewMatrix();
}

void Camera::SetRotation(glm::vec3 _rotation)
{
	rotation = _rotation;
	UpdateViewMatrix();
}

void Camera::SetPerspective(float _fov, float _aspectRatio, float _zNear, float _zFar)
{
	fov = _fov;
	zNear = _zNear;
	zFar = _zFar;

	matrices.perspective = glm::perspective(glm::radians(_fov), _aspectRatio, _zNear, _zFar);
	if (flipY)
	{
		matrices.perspective[1][1] *= -1.0f;
	}
}

void Camera::UpdateAspectRatio(float _aspectRatio)
{
	matrices.perspective = glm::perspective(glm::radians(fov), _aspectRatio, zNear, zFar);
	if (flipY)
	{
		matrices.perspective[1][1] *= -1.0f;
	}
}

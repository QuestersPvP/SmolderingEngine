#include "Object.h"

Object::Object()
{
	SetParent(nullptr);
	SetPosition({0, 0, 0});
	SetRotation({0, 0, 0});
}

Object::Object(glm::vec3 _position, glm::vec3 _rotation)
{
	SetParent(nullptr);
	SetPosition(_position);
	SetRotation(_rotation);
}

void Object::Update()
{
}

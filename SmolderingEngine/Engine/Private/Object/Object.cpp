#include "Engine/Public/Object/Object.h"

void Object::AddChildObject(Object* inChild)
{
	childObjects.push_back(inChild);
}

void Object::RemoveChildObject(Object* inChild)
{
	childObjects.erase(std::remove(childObjects.begin(), childObjects.end(), inChild), childObjects.end());
}

bool Object::HasChildObjects()
{
	return childObjects.size() >= 1;
}

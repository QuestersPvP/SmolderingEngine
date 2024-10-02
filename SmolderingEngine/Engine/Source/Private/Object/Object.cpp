#include "Engine/Source/Public/Object/Object.h"

void Object::AddChildObject(Object* inChild)
{
	childObjects.push_back(inChild);
}

void Object::RemoveChildObject(Object* inChild)
{
	childObjects.erase(std::remove(childObjects.begin(), childObjects.end(), inChild), childObjects.end());
}

std::vector<Object*> Object::GetChildObjects()
{
	return childObjects;
}

bool Object::HasChildObjects()
{
	return childObjects.size() >= 1;
}
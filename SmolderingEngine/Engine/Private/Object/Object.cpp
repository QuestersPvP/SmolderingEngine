#include "Engine/Public/Object/Object.h"

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

void Object::SetObjectID(int inObjectID)
{
	objectID = inObjectID;
}

const int Object::GetObjectID()
{
	return objectID;
}

void Object::SetObjectParentID(int inParentObjectID)
{
	parentObjectID = inParentObjectID;
}

const int Object::GetParentObjectID()
{
	return parentObjectID;
}

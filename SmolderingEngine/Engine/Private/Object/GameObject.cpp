#include "Engine/Public/Object/GameObject.h"

GameObject::GameObject()
{
	objectModel.modelMatrix = glm::mat4(1.0f);
}

void GameObject::ApplyLocalTransform(glm::vec3 inTransform)
{
	objectModel.modelMatrix[3].x = inTransform.x;
	objectModel.modelMatrix[3].y = inTransform.y;
	objectModel.modelMatrix[3].z = inTransform.z;
}

void GameObject::ApplyLocalYRotation(float inAngle)
{
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(inAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	// Apply the rotation to the parent object
	SetModel(rotationMatrix * GetModel().modelMatrix);

	std::vector<Object*> objectsToRotate = childObjects;

	while (!objectsToRotate.empty())
	{
		Object* currObject = objectsToRotate.back();
		objectsToRotate.pop_back();

		currObject->SetModel(rotationMatrix * currObject->GetModel().modelMatrix);

		for (Object* grandchild : currObject->GetChildObjects())
		{
			objectsToRotate.push_back(grandchild);
		}
	}
}

void GameObject::SetModel(glm::mat4 inModel)
{
	objectModel.modelMatrix = inModel;
	//CalculateMeshAABB(initialVertexPositions);
}

Model GameObject::GetModel()
{
	return objectModel;
}

int GameObject::GetUseTexture()
{
	return objectModel.useTexture;
}

void GameObject::SetUseTexture(int inUseTexture)
{
	objectModel.useTexture = inUseTexture;
}
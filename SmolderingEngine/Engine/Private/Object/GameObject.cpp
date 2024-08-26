#include "Engine/Public/Object/GameObject.h"

GameObject::GameObject()
{
	objectModel.modelMatrix = glm::mat4(1.0f);
}

void GameObject::ApplyLocalYRotation(float inAngle)
{
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(inAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	// Apply the rotation to the parent object
	SetModel(rotationMatrix);

	// Apply the rotation to all child objects
	for (auto& child : childObjects)
	{
		child->SetModel(GetModel().modelMatrix * child->GetModel().modelMatrix);
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
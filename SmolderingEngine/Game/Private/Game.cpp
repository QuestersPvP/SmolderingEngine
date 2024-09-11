#include "Game/Public/Game.h"

// Engine includes
#include "Engine/Public/Rendering/Renderer.h"

void Game::DestroyMeshes()
{
	// TODO: FIX
	for (int i = gameObjects.size()-1; i >= 0; i--)
	{
		gameObjects[i]->objectMeshModel.DestroyMeshModel();
		delete gameObjects[i];
		gameObjects.pop_back();
	}
}

void Game::SubscribeObjectsToCollisionManager(CollisionManager* inManager, int inParent)
{
	for (GameObject* object : gameObjects)
	{
		if (object->GetParentObjectID() == inParent && object->GetObjectID() == 11)
			inManager->SubscribeObjectToCollisionManager(object, CollisionTypes::MovableCollision);
		else if (object->GetParentObjectID() == inParent)
			inManager->SubscribeObjectToCollisionManager(object, CollisionTypes::StaticCollision);
	}
}

void Game::UnsubscribeObjectsFromCollisionManager(CollisionManager* inManager)
{
	for (GameObject* object : gameObjects)
			inManager->UnsubscribeObjectFromCollisionManager(object);

}

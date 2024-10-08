#pragma once

// Standard Library
#include <set>
#include <vector> // TODO: eventually maybe move to MAP instead of VECTOR to hold game objects. For now, one thing at a time.

class ObjectManager
{
	/* Variables */
public:

private:
	int nextID = 0;
	std::set<int> reusableIDs;
	std::vector<class GameObject*> gameObjects;

	// Reference to EngineManager
	class EngineManager* seEngineManager = nullptr;

	/* Functions */
public:
	// Creates a game object and returns it
	class GameObject* CreateGameObject(struct ObjectData _objectData, class Mesh* _mesh, class MeshModel* _meshModel);
	
	// Deletes all game objects within a level
	void DestroyAllGameObjects();
	//void Game::DestroyMeshes()
	//{
	//	// TODO: FIX
	//	for (int i = gameObjects.size() - 1; i >= 0; i--)
	//	{
	//		gameObjects[i]->objectMeshModel->DestroyMeshModel();
	//		delete gameObjects[i];
	//		gameObjects.pop_back();
	//	}
	//}

	/* Getters + Setters */

	const std::vector<class GameObject*> GetGameObjects() { return gameObjects; };
};
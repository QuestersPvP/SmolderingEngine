#pragma once

// Standard Library
#include <iostream>s
#include <vector>
#include <unordered_map>

// Project includes
#include "Game/Source/Public/Game.h"
#include "Engine/Source/Public/Rendering/Utilities.h"
#include "Engine/Source/Public/Object/GameObject.h"
#include "Engine/Source/Public/Rendering/Mesh.h"

class CollisionManager
{
	/* Variables */
public:

private:
	std::unordered_map<CollisionTypes, std::vector<GameObject*>> collisionObserver;

	/* Functions */
public:

	// Adds an object to the collision manager.
	void SubscribeObjectToCollisionManager(GameObject* inObject, CollisionTypes inCollisionType);
	// Removes a specific object from the collision manager.
	void UnsubscribeObjectFromCollisionManager(GameObject* inObject);
	// Notifies all objects of a collision
	std::pair<float, float> NotifyCollisionManagerOfMovement(GameObject* inObject);
	// Checks for collisions of objects within the collision system
	std::pair<float, float> CheckForCollisions();

private:
	AABB CalculateMeshAABB(GameObject* inObject);
	bool AABBIntersect(AABB first, AABB second);
};

#pragma once

// Standard Library
#include <iostream>
#include <vector>
#include <unordered_map>

// Project includes
#include "Game/Public/Game.h"
#include "Engine/Public/Rendering/Utilities.h"

class CollisionManager
{
	/* Variables */
public:

private:
	std::unordered_map<CollisionTypes, std::vector<Mesh*>> collisionObserver;

	/* Functions */
public:

	// Adds an object to the collision manager.
	void SubscribeObjectToCollisionManager(Mesh* inObject, CollisionTypes inCollisionType);
	// Removes a specific object from the collision manager.
	void UnsubscribeObjectFromCollisionManager(Mesh* inObject);
	// Notifies all objects of a collision
	void NotifyCollisionManagerOfMovement(Mesh* inObject);
	// Checks for collisions of objects within the collision system
	void CheckForCollisions();

private:
	AABB CalculateMeshAABB(Mesh inMesh);
	bool AABBIntersect(AABB first, AABB second);
};

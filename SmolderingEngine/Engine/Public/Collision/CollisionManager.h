#pragma once

// Standard Library
#include <iostream>

// Project includes
#include "Game/Public/Game.h"
#include "Engine/Public/Rendering/Utilities.h"

class CollisionManager
{
	/* Variables */
public:

	/* Functions */
public:
	bool CheckForCollisions(Game* inGame);

private:
	AABB CalculateMeshAABB(Mesh inMesh);
	bool AABBIntersect(AABB first, AABB second);
};

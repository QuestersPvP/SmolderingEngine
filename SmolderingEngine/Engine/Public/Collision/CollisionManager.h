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
	void CheckForCollisions(Game* inGame);

private:
	AABB CalculateAABB(Model inModel);
	bool AABBIntersect(AABB first, AABB second);
};

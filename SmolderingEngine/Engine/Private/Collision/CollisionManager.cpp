#include "Engine/Public/Collision/CollisionManager.h"

void CollisionManager::CheckForCollisions(Game* inGame)
{
	// skip the floor (it is index 0) and skip the player (the player is the last index)
	for (size_t i = 1; i < inGame->GameMeshes.size()-1; i++)
	{
        // Get the player
        Model player = inGame->GameMeshes[inGame->GameMeshes.size() - 1].GetModel();
        AABB playerAABB = CalculateAABB(player);

        // find AABB of other object
		AABB otherAABB = CalculateAABB(inGame->GameMeshes[i].GetModel());

        bool detectedCollision = AABBIntersect(playerAABB, otherAABB);

        // everything else = you die
        if (detectedCollision)
            std::cout << "you died" << std::endl;

        // inGame->GameMeshes.size()-2 = win mesh, so you win
	}


}

AABB CollisionManager::CalculateAABB(Model inModel)
{
    return AABB();
}

bool CollisionManager::AABBIntersect(AABB first, AABB second)
{
	return	(first.minimumX <= second.maximumX && first.maximumX >= second.minimumX) &&
			(first.minimumY <= second.maximumY && first.maximumY >= second.minimumY);
}

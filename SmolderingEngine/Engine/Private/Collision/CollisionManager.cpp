#include "Engine/Public/Collision/CollisionManager.h"

void CollisionManager::CheckForCollisions(Game* inGame)
{
	// skip the floor (it is index 0) and skip the player (the player is the last index)
	for (size_t i = 1; i < inGame->GameMeshes.size()-1; i++)
	{
        // Get the player
        Mesh playerMesh = inGame->GameMeshes[inGame->GameMeshes.size() - 1];
		AABB playerAABB = CalculateMeshAABB(playerMesh);

        // Get other object
        Mesh otherMesh = inGame->GameMeshes[i];
        AABB otherAABB = CalculateMeshAABB(otherMesh);

        bool detectedCollision = AABBIntersect(playerAABB, otherAABB);

        // everything else = you die
		if (detectedCollision && (i != inGame->GameMeshes.size() - 2))
		{
            std::cout << "you died" << std::endl;
		}
		else if (detectedCollision && (i == inGame->GameMeshes.size() - 2))
		{
			std::cout << "you win" << std::endl;
		}
		
	}
}

AABB CollisionManager::CalculateMeshAABB(Mesh inMesh)
{
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto vertex : inMesh.GetVertices())
	{
		glm::vec4 transformedPosition = inMesh.GetModel().modelMatrix * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);

		if (transformedPosition.x < minX) minX = transformedPosition.x;
		if (transformedPosition.y < minY) minY = transformedPosition.y;
		if (transformedPosition.x > maxX) maxX = transformedPosition.x;
		if (transformedPosition.y > maxY) maxY = transformedPosition.y;
	}

	AABB modelAABB;
	modelAABB.minimumX = minX;
	modelAABB.maximumX = maxX;
	modelAABB.minimumY = minY;
	modelAABB.maximumY = maxY;

	return modelAABB;
}

bool CollisionManager::AABBIntersect(AABB first, AABB second)
{
    bool xOverlap = (first.minimumX <= second.maximumX) && (first.maximumX >= second.minimumX);
    bool yOverlap = (first.minimumY <= second.maximumY) && (first.maximumY >= second.minimumY);

    return xOverlap && yOverlap;
}

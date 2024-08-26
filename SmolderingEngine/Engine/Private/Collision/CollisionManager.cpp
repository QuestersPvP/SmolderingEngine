#include "Engine/Public/Collision/CollisionManager.h"

void CollisionManager::SubscribeObjectToCollisionManager(Mesh* inObject, CollisionTypes inCollisionType)
{
	collisionObserver[inCollisionType].push_back(inObject);
}

void CollisionManager::UnsubscribeObjectFromCollisionManager(Mesh* inObject)
{
	// TODO: Work on object / mesh / game object classes then worry about this stuff
}

void CollisionManager::NotifyCollisionManagerOfMovement(Mesh* inObject)
{
	// Check if the object is movable and then run collision detection
	auto it = std::find(collisionObserver[CollisionTypes::MovableCollision].begin(), collisionObserver[CollisionTypes::MovableCollision].end(), inObject);
	
	if (it != collisionObserver[CollisionTypes::MovableCollision].end())
		CheckForCollisions();
}

void CollisionManager::CheckForCollisions()
{
	const auto& movableObjects = collisionObserver[CollisionTypes::MovableCollision];
	const auto& staticObjects = collisionObserver[CollisionTypes::StaticCollision];

	for (Mesh* moveableObject : movableObjects)
	{
		AABB movableAABB = CalculateMeshAABB(*moveableObject);

		for (Mesh* staticObject : staticObjects)
		{
			AABB staticAABB = CalculateMeshAABB(*staticObject);

			if (AABBIntersect(movableAABB, staticAABB))
				std::cout << "Collision detected!" << std::endl;
		}
	}
}

AABB CollisionManager::CalculateMeshAABB(Mesh inMesh)
{
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	//for (const auto vertex : inMesh.GetVertices())
	//{
	//	glm::vec4 transformedPosition = inMesh.GetModel().modelMatrix * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);

	//	if (transformedPosition.x < minX) minX = transformedPosition.x;
	//	if (transformedPosition.y < minY) minY = transformedPosition.y;
	//	if (transformedPosition.x > maxX) maxX = transformedPosition.x;
	//	if (transformedPosition.y > maxY) maxY = transformedPosition.y;
	//}

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

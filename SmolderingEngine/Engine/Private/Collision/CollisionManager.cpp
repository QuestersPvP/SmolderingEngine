#include "Engine/Public/Collision/CollisionManager.h"

void CollisionManager::SubscribeObjectToCollisionManager(GameObject* inObject, CollisionTypes inCollisionType)
{
	collisionObserver[inCollisionType].push_back(inObject);
}

void CollisionManager::UnsubscribeObjectFromCollisionManager(GameObject* inObject)
{
	for (auto& [collisionType, gameObjectList] : collisionObserver) 
	{
		// Find the GameObject* to remove using std::remove
		auto it = std::remove(gameObjectList.begin(), gameObjectList.end(), inObject);

		// If the object was found, remove it from the vector
		if (it != gameObjectList.end()) {
			gameObjectList.erase(it, gameObjectList.end());
		}
	}
}

std::pair<float, float> CollisionManager::NotifyCollisionManagerOfMovement(GameObject* inObject)
{
	// Check if the object is movable and then run collision detection
	auto object = std::find(collisionObserver[CollisionTypes::MovableCollision].begin(), collisionObserver[CollisionTypes::MovableCollision].end(), inObject);
	
	if (object != collisionObserver[CollisionTypes::MovableCollision].end())
	{
		std::pair<float, float> check = CheckForCollisions();
		return check;
	}

	return std::pair<float, float>(0.0f, 0.0f);
}

std::pair<float, float> CollisionManager::CheckForCollisions()
{
	const auto& movableObjects = collisionObserver[CollisionTypes::MovableCollision];
	const auto& staticObjects = collisionObserver[CollisionTypes::StaticCollision];

	for (GameObject* moveableObject : movableObjects)
	{
		AABB movableAABB = CalculateMeshAABB(moveableObject);

		for (GameObject* staticObject : staticObjects)
		{
			AABB staticAABB = CalculateMeshAABB(staticObject);

			if (AABBIntersect(movableAABB, staticAABB))
			{
				std::cout << "Collision detected!" << std::endl;
				return std::pair<float, float>(staticAABB.maximumY, std::abs(movableAABB.minimumY - staticAABB.maximumY));
			}
		}
	}
}

AABB CollisionManager::CalculateMeshAABB(GameObject* inObject)
{
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto vertex : inObject->objectMesh.GetVertices())
	{
		glm::vec4 transformedPosition = inObject->GetModel().modelMatrix * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);

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

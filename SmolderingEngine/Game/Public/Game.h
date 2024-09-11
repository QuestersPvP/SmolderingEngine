#pragma once

// Standard Library
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Third Party
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Engine
#include "Engine/Public/Rendering/Mesh.h"
#include "Engine/Public/Rendering/MeshModel.h"
#include "Engine/Public/Object/GameObject.h"
#include "Engine/Public/Collision/CollisionManager.h"

class Game
{
    /* Variables */
public:
    std::vector<GameObject*> gameObjects;

    /* Functions */
public:
    void DestroyMeshes();

    void SubscribeObjectsToCollisionManager(class CollisionManager* inManager, int inParent);
    void UnsubscribeObjectsFromCollisionManager(class CollisionManager* inManager);
};
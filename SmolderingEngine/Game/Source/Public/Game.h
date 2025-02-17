#pragma once

// Standard Library
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <string>
//#include <vector>

// Third Party
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

// Project includes
//#include "Engine/Source/Public/Rendering/Mesh.h"
//#include "Engine/Source/Public/Rendering/MeshModel.h"
//#include "Engine/Source/Public/Object/GameObject.h"
//#include "Engine/Source/Public/Collision/CollisionManager.h"

class Game
{
    /* Variables */
public:
    /*std::vector<class GameObject*> gameObjects;*/

    /* Functions */
public:
    /*void DestroyMeshes();*/

    void SubscribeObjectsToCollisionManager(class CollisionManager* inManager, int inParent);
    void UnsubscribeObjectsFromCollisionManager(class CollisionManager* inManager);
};
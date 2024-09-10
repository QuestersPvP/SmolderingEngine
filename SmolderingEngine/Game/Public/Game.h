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


/*
layout(location = 0) out vec3 fragColor; // Output color for vertex (Location is required)

// Triangle vertex positions
vec3 positions[3] = vec3[](
    vec3(0.0, -0.4, 0.0),
    vec3(0.4, 0.4, 0.0),
    vec3(-0.4, 0.4, 0.0)
);

// Triangle vertex colors
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);
*/

class Game
{
    /* Variables */
public:
    std::vector<GameObject*> gameObjects;

    /* Functions */
public:
    //void LoadMeshes(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool);
    //void LoadMeshModel(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
    //    std::string inModelFile, class Renderer* inRenderer);
    void DestroyMeshes();

    void SubscribeObjectsToCollisionManager(class CollisionManager* inManager, int inParent);
    void UnsubscribeObjectsFromCollisionManager(class CollisionManager* inManager);
};
#pragma once

// Third Party
#include "GLM/glm.hpp"

// Engine
#include "Engine/Public/Rendering/Mesh.h"


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
    Mesh MeshOne;

    /* Functions */
public:
    void LoadMeshes(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice);
    void DestroyMeshes();
};
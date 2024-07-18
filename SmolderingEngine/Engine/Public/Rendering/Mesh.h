#pragma once

// Standard Library
#include <vector>

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Engine
#include "Engine/Public/Rendering/Utilities.h"



class Mesh
{
	/* Variables */
private:
	int VertexCount;
	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;

	VkDevice LogicalDevice;
	VkPhysicalDevice PhysicalDevice;

	/* Functions */
public:
	Mesh();
	Mesh(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, std::vector<Vertex>* InVerticies);
	void DestroyMesh();
	
	int GetVertexCount();
	VkBuffer GetVertexBuffer();

private:
	void CreateVertexBuffer(std::vector<Vertex>* InVerticies);
	uint32_t FindMemoryTypeIndex(uint32_t InAllowedTypes, VkMemoryPropertyFlags InMemoryPropertyFlags);
};

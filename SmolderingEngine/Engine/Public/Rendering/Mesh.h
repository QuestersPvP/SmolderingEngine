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
	// Vertex
	int VertexCount;
	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;

	// Index
	int IndexCount;
	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;

	// VK devices
	VkDevice LogicalDevice;
	VkPhysicalDevice PhysicalDevice;

	Model model;

	/* Functions */
public:
	Mesh();
	Mesh(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, VkQueue InTransferQueue, VkCommandPool InTransferCommandPool,
		std::vector<Vertex>* InVerticies, std::vector<uint32_t>* InIndicies);
	void DestroyMesh();

	void SetModel(glm::mat4 inModel);
	Model GetModel();
	
	int GetVertexCount();
	VkBuffer GetVertexBuffer();	
	
	int GetIndexCount();
	VkBuffer GetIndexBuffer();

private:
	void CreateVertexBuffer(VkQueue InTransferQueue, VkCommandPool InTransferCommandPool, std::vector<Vertex>* InVerticies);
	void CreateIndexBuffer(VkQueue InTransferQueue, VkCommandPool InTransferCommandPool, std::vector<uint32_t>* InIndicies);
};

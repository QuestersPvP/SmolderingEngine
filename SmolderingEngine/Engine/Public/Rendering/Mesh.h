#pragma once

// Standard Library
#include <vector>
#include <algorithm>

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
	std::vector<glm::vec3> initialVertexPositions;
	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	// Index
	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	// VK devices
	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice;

	std::string textureFilePath;
	int textureID;

	int useTexture;

	/* Functions */
public:
	Mesh();
	Mesh(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
		std::vector<Vertex>* inVertices, std::vector<uint32_t>* inIndicies);
	void DestroyMesh();

	void SetTextureFilePath(std::string inFilePath);
	std::string GetTextureFilePath();

	int GetTextureID();
	void SetTextureID(int inTextureID);
	
	int GetVertexCount();
	VkBuffer GetVertexBuffer();	
	std::vector<glm::vec3> GetVertices();
	
	int GetIndexCount();
	VkBuffer GetIndexBuffer();

private:
	// For rendering
	void CreateVertexBuffer(VkQueue inTransferQueue, VkCommandPool inTransferCommandPool, std::vector<Vertex>* inVertices);
	void CreateIndexBuffer(VkQueue inTransferQueue, VkCommandPool inTransferCommandPool, std::vector<uint32_t>* inIndicies);
};

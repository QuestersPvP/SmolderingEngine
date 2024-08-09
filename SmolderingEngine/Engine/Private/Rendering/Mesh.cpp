#include "Engine/Public/Rendering/Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
	std::vector<Vertex>* inVertices, std::vector<uint32_t>* inIndicies)
{
	physicalDevice = inPhysicalDevice;
	logicalDevice = inLogicalDevice;

	vertexCount = inVertices->size();
	indexCount = inIndicies->size();

	CreateVertexBuffer(inTransferQueue, inTransferCommandPool, inVertices);
	CreateIndexBuffer(inTransferQueue, inTransferCommandPool, inIndicies);

	// Define the model matrix and then calculate the AABB in world space.
	initialVertexPositions.reserve(inVertices->size());
	// this algorithm just copies the position data of from the Vertex struct instead of having to loop
	std::transform(inVertices->begin(), inVertices->end(), std::back_inserter(initialVertexPositions),
		[](const Vertex& vertex) {
			return glm::vec3(vertex.position[0], vertex.position[1], vertex.position[2]);
		});

	model.modelMatrix = glm::mat4(1.0f);
}

void Mesh::DestroyMesh()
{
	vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
	vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);
	vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);	
	vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);
}

void Mesh::SetModel(glm::mat4 inModel)
{
	model.modelMatrix = inModel;
	//CalculateMeshAABB(initialVertexPositions);
}

Model Mesh::GetModel()
{
	return model;
}

int Mesh::GetTextureID()
{
	return textureID;
}

void Mesh::SetTextureID(int inTextureID)
{
	textureID = inTextureID;
}

int Mesh::GetVertexCount()
{
	return vertexCount;
}

VkBuffer Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

std::vector<glm::vec3> Mesh::GetVertices()
{
	return initialVertexPositions;
}

int Mesh::GetIndexCount()
{
	return indexCount;
}

VkBuffer Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

void Mesh::CreateVertexBuffer(VkQueue inTransferQueue, VkCommandPool inTransferCommandPool, std::vector<Vertex>* inVertices)
{
	// size of buffer needed to hold all verticies
	VkDeviceSize bufferSize = sizeof(Vertex) * inVertices->size();

	// Temporary buffer to stage vretex data before transferring to the GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create a staging buffer and allocate memory to it
	CreateBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// Map memory to the vertex buffer
	void* data;																				// create pointer to memory
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);				// map the vertex buffer memory to the pointer
	memcpy(data, inVertices->data(), (size_t)bufferSize);									// Copy memory from InVerticies vector to that point
	vkUnmapMemory(logicalDevice, stagingBufferMemory);										// unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT and VERTEX_BUFFER_BIT so it can receive transfer data used for vertex buffer
	// this memory is DEVICE_LOCAL because the memory is on the GPU and only accessible by the GPU (we do not want CPU to have access) 
	CreateBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	// Copy the staging buffer to vertex buffer on the GPU
	CopyBuffer(logicalDevice, inTransferQueue, inTransferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	// destroy the staging buffer and free the memory
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void Mesh::CreateIndexBuffer(VkQueue inTransferQueue, VkCommandPool inTransferCommandPool, std::vector<uint32_t>* inIndicies)
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * inIndicies->size();

	// Temporary buffer to stage index data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Create a staging buffer and allocate memory to it
	CreateBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// Map memory to the index buffer
	void* data;																
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, inIndicies->data(), (size_t)bufferSize);					
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	// Create buffer for index data for GPU access only
	CreateBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

	// copy from staging buffer to gpu access buffer
	CopyBuffer(logicalDevice, inTransferQueue, inTransferCommandPool, stagingBuffer, indexBuffer, bufferSize);

	// destroy/free staging buffer
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

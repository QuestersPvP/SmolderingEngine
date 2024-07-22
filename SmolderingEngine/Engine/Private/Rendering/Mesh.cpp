#include "Engine/Public/Rendering/Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, VkQueue InTransferQueue, VkCommandPool InTransferCommandPool,
	std::vector<Vertex>* InVerticies, std::vector<uint32_t>* InIndicies)
{
	PhysicalDevice = InPhysicalDevice;
	LogicalDevice = InLogicalDevice;

	VertexCount = InVerticies->size();
	IndexCount = InIndicies->size();

	CreateVertexBuffer(InTransferQueue, InTransferCommandPool, InVerticies);
	CreateIndexBuffer(InTransferQueue, InTransferCommandPool, InIndicies);

	uboModel.model = glm::mat4(1.0f);
}

void Mesh::DestroyMesh()
{
	vkDestroyBuffer(LogicalDevice, IndexBuffer, nullptr);
	vkFreeMemory(LogicalDevice, IndexBufferMemory, nullptr);
	vkDestroyBuffer(LogicalDevice, VertexBuffer, nullptr);	
	vkFreeMemory(LogicalDevice, VertexBufferMemory, nullptr);
}

void Mesh::SetModel(glm::mat4 inModel)
{
	uboModel.model = inModel;
}

UniformBufferObjectModel Mesh::GetModel()
{
	return uboModel;
}

int Mesh::GetVertexCount()
{
	return VertexCount;
}

VkBuffer Mesh::GetVertexBuffer()
{
	return VertexBuffer;
}

int Mesh::GetIndexCount()
{
	return IndexCount;
}

VkBuffer Mesh::GetIndexBuffer()
{
	return IndexBuffer;
}

void Mesh::CreateVertexBuffer(VkQueue InTransferQueue, VkCommandPool InTransferCommandPool, std::vector<Vertex>* InVerticies)
{
	// size of buffer needed to hold all verticies
	VkDeviceSize BufferSize = sizeof(Vertex) * InVerticies->size();

	// Temporary buffer to stage vretex data before transferring to the GPU
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;

	// Create a staging buffer and allocate memory to it
	CreateBuffer(PhysicalDevice, LogicalDevice, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&StagingBuffer, &StagingBufferMemory);

	// Map memory to the vertex buffer
	void* data;																				// create pointer to memory
	vkMapMemory(LogicalDevice, StagingBufferMemory, 0, BufferSize, 0, &data);				// map the vertex buffer memory to the pointer
	memcpy(data, InVerticies->data(), (size_t)BufferSize);									// Copy memory from InVerticies vector to that point
	vkUnmapMemory(LogicalDevice, StagingBufferMemory);										// unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT and VERTEX_BUFFER_BIT so it can receive transfer data used for vertex buffer
	// this memory is DEVICE_LOCAL because the memory is on the GPU and only accessible by the GPU (we do not want CPU to have access) 
	CreateBuffer(PhysicalDevice, LogicalDevice, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &VertexBuffer, &VertexBufferMemory);

	// Copy the staging buffer to vertex buffer on the GPU
	CopyBuffer(LogicalDevice, InTransferQueue, InTransferCommandPool, StagingBuffer, VertexBuffer, BufferSize);

	// destroy the staging buffer and free the memory
	vkDestroyBuffer(LogicalDevice, StagingBuffer, nullptr);
	vkFreeMemory(LogicalDevice, StagingBufferMemory, nullptr);
}

void Mesh::CreateIndexBuffer(VkQueue InTransferQueue, VkCommandPool InTransferCommandPool, std::vector<uint32_t>* InIndicies)
{
	VkDeviceSize BufferSize = sizeof(uint32_t) * InIndicies->size();

	// Temporary buffer to stage index data before transferring to GPU
	VkBuffer StagingBuffer;
	VkDeviceMemory StagingBufferMemory;

	// Create a staging buffer and allocate memory to it
	CreateBuffer(PhysicalDevice, LogicalDevice, BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&StagingBuffer, &StagingBufferMemory);

	// Map memory to the index buffer
	void* data;																
	vkMapMemory(LogicalDevice, StagingBufferMemory, 0, BufferSize, 0, &data);
	memcpy(data, InIndicies->data(), (size_t)BufferSize);					
	vkUnmapMemory(LogicalDevice, StagingBufferMemory);

	// Create buffer for index data for GPU access only
	CreateBuffer(PhysicalDevice, LogicalDevice, BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &IndexBuffer, &IndexBufferMemory);

	// copy from staging buffer to gpu access buffer
	CopyBuffer(LogicalDevice, InTransferQueue, InTransferCommandPool, StagingBuffer, IndexBuffer, BufferSize);

	// destroy/free staging buffer
	vkDestroyBuffer(LogicalDevice, StagingBuffer, nullptr);
	vkFreeMemory(LogicalDevice, StagingBufferMemory, nullptr);
}

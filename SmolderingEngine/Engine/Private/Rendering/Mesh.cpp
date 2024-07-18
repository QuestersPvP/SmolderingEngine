#include "Engine/Public/Rendering/Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, std::vector<Vertex>* InVerticies)
{
	PhysicalDevice = InPhysicalDevice;
	LogicalDevice = InLogicalDevice;
	VertexCount = InVerticies->size();
	CreateVertexBuffer(InVerticies);
}

void Mesh::DestroyMesh()
{
	vkFreeMemory(LogicalDevice, VertexBufferMemory, nullptr);
	vkDestroyBuffer(LogicalDevice, VertexBuffer, nullptr);
}

int Mesh::GetVertexCount()
{
	return VertexCount;
}

VkBuffer Mesh::GetVertexBuffer()
{
	return VertexBuffer;
}

void Mesh::CreateVertexBuffer(std::vector<Vertex>* InVerticies)
{
	VkBufferCreateInfo BufferCreateInfo = {};
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;	
	BufferCreateInfo.size = sizeof(Vertex) * InVerticies->size();	
	BufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
	VkResult Result = vkCreateBuffer(LogicalDevice, &BufferCreateInfo, nullptr, &VertexBuffer);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a Vertex Buffer!");

	// Get memory requirements
	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(LogicalDevice, VertexBuffer, &MemoryRequirements);
	
	// Allocate memory to buffer
	VkMemoryAllocateInfo MemoryAllocationInfo = {};
	MemoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocationInfo.allocationSize = MemoryRequirements.size;
	MemoryAllocationInfo.memoryTypeIndex = FindMemoryTypeIndex(MemoryRequirements.memoryTypeBits,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |	// visibile to the CPU / is able to interact with it
																VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);	// Allows placement of data straight into buffer after mapping
	// Allocate memory to VkDeviceMemory
	Result = vkAllocateMemory(LogicalDevice, &MemoryAllocationInfo, nullptr, &VertexBufferMemory);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate Vertex buffer memory!");

	// Allocate the memory we just made to the vertex buffer
	vkBindBufferMemory(LogicalDevice, VertexBuffer, VertexBufferMemory, 0);

	// Map memory to the vertex buffer
	void* data;																				// create pointer to memory
	vkMapMemory(LogicalDevice, VertexBufferMemory, 0, BufferCreateInfo.size, 0, &data);		// map the vertex buffer memory to the pointer
	memcpy(data, InVerticies->data(), (size_t)BufferCreateInfo.size);						// Copy memory from InVerticies vector to that point
	vkUnmapMemory(LogicalDevice, VertexBufferMemory);										// unmap the vertex buffer memory
}

uint32_t Mesh::FindMemoryTypeIndex(uint32_t InAllowedTypes, VkMemoryPropertyFlags InMemoryPropertyFlags)
{
	// Get properties of physical device
	VkPhysicalDeviceMemoryProperties MemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);

	for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++)
	{
		if ((InAllowedTypes & (1 << i))													// Index of memory type must match corresponding bit in InAllowedTypes
			&& (MemoryProperties.memoryTypes[i].propertyFlags & InMemoryPropertyFlags)	// Make sure desired property bit flags are part of memory types property flags
			== InMemoryPropertyFlags)
		{
			// memory type is valid so return its index
			return i;
		}
	}

	return 0;
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include  <GLFW/glfw3.h>

#include <GLM/glm.hpp>

#include <fstream>

const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 32;

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
};

// Indicies (locations) of Queue families
struct QueueFamilyIndicies
{
	int graphicsFamily = -1;		// Location of Graphics Queue Family
	int presentationFamily = -1;	// Location of Presentation Queue Family

	/* Check if the Queue Family is valid */
	bool IsValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapchainDetails
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		// Surface properties
	std::vector<VkSurfaceFormatKHR> surfaceFormats;		// Surface image formats
	std::vector<VkPresentModeKHR> presentationModes;	// How images should be presented
};

struct SwapchainImage
{
	VkImage image;
	VkImageView imageView;
};

struct UniformBufferObjectViewProjection
{
	glm::mat4 projection;
	glm::mat4 view;
};

struct UniformBufferObjectModel
{
	glm::mat4 model;
};

static std::vector<char> ReadFile(const std::string& inFileName)
{
	// std::ios::ate - start reading from end of file
	std::ifstream file(inFileName, std::ios::binary | std::ios::ate);

	if (!file.is_open())
		throw std::runtime_error("Failed to open file!");

	// get the size of the file
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// move to start of file
	file.seekg(0);

	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}

static uint32_t FindMemoryTypeIndex(VkPhysicalDevice inPhysicalDevice, uint32_t inAllowedTypes, VkMemoryPropertyFlags inMemoryPropertyFlags)
{
	// Get properties of physical device
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(inPhysicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((inAllowedTypes & (1 << i))													// Index of memory type must match corresponding bit in InAllowedTypes
			&& (memoryProperties.memoryTypes[i].propertyFlags & inMemoryPropertyFlags)	// Make sure desired property bit flags are part of memory types property flags
			== inMemoryPropertyFlags)
		{
			// memory type is valid so return its index
			return i;
		}
	}

	return 0;
}

static void CreateBuffer(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, VkDeviceSize InBufferSize, VkBufferUsageFlags InBufferFlags,
	VkMemoryPropertyFlags InBufferProperties, VkBuffer* InBuffer, VkDeviceMemory* InBufferMemory)
{
	VkBufferCreateInfo BufferCreateInfo = {};
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.size = InBufferSize;
	BufferCreateInfo.usage = InBufferFlags;
	BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult Result = vkCreateBuffer(InLogicalDevice, &BufferCreateInfo, nullptr, InBuffer);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a Vertex Buffer!");

	// Get memory requirements
	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(InLogicalDevice, *InBuffer, &MemoryRequirements);

	// Allocate memory to buffer
	VkMemoryAllocateInfo MemoryAllocationInfo = {};
	MemoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocationInfo.allocationSize = MemoryRequirements.size;
	MemoryAllocationInfo.memoryTypeIndex = FindMemoryTypeIndex(InPhysicalDevice, MemoryRequirements.memoryTypeBits, InBufferProperties);
	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT - visibile to the CPU / is able to interact with it
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT - Allows placement of data straight into buffer after mapping
	
	// Allocate memory to VkDeviceMemory
	Result = vkAllocateMemory(InLogicalDevice, &MemoryAllocationInfo, nullptr, InBufferMemory);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate Vertex buffer memory!");

	// Allocate the memory we just made to the vertex buffer
	vkBindBufferMemory(InLogicalDevice, *InBuffer, *InBufferMemory, 0);
}

static void CopyBuffer(VkDevice InLogicalDevice, VkQueue InTranferQueue, VkCommandPool InTransferCommandPool,
	VkBuffer InSourceBuffer, VkBuffer InDestinationBuffer, VkDeviceSize InBufferSize)
{
	// Command buffer to hold transfer commands
	VkCommandBuffer TransferCommandBuffer;

	// command buffer details
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandPool = InTransferCommandPool;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	
	VkResult Result = vkAllocateCommandBuffers(InLogicalDevice, &CommandBufferAllocateInfo, &TransferCommandBuffer);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffer!");

	// info to start recording command buffer
	VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// we only want to use this buffer once.

	// Begin recording transfer commands
	Result = vkBeginCommandBuffer(TransferCommandBuffer, &CommandBufferBeginInfo);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer!");

	// region of data to copy from and to
	VkBufferCopy BufferCopyRegion = {};
	BufferCopyRegion.srcOffset = 0;
	BufferCopyRegion.dstOffset = 0;
	BufferCopyRegion.size = InBufferSize;

	// copy from source to destination buffer
	vkCmdCopyBuffer(TransferCommandBuffer, InSourceBuffer, InDestinationBuffer, 1, &BufferCopyRegion);

	// End recording transfer commands
	Result = vkEndCommandBuffer(TransferCommandBuffer);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to stop recording command buffer!");

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &TransferCommandBuffer;

	// submit transfer command to queue and wait until its done
	Result = vkQueueSubmit(InTranferQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to submit queue!");

	Result = vkQueueWaitIdle(InTranferQueue);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to wait for queue!");

	// free temporary command buffer
	vkFreeCommandBuffers(InLogicalDevice, InTransferCommandPool, 1, &TransferCommandBuffer);
}

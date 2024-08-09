#pragma once

#define GLFW_INCLUDE_VULKAN
#include  <GLFW/glfw3.h>

#include <GLM/glm.hpp>

#include <fstream>

const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 32;
const bool ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS = true;

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texture;	// Texture coordinates (U, V)
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

struct AABB
{
	float minimumX;
	float maximumX;

	float minimumY;
	float maximumY;
};

struct Model
{
	glm::mat4 modelMatrix;
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

static VkCommandBuffer BeginCommandBuffer(VkDevice inLogicalDevice, VkCommandPool inCommandPool)
{
	// Command buffer to hold transfer commands
	VkCommandBuffer commandBuffer;

	// Command Buffer details
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = inCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	vkAllocateCommandBuffers(inLogicalDevice, &allocInfo, &commandBuffer);

	// Information to begin the command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// We're only using the command buffer once, so set up for one time submit

	// Begin recording transfer commands
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

static void EndAndSubmitCommandBuffer(VkDevice inLogicalDevice, VkCommandPool inCommandPool, VkQueue inQueue, VkCommandBuffer inCommandBuffer)
{
	// End commands
	vkEndCommandBuffer(inCommandBuffer);

	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &inCommandBuffer;

	// Submit transfer command to transfer queue and wait until it finishes
	vkQueueSubmit(inQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(inQueue);

	// Free temporary command buffer back to pool
	vkFreeCommandBuffers(inLogicalDevice, inCommandPool, 1, &inCommandBuffer);
}

static void CopyBuffer(VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
	VkBuffer inSourceBuffer, VkBuffer inDestinationBuffer, VkDeviceSize inBufferSize)
{
	// Create buffer
	VkCommandBuffer transferCommandBuffer = BeginCommandBuffer(inLogicalDevice, inTransferCommandPool);

	// Region of data to copy from and to
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = inBufferSize;

	// Command to copy src buffer to dst buffer
	vkCmdCopyBuffer(transferCommandBuffer, inSourceBuffer, inDestinationBuffer, 1, &bufferCopyRegion);

	EndAndSubmitCommandBuffer(inLogicalDevice, inTransferCommandPool, inTransferQueue, transferCommandBuffer);
}

static void CopyImageBuffer(VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
	VkBuffer inSourceBuffer, VkImage inImage, uint32_t inWidth, uint32_t inHeight)
{
	// Create buffer
	VkCommandBuffer transferCommandBuffer = BeginCommandBuffer(inLogicalDevice, inTransferCommandPool);

	VkBufferImageCopy imageRegion = {};
	imageRegion.bufferOffset = 0;											// Offset into data
	imageRegion.bufferRowLength = 0;										// Row length of data to calculate data spacing
	imageRegion.bufferImageHeight = 0;										// Image height to calculate data spacing
	imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Which aspect of image to copy
	imageRegion.imageSubresource.mipLevel = 0;								// Mipmap level to copy
	imageRegion.imageSubresource.baseArrayLayer = 0;						// Starting array layer (if array)
	imageRegion.imageSubresource.layerCount = 1;							// Number of layers to copy starting at baseArrayLayer
	imageRegion.imageOffset = { 0, 0, 0 };									// Offset into image (as opposed to raw data in bufferOffset)
	imageRegion.imageExtent = { inWidth, inHeight, 1 };							// Size of region to copy as (x, y, z) values

	// Copy buffer to given image
	vkCmdCopyBufferToImage(transferCommandBuffer, inSourceBuffer, inImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

	EndAndSubmitCommandBuffer(inLogicalDevice, inTransferCommandPool, inTransferQueue, transferCommandBuffer);
}

static void TransitionImageLayout(VkDevice inLogicalDevice, VkQueue inQueue, VkCommandPool inCommandPool, VkImage inImage, VkImageLayout inOldLayout, VkImageLayout inNewLayout)
{
	// Create buffer
	VkCommandBuffer commandBuffer = BeginCommandBuffer(inLogicalDevice, inCommandPool);

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = inOldLayout;									// Layout to transition from
	imageMemoryBarrier.newLayout = inNewLayout;									// Layout to transition to
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// Queue family to transition from
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;			// Queue family to transition to
	imageMemoryBarrier.image = inImage;											// Image being accessed and modified as part of barrier
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Aspect of image being altered
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;						// First mip level to start alterations on
	imageMemoryBarrier.subresourceRange.levelCount = 1;							// Number of mip levels to alter starting from baseMipLevel
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;						// First layer to start alterations on
	imageMemoryBarrier.subresourceRange.layerCount = 1;							// Number of layers to alter starting from baseArrayLayer

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// If transitioning from new image to image ready to receive data...
	if (inOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && inNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;								// Memory access stage transition must after...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// Memory access stage transition must before...

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// If transitioning from transfer destination to shader readable...
	else if (inOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && inNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,		// Pipeline stages (match to src and dst AccessMasks)
		0,						// Dependency flags
		0, nullptr,				// Memory Barrier count + data
		0, nullptr,				// Buffer Memory Barrier count + data
		1, &imageMemoryBarrier	// Image Memory Barrier count + data
	);

	EndAndSubmitCommandBuffer(inLogicalDevice, inCommandPool, inQueue, commandBuffer);
}

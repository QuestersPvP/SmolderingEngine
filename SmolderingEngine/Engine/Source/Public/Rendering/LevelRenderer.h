#pragma once

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stb_image.h>

// Project includes
#include "Engine/Source/Public/Rendering/Renderer.h"

class LevelRenderer
{
	/* Variables */
public:

private:
	/* Struct that holds general vulkan resources (already created by Renderer) */
	const VulkanResources* vulkanResources;

	// Graphics pipeline
	VkPipeline graphicsPipeline;
	VkPipelineLayout graphicsPipelineLayout;

	// Use push constants to make things move
	VkPushConstantRange pushConstantRange;

	// Uniform bufferz
	std::vector<VkBuffer> viewProjectionUniformBuffers;
	std::vector<VkDeviceMemory> viewProjectionUniformBufferMemory;

	// Descriptor Sets for UBO + Textures
	VkDescriptorSetLayout uboDescriptorSetLayout;
	VkDescriptorSetLayout samplerSetLayout;
	std::vector<VkDescriptorSet> uboDescriptorSets;
	std::vector<VkDescriptorSet> samplerDescriptorSets;
	VkDescriptorPool uboDescriptorPool;
	VkDescriptorPool samplerDescriptorPool;

	// For texturing objects
	VkSampler textureSampler;
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

	/* Functions */
public:
	LevelRenderer() {};
	LevelRenderer(const VulkanResources* _resources);
	void DestroyLevelRenderer();

	// Destroys all textures that the level renderer holds
	void DestroyAllRendererTextures();

	// Handle drawing commands
	void RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex);
	void UpdateUniformBuffer(const class Camera* _camera, uint32_t _imageIndex);
	void ResizeRenderer();

	// Create needed resources
	void CreateDescriptorSetLayout();
	void CreatePushConstantRange();
	void CreateGraphicsPipeline();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateDescriptorPool();

	void AllocateDescriptorSets();

	// Image creation - TODO: MOVE THIS INTO RENDERER UTILS ONCE EVERYTHING IS FINISHED
	VkImage CreateImage(uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling,
		VkImageUsageFlags _usageFlags, VkMemoryPropertyFlags _propertyFlags, VkDeviceMemory* _imageMemory);
	VkImageView CreateImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags);

	// Handles textures
	stbi_uc* LoadTextureFile(std::string _fileName, int* _width, int* _height, VkDeviceSize* _imageSize);
	int CreateTextureImage(std::string _fileName);
	int CreateTexture(std::string _fileName);
	int CreateTextureDescriptor(VkImageView _textureImage);
};
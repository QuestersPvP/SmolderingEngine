#pragma once

// Standard Library
#include <string>
#include <vector>

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


// Project includes
#include "Engine/Source/Public/Rendering/Renderer.h"

class SkyboxRenderer
{
	/* Variables */
public:

private:
	/* Struct that holds general vulkan resources (already created by Renderer) */
	const VulkanResources* vulkanResources;

	// Sampler to get textures
	VkSampler cubemapTextureSampler;
	
	// Store texture image views
	VkImage cubemapImage;
	VkDeviceMemory cubemapImageMemory;
	VkImageView cubemapImageView;

	// Uniform buffers for each frame
	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<VkDeviceMemory> cubemapUniformBufferMemories;

	// UBO and Texture descriptors 
	VkDescriptorSet cubemapSamplerDescriptorSet;
	std::vector<VkDescriptorSet> cubemapUBODescriptorSets;
	
	VkDescriptorPool cubemapSamplerDescriptorPool;
	VkDescriptorPool cubemapUBODescriptorPool;

	VkDescriptorSetLayout cubemapSamplerSetLayout;
	VkDescriptorSetLayout cubemapUBOSetLayout;

	// Graphics pipeline
	VkPipeline cubemapGraphicsPipeline;
	VkPipelineLayout cubemapPipelineLayout;

	// Cube that the skybox renders
	std::vector<struct Vertex> skyboxVertices;
	VkBuffer skyboxVertexBuffer;
	VkDeviceMemory skyboxVertexBufferMemory;

	/* Functions */
public:
	SkyboxRenderer();
	SkyboxRenderer(std::string _fileLocation, std::vector<std::string> _fileNames, const VulkanResources* _resources);
	void DestroySkyboxRenderer();

	// Handle drawing commands
	void RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex);
	void UpdateUniformBuffer(const class Camera* _camera, uint32_t _imageIndex);
	void ResizeRenderer();
	
	// Create needed resources
	void CreateCubemapTextureSampler();
	void CreateCubemapDescriptorSetLayout();
	void CreateCubemapUniformBuffer();
	void CreateCubemapDescriptorPool();
	void CreateCubemapGraphicsPipeline();
	void CreateVertexBuffer();

	void CreateCubemapTextureImage(std::string _fileLocation, std::vector<std::string> _fileNames);
	VkImage CreateCubemapImage(uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling,
		VkImageUsageFlags _usageFlags, VkMemoryPropertyFlags _propertyFlags, VkDeviceMemory* _imageMemory);
	VkImageView CreateCubemapImageView(VkImage _image, VkFormat _format);
	void CreateCubemapTextureDescriptor(VkImageView _cubemapImageView);

	void CopyBufferToCubemapImage(VkDevice _logicalDevice, VkQueue _queue, VkCommandPool _commandPool,
		VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height);

	// Create skybox vertices
	void CreateSkyboxVertices();
};

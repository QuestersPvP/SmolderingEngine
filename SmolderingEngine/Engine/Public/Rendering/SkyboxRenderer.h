#pragma once

// Standard Library
#include <string>
#include <vector>

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class SkyboxRenderer
{
	/* Variables */
public:
	// Retrieved from the Renderer
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkExtent2D swapchainExtent;
	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> swapchainFramebuffers;

	VkSampler cubemapTextureSampler;
	
	VkImage cubemapImage;
	VkDeviceMemory cubemapImageMemory;
	VkImageView cubemapImageView;

	VkDescriptorSet cubemapDescriptorSet;
	VkDescriptorPool cubemapDescriptorPool;
	VkDescriptorSetLayout cubemapSamplerSetLayout;

	VkBuffer cubemapUniformBuffer;
	VkDeviceMemory cubemapUniformBufferMemory;

	VkDescriptorSet cubemapUBODescriptorSet;
	VkDescriptorSetLayout cubemapUBOSetLayout;
	VkDescriptorPool cubemapUBODescriptorPool;

	VkPipeline cubemapGraphicsPipeline;
	VkPipelineLayout cubemapPipelineLayout;
	//VkRenderPass cubemapRenderPass;

	// Cube that the skybox renders
	std::vector<struct Vertex> skyboxVertices;
	VkBuffer skyboxVertexBuffer;
	VkDeviceMemory skyboxVertexBufferMemory;

	/* Functions */
public:
	SkyboxRenderer();
	int InitSkyboxRenderer(std::string _fileLocation, std::vector<std::string> _fileNames, class Renderer* _renderer);
	void DestroySkyboxRenderer();

	void DrawSkybox();
	void RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex);
	void UpdateUniformBuffer(const class Camera* _camera);
	
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
};

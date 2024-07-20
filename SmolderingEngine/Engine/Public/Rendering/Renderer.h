#pragma once

// Standard Library
#include <stdexcept>
#include <vector>
#include <array>
#include <iostream>
#include <set>
#include <algorithm>

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Project includes
#include "Utilities.h"
#include "Game/Public/Game.h"
#include "Mesh.h"

struct VulkanDevices
{
	VkPhysicalDevice PhysicalDevice;
	VkDevice LogicalDevice;
};

class Renderer
{
	/* Variables */
public:
	// Scene (TODO: MOVE)
	ModelViewProjection modelViewProjection;
	void UpdateModelPosition(glm::mat4 inModelMatrix);

private:
	GLFWwindow* Window;
	int CurrentFrame = 0;

	Game SEGame;

	// Vulkan Components
	VkInstance VulkanInstance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VulkanDevices Devices;
	VkQueue GraphicsQueue;
	VkQueue PresentationQueue;
	VkSurfaceKHR VulkanSurface;
	VkSwapchainKHR Swapchain;
	VkFormat SwapchainImageFormat;
	VkExtent2D SwapchainExtent;

	std::vector<SwapchainImage> SwapchainImages;
	std::vector<VkFramebuffer> SwapchainFramebuffers;
	std::vector<VkCommandBuffer> CommandBuffers;

	// Vulkan Pools
	VkCommandPool GraphicsCommandPool;

	// Vulkan Pipeline
	VkPipeline GraphicsPipeline;
	VkPipelineLayout PipelineLayout;
	VkRenderPass RenderPass;

	// Synchronisation
	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderingCompleteSemaphores;
	std::vector<VkFence> DrawFences;

	// Descriptors
	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBufferMemory;

	// Vulkan Validation Layers
	const std::vector<const char*> validationLayers = 
	{
	"VK_LAYER_KHRONOS_validation"
	};

	/* Functions */

public:
	Renderer();
	~Renderer();

	int InitRenderer(GLFWwindow* InWindow, Game InGame);
	void Draw();
	void DestroyRenderer();

	// Vulkan functions
	void CreateVulkanInstance();
	void CreateLogicalDevice();
	void CreateVulkanSurface();
	void CreateSwapChain();
	void CreateRenderpass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateSynchronizationPrimatives();
	void CreateUniformBuffers();
	void CreateDescriptorPool();

	void AllocateDescriptorSets();
	void AllocateCommandBuffers();

	VkImageView CreateImageView(VkImage InImage, VkFormat InFormat, VkImageAspectFlags InAspectFlags);
	VkShaderModule CreateShaderModule(const std::vector<char>& InCode);

	void RecordCommands();
	void UpdateUniformBuffer(uint32_t inImageIndex);

	// Not allocating memory, no need to delete.
	void GetPhysicalDevice();

	// Support functions
	bool CheckInstanceExtensionSupport(std::vector<const char*>* InExtensionsToCheck);
	bool CheckForBestPhysicalDevice(VkPhysicalDevice InPhysicalDevice);
	bool CheckDeviceExtentionSupport(VkPhysicalDevice InPhysicalDevice);
	bool CheckValidationLayerSupport();


	VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& InSurfaceFormats);
	VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> InPresentationMode);
	VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& InSurfaceCapabilities);

	QueueFamilyIndicies GetQueueFamilies(VkPhysicalDevice InPhysicalDevice);
	SwapchainDetails GetSwapchainDetails(VkPhysicalDevice InPhysicalDevice);

	// Validation Layer Callback Functions

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo);
	void SetupDebugMessenger();

	/*
	https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers
	The first parameter specifies the severity of the message, which is one of the following flags:
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes
	*/
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
};


#pragma once

// Standard Library
#include <stdexcept>
#include <vector>
#include <array>
#include <iostream>
#include <set>
#include <algorithm>

// TEMP STD
#include <windows.h>
#include <commdlg.h>

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#include <imgui.h>
//#include <backends/imgui_impl_glfw.h>
//#include <backends/imgui_impl_vulkan.h>

// Project includes
#include "Utilities.h"
//#include "Game/Source/Public/Game.h"
#include "Mesh.h"
#include "MeshModel.h"
#include "Engine/Source/Public/Object/Object.h"
#include "Engine/Source/Public/Object/GameObject.h"

struct VulkanResources
{
	// Instace + Devices
	VkInstance vulkanInstance;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	// Graphics 
	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;

	// Swapchain info
	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	std::vector<SwapchainImage> swapchainImages;

	// Renderpass info
	VkRenderPass renderPass;
};

class Renderer
{
	/* Variables */
public:

private:
	GLFWwindow* window;
	/*Game* seGame;*/
	class Camera* seCamera;
	class EngineLevelManager* seLevelManager;
	int currentFrame = 0;

	// Other renderer references
	class SkyboxRenderer* seSkyboxRenderer;
	class EngineGUIRenderer* seEngineGUIRenderer;
	class LevelRenderer* seLevelRenderer;

	/* General Vulkan Resources that other renderers will need */
	VulkanResources* vulkanResources;

	// Vulkan Components
	VkQueue presentationQueue;
	VkSurfaceKHR vulkanSurface;
	VkFormat swapchainImageFormat;

	std::vector<VkFramebuffer> swapchainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	// Depth Buffer
	VkImage depthBufferImage;
	VkDeviceMemory depthBufferImageMemory;
	VkImageView depthBufferImageView;

	// Synchronisation
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderingCompleteSemaphores;
	std::vector<VkFence> drawFences;

	// Debug
	VkDebugUtilsMessengerEXT debugMessenger;

	// Vulkan Validation Layers
	const std::vector<const char*> validationLayers = 
	{
	"VK_LAYER_KHRONOS_validation"
	};

	/* Functions */

public:
	Renderer() {};
	Renderer(GLFWwindow* _window, class Camera* _camera);
	void DestroyRenderer();

	void Draw();
	void RecordCommands(uint32_t _imageIndex);

	// Re-creates window based off of new window size
	void ResizeRenderer(int inWidth, int inHeight);

	// Vulkan functions
	void CreateVulkanInstance();
	void CreateLogicalDevice();
	void CreateVulkanSurface();
	void CreateSwapChain();
	void CreateRenderpass();
	void CreateDepthBufferImage();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateSynchronizationPrimatives();

	void AllocateCommandBuffers();

	//TODO: MOVE THIS INTO RENDERER UTILS ONCE EVERYTHING IS FINISHED
	VkImageView CreateImageView(VkImage InImage, VkFormat InFormat, VkImageAspectFlags InAspectFlags);
	VkImage CreateImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling,
		VkImageUsageFlags inUsageFlags, VkMemoryPropertyFlags inPropertyFlags, VkDeviceMemory* outImageMemory);


	// Not allocating memory, no need to delete.
	void RetrievePhysicalDevice();

	// Support functions
	bool CheckInstanceExtensionSupport(std::vector<const char*>* InExtensionsToCheck);
	bool CheckForBestPhysicalDevice(VkPhysicalDevice InPhysicalDevice);
	bool CheckDeviceExtentionSupport(VkPhysicalDevice InPhysicalDevice);
	bool CheckValidationLayerSupport();

	VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& inFormats, VkImageTiling inTiling, VkFormatFeatureFlags inFeatureFlags);
	VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& InSurfaceFormats);
	VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> InPresentationMode);
	VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& InSurfaceCapabilities);
	
	// Getters & setters
	VulkanResources GetVulkanResources() { return *vulkanResources; };
	// TODO: REMOVE ASAP
	class LevelRenderer* GetLevelRenderer() { return seLevelRenderer; };
	void SetEngineLevelManager(class EngineLevelManager* inLevel) { seLevelManager = inLevel; };
	VkDevice GetLogicalDevice() { return vulkanResources->logicalDevice; };
	VkPhysicalDevice GetPhysicalDevice() { return vulkanResources->physicalDevice; };
	VkQueue GetGraphicsQueue() { return vulkanResources->graphicsQueue; };
	VkCommandPool GetGraphicsCommandPool() { return vulkanResources->graphicsCommandPool; };
	VkRenderPass GetRenderPass() { return vulkanResources->renderPass; };
	uint32_t GetSwapchainImageSize() { return vulkanResources->swapchainImages.size(); };
	std::vector<VkFramebuffer> GetSwapchainFramebuffers() { return swapchainFramebuffers; };

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


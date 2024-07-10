#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Standard Library
#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>

// Project includes
#include "Utilities.h"

struct VulkanDevices
{
	VkPhysicalDevice PhysicalDevice;
	VkDevice LogicalDevice;
};

class Renderer
{
	/* Variables */
public:

private:
	GLFWwindow* Window;

	// Vulkan Components
	VkInstance VulkanInstance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VulkanDevices Devices;
	VkQueue GraphicsQueue;
	VkQueue PresentationQueue;
	VkSurfaceKHR VulkanSurface;

	// Vulkan Validation
	const std::vector<const char*> ValidationLayers = 
	{
	"VK_LAYER_KHRONOS_validation"
	};

	/* Functions */

public:
	Renderer();
	~Renderer();

	int InitRenderer(GLFWwindow* InWindow);
	void DestroyRenderer();

	// Vulkan functions
	void CreateVulkanInstance();
	void CreateLogicalDevice();
	void CreateVulkanSurface();

	// Not allocating memory, no need to delete.
	void GetPhysicalDevice();

	// Support functions
	bool CheckInstanceExtensionSupport(std::vector<const char*>* InExtensionsToCheck);
	bool CheckForBestPhysicalDevice(VkPhysicalDevice InPhysicalDevice);
	bool CheckDeviceExtentionSupport(VkPhysicalDevice InPhysicalDevice);
	bool CheckValidationLayerSupport();

	QueueFamilyIndicies GetQueueFamilies(VkPhysicalDevice InPhysicalDevice);

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


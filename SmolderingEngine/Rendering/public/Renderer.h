#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

// Project includes
#include "../../Utilities.h"

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
	VulkanDevices Devices;
	VkQueue GraphicsQueue;

	/* Functions */

public:
	Renderer();
	~Renderer();

	int InitRenderer(GLFWwindow* InWindow);
	void DestroyRenderer();

	// Vulkan functions
	void CreateVulkanInstance();
	void CreateLogicalDevice();

	void GetPhysicalDevice();

	// Support functions
	bool CheckInstanceExtensionSupport(std::vector<const char*>* InExtensionsToCheck);
	bool CheckForBestPhysicalDevice(VkPhysicalDevice InPhysicalDevice);

	QueueFamilyIndicies GetQueueFamilies(VkPhysicalDevice InPhysicalDevice);
};


#pragma once

#include <vulkan\vulkan.h> 
#include <stdexcept> 
#include <iostream> 
#include <vector> 
#include <set> 

//#include "../../Rendering/Instances/public/VulkanInstance.h" 
#include "../public/ValidationLayersAndExtensions.h" 

struct SwapChainSupportDetails
{
   VkSurfaceCapabilitiesKHR surfaceCapabilities; // size and images in swapchain
   std::vector<VkSurfaceFormatKHR> surfaceFormats;
   std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
   int graphicsFamily = -1;
   int presentFamily = -1;

   // returns if there is atleast one family present
   bool ArePresent() 
   {
	   return graphicsFamily >= 0 && presentFamily >= 0;
   }
};

class DeviceCheck
{
public:

	DeviceCheck();
	~DeviceCheck();

	// ++++++++++++++ 
	// Physical device 
	// ++++++++++++++ 

	VkPhysicalDevice physicalDevice;
	SwapChainSupportDetails swapchainSupport;
	QueueFamilyIndices queueFamiliyIndices;

	// Used for double buffering in the swapchain if it is supported.
	std::vector<const char*>deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	/*void PickPhysicalDevice(VulkanInstance* vInstance, VkSurfaceKHR surface);*/
	bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
	bool CheckDeviceExtensionSupported(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	QueueFamilyIndices GetQueueFamiliesIndicesOfCurrentDevice();

	// ++++++++++++++ 
	// Logical device 
	// ++++++++++++++ 

	void CreateLogicalDevice(VkSurfaceKHR surface, bool isValidationLayersEnabled,
		ValidationLayersAndExtensions* appValLayersAndExtentions);

	VkDevice logicalDevice;

	// handle to the graphics queue from the queue families of the gpu 
	VkQueue graphicsQueue; // we can also have seperate queue for compute, memory transfer, etc.
	VkQueue presentQueue; // queue for displaying the framebuffer 

	void Destroy();
};


#pragma once

#include "../../Common/Common.h"

namespace SmolderingEngine
{
	// Connect vulkan functions and create a vulkan instance
	bool CreateVulkanInstanceAndFunctions(std::vector<char const*>& _extensions, LIBRARY_TYPE& _vulkanLibrary, VkInstance& _instance);

	// These functions are responsible for connecting
	// the vulkan library and load vulkan functions using macros.
	bool ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE& _vulkanLibrary);
	bool LoadFunctionExportedFromVulkanLoaderLibrary(LIBRARY_TYPE const& _vulkanLibrary);
	bool LoadGlobalLevelFunctions();
	bool LoadInstanceLevelFunctions(VkInstance _instance, std::vector<char const*> const& _enabledExtensions);
	bool LoadDeviceLevelFunctions(VkDevice _logicalDevice, std::vector<char const*> const& _enabledExtensions);

	// These functions are responsible for making the VkInstance
	bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties>& _availableExtensions);
	bool CreateVulkanInstance(std::vector<char const*>& _desiredExtensions, char const* const _applicationName, VkInstance& _instance);

	// These functions are all needed to create a logical device w/ geomitry shaders, and graphics/compute queues
	bool EnumerateAvailablePhysicalDevices(VkInstance _instance, std::vector<VkPhysicalDevice>& _availableDevices);
	bool CheckAvailableDeviceExtensions(VkPhysicalDevice _physicalDevice, std::vector<VkExtensionProperties>& _availableExtensions);
	void GetFeaturesAndPropertiesOfPhysicalDevice(VkPhysicalDevice _physicalDevice, VkPhysicalDeviceFeatures& _deviceFeatures, VkPhysicalDeviceProperties& _deviceProperties);
	bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice _physicalDevice, std::vector<VkQueueFamilyProperties>& _queueFamilies);
	bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice _physicalDevice, VkQueueFlags _desiredCapabilities, uint32_t& _queueFamilyIndex);
	bool CreateLogicalDevice(VkPhysicalDevice _physicalDevice, std::vector<QueueInfo> _queueInfo, std::vector<char const*>& _desiredExtensions, VkPhysicalDeviceFeatures* _desiredFeatures, VkDevice& _logicalDevice);
	void GetDeviceQueue(VkDevice _logicalDevice, uint32_t _queueFamilyIndex, uint32_t _queueIndex, VkQueue& _queue);
	bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(VkInstance _instance, VkDevice& _logicalDevice, VkQueue& _graphicsQueue, VkQueue& _computeQueue);

	// Choose the best the physical and logical device
	bool ChoosePhysicalAndLogicalDevices(VkInstance _instance, std::vector<VkPhysicalDevice> _physicalDevices, VkPhysicalDevice& _physicalDevice, VkDevice& _logicalDevice,
		uint32_t& _graphicsQueueFamilyIndex, uint32_t& _presentQueueFamilyIndex, VkSurfaceKHR _presentationSurface, VkQueue& _graphicsQueue, VkQueue& _presentQueue);

	// Clean-Up
	void DestroyLogicalDevice(VkDevice& _logicalDevice);
	void DestroyVulkanInstance(VkInstance& _instance);
	void ReleaseVulkanLoaderLibrary(LIBRARY_TYPE& _vulkanLibrary);
};


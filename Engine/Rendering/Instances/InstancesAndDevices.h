#pragma once

#include "../../Common/Common.h"

namespace SmoulderingEngine
{
	// These functions are responsible for connecting
	// the vulkan library and load vulkan functions using macros.
	bool ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE& vulkan_library);
	bool LoadFunctionExportedFromVulkanLoaderLibrary(LIBRARY_TYPE const& _vulkanLibrary);
	bool LoadGlobalLevelFunctions();
	bool LoadInstanceLevelFunctions(VkInstance instance, std::vector<char const*> const& enabled_extensions);

	// These functions are responsible for making the VkInstance
	bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties>& _availableExtensions);
	bool CreateVulkanInstance(std::vector<char const*> const& desired_extensions,char const* const application_name, VkInstance& instance);
};


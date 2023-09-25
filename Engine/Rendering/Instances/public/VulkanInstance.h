#pragma once

#include <vulkan\vulkan.h> 

#include "../../../SystemChecks/public/ValidationLayersAndExtensions.h"

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	VkInstance vkInstance;

	void CreateAppAndVkInstance(bool enableValidationLayers, 
		ValidationLayersAndExtensions* valLayersAndExtentions);
};


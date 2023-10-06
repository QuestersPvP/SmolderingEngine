#pragma once

/*
This is the Smouldering Engine namespace, we will be used to store pointers to Vulkan API functions.
This is done through macro definitions to increase performance. 
*/

#include <vulkan/vulkan.h>

namespace SmolderingEngine
{
	#define EXPORTED_VULKAN_FUNCTION(name) extern PFN_##name name; 
	#define GLOBAL_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
	#define INSTANCE_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
	#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension) extern PFN_##name name;
	#define DEVICE_LEVEL_VULKAN_FUNCTION(name) extern PFN_##name name;
	#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension) extern PFN_##name name;
	
	#include "VulkanFunctionsList.inl" 
}
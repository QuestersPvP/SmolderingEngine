/*
The way I was origionally making the engine was using #include <vulkan\vulkan.h> in almost every file.
To increase performance I have learned you can  load function pointers dynamically instead of including
Vulkan in every file that you need.
*/


#ifndef EXPORTED_VULKAN_FUNCTION 
#define EXPORTED_VULKAN_FUNCTION( function ) 
#endif 

EXPORTED_VULKAN_FUNCTION(vkGetInstanceProcAddr)

#undef EXPORTED_VULKAN_FUNCTION 
// ---------------------------------------------------------------------- //
#ifndef GLOBAL_LEVEL_VULKAN_FUNCTION 
#define GLOBAL_LEVEL_VULKAN_FUNCTION( function ) 
#endif 

GLOBAL_LEVEL_VULKAN_FUNCTION(vkEnumerateInstanceExtensionProperties)
GLOBAL_LEVEL_VULKAN_FUNCTION(vkEnumerateInstanceLayerProperties)
GLOBAL_LEVEL_VULKAN_FUNCTION(vkCreateInstance)

#undef GLOBAL_LEVEL_VULKAN_FUNCTION 
// ---------------------------------------------------------------------- //
#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION 
#define INSTANCE_LEVEL_VULKAN_FUNCTION( function ) 
#endif 

INSTANCE_LEVEL_VULKAN_FUNCTION(vkEnumeratePhysicalDevices)
INSTANCE_LEVEL_VULKAN_FUNCTION(vkGetPhysicalDeviceProperties)
INSTANCE_LEVEL_VULKAN_FUNCTION(vkGetPhysicalDeviceFeatures)
INSTANCE_LEVEL_VULKAN_FUNCTION(vkCreateDevice)
INSTANCE_LEVEL_VULKAN_FUNCTION(vkGetDeviceProcAddr)

#undef INSTANCE_LEVEL_VULKAN_FUNCTION 
// ---------------------------------------------------------------------- //
#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION 
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension ) 
#endif 

INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkGetPhysicalDeviceSurfaceSupportKHR, VK_KHR_SURFACE_EXTENSION_NAME)
INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, VK_KHR_SURFACE_EXTENSION_NAME)
INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkGetPhysicalDeviceSurfaceFormatsKHR, VK_KHR_SURFACE_EXTENSION_NAME)

#ifdef VK_USE_PLATFORM_WIN32_KHR 
INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkCreateWin32SurfaceKHR, VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
#elif defined VK_USE_PLATFORM_XCB_KHR 
INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkCreateXcbSurfaceKHR, VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
#elif defined VK_USE_PLATFORM_XLIB_KHR 
INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkCreateXlibSurfaceKHR, VK_KHR_XCB_SURFACE_EXTENSION_NAME)
#endif 

#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION 
// ---------------------------------------------------------------------- //
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION 
#define DEVICE_LEVEL_VULKAN_FUNCTION( function ) 
#endif 

DEVICE_LEVEL_VULKAN_FUNCTION(vkGetDeviceQueue)
DEVICE_LEVEL_VULKAN_FUNCTION(vkDeviceWaitIdle)
DEVICE_LEVEL_VULKAN_FUNCTION(vkDestroyDevice)

DEVICE_LEVEL_VULKAN_FUNCTION(vkCreateBuffer)
DEVICE_LEVEL_VULKAN_FUNCTION(vkGetBufferMemoryRequirements)

#undef DEVICE_LEVEL_VULKAN_FUNCTION 
// ---------------------------------------------------------------------- //
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION 
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension)
#endif 

DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkCreateSwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkGetSwapchainImagesKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkAcquireNextImageKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkQueuePresentKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)
DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(vkDestroySwapchainKHR, VK_KHR_SWAPCHAIN_EXTENSION_NAME)

#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

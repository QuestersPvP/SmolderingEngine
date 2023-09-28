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
#undef INSTANCE_LEVEL_VULKAN_FUNCTION 
// ---------------------------------------------------------------------- //
#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION 
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension ) 
#endif 
#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION 
// ---------------------------------------------------------------------- //
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION 
#define DEVICE_LEVEL_VULKAN_FUNCTION( function ) 
#endif 
#undef DEVICE_LEVEL_VULKAN_FUNCTION 
// ---------------------------------------------------------------------- //
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION 
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension)
#endif 
#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

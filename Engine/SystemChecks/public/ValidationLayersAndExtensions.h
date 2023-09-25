#pragma once

#include <vulkan\vulkan.h> 
#include <vector> 
#include <iostream> 

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h> 

class ValidationLayersAndExtensions 
{

public:
    ValidationLayersAndExtensions();
   ~ValidationLayersAndExtensions();

   const std::vector<const char*> requiredValidationLayers = 
   {
       // NOTE: if this does not work it may be depreciated, 
       // https://vulkan.lunarg.com/doc/view/1.2.189.0/windows/layer_configuration.html
       "VK_LAYER_KHRONOS_validation"
   };

   bool CheckValidationLayerSupport();
   std::vector<const char*>GetRequiredExtensions(bool isValidationLayersEnabled);


   // Debug Callback 
   VkDebugReportCallbackEXT callback;

   void SetupDebugCallback(bool isValidationLayersEnabled, VkInstance vkInstance);
   void Destroy(VkInstance instance, bool isValidationLayersEnabled);


   // Callback 

   int VkResultcreateDebugReportCallbackEXT(
        VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
   {

         auto func = (PFN_vkCreateDebugReportCallbackEXT)
                     vkGetInstanceProcAddr(instance,
                     "vkCreateDebugReportCallbackEXT");

         if (func != nullptr) {
               return func(instance, pCreateInfo, pAllocator, pCallback);
         }
         else {
               return VK_ERROR_EXTENSION_NOT_PRESENT;
         }

   }

   void DestroyDebugReportCallbackEXT(
         VkInstance instance, VkDebugReportCallbackEXT callback,
         const VkAllocationCallbacks* pAllocator)
   {

         auto func = (PFN_vkDestroyDebugReportCallbackEXT)
                     vkGetInstanceProcAddr(instance,
                     "vkDestroyDebugReportCallbackEXT");
         if (func != nullptr) {
               func(instance, callback, pAllocator);
         }
   }
 
   static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
       VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objExt,
       uint64_t obj, size_t location, int32_t code, const char* layerPrefix,
       const char* msg, void* userData) 
   {
       std::cerr << "validation layer: " << msg << std::endl << std::endl;

       return false;
   }
};

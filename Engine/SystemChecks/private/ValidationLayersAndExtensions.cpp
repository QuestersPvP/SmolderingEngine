#include "../public/ValidationLayersAndExtensions.h"

ValidationLayersAndExtensions::ValidationLayersAndExtensions()
{

}

ValidationLayersAndExtensions::~ValidationLayersAndExtensions()
{

}

bool ValidationLayersAndExtensions::CheckValidationLayerSupport() {

    uint32_t layerCount;

    // Get count of validation layers available 
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    // Get the available validation layers names  
    std::vector<VkLayerProperties>availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount,
        availableLayers.data());

    //layers we require
    for (const char* layerName : requiredValidationLayers) 
    {
        // boolean to check if the layer was found 
        bool layerFound = false;

        for (const auto& layerproperties : availableLayers) 
        {
            // If layer is found set the layar found boolean to true 
            if (strcmp(layerName, layerproperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
        
        return true;
    }
}

std::vector<const char*>ValidationLayersAndExtensions::GetRequiredExtensions(bool isValidationLayersEnabled)
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    // Get extensions 
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*>extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    //debug report extention is added. 
    if (isValidationLayersEnabled) 
    {
        extensions.push_back("VK_EXT_debug_report");
        
        // This extension is needed in order to call vkSetDebugUtilsObjectNameEXT()
        extensions.push_back("VK_EXT_debug_utils");
    }

    return extensions;
}

void ValidationLayersAndExtensions::SetupDebugCallback(bool isValidationLayersEnabled, VkInstance vkInstance)
{
    if (!isValidationLayersEnabled)
        return;

    printf("setup call back \n");

    VkDebugReportCallbackCreateInfoEXT info = {};

    info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    info.pfnCallback = DebugCallback; // callback function 

    //createDebugReportCallbackEXT
    if (VkResultcreateDebugReportCallbackEXT(vkInstance, &info, nullptr, &callback) != VK_SUCCESS)
        throw std::runtime_error("failed to set debug callback!");
}

void ValidationLayersAndExtensions::Destroy(VkInstance instance, bool isValidationLayersEnabled)
{
    if (isValidationLayersEnabled)
        DestroyDebugReportCallbackEXT(instance, callback, nullptr);
}

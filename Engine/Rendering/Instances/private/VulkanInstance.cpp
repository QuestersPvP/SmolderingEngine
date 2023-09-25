#include "../public/VulkanInstance.h"

VulkanInstance::VulkanInstance()
{
}

VulkanInstance::~VulkanInstance()
{
}

void VulkanInstance::CreateAppAndVkInstance(bool enableValidationLayers, ValidationLayersAndExtensions* valLayersAndExtentions)
{
    // links the application to the Vulkan library 
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Smouldering Engine";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.pEngineName = "Smouldering Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo vkInstanceInfo = {};
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceInfo.pApplicationInfo = &appInfo;

    // specify extensions and validation layers 
    // these are global meaning they are applicable to whole program not just the device
    std::vector<const char*> extensions = valLayersAndExtentions->GetRequiredExtensions(enableValidationLayers);

    vkInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
    vkInstanceInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers) 
    {
        vkInstanceInfo.enabledLayerCount = static_cast<uint32_t>(valLayersAndExtentions->requiredValidationLayers.size());
        vkInstanceInfo.ppEnabledLayerNames = valLayersAndExtentions->requiredValidationLayers.data();
    }
    else 
    {
        vkInstanceInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&vkInstanceInfo, nullptr, &vkInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vkInstance ");
    }
}

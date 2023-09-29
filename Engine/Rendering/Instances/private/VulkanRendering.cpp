#include "../public/VulkanRendering.h"

VulkanRendering* VulkanRendering::instance = nullptr;

VulkanRendering* VulkanRendering::GetInstance()
{
    if (!instance)
        instance = new VulkanRendering();
    
    return instance;
}

VulkanRendering::~VulkanRendering()
{
}

void VulkanRendering::InitVulkan(GLFWwindow* window)
{
    // -- Platform Specific 
    // Validation and Extension Layers 
    //valLayersAndExt = new ValidationLayersAndExtensions();

    //if (isValidationLayersEnabled && !valLayersAndExt->CheckValidationLayerSupport()) 
    //    throw std::runtime_error("Needed Validation Layers Not Available!"); 
    //

    //// Create App And Vulkan Instance() 
    //vInstance = new VulkanInstance();
    //vInstance->CreateAppAndVkInstance(isValidationLayersEnabled, valLayersAndExt);

    //// Debug CallBack 
    //valLayersAndExt->SetupDebugCallback(isValidationLayersEnabled, vInstance->vkInstance);

    //// Create Surface 
    //if (glfwCreateWindowSurface(vInstance->vkInstance, window, nullptr, &surface) != VK_SUCCESS)
    //    throw std::runtime_error("Failed to create window surface!"); 

    //// Create a new device
    //device = new DeviceCheck();
    //device->PickPhysicalDevice(vInstance, surface);
    //device->CreateLogicalDevice(surface, isValidationLayersEnabled, valLayersAndExt);

    //// Create SwapChain
    //swapChain = new SwapChain();
    //swapChain->Create(surface);
}

DeviceCheck* VulkanRendering::GetDevice()
{
    return device;
}

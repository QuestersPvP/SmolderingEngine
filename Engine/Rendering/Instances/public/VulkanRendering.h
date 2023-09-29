
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h> 

#include <vulkan\vulkan.h> 

#include "../../../SystemChecks/public/ValidationLayersAndExtensions.h"
#include "../../../SystemChecks/public/DeviceCheck.h"

//#include "../public/VulkanInstance.h"
#include "../../RenderPasses/public/SwapChain.h"


#ifdef _DEBUG
const bool isValidationLayersEnabled = true;
#else 
const bool isValidationLayersEnabled = false;
#endif 

class VulkanRendering
{
public:

    static VulkanRendering* instance;
    static VulkanRendering* GetInstance();

    ~VulkanRendering();

    void InitVulkan(GLFWwindow* window);

    DeviceCheck* GetDevice();

private:

    // My Classes 
    ValidationLayersAndExtensions* valLayersAndExt;
    /*VulkanInstance* vInstance;*/
    DeviceCheck* device;

    //surface 
    VkSurfaceKHR surface;

    // Swap chain reference
    SwapChain* swapChain;
};


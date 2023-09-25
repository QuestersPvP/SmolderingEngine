#include "../public/DeviceCheck.h"

DeviceCheck::DeviceCheck()
{
}

DeviceCheck::~DeviceCheck()
{
}

void DeviceCheck::PickPhysicalDevice(VulkanInstance* vInstance, VkSurfaceKHR surface)
{
    // Store count of physical devices
    uint32_t deviceCount = 0;

    // Get number of available GPUs
    vkEnumeratePhysicalDevices(vInstance->vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with vulkan support!"); 

    std::cout << "Device Count: " << deviceCount << std::endl;

    // Now that we know there are devices get them and store them into our vector
    std::vector<VkPhysicalDevice>devices(deviceCount);
    vkEnumeratePhysicalDevices(vInstance->vkInstance, &deviceCount, devices.data());

    std::cout << std::endl;
    std::cout << "DEVICE PROPERTIES" << std::endl;
    std::cout << "=================" << std::endl;

    for (const auto& device : devices) 
    {
        // Get properties of the devices we found
        VkPhysicalDeviceProperties  deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        std::cout << std::endl;
        std::cout << "Device name: " << deviceProperties.deviceName << std::endl;

        if (IsDeviceSuitable(device, surface))
            physicalDevice = device;

        break;
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find suitable GPU!");
}

bool DeviceCheck::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    // Find queue families the device supports 
    QueueFamilyIndices qFamilyIndices = FindQueueFamilies(device, surface);

    // Check device extentions supported 
    bool extensionSupported = CheckDeviceExtensionSupported(device);

    bool swapChainAdequate = false;

    // If swapchain extension is present  
    // Check surface formats and presentation modes are supported 
    if (extensionSupported) 
    {
        swapchainSupport = QuerySwapChainSupport(device, surface);

        swapChainAdequate = !swapchainSupport.surfaceFormats.empty()
            && !swapchainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return qFamilyIndices.ArePresent() && extensionSupported &&
        swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool DeviceCheck::CheckDeviceExtensionSupported(VkPhysicalDevice device)
{
    uint32_t extensionCount;

    // Get number of supported extensions of the device 
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    // Get available device extentions and store them
    std::vector<VkExtensionProperties>availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // Populate with required device exentions we need 
    std::set<std::string>requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // Check if the required extention is present 
    for (const auto& extension : availableExtensions) 
    {
        requiredExtensions.erase(extension.extensionName);
    }

    // If device has the required device extention then return  
    return requiredExtensions.empty();
}

SwapChainSupportDetails DeviceCheck::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    // Get the surface capabilities
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);

    // Get the number of formats present
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) 
    {
        // Store the formats inside the vector of our struct
        details.surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data());
    }

    // Get the number of modes present
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) 
    {
        // Store the modes inside the vector of our struct
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

QueueFamilyIndices DeviceCheck::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    // Get the amount of queue families that are available
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    // Get the actual properties from the queue family
    std::vector<VkQueueFamilyProperties>queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;

    for (const auto& queueFamily : queueFamilies) 
    {
        // Check if the queue family supports graphics queing
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamiliyIndices.graphicsFamily = i;

        // Check if the family supports graphics presentation
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport) 
            queueFamiliyIndices.presentFamily = i;

        // If the family supports graphics and presentation we are done
        if (queueFamiliyIndices.ArePresent())
            break;

        i++;
    }

    return queueFamiliyIndices;
}

QueueFamilyIndices DeviceCheck::GetQueueFamiliesIndicesOfCurrentDevice()
{
    return queueFamiliyIndices;
}

void DeviceCheck::CreateLogicalDevice(VkSurfaceKHR surface, bool isValidationLayersEnabled, ValidationLayersAndExtensions* appValLayersAndExtentions)
{
    // find queue families like graphics and presentation 
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    
    // Store any needed information about the graphics and presentation queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // Store indicies of graphics and presentation queues
    std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

    float queuePriority = 1.0f;

    // For every queue family populate the information
    for (int queueFamily : uniqueQueueFamilies) 
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1; // we only require 1 queue 
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify device features that we will use
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Set up device info struct to aid in making the logical device
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    // Set extension count / names
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (isValidationLayersEnabled) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(appValLayersAndExtentions->requiredValidationLayers.size());
        createInfo.ppEnabledLayerNames = appValLayersAndExtentions->requiredValidationLayers.data();
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
    }

    // Create logical device 
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device !"); 

    // Get handle to the graphics queue of the gpu 
    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0,
        &graphicsQueue);

    // Get handle to the presentation queue of the gpu 
    vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);
}

void DeviceCheck::Destroy()
{
}

#include "Renderer.h"

bool Renderer::InitRendererClass(const WindowParameters& _window)
{
    camera.type = Camera::CameraType::lookat;
    camera.SetPosition(glm::vec3(0.0f, 0.0f, -10.5f));
    camera.SetRotation(glm::vec3(-25.0f, 15.0f, 0.0f));
    camera.SetRotationSpeed(0.5f);
    camera.SetPerspective(60.0f, (float)(width/* / 3.0f*/) / (float)height, 0.1f, 256.0f);

    /*
    NOTES:

    swapChain.queueNodeIndex = graphicsQueue.familyIndex
    swapChain.Surface = presentationSurface
    SCENE = MODEL
    */

    /* Init Vulkan */
    //if (!CreateVulkanInstanceAndFunctions(instanceExtensions, vulkanLibrary, instance))
    //    return false;
    //
    //// Create presentation surface
    //if (!CreatePresentationSurface(instance, _window, presentationSurface))
    //    return false;
    //
    //if (!ChoosePhysicalAndLogicalDevices(instance, physicalDevices, physicalDevice, logicalDevice, graphicsQueue.familyIndex, presentQueue.familyIndex, presentationSurface, graphicsQueue.handle, presentQueue.handle))
    //    return false;
    //
    //if (!GetSupportedDepthFormat(physicalDevice, &depthFormat))
    //    return false;
    //
    //swapchain.handle = VK_NULL_HANDLE;
    //swapchain.instance = instance;
    //swapchain.physicalDevice = physicalDevice;
    //swapchain.logicalDevice = logicalDevice;
    //
    //if (!GenerateSemaphore(logicalDevice, semaphores.presentComplete))
    //    return false;
    //
    //if (!GenerateSemaphore(logicalDevice, semaphores.renderComplete))
    //    return false;
    //
    //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //submitInfo.pWaitDstStageMask = &submitPipelineStages;
    //submitInfo.waitSemaphoreCount = 1;
    //submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    //submitInfo.signalSemaphoreCount = 1;
    //submitInfo.pSignalSemaphores = &semaphores.renderComplete;
    //
    //vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    /*----------------------------------------------------------------------------------------------------------------------*/

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = name.c_str();
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> instanceExtensions;
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    // Get extensions supported by the instance and store for later use
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0)
    {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
        {
            for (VkExtensionProperties _extension : extensions)
            {
                supportedInstanceExtensions.push_back(_extension.extensionName);
            }
        }
    }

    // Enabled requested instance extensions
    if (enabledInstanceExtensions.size() > 0)
    {
        for (const char* enabledExtension : enabledInstanceExtensions)
        {
            // Output message if requested extension is not available
            if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end())
            {
                std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
            }
            instanceExtensions.push_back(enabledExtension);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = NULL;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    // TODO: MAYBE?
    //instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (instanceExtensions.size() > 0)
    {
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        std::cout << "Error creating vulkan instance" << std::endl;
        return false;
    }

    // Physical device
    uint32_t gpuCount = 0;

    // Get number of available physical devices
    if (vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr) != VK_SUCCESS)
    {
        std::cout << "Error enumerating physical devices" << std::endl;
        return false;
    }
    if (gpuCount == 0) 
    {
        std::cout << "No GPUs found!" << std::endl;
        return false;
    }

    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    if (vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data()) != VK_SUCCESS)
    {
        std::cout << "Error enumerating physical devices" << std::endl;
        return false;
    }

    // GPU selection
    // TODO: RE-WORK DEVICE SELECTION
    uint32_t selectedDevice = 0;

    physicalDevice = physicalDevices[selectedDevice];

    // Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Fill mode non solid is required for wireframe display
    if (deviceFeatures.fillModeNonSolid) 
    {
        enabledFeatures.fillModeNonSolid = VK_TRUE;
    };

    // Wide lines must be present for line width > 1.0f
    if (deviceFeatures.wideLines) 
    {
        enabledFeatures.wideLines = VK_TRUE;
    }

    assert(physicalDevice);
    // Store Properties features, limits and properties of the physical device for later use
    // Device properties also contain limits and sparse properties
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    // Features should be checked by the examples before using them
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    // Memory properties are used regularly for creating all kinds of buffers
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    // Queue family properties, used for setting up requested queues upon device creation
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount > 0);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    // Get list of supported extensions
    extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
    if (extCount > 0)
    {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
        {
            for (auto _ext : extensions)
            {
                supportedExtensions.push_back(_ext.extensionName);
            }
        }
    }

    // Desired queues need to be requested upon logical device creation
    // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
    // requests different queue types
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    // Get queue family indices for the requested queue family types
    // Note that the indices may overlap depending on the implementation
    const float defaultQueuePriority(0.0f);

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
    {
        queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queueFamilyProperties);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    }
    else
    {
        queueFamilyIndices.graphics = 0;
    }

    // Dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
    {
        queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, queueFamilyProperties);
        if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
        {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    }
    else
    {
        // Else we use the same queue
        queueFamilyIndices.compute = queueFamilyIndices.graphics;
    }

    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
    {
        queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queueFamilyProperties);
        if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
        {
            // If transfer family index differs, we need an additional queue create info for the transfer queue
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    }
    else
    {
        // Else we use the same queue
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
    }

    // Create the logical device representation
    std::vector<const char*> deviceExtensions(enabledDeviceExtensions);
    // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    // If a pNext(Chain) has been passed, we need to add it to the device creation info
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    if (deviceCreatepNextChain)
    {
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.features = enabledFeatures;
        physicalDeviceFeatures2.pNext = deviceCreatepNextChain;
        deviceCreateInfo.pEnabledFeatures = nullptr;
        deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    }

    if (deviceExtensions.size() > 0)
    {
        for (const char* enabledExtension : deviceExtensions)
        {
            if (!(std::find(supportedExtensions.begin(), supportedExtensions.end(), enabledExtension) != supportedExtensions.end()))
            {
                std::cout << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
            }
        }

        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS)
    {
        std::cout << "Error creating logical device!" << std::endl;
        return false;
    }

    // Create a default command pool for graphics command buffers
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        std::cout << "Error creating command pool" << std::endl;
        return false;
    }

    // Get a graphics queue from the device
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphics, 0, &queue);

    // Find a suitable depth and/or stencil format
    VkBool32 validFormat{ false };
    validFormat = GetSupportedDepthFormat(physicalDevice, &depthFormat);
    assert(validFormat);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = SemaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queue
    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete) != VK_SUCCESS)
    {
        std::cout << "Error creating semaphore" << std::endl;
        return false;
    }
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been submitted and executed
    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete) != VK_SUCCESS)
    {
        std::cout << "Error creating semaphore" << std::endl;
        return false;
    }

    // Set up submit info structure
    // Semaphores will stay the same during application lifetime
    // Command buffer submission info is set by each example
    submitInfo = SubmitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;
    /*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    //TODO: HERE IS WHERE WINDOW IS SETUP

    /* Prepare for rendering */

    /* Init Swapchain */
    //VkColorSpaceKHR imageColorSpace;
    //if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.colorFormat, imageColorSpace))
    //    return false;
    //
    //swapchain.colorSpace = imageColorSpace;

    /*------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = _window.HInstance;
    surfaceCreateInfo.hwnd = _window.HWnd;
    if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
    {
        std::cout << "Error creating presentation surface" << std::endl;
        return false;
    }

    // Get available queue family properties
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);

    assert(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    // Iterate over each queue to learn whether it supports presenting:
    // Find a queue with present support
    // Will be used to present the swap chain images to the windowing system
    std::vector<VkBool32> supportsPresent(queueCount);
    for (uint32_t i = 0; i < queueCount; i++)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < queueCount; i++)
    {
        if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (graphicsQueueNodeIndex == UINT32_MAX)
            {
                graphicsQueueNodeIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE)
            {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    if (presentQueueNodeIndex == UINT32_MAX)
    {
        // If there's no queue that supports both present and graphics
        // try to find a separate present queue
        for (uint32_t i = 0; i < queueCount; ++i)
        {
            if (supportsPresent[i] == VK_TRUE)
            {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    // Exit if either a graphics or a presenting queue hasn't been found
    if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
    {
        std::cout << "Could not find a graphics and/or presenting queue!" << std::endl;
        return false;
    }

    // TODO: Add support for separate graphics and presenting queue
    if (graphicsQueueNodeIndex != presentQueueNodeIndex)
    {
        std::cout << "Separate graphics and presenting queues are not supported yet!" << std::endl;
        return false;
    }

    queueNodeIndex = graphicsQueueNodeIndex;

    // Get list of supported surface formats
    uint32_t formatCount;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL) != VK_SUCCESS)
    {
        std::cout << "Error cannot get physical device surface formats!" << std::endl;
        return false;
    }
    assert(formatCount > 0);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()) != VK_SUCCESS)
    {
        std::cout << "Error getting physical device sufrace formats!" << std::endl;
        return false;
    }

    // We want to get a format that best suits our needs, so we try to get one from a set of preferred formats
    // Initialize the format to the first one returned by the implementation in case we can't find one of the preffered formats
    VkSurfaceFormatKHR selectedFormat = surfaceFormats[0];
    std::vector<VkFormat> preferredImageFormats = 
    {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32
    };

    for (auto& availableFormat : surfaceFormats) 
    {
        if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) 
        {
            selectedFormat = availableFormat;
            break;
        }
    }

    colorFormat = selectedFormat.format;
    colorSpace = selectedFormat.colorSpace;

    /* Creating Command Pool */
    
    //if (!CreateCommandPool(logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool))
    //    return false;

    // TODO: MAYBE NEED TO RE-VISIT THIS?
    /*
    12/14 - The issue I noticed is I am re-defining the CommandPool. Inside of the Init function it is already being created
    */
    VkCommandPoolCreateInfo cmdPoolInfoOther = {};
    cmdPoolInfoOther.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfoOther.queueFamilyIndex = queueNodeIndex;
    cmdPoolInfoOther.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(logicalDevice, &cmdPoolInfoOther, nullptr, &commandPool_OTHER) != VK_SUCCESS)
    {
        std::cout << "Error creating command pool" << std::endl;
        return false;
    }

    /* Setup Swapchain */

    //VkSwapchainKHR oldSwapchain = swapchain.handle;

    //VkSurfaceCapabilitiesKHR surfaceCapabilities;
    //if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
    //    return false;

    //if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
    //    return false;

    //// Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
    //VkPresentModeKHR desiredPresentMode;
    //if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
    //    return false;

    //uint32_t numberOfImages;
    //if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
    //    return false;

    //VkSurfaceTransformFlagBitsKHR surfaceTransform;
    //if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
    //    return false;

    //// TODO: FIX VkCompositeAlphaFlagBitsKHR BY ADDING IT AS PARAMETER IN CREATESWAPCHAIN();
    ////// Find a supported composite alpha format (not all devices support alpha opaque)
    ////VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ////// Simply select the first composite alpha format available
    ////std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = 
    ////{
    ////    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    ////    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    ////    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    ////    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    ////};

    ////for (auto& compositeAlphaFlag : compositeAlphaFlags)
    ////{
    ////    if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
    ////    {
    ////        compositeAlpha = compositeAlphaFlag;
    ////        break;
    ////    };
    ////}

    //if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.colorFormat, imageColorSpace }, swapchain.size, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
    //    return false;

    //if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
    //    return false;

    //// TODO: MAKE THE FOR LOOP BELOW INTO A FUNCTION
    //// Get the swap chain buffers containing the image and imageview
    //swapchain.buffers.resize(swapchain.images.size());
    //for (uint32_t i = 0; i < swapchain.images.size(); i++)
    //{
    //    VkImageViewCreateInfo colorAttachmentView = {};
    //    colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //    colorAttachmentView.pNext = NULL;
    //    colorAttachmentView.format = swapchain.colorFormat;
    //    colorAttachmentView.components = {
    //        VK_COMPONENT_SWIZZLE_R,
    //        VK_COMPONENT_SWIZZLE_G,
    //        VK_COMPONENT_SWIZZLE_B,
    //        VK_COMPONENT_SWIZZLE_A
    //    };
    //    colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    colorAttachmentView.subresourceRange.baseMipLevel = 0;
    //    colorAttachmentView.subresourceRange.levelCount = 1;
    //    colorAttachmentView.subresourceRange.baseArrayLayer = 0;
    //    colorAttachmentView.subresourceRange.layerCount = 1;
    //    colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //    colorAttachmentView.flags = 0;

    //    swapchain.buffers[i].image = swapchain.images[i];

    //    colorAttachmentView.image = swapchain.buffers[i].image;

    //    if (vkCreateImageView(logicalDevice, &colorAttachmentView, nullptr, &swapchain.buffers[i].view) != VK_SUCCESS)
    //    {
    //        std::cout << "Could not create an image view." << std::endl;
    //        return false;
    //    }
    //}

    /*---------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    // Store the current swap chain handle so we can use it later on to ease up recreation
    VkSwapchainKHR oldSwapchain = swapChain;

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR surfCaps;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps))
    {
        std::cout << "Error obtaining surface capabilities" << std::endl;
        return false;
    }

    // Get available present modes
    uint32_t presentModeCount;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL))
    {
        std::cout << "Error getting presentation modes" << std::endl;
        return false;
    }
    assert(presentModeCount > 0);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()))
    {
        std::cout << "Error getting presentation modes" << std::endl;
        return false;
    }

    VkExtent2D swapchainExtent = {};
    // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
    if (surfCaps.currentExtent.width == (uint32_t)-1)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width = width;
        swapchainExtent.height = height;
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCaps.currentExtent;
        width = surfCaps.currentExtent.width;
        height = surfCaps.currentExtent.height;
    }

    // Select a present mode for the swapchain
    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (size_t i = 0; i < presentModeCount; i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    // Determine the number of images
    uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;

    if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
    {
        desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
    }

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        // We prefer a non-rotated transform
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = surfCaps.currentTransform;
    }

    // Find a supported composite alpha format (not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Simply select the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = 
    {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& compositeAlphaFlag : compositeAlphaFlags) 
    {
        if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) 
        {
            compositeAlpha = compositeAlphaFlag;
            break;
        };
    }

    VkSwapchainCreateInfoKHR swapchainCI = {};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = surface;
    swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
    swapchainCI.imageFormat = colorFormat;
    swapchainCI.imageColorSpace = colorSpace;
    swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCI.queueFamilyIndexCount = 0;
    swapchainCI.presentMode = swapchainPresentMode;
    // Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
    swapchainCI.oldSwapchain = oldSwapchain;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.compositeAlpha = compositeAlpha;

    // Enable transfer source on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (vkCreateSwapchainKHR(logicalDevice, &swapchainCI, nullptr, &swapChain) != VK_SUCCESS)
    {
        std::cout << "Error creating swapchain" << std::endl;
        return false;
    }

    // If an existing swap chain is re-created, destroy the old swap chain
    // This also cleans up all the presentable images
    if (oldSwapchain != VK_NULL_HANDLE)
    {
        for (uint32_t i = 0; i < imageCount; i++)
        {
            vkDestroyImageView(logicalDevice, buffers[i].view, nullptr);
        }
        vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
    }
    if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, NULL))
    {
        std::cout << "Error getting swapchain images" << std::endl;
        return false;
    }

    // Get the swap chain images
    images.resize(imageCount);
    if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, images.data()))
    {
        std::cout << "Error getting swapchain images" << std::endl;
        return false;
    }

    // Get the swap chain buffers containing the image and imageview
    buffers.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = NULL;
        colorAttachmentView.format = colorFormat;
        colorAttachmentView.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentView.subresourceRange.baseMipLevel = 0;
        colorAttachmentView.subresourceRange.levelCount = 1;
        colorAttachmentView.subresourceRange.baseArrayLayer = 0;
        colorAttachmentView.subresourceRange.layerCount = 1;
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.flags = 0;

        buffers[i].image = images[i];

        colorAttachmentView.image = buffers[i].image;

        if (vkCreateImageView(logicalDevice, &colorAttachmentView, nullptr, &buffers[i].view))
        {
            std::cout << "Error creating image views" << std::endl;
            return false;
        }
    }

    /* Create Command Buffers */

    //if (!AllocateCommandBuffers(logicalDevice, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, imageCount, drawCmdBuffers))
    //    return false;

    drawCmdBuffers.resize(imageCount);

    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        commandBufferAllocateInfo(
            commandPool_OTHER,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            static_cast<uint32_t>(drawCmdBuffers.size()));

    if (vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, drawCmdBuffers.data()) != VK_SUCCESS)
    {
        std::cout << "Error creating command buffers" << std::endl;
        return false;
    }

    /* Synchronization Primatives */

    //waitFences.resize(drawCmdBuffers.size());
    //
    //for (auto& _fence : waitFences)
    //{
    //    if (!CreateFence(logicalDevice, true, _fence))
    //        return false;
    //}

    // Wait fences to sync command buffer access
    VkFenceCreateInfo fenceCreateInfo = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    waitFences.resize(drawCmdBuffers.size());
    for (auto& fence : waitFences) 
    {
        if (vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
        {
            std::cout << "Error creating a fence" << std::endl;
            return false;
        }
    }

    /* Depth Stencil */
    
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    imageCreateInfo.extent = { width, height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &depthStencil.image) != VK_SUCCESS)
    {
        std::cout << "Could not create an image." << std::endl;
        return false;
    }

    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements(logicalDevice, depthStencil.image, &memReqs);

    VkMemoryAllocateInfo memAllloc{};
    memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllloc.allocationSize = memReqs.size;
    memAllloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryProperties);

    if (vkAllocateMemory(logicalDevice, &memAllloc, nullptr, &depthStencil.mem) != VK_SUCCESS)
    {
        std::cout << "Could not allocate memory!" << std::endl;
        return false;
    }
    if (vkBindImageMemory(logicalDevice, depthStencil.image, depthStencil.mem, 0) != VK_SUCCESS)
    {
        std::cout << "Could not bind memory object to an image." << std::endl;
        return false;
    }

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.image = depthStencil.image;
    imageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT; // DEPTH FORMAT
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &depthStencil.view) != VK_SUCCESS)
    {
        std::cout << "Could not create an image view." << std::endl;
        return false;
    }

    /* Setup the Render pass */

    //std::vector<VkAttachmentDescription> attachments;
    //attachments.resize(2);
    //// Color attachment
    //attachments[0].format = swapchain.colorFormat;
    //attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    //attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //// Depth attachment
    //attachments[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    //attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    //attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //VkAttachmentReference colorReference = {};
    //colorReference.attachment = 0;
    //colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //VkAttachmentReference depthReference = {};
    //depthReference.attachment = 1;
    //depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //VkSubpassDescription subpassDescription = {};
    //subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //subpassDescription.colorAttachmentCount = 1;
    //subpassDescription.pColorAttachments = &colorReference;
    //subpassDescription.pDepthStencilAttachment = &depthReference;
    //subpassDescription.inputAttachmentCount = 0;
    //subpassDescription.pInputAttachments = nullptr;
    //subpassDescription.preserveAttachmentCount = 0;
    //subpassDescription.pPreserveAttachments = nullptr;
    //subpassDescription.pResolveAttachments = nullptr;

    //// Subpass dependencies for layout transitions
    //std::vector<VkSubpassDependency> dependencies;
    //dependencies.resize(2);

    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass = 0;
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    //dependencies[0].dependencyFlags = 0;

    //dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[1].dstSubpass = 0;
    //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].srcAccessMask = 0;
    //dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    //dependencies[1].dependencyFlags = 0;

    //if (!CreateRenderPass(logicalDevice, attachments, {}, dependencies, renderPass))
    //    return false;

    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format = colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        std::cout << "Error creating render pass" << std::endl;
        return false;
    }

    /* Pipeline Cache */

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    if (vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
    {
        std::cout << "Error creating pipeline cache" << std::endl;
        return false;
    }

    /* Frame Buffer */

    //VkImageView frameBufferattachments[2];

    //// Depth/Stencil attachment is the same for all frame buffers
    //frameBufferattachments[1] = depthStencil.view;

    //VkFramebufferCreateInfo frameBufferCreateInfo = {};
    //frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //frameBufferCreateInfo.pNext = NULL;
    //frameBufferCreateInfo.renderPass = renderPass;
    //frameBufferCreateInfo.attachmentCount = 2;
    //frameBufferCreateInfo.pAttachments = frameBufferattachments;
    //frameBufferCreateInfo.width = width;
    //frameBufferCreateInfo.height = height;
    //frameBufferCreateInfo.layers = 1;

    //// Create frame buffers for every swap chain image
    //frameBuffers.resize(imageCount);
    //for (uint32_t i = 0; i < frameBuffers.size(); i++)
    //{
    //    frameBufferattachments[0] = swapchain.buffers[i].view;
    //    if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
    //        return false;
    //    //if (!CreateFramebuffer(logicalDevice, renderPass, frameBufferattachments, swapchain.size.width, swapchain.size.height, 1, framesResources[frame_index].framebuffer))
    //    //    return false;
    //}

    VkImageView frameAttachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    frameAttachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = frameAttachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize(imageCount);
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        frameAttachments[0] = buffers[i].view;
        if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
        {
            std::cout << "Error creating frame buffer" << std::endl;
            return false;
        }
    }

    /* Load Objects */

    // TODO: FINISH
    // 3D model 
    //if (!Load3DModelFromObjFile("S:/SmoulderingEngine/Engine/Application/Source/Other/Models/cube.obj", true, false, false, true, model))
    //    return false;

    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    //model.loadFromFile("S:/SmoulderingEngine/Engine/Application/Source/Other/Models/sphere.gltf", logicalDevice, commandPool, queue, memoryProperties, glTFLoadingFlags);
    model.loadFromFile("S:/vulkan-tutorials/Vulkan/assets/models/treasure_smooth.gltf", logicalDevice, commandPool, queue, memoryProperties, glTFLoadingFlags);

    //model.rotationMatrix = PrepareRotationMatrix(40.0f, { 0.0f, -1.0f, 0.0f });
    //model.translationMatrix = PrepareTranslationMatrix(0.0f, 0.0f, -3.0f);
    //model.modelViewMatrix = model.translationMatrix * model.rotationMatrix;
    //model.perspectiveMatrix = PreparePerspectiveProjectionMatrix(static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height), 50.0f, 1.0f, 2.6f);

    /* Uniform Buffers */

    //uint32_t bufferSize = sizeof(Mesh);             //2 * 16 * sizeof(float)
    ////if (!CreateUniformBuffer(physicalDevice, logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer, uniformBufferMemory))
    ////    return false;

    ////if (!UpdateUniformBuffer(swapchain, physicalDevice, logicalDevice, uniformBuffer, graphicsQueue, framesResources, model))
    ////    return false;

    //uniformBuffer.logicalDevice = logicalDevice;

    //CreateBuffer(logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer.buffer);

    ////// Create the buffer handle
    ////VkBufferCreateInfo bufferCreateInfo;
    ////bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ////bufferCreateInfo.flags = 0;
    ////bufferCreateInfo.usage = 16;
    ////bufferCreateInfo.size = bufferSize; // 144
    ////bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ////bufferCreateInfo.queueFamilyIndexCount = 0;
    ////
    ////if (vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &uniformBuffer.buffer) != VK_SUCCESS)
    ////{
    ////    std::cout << "Error while creating buffer" << std::endl;
    ////    return false;
    ////}

    //// Create the memory backing up the buffer handle
    //VkMemoryRequirements uBufferMemReqs;
    //VkMemoryAllocateInfo memAlloc;
    //memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    //vkGetBufferMemoryRequirements(logicalDevice, uniformBuffer.buffer, &uBufferMemReqs);
    //memAlloc.allocationSize = uBufferMemReqs.size;
    //// Find a memory type index that fits the properties of the buffer
    //memAlloc.memoryTypeIndex = GetMemoryType(uBufferMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryProperties);
    //// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    ////VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    ////if (VK_BUFFER_USAGE_TRANSFER_SRC_BIT & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    ////{
    ////    allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
    ////    allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    ////    memAlloc.pNext = &allocFlagsInfo;
    ////}
    ////if (vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &uniformBuffer.memory) != VK_SUCCESS)
    ////{
    ////    std::cout << "Error allocating memory" << std::endl;
    ////    return false;
    ////}

    //AllocateMemory(logicalDevice, uniformBuffer, bufferSize, GetMemoryType(uBufferMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryProperties));

    ////uniformBuffer.alignment = uBufferMemReqs.alignment;
    ////uniformBuffer.size = bufferSize;
    ////uniformBuffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    ////uniformBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    //// Initialize a default descriptor that covers the whole buffer size
    //uniformBuffer.descriptor.offset = 0;
    //uniformBuffer.descriptor.buffer = uniformBuffer.buffer;
    //uniformBuffer.descriptor.range = VK_WHOLE_SIZE;

    //// Attach the memory to the buffer object
    //if (vkBindBufferMemory(uniformBuffer.logicalDevice, uniformBuffer.buffer, uniformBuffer.memory, 0) != VK_SUCCESS)
    //{
    //    std::cout << "Error binding buffer memory" << std::endl;
    //    return false;
    //}

    ////VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
    ////{
    ////    return vkMapMemory(device, memory, offset, size, 0, &mapped);
    ////}
    //if (vkMapMemory(uniformBuffer.logicalDevice, uniformBuffer.memory, 0, VK_WHOLE_SIZE, 0, &uniformBuffer.mapped) != VK_SUCCESS)
    //{
    //    std::cout << "Error mapping memory" << std::endl;
    //    return false;
    //}

    //// TODO: MAY BE WRONG
    ////projection = camera.matrices.perspective;
    ////uboVS.modelView = camera.matrices.view;
    //memcpy(uniformBuffer.mapped, &model, sizeof(model));

    /*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*
    		VkBufferUsageFlags usageFlags, = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VkMemoryPropertyFlags memoryPropertyFlags, = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vks::Buffer *buffer, = &uniformBuffer,
			VkDeviceSize size, = sizeof(uboVS),
            void *data = nullptr
    */
    
    usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    size = sizeof(uboVS);

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = BufferCreateInfo(usageFlags, size);
    if (vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        std::cout << "Error creating buffer" << std::endl;
        return false;
    }

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqsNew; 
    VkMemoryAllocateInfo memAlloc = memoryAllocateInfo();
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memReqsNew);
    memAlloc.allocationSize = memReqsNew.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = GetMemoryType(memReqsNew.memoryTypeBits, (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), memoryProperties);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    if (vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &memory) != VK_SUCCESS)
    {
        std::cout << "Error mapping memory" << std::endl;
        return false;
    }

    alignment = memReqsNew.alignment;
    size = size;
    usageFlags = usageFlags;
    memoryPropertyFlags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Initialize a default descriptor that covers the whole buffer size
    descriptor.offset = 0;
    descriptor.buffer = buffer;
    descriptor.range = VK_WHOLE_SIZE;

    if (vkBindBufferMemory(logicalDevice, buffer, memory, 0) != VK_SUCCESS)
    {
        std::cout << "Error binding memory" << std::endl;
        return false;
    }

    // Map persistent
    if (vkMapMemory(logicalDevice, memory, 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS)
    {
        std::cout << "Error mapping memory" << std::endl;
        return false;
    }

    UpdateModelPositions();

    /* Descriptor Set Layout */


    //VkDescriptorSetLayoutBinding descriptorSetLayoutBinding =
    //{
    //  0,                                            // uint32_t             binding
    //  VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // VkDescriptorType     descriptorType
    //  1,                                            // uint32_t             descriptorCount
    //  VK_SHADER_STAGE_VERTEX_BIT,                   // VkShaderStageFlags   stageFlags
    //  nullptr                                       // const VkSampler    * pImmutableSamplers
    //};
    //
    //if (!CreateDescriptorSetLayout(logicalDevice, { descriptorSetLayoutBinding }, descriptorSetLayout))
    //    return false;
    //
    //VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    //pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    //pipelineLayoutCreateInfo.setLayoutCount = 1;
    //
    //if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    //    return false;

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
    {
        // Binding 0 : Vertex shader uniform buffer
        descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            setLayoutBindings.size());

    if (vkCreateDescriptorSetLayout(logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        std::cout << "Error creating descriptor set layout" << std::endl;
        return false;
    }

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        pipelineLayoutCreateInfo(
            &descriptorSetLayout,
            1);

    if (vkCreatePipelineLayout(logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        std::cout << "Error creating descriptor set layout" << std::endl;
        return false;
    }

    /* Pipelines */

    //std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions =
    //{
    //    {
    //      0,                            // uint32_t                     binding
    //      6 * sizeof(float),            // uint32_t                     stride
    //      VK_VERTEX_INPUT_RATE_VERTEX   // VkVertexInputRate            inputRate
    //    }
    //};

    //std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
    //{
    //  {
    //    0,                                                                        // uint32_t   location
    //    0,                                                                        // uint32_t   binding
    //    VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
    //    0                                                                         // uint32_t   offset
    //  },
    //  {
    //    1,                                                                        // uint32_t   location
    //    0,                                                                        // uint32_t   binding
    //    VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
    //    3 * sizeof(float)                                                         // uint32_t   offset
    //  }
    //};

    //VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    //SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, vertexAttributeDescriptions, vertexInputStateCreateInfo);

    //VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    //VkPipelineRasterizationStateCreateInfo rasterizationState = pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    //VkPipelineColorBlendAttachmentState blendAttachmentState = pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    //VkPipelineColorBlendStateCreateInfo colorBlendState = pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    //VkPipelineDepthStencilStateCreateInfo depthStencilState = pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    //VkPipelineViewportStateCreateInfo viewportState = pipelineViewportStateCreateInfo(1, 1, 0);
    //VkPipelineMultisampleStateCreateInfo multisampleState = pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
    //std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
    //VkPipelineDynamicStateCreateInfo dynamicState = pipelineDynamicStateCreateInfo(dynamicStateEnables);

    //VkGraphicsPipelineCreateInfo pipelineCI = pipelineCreateInfo(pipelineLayout, renderPass);

    //std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    //pipelineCI.pInputAssemblyState = &inputAssemblyState;
    //pipelineCI.pRasterizationState = &rasterizationState;
    //pipelineCI.pColorBlendState = &colorBlendState;
    //pipelineCI.pMultisampleState = &multisampleState;
    //pipelineCI.pViewportState = &viewportState;
    //pipelineCI.pDepthStencilState = &depthStencilState;
    //pipelineCI.pDynamicState = &dynamicState;
    //pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    //pipelineCI.pStages = shaderStages.data();
    //pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::Color });

    //// Create the graphics pipeline state objects

    //// We are using this pipeline as the base for the other pipelines (derivatives)
    //// Pipeline derivatives can be used for pipelines that share most of their state
    //// Depending on the implementation this may result in better performance for pipeline
    //// switching and faster creation time
    //pipelineCI.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState = pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineViewportStateCreateInfo viewportState = pipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
    VkPipelineDynamicStateCreateInfo dynamicState = pipelineDynamicStateCreateInfo(dynamicStateEnables);
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = pipelineCreateInfo(pipelineLayout, renderPass);

    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::Color }); // GOOD

    // Create the graphics pipeline state objects

    // We are using this pipeline as the base for the other pipelines (derivatives)
    // Pipeline derivatives can be used for pipelines that share most of their state
    // Depending on the implementation this may result in better performance for pipeline
    // switching and faster creation time
    pipelineCI.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

    //shaderStages[0] = LoadShader("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, logicalDevice);
    //shaderStages[1] = LoadShader("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, logicalDevice);
    shaderStages[0] = LoadShader("S:/vulkan-tutorials/Vulkan/shaders/glsl/pipelines/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, logicalDevice);
    shaderStages[1] = LoadShader("S:/vulkan-tutorials/Vulkan/shaders/glsl/pipelines/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, logicalDevice);

    shaderModules.push_back(shaderStages[0].module);
    shaderModules.push_back(shaderStages[1].module);
    
    if (vkCreateGraphicsPipelines(logicalDevice, pipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        std::cout << "Error while creating graphics pipeline!" << std::endl;
        return false;
    }
    //if (!CreateGraphicsPipeline(logicalDevice, pipelineCache, pipelineLayout, renderPass, graphicsPipeline))
    //    return false;

    /* Descriptor Pool */

    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = descriptorPoolCreateInfo(poolSizes, 2);

    if (vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        std::cout << "Error while creating Descriptor Pool" << std::endl;
        return false;
    }

    /* Descriptor Set */

    VkDescriptorSetAllocateInfo allocInfo = descriptorSetAllocateInfo(
            descriptorPool,
            &descriptorSetLayout,
            1);

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
    {
        std::cout << "Error making descriptor sets" << std::endl;
        return false;
    }

    std::vector<VkWriteDescriptorSet> writeDescriptorSets =
    {
        // Binding 0 : Vertex shader uniform buffer
        writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &descriptor)
    };

    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

    /* Command Buffer */

    VkCommandBufferBeginInfo cmdBufInfo = commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = RenderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        if (vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo) != VK_SUCCESS)
        {
            std::cout << "Error begining command buffer" << std::endl;
            return false;
        }

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = Viewport((float)width, (float)height, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = rect2D(width, height, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
        model.bindBuffers(drawCmdBuffers[i]);

        const VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &model.vertices.buffer, offsets);
        vkCmdBindIndexBuffer(drawCmdBuffers[i], model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
        //buffersBound = true;

        viewport.width = (float)width;
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdSetLineWidth(drawCmdBuffers[i], 1.0f);
        model.draw(drawCmdBuffers[i]);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        if (vkEndCommandBuffer(drawCmdBuffers[i]) != VK_SUCCESS)
        {
            std::cout << "Error ending command buffers" << std::endl;
            return false;
        }
    }

//	swapchain.handle = VK_NULL_HANDLE;
//	oldSwapchain = std::move(swapchain.handle);
//
//    if (!CreateVulkanInstanceAndFunctions(instanceExtensions, vulkanLibrary, instance))
//        return false;
//
//    // Create presentation surface
//    if (!CreatePresentationSurface(instance, _window, presentationSurface))
//        return false;
//
//    if (!ChoosePhysicalAndLogicalDevices(instance, physicalDevices, physicalDevice, logicalDevice, graphicsQueue.familyIndex, presentQueue.familyIndex, presentationSurface, graphicsQueue.handle, presentQueue.handle))
//        return false;
//
//    if (!CreateCommandPool(logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool))
//        return false;
//
//    for (uint32_t i = 0; i < 3; i++)
//    {
//        std::vector<VkCommandBuffer> _commandBuffer;
//        VkSemaphore image_acquired_semaphore = VK_NULL_HANDLE;
//        VkSemaphore ready_to_present_semaphore = VK_NULL_HANDLE;
//        VkFence drawing_finished_fence = VK_NULL_HANDLE;
//        VkImageView depth_attachment = VK_NULL_HANDLE;
//        VkFramebuffer tempIdea = VK_NULL_HANDLE;
//
//        if (!AllocateCommandBuffers(logicalDevice, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, _commandBuffer))
//            return false;
//
//        if (!GenerateSemaphore(logicalDevice, image_acquired_semaphore))
//            return false;
//
//        if (!GenerateSemaphore(logicalDevice, ready_to_present_semaphore))
//            return false;
//
//        if (!CreateFence(logicalDevice, true, drawing_finished_fence))
//            return false;
//
//        framesResources.emplace_back(_commandBuffer[0], image_acquired_semaphore, ready_to_present_semaphore, drawing_finished_fence, depth_attachment, tempIdea);
//    }
//
////TODO: MAKE THIS A FUNCTION
//#pragma region Swapchain Creation
//
//    swapchain.ImageViewsRaw.clear();
//    swapchain.imageViews.clear();
//    swapchain.images.clear();
//
//    // Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
//    VkPresentModeKHR desiredPresentMode;
//    if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
//        return false;
//
//    // Get capabilities.
//    VkSurfaceCapabilitiesKHR surfaceCapabilities;
//    if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
//        return false;
//
//    uint32_t numberOfImages;
//    if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
//        return false;
//
//    bool skip = false;
//    if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
//        return false;
//
//    if ((0 == swapchain.size.width) || (0 == swapchain.size.height))
//        skip = true;
//
//    if (!skip)
//    {
//        VkImageUsageFlags imageUsage;
//        if (!SelectDesiredUsageScenariosOfSwapchainImages(surfaceCapabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, imageUsage))
//            return false;
//
//
//        VkSurfaceTransformFlagBitsKHR surfaceTransform;
//        if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
//            return false;
//
//
//        VkColorSpaceKHR imageColorSpace;                                        /* VK_FORMAT_R8G8B8A8_UNORM */
//        if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.format, imageColorSpace))
//            return false;
//
//        if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.format, imageColorSpace }, swapchain.size, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
//            return false;
//
//
//        if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
//            return false;
//    }
//
//    for (size_t i = 0; i < swapchain.images.size(); ++i)
//    {
//        VkImageView* imageView = new VkImageView;
//        swapchain.imageViews.emplace_back(imageView);
//        if (!CreateImageView(logicalDevice, swapchain.images[i], VK_IMAGE_VIEW_TYPE_2D, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, *swapchain.imageViews.back()))
//        {
//            return false;
//        }
//        swapchain.ImageViewsRaw.push_back(*swapchain.imageViews.back());
//    }
//#pragma endregion
//
//    // 3D model 
//    if (!Load3DModelFromObjFile("S:/SmoulderingEngine/Engine/Application/Source/Other/Models/cube.obj", true, false, false, true, model))
//        return false;
//
//    model.rotationMatrix = PrepareRotationMatrix(40.0f, { 0.0f, -1.0f, 0.0f });
//    model.translationMatrix = PrepareTranslationMatrix(0.0f, 0.0f, -3.0f);
//    model.modelViewMatrix = model.translationMatrix * model.rotationMatrix;
//    model.perspectiveMatrix = PreparePerspectiveProjectionMatrix(static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height), 50.0f, 1.0f, 2.6f);
//
//    if (!CreateBuffer(logicalDevice, sizeof(model.data[0]) * model.data.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer))
//        return false;
//
//    if (!AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferMemory))
//        return false;
//
//    if (!UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(model.data[0]) * model.data.size(), &model.data[0], vertexBuffer, 0, 0,
//        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, graphicsQueue.handle, framesResources.front().commandBuffer, {}))
//        return false;
//
//    if (!CreateUniformBuffer(physicalDevice, logicalDevice, 2 * 16 * sizeof(float), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer, uniformBufferMemory))
//        return false;
//
//    if (!UpdateUniformBuffer(swapchain, physicalDevice, logicalDevice, /*commandBuffer,*/ uniformBuffer, graphicsQueue, framesResources, model))
//        return false;
//
//    // Descriptor set with uniform buffer
//    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding =
//    {
//      0,                                          // uint32_t             binding
//      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType     descriptorType
//      1,                                          // uint32_t             descriptorCount
//      VK_SHADER_STAGE_VERTEX_BIT |                // VkShaderStageFlags   stageFlags
//      VK_SHADER_STAGE_GEOMETRY_BIT,
//      nullptr                                     // const VkSampler    * pImmutableSamplers
//    };
//
//    if (!CreateDescriptorSetLayout(logicalDevice, { descriptorSetLayoutBinding }, descriptorSetLayout))
//        return false;
//
//    VkDescriptorPoolSize descriptorPoolSize =
//    {
//      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType     type
//      1                                           // uint32_t             descriptorCount
//    };
//
//    if (!CreateDescriptorPool(logicalDevice, false, 1, { descriptorPoolSize }, descriptorPool))
//        return false;
//
//    if (!AllocateDescriptorSets(logicalDevice, descriptorPool, { descriptorSetLayout }, descriptorSets))
//        return false;
//
//    BufferDescriptorInfo bufferDescriptorUpdate =
//    {
//      descriptorSets[0],                            // VkDescriptorSet                      TargetDescriptorSet
//      0,                                            // uint32_t                             TargetDescriptorBinding
//      0,                                            // uint32_t                             TargetArrayElement
//      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // VkDescriptorType                     TargetDescriptorType
//      {                                             // std::vector<VkDescriptorBufferInfo>  BufferInfos
//        {
//          uniformBuffer,                            // VkBuffer                             buffer
//          0,                                        // VkDeviceSize                         offset
//          VK_WHOLE_SIZE                             // VkDeviceSize                         range
//        }
//      }
//    };
//
//    UpdateDescriptorSets(logicalDevice, {}, { bufferDescriptorUpdate }, {}, {});
//
//    // Render pass
//    std::vector<VkAttachmentDescription> attachmentDescriptions =
//    {
//      {
//        0,                                // VkAttachmentDescriptionFlags     flags
//        swapchain.format,                 // VkFormat                         format
//        VK_SAMPLE_COUNT_1_BIT,            // VkSampleCountFlagBits            samples
//        VK_ATTACHMENT_LOAD_OP_CLEAR,      // VkAttachmentLoadOp               loadOp
//        VK_ATTACHMENT_STORE_OP_STORE,     // VkAttachmentStoreOp              storeOp
//        VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // VkAttachmentLoadOp               stencilLoadOp
//        VK_ATTACHMENT_STORE_OP_DONT_CARE, // VkAttachmentStoreOp              stencilStoreOp
//        VK_IMAGE_LAYOUT_UNDEFINED,        // VkImageLayout                    initialLayout
//        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR   // VkImageLayout                    finalLayout
//      }
//    };
//
//    std::vector<SubpassParameters> subpassParameters =
//    {
//      {
//        VK_PIPELINE_BIND_POINT_GRAPHICS,            // VkPipelineBindPoint                  PipelineType
//        {},                                         // std::vector<VkAttachmentReference>   InputAttachments
//        {
//          {                                         // std::vector<VkAttachmentReference>   ColorAttachments
//            0,                                        // uint32_t                             attachment
//            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout                        layout
//          }
//        },
//        {},                                         // std::vector<VkAttachmentReference>   ResolveAttachments
//        nullptr,                                    // VkAttachmentReference const        * DepthStencilAttachment
//        {}                                          // std::vector<uint32_t>                PreserveAttachments
//      }
//    };
//
//    std::vector<VkSubpassDependency> subpassDependencies =
//    {
//      {
//        VK_SUBPASS_EXTERNAL,                            // uint32_t                   srcSubpass
//        0,                                              // uint32_t                   dstSubpass
//        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // VkPipelineStageFlags       srcStageMask
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags       dstStageMask
//        VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags              srcAccessMask
//        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags              dstAccessMask
//        VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags          dependencyFlags
//      },
//      {
//        0,                                              // uint32_t                   srcSubpass
//        VK_SUBPASS_EXTERNAL,                            // uint32_t                   dstSubpass
//        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags       srcStageMask
//        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // VkPipelineStageFlags       dstStageMask
//        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags              srcAccessMask
//        VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags              dstAccessMask
//        VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags          dependencyFlags
//      }
//    };
//
//    if (!CreateRenderPass(logicalDevice, attachmentDescriptions, subpassParameters, subpassDependencies, renderPass))
//        return false;
//
//    // Graphics pipeline
//    std::vector<unsigned char> vertexShaderSpirv;
//    if (!GetBinaryFileContents("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.vert.spv", vertexShaderSpirv))
//        return false;
//
//    VkShaderModule vertexShaderModule;
//    if (!CreateShaderModule(logicalDevice, vertexShaderSpirv, vertexShaderModule))
//        return false;
//
//    std::vector<unsigned char> fragmentShaderSpirv;
//    if (!GetBinaryFileContents("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.frag.spv", fragmentShaderSpirv))
//        return false;
//
//    VkShaderModule fragmentShaderModule;
//    if (!CreateShaderModule(logicalDevice, fragmentShaderSpirv, fragmentShaderModule))
//        return false;
//
//    std::vector<ShaderStageParameters> shaderStageParams =
//    {
//      {
//        VK_SHADER_STAGE_VERTEX_BIT,     // VkShaderStageFlagBits        ShaderStage
//        vertexShaderModule,             // VkShaderModule               ShaderModule
//        "main",                         // char const                 * EntryPointName
//        nullptr                         // VkSpecializationInfo const * SpecializationInfo
//      },
//      {
//        VK_SHADER_STAGE_FRAGMENT_BIT, // VkShaderStageFlagBits        ShaderStage
//        fragmentShaderModule,      // VkShaderModule               ShaderModule
//        "main",                       // char const                 * EntryPointName
//        nullptr                       // VkSpecializationInfo const * SpecializationInfo
//      }
//    };
//
//    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
//    SpecifyPipelineShaderStages(shaderStageParams, shaderStageCreateInfos);
//
//    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions =
//    {
//        {
//          0,                            // uint32_t                     binding
//          6 * sizeof(float),            // uint32_t                     stride
//          VK_VERTEX_INPUT_RATE_VERTEX   // VkVertexInputRate            inputRate
//        }
//    };
//
//    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
//    {
//      {
//        0,                                                                        // uint32_t   location
//        0,                                                                        // uint32_t   binding
//        VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
//        0                                                                         // uint32_t   offset
//      },
//      {
//        1,                                                                        // uint32_t   location
//        0,                                                                        // uint32_t   binding
//        VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
//        3 * sizeof(float)                                                         // uint32_t   offset
//      }
//    };
//
//    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
//    SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, vertexAttributeDescriptions, vertexInputStateCreateInfo);
//
//    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
//    SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, inputAssemblyStateCreateInfo);
//
//    ViewportInfo viewportInfos =
//    {
//      {                     // std::vector<VkViewport>   Viewports
//        {
//          0.0f,               // float          x
//          0.0f,               // float          y
//          500.0f,             // float          width
//          500.0f,             // float          height
//          0.0f,               // float          minDepth
//          1.0f                // float          maxDepth
//        }
//      },
//      {                     // std::vector<VkRect2D>     Scissors;
//        {
//          {                   // VkOffset2D     offset
//            0,                  // int32_t        x
//            0                   // int32_t        y
//          },
//          {                   // VkExtent2D     extent
//            500,                // uint32_t       width
//            500                 // uint32_t       height
//          }
//        }
//      }
//    };
//
//    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
//    SpecifyPipelineViewportAndScissorTestState(viewportInfos, viewportStateCreateInfo);
//
//    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
//    SpecifyPipelineRasterizationState(false, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f, rasterizationStateCreateInfo);
//
//    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
//    SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, multisampleStateCreateInfo);
//
//    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates =
//    {
//        {
//            false,                          // VkBool32                 blendEnable
//            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcColorBlendFactor
//            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstColorBlendFactor
//            VK_BLEND_OP_ADD,                // VkBlendOp                colorBlendOp
//            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcAlphaBlendFactor
//            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstAlphaBlendFactor
//            VK_BLEND_OP_ADD,                // VkBlendOp                alphaBlendOp
//            VK_COLOR_COMPONENT_R_BIT |      // VkColorComponentFlags    colorWriteMask
//            VK_COLOR_COMPONENT_G_BIT |
//            VK_COLOR_COMPONENT_B_BIT |
//            VK_COLOR_COMPONENT_A_BIT
//        }
//    };
//    VkPipelineColorBlendStateCreateInfo blendStateCreateInfo;
//    SpecifyPipelineBlendState(false, VK_LOGIC_OP_COPY, attachmentBlendStates, { 1.0f, 1.0f, 1.0f, 1.0f }, blendStateCreateInfo);
//
//    std::vector<VkDynamicState> dynamicStates =
//    {
//        VK_DYNAMIC_STATE_VIEWPORT,
//        VK_DYNAMIC_STATE_SCISSOR
//    };
//
//    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
//    SpecifyPipelineDynamicStates(dynamicStates, dynamicStateCreateInfo);
//
//    if (!CreatePipelineLayout(logicalDevice, { descriptorSetLayout }, {}, pipelineLayout))
//        return false;
//
//    VkGraphicsPipelineCreateInfo model_pipeline_create_info;
//    SpecifyGraphicsPipelineCreationParameters(0, shaderStageCreateInfos, vertexInputStateCreateInfo, inputAssemblyStateCreateInfo,
//        nullptr, &viewportStateCreateInfo, rasterizationStateCreateInfo, &multisampleStateCreateInfo, nullptr, &blendStateCreateInfo,
//        &dynamicStateCreateInfo, pipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1, model_pipeline_create_info);
//
//    std::vector<VkPipeline> model_pipeline;
//    if (!CreateGraphicsPipelines(logicalDevice, { model_pipeline_create_info }, VK_NULL_HANDLE, model_pipeline))
//        return false;
//
//    modelPipeline = model_pipeline[0];

    return true;
}

bool Renderer::UpdateRendererClass()
{
    // Acquire the next image from the swap chain
    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, semaphores.presentComplete, (VkFence)nullptr, &frame_index);// swapchain.acquireNextImage(semaphores.presentComplete, &currentBuffer);
    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
    // SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain images will change on resize
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) 
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            //windowResize();
            return false;
        }
        //return;
    }
    else 
    {
        if (result != VK_SUCCESS)
        {
            std::cout << "Error acquiring next image!" << std::endl;
            return false;
        }
    }

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[frame_index];

    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        std::cout << "Error Submitting queue" << std::endl;
        return false;
    }

    //result = swapchain.queuePresent(queue, currentBuffer, semaphores.renderComplete);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &frame_index;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (semaphores.renderComplete != VK_NULL_HANDLE)
    {
        presentInfo.pWaitSemaphores = &semaphores.renderComplete;
        presentInfo.waitSemaphoreCount = 1;
    }
    result = vkQueuePresentKHR(queue, &presentInfo);

    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) 
    {
        //windowResize();
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            return false;
        }
    }
    else 
    {
        if (result != VK_SUCCESS)
        {
            std::cout << "Error acquiring next image!" << std::endl;
            return false;
        }
    }
    if (vkQueueWaitIdle(queue) != VK_SUCCESS)
    {
        std::cout << "Error waiting for queue" << std::endl;
        return false;
    }

    frame_index = (frame_index + 1) % drawCmdBuffers.size();


























    //UpdateModelPositions();

    //if (!UpdateUniformBuffer(swapchain, physicalDevice, logicalDevice, uniformBuffer, graphicsQueue, framesResources, model))
    //    return false;

    //// Rendering
    //if (!WaitForFences(logicalDevice, { framesResources[frame_index].drawingFinishedFence }, false, 2000000000))
    //    return false;
    //
    //if (!ResetFences(logicalDevice, { framesResources[frame_index].drawingFinishedFence }))
    //    return false;
    //
    //uint32_t image_index;
    //if (!AcquireSwapchainImage(logicalDevice, swapchain.handle, framesResources[frame_index].imageAcquiredSemaphore, VK_NULL_HANDLE, image_index)) 
    //{
    //    return false;
    //}
    //
    //std::vector<VkImageView> attachments = { swapchain.ImageViewsRaw[image_index] };
    //if (VK_NULL_HANDLE != framesResources[frame_index].depthAttachment) 
    //{
    //    attachments.push_back(framesResources[frame_index].depthAttachment);
    //}
    //
    //if (!CreateFramebuffer(logicalDevice, renderPass, attachments, swapchain.size.width, swapchain.size.height, 1, framesResources[frame_index].framebuffer))
    //{
    //    return false;
    //}
    //
    //if (!BeginCommandBufferRecordingOperation(framesResources[frame_index].commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) 
    //{
    //    return false;
    //}
    //
    //if (presentQueue.familyIndex != graphicsQueue.familyIndex) 
    //{
    //    ImageTransition image_transition_before_drawing = 
    //    {
    //        swapchain.images[image_index],              // VkImage              Image
    //        VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags        CurrentAccess
    //        VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags        NewAccess
    //        VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout        CurrentLayout
    //        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout        NewLayout
    //        presentQueue.familyIndex,                   // uint32_t             CurrentQueueFamily
    //        graphicsQueue.familyIndex,                  // uint32_t             NewQueueFamily
    //        VK_IMAGE_ASPECT_COLOR_BIT                   // VkImageAspectFlags   Aspect
    //    };
    //    SetImageMemoryBarrier(framesResources[frame_index].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
    //}
    //
    //// Drawing
    //BeginRenderPass(framesResources[frame_index].commandBuffer, renderPass, framesResources[frame_index].framebuffer, { { 0, 0 }, swapchain.size }, { { 0.1f, 0.2f, 0.3f, 1.0f } }, VK_SUBPASS_CONTENTS_INLINE);
    //
    //VkViewport viewport = 
    //{
    //  0.0f,                                       // float    x
    //  0.0f,                                       // float    y
    //  static_cast<float>(swapchain.size.width),   // float    width
    //  static_cast<float>(swapchain.size.height),  // float    height
    //  0.0f,                                       // float    minDepth
    //  1.0f,                                       // float    maxDepth
    //};
    //SetViewportStateDynamically(framesResources[frame_index].commandBuffer, 0, { viewport });
    //
    //VkRect2D scissor = 
    //{
    //  {                                           // VkOffset2D     offset
    //    0,                                          // int32_t        x
    //    0                                           // int32_t        y
    //  },
    //  {                                           // VkExtent2D     extent
    //    swapchain.size.width,                       // uint32_t       width
    //    swapchain.size.height                       // uint32_t       height
    //  }
    //};
    //
    //SetScissorStateDynamically(framesResources[frame_index].commandBuffer, 0, { scissor });
    //
    //BindVertexBuffers(framesResources[frame_index].commandBuffer, 0, { { vertexBuffer, 0 } });
    //
    //BindDescriptorSets(framesResources[frame_index].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSets, {});
    //
    //BindPipelineObject(framesResources[frame_index].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
    //
    //for (size_t i = 0; i < model.parts.size(); ++i) 
    //{
    //    DrawGeometry(framesResources[frame_index].commandBuffer, model.parts[i].vertexCount, 1, model.parts[i].vertexOffset, 0);
    //}
    //
    //EndRenderPass(framesResources[frame_index].commandBuffer);
    //
    //if (presentQueue.familyIndex != graphicsQueue.familyIndex) 
    //{
    //    ImageTransition image_transition_before_present = 
    //    {
    //      swapchain.images[image_index],  // VkImage              Image
    //      VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        CurrentAccess
    //      VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        NewAccess
    //      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // VkImageLayout        CurrentLayout
    //      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // VkImageLayout        NewLayout
    //      graphicsQueue.familyIndex,                // uint32_t             CurrentQueueFamily
    //      presentQueue.familyIndex,                 // uint32_t             NewQueueFamily
    //      VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
    //    };
    //    SetImageMemoryBarrier(framesResources[frame_index].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { image_transition_before_present });
    //}
    //
    //if (!EndCommandBufferRecordingOperation(framesResources[frame_index].commandBuffer)) 
    //{
    //    return false;
    //}
    //
    //std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
    //wait_semaphore_infos.push_back({
    //  framesResources[frame_index].imageAcquiredSemaphore,  // VkSemaphore            Semaphore
    //  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT      ,  // VkPipelineStageFlags   WaitingStage
    //    });
    //
    //if (!SubmitCommandBuffersToQueue(graphicsQueue.handle, wait_semaphore_infos, { framesResources[frame_index].commandBuffer }, { framesResources[frame_index].readyToPresentSemaphore }, framesResources[frame_index].drawingFinishedFence))
    //{
    //    return false;
    //}
    //
    //PresentInfo present_info = 
    //{
    //  swapchain.handle,             // VkSwapchainKHR         Swapchain
    //  image_index                   // uint32_t               ImageIndex
    //};
    //
    //if (!PresentImage(presentQueue.handle, { framesResources[frame_index].readyToPresentSemaphore }, { present_info })) 
    //{
    //    return false;
    //}
    //
    //// Destroy the Frame buffer or else the program will run out of memory eventually!
    //DestroyFramebuffer(logicalDevice, framesResources[frame_index].framebuffer);
    //
    //frame_index = (frame_index + 1) % framesResources.size();

    return true;
}

void Renderer::ShutdownRendererClass()
{
    // TODO: Handle memory
}


bool Renderer::ResizeWindow()
{
    //WaitForAllSubmittedCommandsToBeFinished(logicalDevice);
    //SetApplicationReadyToRender(false);

    //swapchain.ImageViewsRaw.clear();
    //swapchain.imageViews.clear();
    //swapchain.images.clear();

    //oldSwapchain = std::move(swapchain.handle);

    //// Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
    //VkPresentModeKHR desiredPresentMode;
    //if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
    //    return false;

    //// Get capabilities.
    //VkSurfaceCapabilitiesKHR surfaceCapabilities;
    //if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
    //    return false;

    //uint32_t numberOfImages;
    //if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
    //    return false;

    //if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
    //    return false;

    //if ((0 == swapchain.size.width) || (0 == swapchain.size.height))
    //    return true;

    //VkImageUsageFlags imageUsage;
    //if (!SelectDesiredUsageScenariosOfSwapchainImages(surfaceCapabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, imageUsage))
    //    return false;
    //
    //VkSurfaceTransformFlagBitsKHR surfaceTransform;
    //if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
    //    return false;
    //
    //VkColorSpaceKHR imageColorSpace;                                        /* VK_FORMAT_R8G8B8A8_UNORM */
    //if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.format, imageColorSpace))
    //    return false;
    //
    //if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.format, imageColorSpace }, swapchain.size, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
    //    return false;
    //
    //if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
    //    return false;

    //for (size_t i = 0; i < swapchain.images.size(); ++i)
    //{
    //    VkImageView* imageView = new VkImageView;
    //    swapchain.imageViews.emplace_back(imageView);
    //    if (!CreateImageView(logicalDevice, swapchain.images[i], VK_IMAGE_VIEW_TYPE_2D, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, *swapchain.imageViews.back()))
    //    {
    //        return false;
    //    }
    //    swapchain.ImageViewsRaw.push_back(*swapchain.imageViews.back());
    //}

    //if (!UpdateUniformBuffer(swapchain, physicalDevice, logicalDevice, uniformBuffer, graphicsQueue, framesResources, model))
    //    return false;

    //SetApplicationReadyToRender(true);
    return true;
}

void Renderer::UpdateModelPositions()
{
    uboVS.projection = camera.matrices.perspective;
    uboVS.modelView = camera.matrices.view;
    memcpy(mapped, &uboVS, sizeof(uboVS));

    //model.rotationMatrix = PrepareRotationMatrix(40.0f, { 0.0f, -1.0f, 0.0f });
    //model.translationMatrix = PrepareTranslationMatrix(GetTranslattionXValue(), 0.0f, GetTranslattionZValue());
    //model.modelViewMatrix = model.translationMatrix * model.rotationMatrix;
    //model.perspectiveMatrix = PreparePerspectiveProjectionMatrix(static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height), 50.0f, 1.0f, 2.6f);
}

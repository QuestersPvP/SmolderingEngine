#include "Renderer.h"

bool Renderer::InitRendererClass(const WindowParameters& _window)
{
    /* Setup Camera */

    camera.type = Camera::CameraType::lookat;
    camera.SetPosition(glm::vec3(0.0f, 0.0f, -10.5f));
    camera.SetRotation(glm::vec3(-25.0f, 15.0f, 0.0f));
    camera.SetRotationSpeed(1.f);
    camera.SetPerspective(60.0f, (float)(width) / (float)height, 0.1f, 256.0f);

    /* Init Vulkan */

    CreatePhysicalAndLogicalDevice(supportedInstanceExtensions, instance, physicalDevice, properties, features, memoryProperties,
        enabledFeatures, queueFamilyProperties, supportedExtensions, requestedQueueTypes, queueFamilyIndices, logicalDevice);

    // Create a default command pool for graphics command buffers
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS)
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

    /* 
    *   Prepare for rendering
    *   Init Swapchain 
    */

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
    
    VkCommandPoolCreateInfo cmdPoolInfoOther = {};
    cmdPoolInfoOther.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfoOther.queueFamilyIndex = queueNodeIndex;
    cmdPoolInfoOther.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(logicalDevice, &cmdPoolInfoOther, nullptr, &commandBufferCommandPool) != VK_SUCCESS)
    {
        std::cout << "Error creating command pool" << std::endl;
        return false;
    }

    /* Setup Swapchain */

    //if (!CreateSwapchain(swapChain, physicalDevice, surface, width, height, imageCount, images, buffers, logicalDevice, colorFormat, colorSpace))
    //{
    //    std::cout << "Error while creating the swapchain" << std::endl;
    //    return false;
    //}

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

    //drawCmdBuffers.resize(imageCount);

    //VkCommandBufferAllocateInfo cmdBufAllocateInfo =
    //    commandBufferAllocateInfo(
    //        commandBufferCommandPool,
    //        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    //        static_cast<uint32_t>(drawCmdBuffers.size()));

    //if (vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, drawCmdBuffers.data()) != VK_SUCCESS)
    //{
    //    std::cout << "Error creating command buffers" << std::endl;
    //    return false;
    //}

    if (!CreateCommandBuffers(drawCmdBuffers, commandBufferCommandPool, imageCount, logicalDevice))
    {
        std::cout << "Error creating command buffers" << std::endl;
        return false;
    }

    /* Synchronization Primatives */

    if (!CreateSynchronizationPrimitives(waitFences, drawCmdBuffers, logicalDevice))
    {
        std::cout << "Error creating synchronization primatives!" << std::endl;
        return false;
    }

    /* Depth Stencil */
    
    if (!CreateDepthStencil(width, height, logicalDevice, &depthStencil, &memoryProperties, &depthFormat))
    {
        std::cout << "Error creating Depth Stencil!" << std::endl;
        return false;
    }

    /* Setup the Render pass */

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

    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
    model.loadFromFile("S:/SmoulderingEngine/Engine/Application/Source/Other/Models/Castle.gltf", logicalDevice, graphicsCommandPool, queue, memoryProperties, glTFLoadingFlags);
        //"S:/SmoulderingEngine/Engine/Application/Source/Other/Models/test.gltf"

    /* Uniform Buffers */
    
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
    /*size = size;
    usageFlags = usageFlags;*/
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

    shaderStages[0] = LoadShader("S:/vulkan-tutorials/Vulkan/shaders/glsl/pipelines/phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, logicalDevice);
    shaderStages[1] = LoadShader("S:/vulkan-tutorials/Vulkan/shaders/glsl/pipelines/phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, logicalDevice);

    shaderModules.push_back(shaderStages[0].module);
    shaderModules.push_back(shaderStages[1].module);
    
    if (vkCreateGraphicsPipelines(logicalDevice, pipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline) != VK_SUCCESS)
    {
        std::cout << "Error while creating graphics pipeline!" << std::endl;
        return false;
    }

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

    /* Everything is in place, we can render now */
    SetApplicationReadyToRender(true);

    return true;
}

bool Renderer::UpdateRendererClass()
{
    auto currTime = std::chrono::high_resolution_clock::now();

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
        return false;
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
        return false;
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

    UpdateModelPositions();

    frame_index = (frame_index + 1) % drawCmdBuffers.size();

    return true;
}

void Renderer::ShutdownRendererClass()
{
    // TODO: Handle memory
}


bool Renderer::ResizeWindow(uint32_t _width, uint32_t _height)
{
    // TODO: FIGURE OUT THE FOLLOWING: CREATESWAPCHAIN, CREATEFRAMEBUFFER, BUILDCOMMANDBUFFERS

    /* If the application is not ready to render we cannot resize the window yet */
    if (!GetApplicationReadyToRender())
    {
        return false;
    }

    SetApplicationReadyToRender(false);
    
    // Ensure all operations on the device have been finished before destroying resources
    vkDeviceWaitIdle(logicalDevice);
    
    // Recreate swap chain
    width = _width;
    height = _height;
    //if (!CreateSwapchain(swapChain, physicalDevice, surface, width, height, imageCount, images, buffers, logicalDevice, colorFormat, colorSpace))
    //{
    //    std::cout << "Error while creating the swapchain" << std::endl;
    //    return false;
    //}
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

    
    // Recreate the frame buffers
    vkDestroyImageView(logicalDevice, depthStencil.view, nullptr);
    vkDestroyImage(logicalDevice, depthStencil.image, nullptr);
    vkFreeMemory(logicalDevice, depthStencil.mem, nullptr);

    if (!CreateDepthStencil(width, height, logicalDevice, &depthStencil, &memoryProperties, &depthFormat))
    {
        std::cout << "Error creating Depth Stencil" << std::endl;
        return false;
    }
    
    for (uint32_t i = 0; i < frameBuffers.size(); i++) 
    {
        vkDestroyFramebuffer(logicalDevice, frameBuffers[i], nullptr);
    }

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

    //if (!CreateFrameBuffer())
    //{
    //    std::cout << "Error creating Frame Buffer" << std::endl;
    //}

    // Command buffers need to be recreated as they may store
    // references to the recreated frame buffer
    DestroyCommandBuffers(logicalDevice, commandBufferCommandPool, drawCmdBuffers);
    CreateCommandBuffers(drawCmdBuffers, commandBufferCommandPool, imageCount, logicalDevice);
    //BuildCommandBuffers();
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

    // SRS - Recreate fences in case number of swapchain images has changed on resize
    for (auto& fence : waitFences) {
        vkDestroyFence(logicalDevice, fence, nullptr);
    }
    CreateSynchronizationPrimitives(waitFences, drawCmdBuffers, logicalDevice);

    vkDeviceWaitIdle(logicalDevice);

    /* Update camera and Uniform Buffers */
    if ((width > 0.0f) && (height > 0.0f))
        camera.UpdateAspectRatio((float)width / (float)height);
    camera.SetPerspective(60.0f, (float)(width) / (float)height, 0.1f, 256.0f);

    UpdateModelPositions();

    /* Allow the application to start looping again */
    SetApplicationReadyToRender(true);

    return true;
}

void Renderer::UpdateModelPositions()
{
    uboVS.projection = camera.matrices.perspective;
    uboVS.modelView = camera.matrices.view;
    memcpy(mapped, &uboVS, sizeof(uboVS));
}

#include "Renderer.h"

bool Renderer::InitRendererClass(const WindowParameters& _window)
{
    /*
    NOTES:

    swapChain.queueNodeIndex = graphicsQueue.familyIndex
    swapChain.Surface = presentationSurface
    */

    /* Init Vulkan */
    if (!CreateVulkanInstanceAndFunctions(instanceExtensions, vulkanLibrary, instance))
        return false;

    // Create presentation surface
    if (!CreatePresentationSurface(instance, _window, presentationSurface))
        return false;

    if (!ChoosePhysicalAndLogicalDevices(instance, physicalDevices, physicalDevice, logicalDevice, graphicsQueue.familyIndex, presentQueue.familyIndex, presentationSurface, graphicsQueue.handle, presentQueue.handle))
        return false;

    swapchain.handle = VK_NULL_HANDLE;
    swapchain.instance = instance;
    swapchain.physicalDevice = physicalDevice;
    swapchain.logicalDevice = logicalDevice;

    if (!GenerateSemaphore(logicalDevice, semaphores.presentComplete))
        return false;

    if (!GenerateSemaphore(logicalDevice, semaphores.renderComplete))
        return false;

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;

    /* Prepare for rendering */

    /* Init Swapchain */
    VkColorSpaceKHR imageColorSpace;
    if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.colorFormat, imageColorSpace))
        return false;

    swapchain.colorSpace = imageColorSpace;

    /* Creating Command Pool */
    
    if (!CreateCommandPool(logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool))
        return false;

    /* Setup Swapchain */

    VkSwapchainKHR oldSwapchain = swapchain.handle;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
        return false;

    if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
        return false;

    // Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
    VkPresentModeKHR desiredPresentMode;
    if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
        return false;

    uint32_t numberOfImages;
    if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
        return false;

    VkSurfaceTransformFlagBitsKHR surfaceTransform;
    if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
        return false;

    // TODO: FIX VkCompositeAlphaFlagBitsKHR BY ADDING IT AS PARAMETER IN CREATESWAPCHAIN();
    //// Find a supported composite alpha format (not all devices support alpha opaque)
    //VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //// Simply select the first composite alpha format available
    //std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = 
    //{
    //    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    //    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    //    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    //    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    //};

    //for (auto& compositeAlphaFlag : compositeAlphaFlags)
    //{
    //    if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
    //    {
    //        compositeAlpha = compositeAlphaFlag;
    //        break;
    //    };
    //}

    if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.colorFormat, imageColorSpace }, swapchain.size, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
        return false;

    if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
        return false;

    // TODO: MAKE THE FOR LOOP BELOW INTO A FUNCTION
    // Get the swap chain buffers containing the image and imageview
    swapchain.buffers.resize(swapchain.images.size());
    for (uint32_t i = 0; i < swapchain.images.size(); i++)
    {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = NULL;
        colorAttachmentView.format = swapchain.colorFormat;
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

        swapchain.buffers[i].image = swapchain.images[i];

        colorAttachmentView.image = swapchain.buffers[i].image;

        if (vkCreateImageView(logicalDevice, &colorAttachmentView, nullptr, &swapchain.buffers[i].view) != VK_SUCCESS)
        {
            std::cout << "Could not create an image view." << std::endl;
            return false;
        }
    }

    /* Create Command Buffers */

    if (!AllocateCommandBuffers(logicalDevice, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, swapchain.images.size(), drawCommandBuffers))
        return false;

    /* Synchronization Primatives */

    waitFences.resize(drawCommandBuffers.size());

    for (auto& _fence : waitFences)
    {
        if (!CreateFence(logicalDevice, true, _fence))
            return false;
    }

    /* Depth Stencil */
    
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    imageCreateInfo.extent = { swapchain.size.width, swapchain.size.height, 1 };
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
    memAllloc.memoryTypeIndex = 1;
    // TODO: FIGURE OUT vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(logicalDevice, &memAllloc, nullptr, &depthStencil.memory) != VK_SUCCESS)
    {
        std::cout << "Could not allocate memory!" << std::endl;
        return false;
    }
    if (vkBindImageMemory(logicalDevice, depthStencil.image, depthStencil.memory, 0) != VK_SUCCESS)
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
    
    // TODO: FIGURE OUT DEPTHFORMAT
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    //if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
    //    imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    //}

    if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &depthStencil.view) != VK_SUCCESS)
    {
        std::cout << "Could not create an image view." << std::endl;
        return false;
    }

    /* Setup the Render pass */

    std::vector<VkAttachmentDescription> attachments;
    attachments.resize(2);
    // Color attachment
    attachments[0].format = swapchain.colorFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
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
    std::vector<VkSubpassDependency> dependencies;
    dependencies.resize(2);

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

    if (!CreateRenderPass(logicalDevice, attachments, {}, dependencies, renderPass))
        return false;

    /* Pipeline Cache */

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache);

    /* Frame Buffer */

    VkImageView frameBufferattachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    frameBufferattachments[1] = depthStencil.view;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = frameBufferattachments;
    frameBufferCreateInfo.width = swapchain.size.width;
    frameBufferCreateInfo.height = swapchain.size.height;
    frameBufferCreateInfo.layers = 1;

    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapchain.images.size());
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        frameBufferattachments[0] = swapchain.buffers[i].view;
        if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
            return false;
        //if (!CreateFramebuffer(logicalDevice, renderPass, frameBufferattachments, swapchain.size.width, swapchain.size.height, 1, framesResources[frame_index].framebuffer))
        //    return false;
    }

    /* Load Objects */

    // 3D model 
    if (!Load3DModelFromObjFile("S:/SmoulderingEngine/Engine/Application/Source/Other/Models/cube.obj", true, false, false, true, model))
        return false;

    model.rotationMatrix = PrepareRotationMatrix(40.0f, { 0.0f, -1.0f, 0.0f });
    model.translationMatrix = PrepareTranslationMatrix(0.0f, 0.0f, -3.0f);
    model.modelViewMatrix = model.translationMatrix * model.rotationMatrix;
    model.perspectiveMatrix = PreparePerspectiveProjectionMatrix(static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height), 50.0f, 1.0f, 2.6f);

    /* Uniform Buffers */
    /* Descriptor Set Layout */
    /* Pipelines */
    /* Descriptor Pool */
    /* Descriptor Set */
    /* Command Buffer */

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
    model.rotationMatrix = PrepareRotationMatrix(40.0f, { 0.0f, -1.0f, 0.0f });
    model.translationMatrix = PrepareTranslationMatrix(GetTranslattionXValue(), 0.0f, GetTranslattionZValue());
    model.modelViewMatrix = model.translationMatrix * model.rotationMatrix;
    model.perspectiveMatrix = PreparePerspectiveProjectionMatrix(static_cast<float>(swapchain.size.width) / static_cast<float>(swapchain.size.height), 50.0f, 1.0f, 2.6f);
}

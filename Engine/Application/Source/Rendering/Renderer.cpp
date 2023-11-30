#include "Renderer.h"

bool Renderer::InitRendererClass(const WindowParameters& _window)
{
	swapchain.handle = VK_NULL_HANDLE;
	oldSwapchain = std::move(swapchain.handle);

    if (!CreateVulkanInstanceAndFunctions(instanceExtensions, vulkanLibrary, instance))
        return false;

    // Create presentation surface
    if (!CreatePresentationSurface(instance, _window, presentationSurface))
        return false;

    if (!ChoosePhysicalAndLogicalDevices(instance, physicalDevices, physicalDevice, logicalDevice, graphicsQueue.familyIndex, presentQueue.familyIndex, presentationSurface, graphicsQueue.handle, presentQueue.handle))
        return false;

    if (!CreateCommandPool(logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool))
        return false;

    for (uint32_t i = 0; i < 3; i++)
    {
        std::vector<VkCommandBuffer> _commandBuffer;
        VkSemaphore image_acquired_semaphore = VK_NULL_HANDLE;
        VkSemaphore ready_to_present_semaphore = VK_NULL_HANDLE;
        VkFence drawing_finished_fence = VK_NULL_HANDLE;
        VkImageView depth_attachment = VK_NULL_HANDLE;
        VkFramebuffer tempIdea = VK_NULL_HANDLE;

        if (!AllocateCommandBuffers(logicalDevice, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, _commandBuffer))
            return false;

        if (!GenerateSemaphore(logicalDevice, image_acquired_semaphore))
            return false;

        if (!GenerateSemaphore(logicalDevice, ready_to_present_semaphore))
            return false;

        if (!CreateFence(logicalDevice, true, drawing_finished_fence))
            return false;

        framesResources.emplace_back(_commandBuffer[0], image_acquired_semaphore, ready_to_present_semaphore, drawing_finished_fence, depth_attachment, tempIdea);
    }

//TODO: MAKE THIS A FUNCTION
#pragma region Swapchain Creation

    swapchain.ImageViewsRaw.clear();
    swapchain.imageViews.clear();
    swapchain.images.clear();

    // Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
    VkPresentModeKHR desiredPresentMode;
    if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
        return false;

    // Get capabilities.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
        return false;

    uint32_t numberOfImages;
    if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
        return false;

    bool skip = false;
    if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
        return false;

    if ((0 == swapchain.size.width) || (0 == swapchain.size.height))
        skip = true;

    if (!skip)
    {
        VkImageUsageFlags imageUsage;
        if (!SelectDesiredUsageScenariosOfSwapchainImages(surfaceCapabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, imageUsage))
            return false;


        VkSurfaceTransformFlagBitsKHR surfaceTransform;
        if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
            return false;


        VkColorSpaceKHR imageColorSpace;                                        /* VK_FORMAT_R8G8B8A8_UNORM */
        if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.format, imageColorSpace))
            return false;

        if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.format, imageColorSpace }, swapchain.size, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
            return false;


        if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
            return false;
    }

    for (size_t i = 0; i < swapchain.images.size(); ++i)
    {
        VkImageView* imageView = new VkImageView;
        swapchain.imageViews.emplace_back(imageView);
        if (!CreateImageView(logicalDevice, swapchain.images[i], VK_IMAGE_VIEW_TYPE_2D, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, *swapchain.imageViews.back()))
        {
            return false;
        }
        swapchain.ImageViewsRaw.push_back(*swapchain.imageViews.back());
    }
#pragma endregion

    // 3D model 
    if (!Load3DModelFromObjFile("S:/SmoulderingEngine/Engine/Application/Source/Other/Models/cube.obj", true, false, false, true, model))
        return false;

    if (!CreateBuffer(logicalDevice, sizeof(model.data[0]) * model.data.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer))
        return false;

    if (!AllocateAndBindMemoryObjectToBuffer(physicalDevice, logicalDevice, vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferMemory))
        return false;

    if (!UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(physicalDevice, logicalDevice, sizeof(model.data[0]) * model.data.size(), &model.data[0], vertexBuffer, 0, 0,
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, graphicsQueue.handle, framesResources.front().commandBuffer, {}))
        return false;

    if (!CreateUniformBuffer(physicalDevice, logicalDevice, 2 * 16 * sizeof(float), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformBuffer, uniformBufferMemory))
        return false;

    if (!UpdateUniformBuffer(swapchain, physicalDevice, logicalDevice, /*commandBuffer,*/ uniformBuffer, graphicsQueue, framesResources))
        return false;

    // Descriptor set with uniform buffer
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding =
    {
      0,                                          // uint32_t             binding
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType     descriptorType
      1,                                          // uint32_t             descriptorCount
      VK_SHADER_STAGE_VERTEX_BIT |                // VkShaderStageFlags   stageFlags
      VK_SHADER_STAGE_GEOMETRY_BIT,
      nullptr                                     // const VkSampler    * pImmutableSamplers
    };

    if (!CreateDescriptorSetLayout(logicalDevice, { descriptorSetLayoutBinding }, descriptorSetLayout))
        return false;

    VkDescriptorPoolSize descriptorPoolSize =
    {
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType     type
      1                                           // uint32_t             descriptorCount
    };

    if (!CreateDescriptorPool(logicalDevice, false, 1, { descriptorPoolSize }, descriptorPool))
        return false;

    if (!AllocateDescriptorSets(logicalDevice, descriptorPool, { descriptorSetLayout }, descriptorSets))
        return false;

    BufferDescriptorInfo bufferDescriptorUpdate =
    {
      descriptorSets[0],                            // VkDescriptorSet                      TargetDescriptorSet
      0,                                            // uint32_t                             TargetDescriptorBinding
      0,                                            // uint32_t                             TargetArrayElement
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // VkDescriptorType                     TargetDescriptorType
      {                                             // std::vector<VkDescriptorBufferInfo>  BufferInfos
        {
          uniformBuffer,                            // VkBuffer                             buffer
          0,                                        // VkDeviceSize                         offset
          VK_WHOLE_SIZE                             // VkDeviceSize                         range
        }
      }
    };

    UpdateDescriptorSets(logicalDevice, {}, { bufferDescriptorUpdate }, {}, {});

    /* NEW STUFF */

    // Render pass
    std::vector<VkAttachmentDescription> attachmentDescriptions =
    {
      {
        0,                                // VkAttachmentDescriptionFlags     flags
        swapchain.format,                 // VkFormat                         format
        VK_SAMPLE_COUNT_1_BIT,            // VkSampleCountFlagBits            samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,      // VkAttachmentLoadOp               loadOp
        VK_ATTACHMENT_STORE_OP_STORE,     // VkAttachmentStoreOp              storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // VkAttachmentLoadOp               stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE, // VkAttachmentStoreOp              stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,        // VkImageLayout                    initialLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR   // VkImageLayout                    finalLayout
      }
    };

    std::vector<SubpassParameters> subpassParameters =
    {
      {
        VK_PIPELINE_BIND_POINT_GRAPHICS,            // VkPipelineBindPoint                  PipelineType
        {},                                         // std::vector<VkAttachmentReference>   InputAttachments
        {
          {                                         // std::vector<VkAttachmentReference>   ColorAttachments
            0,                                        // uint32_t                             attachment
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout                        layout
          }
        },
        {},                                         // std::vector<VkAttachmentReference>   ResolveAttachments
        nullptr,                                    // VkAttachmentReference const        * DepthStencilAttachment
        {}                                          // std::vector<uint32_t>                PreserveAttachments
      }
    };

    std::vector<VkSubpassDependency> subpassDependencies =
    {
      {
        VK_SUBPASS_EXTERNAL,                            // uint32_t                   srcSubpass
        0,                                              // uint32_t                   dstSubpass
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // VkPipelineStageFlags       srcStageMask
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags       dstStageMask
        VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags              srcAccessMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags              dstAccessMask
        VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags          dependencyFlags
      },
      {
        0,                                              // uint32_t                   srcSubpass
        VK_SUBPASS_EXTERNAL,                            // uint32_t                   dstSubpass
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags       srcStageMask
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // VkPipelineStageFlags       dstStageMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags              srcAccessMask
        VK_ACCESS_MEMORY_READ_BIT,                      // VkAccessFlags              dstAccessMask
        VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags          dependencyFlags
      }
    };

    if (!CreateRenderPass(logicalDevice, attachmentDescriptions, subpassParameters, subpassDependencies, renderPass))
        return false;

    // Graphics pipeline
    std::vector<unsigned char> vertexShaderSpirv;
    if (!GetBinaryFileContents("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.vert.spv", vertexShaderSpirv))
        return false;

    VkShaderModule vertexShaderModule;
    if (!CreateShaderModule(logicalDevice, vertexShaderSpirv, vertexShaderModule))
        return false;

    std::vector<unsigned char> fragmentShaderSpirv;
    if (!GetBinaryFileContents("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.frag.spv", fragmentShaderSpirv))
        return false;

    VkShaderModule fragmentShaderModule;
    if (!CreateShaderModule(logicalDevice, fragmentShaderSpirv, fragmentShaderModule))
        return false;

    std::vector<ShaderStageParameters> shaderStageParams =
    {
      {
        VK_SHADER_STAGE_VERTEX_BIT,     // VkShaderStageFlagBits        ShaderStage
        vertexShaderModule,             // VkShaderModule               ShaderModule
        "main",                         // char const                 * EntryPointName
        nullptr                         // VkSpecializationInfo const * SpecializationInfo
      },
      {
        VK_SHADER_STAGE_FRAGMENT_BIT, // VkShaderStageFlagBits        ShaderStage
        fragmentShaderModule,      // VkShaderModule               ShaderModule
        "main",                       // char const                 * EntryPointName
        nullptr                       // VkSpecializationInfo const * SpecializationInfo
      }
    };

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
    SpecifyPipelineShaderStages(shaderStageParams, shaderStageCreateInfos);

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions =
    {
        {
          0,                            // uint32_t                     binding
          6 * sizeof(float),            // uint32_t                     stride
          VK_VERTEX_INPUT_RATE_VERTEX   // VkVertexInputRate            inputRate
        }
    };

    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
    {
      {
        0,                                                                        // uint32_t   location
        0,                                                                        // uint32_t   binding
        VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
        0                                                                         // uint32_t   offset
      },
      {
        1,                                                                        // uint32_t   location
        0,                                                                        // uint32_t   binding
        VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
        3 * sizeof(float)                                                         // uint32_t   offset
      }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, vertexAttributeDescriptions, vertexInputStateCreateInfo);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    SpecifyPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false, inputAssemblyStateCreateInfo);

    ViewportInfo viewportInfos =
    {
      {                     // std::vector<VkViewport>   Viewports
        {
          0.0f,               // float          x
          0.0f,               // float          y
          500.0f,             // float          width
          500.0f,             // float          height
          0.0f,               // float          minDepth
          1.0f                // float          maxDepth
        }
      },
      {                     // std::vector<VkRect2D>     Scissors;
        {
          {                   // VkOffset2D     offset
            0,                  // int32_t        x
            0                   // int32_t        y
          },
          {                   // VkExtent2D     extent
            500,                // uint32_t       width
            500                 // uint32_t       height
          }
        }
      }
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    SpecifyPipelineViewportAndScissorTestState(viewportInfos, viewportStateCreateInfo);

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    SpecifyPipelineRasterizationState(false, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, false, 0.0f, 0.0f, 0.0f, 1.0f, rasterizationStateCreateInfo);

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    SpecifyPipelineMultisampleState(VK_SAMPLE_COUNT_1_BIT, false, 0.0f, nullptr, false, false, multisampleStateCreateInfo);

    std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates =
    {
        {
            false,                          // VkBool32                 blendEnable
            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcColorBlendFactor
            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstColorBlendFactor
            VK_BLEND_OP_ADD,                // VkBlendOp                colorBlendOp
            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            srcAlphaBlendFactor
            VK_BLEND_FACTOR_ONE,            // VkBlendFactor            dstAlphaBlendFactor
            VK_BLEND_OP_ADD,                // VkBlendOp                alphaBlendOp
            VK_COLOR_COMPONENT_R_BIT |      // VkColorComponentFlags    colorWriteMask
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
        }
    };
    VkPipelineColorBlendStateCreateInfo blendStateCreateInfo;
    SpecifyPipelineBlendState(false, VK_LOGIC_OP_COPY, attachmentBlendStates, { 1.0f, 1.0f, 1.0f, 1.0f }, blendStateCreateInfo);

    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    SpecifyPipelineDynamicStates(dynamicStates, dynamicStateCreateInfo);

    if (!CreatePipelineLayout(logicalDevice, { descriptorSetLayout }, {}, pipelineLayout))
        return false;

    VkGraphicsPipelineCreateInfo model_pipeline_create_info;
    SpecifyGraphicsPipelineCreationParameters(0, shaderStageCreateInfos, vertexInputStateCreateInfo, inputAssemblyStateCreateInfo,
        nullptr, &viewportStateCreateInfo, rasterizationStateCreateInfo, &multisampleStateCreateInfo, nullptr, &blendStateCreateInfo,
        &dynamicStateCreateInfo, pipelineLayout, renderPass, 0, VK_NULL_HANDLE, -1, model_pipeline_create_info);

    std::vector<VkPipeline> model_pipeline;
    if (!CreateGraphicsPipelines(logicalDevice, { model_pipeline_create_info }, VK_NULL_HANDLE, model_pipeline))
        return false;

    modelPipeline = model_pipeline[0];

    return true;
}

bool Renderer::UpdateRendererClass()
{
    // Show the window (assuming windows OS)
    //ShowWindow(windowParams.HWnd, SW_SHOWNORMAL);
    //UpdateWindow(windowParams.HWnd);

    //MSG message;
    //while (true)
    //{
        //if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
        //{
        //    // TODO: add event handeling (e.g. x button clicked, resized, etc.)

        //    TranslateMessage(&message);
        //    DispatchMessage(&message);
        //}
        //else
        //{
            // Rendering
            if (!WaitForFences(logicalDevice, { framesResources[frame_index].drawingFinishedFence }, false, 2000000000)) {
                return false;
            }

            if (!ResetFences(logicalDevice, { framesResources[frame_index].drawingFinishedFence })) {
                return false;
            }

            uint32_t image_index;
            if (!AcquireSwapchainImage(logicalDevice, swapchain.handle, framesResources[frame_index].imageAcquiredSemaphore, VK_NULL_HANDLE, image_index)) 
            {
                return false;
            }

            std::vector<VkImageView> attachments = { swapchain.ImageViewsRaw[image_index] };
            if (VK_NULL_HANDLE != framesResources[frame_index].depthAttachment) 
            {
                attachments.push_back(framesResources[frame_index].depthAttachment);
            }

            if (!CreateFramebuffer(logicalDevice, renderPass, attachments, swapchain.size.width, swapchain.size.height, 1, framesResources[frame_index].framebuffer))
            {
                return false;
            }

            if (!BeginCommandBufferRecordingOperation(framesResources[frame_index].commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) 
            {
                return false;
            }

            if (presentQueue.familyIndex != graphicsQueue.familyIndex) 
            {
                ImageTransition image_transition_before_drawing = 
                {
                    swapchain.images[image_index],              // VkImage              Image
                    VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags        CurrentAccess
                    VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags        NewAccess
                    VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout        CurrentLayout
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout        NewLayout
                    presentQueue.familyIndex,                   // uint32_t             CurrentQueueFamily
                    graphicsQueue.familyIndex,                  // uint32_t             NewQueueFamily
                    VK_IMAGE_ASPECT_COLOR_BIT                   // VkImageAspectFlags   Aspect
                };
                SetImageMemoryBarrier(framesResources[frame_index].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { image_transition_before_drawing });
            }

            // Drawing
            BeginRenderPass(framesResources[frame_index].commandBuffer, renderPass, framesResources[frame_index].framebuffer, { { 0, 0 }, swapchain.size }, { { 0.1f, 0.2f, 0.3f, 1.0f } }, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = 
            {
              0.0f,                                       // float    x
              0.0f,                                       // float    y
              static_cast<float>(swapchain.size.width),   // float    width
              static_cast<float>(swapchain.size.height),  // float    height
              0.0f,                                       // float    minDepth
              1.0f,                                       // float    maxDepth
            };
            SetViewportStateDynamically(framesResources[frame_index].commandBuffer, 0, { viewport });
            
            VkRect2D scissor = 
            {
              {                                           // VkOffset2D     offset
                0,                                          // int32_t        x
                0                                           // int32_t        y
              },
              {                                           // VkExtent2D     extent
                swapchain.size.width,                       // uint32_t       width
                swapchain.size.height                       // uint32_t       height
              }
            };

            SetScissorStateDynamically(framesResources[frame_index].commandBuffer, 0, { scissor });
            
            BindVertexBuffers(framesResources[frame_index].commandBuffer, 0, { { vertexBuffer, 0 } });
            
            BindDescriptorSets(framesResources[frame_index].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSets, {});
            
            BindPipelineObject(framesResources[frame_index].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);
            
            for (size_t i = 0; i < model.parts.size(); ++i) 
            {
                DrawGeometry(framesResources[frame_index].commandBuffer, model.parts[i].vertexCount, 1, model.parts[i].vertexOffset, 0);
            }
            
            EndRenderPass(framesResources[frame_index].commandBuffer);
            
            if (presentQueue.familyIndex != graphicsQueue.familyIndex) 
            {
                ImageTransition image_transition_before_present = 
                {
                  swapchain.images[image_index],  // VkImage              Image
                  VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        CurrentAccess
                  VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        NewAccess
                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // VkImageLayout        CurrentLayout
                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // VkImageLayout        NewLayout
                  graphicsQueue.familyIndex,                // uint32_t             CurrentQueueFamily
                  presentQueue.familyIndex,                 // uint32_t             NewQueueFamily
                  VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
                };
                SetImageMemoryBarrier(framesResources[frame_index].commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { image_transition_before_present });
            }
            
            if (!EndCommandBufferRecordingOperation(framesResources[frame_index].commandBuffer)) 
            {
                return false;
            }
            
            std::vector<WaitSemaphoreInfo> wait_semaphore_infos = {};
            wait_semaphore_infos.push_back({
              framesResources[frame_index].imageAcquiredSemaphore,  // VkSemaphore            Semaphore
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT      ,  // VkPipelineStageFlags   WaitingStage
                });
            
            if (!SubmitCommandBuffersToQueue(graphicsQueue.handle, wait_semaphore_infos, { framesResources[frame_index].commandBuffer }, { framesResources[frame_index].readyToPresentSemaphore }, framesResources[frame_index].drawingFinishedFence))
            {
                return false;
            }
            
            PresentInfo present_info = 
            {
              swapchain.handle,             // VkSwapchainKHR         Swapchain
              image_index                   // uint32_t               ImageIndex
            };

            if (!PresentImage(presentQueue.handle, { framesResources[frame_index].readyToPresentSemaphore }, { present_info })) 
            {
                return false;
            }

            // Destroy the Frame buffer or else the program will run out of memory eventually!
            DestroyFramebuffer(logicalDevice, framesResources[frame_index].framebuffer);

            frame_index = (frame_index + 1) % framesResources.size();
        //}
    //}

    return true;
}

void Renderer::ShutdownRendererClass()
{
    // TODO: Handle memory
}


bool Renderer::ResizeWindow()
{
    WaitForAllSubmittedCommandsToBeFinished(logicalDevice);

    swapchain.ImageViewsRaw.clear();
    swapchain.imageViews.clear();
    swapchain.images.clear();
    bool skip = false;

    oldSwapchain = std::move(swapchain.handle);

    // Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
    VkPresentModeKHR desiredPresentMode;
    if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
        return false;

    // Get capabilities.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
        return false;

    uint32_t numberOfImages;
    if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
        return false;

    if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
        return false;

    if ((0 == swapchain.size.width) || (0 == swapchain.size.height))
        skip = true;

    if (!skip)
    {
        VkImageUsageFlags imageUsage;
        if (!SelectDesiredUsageScenariosOfSwapchainImages(surfaceCapabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, imageUsage))
            return false;


        VkSurfaceTransformFlagBitsKHR surfaceTransform;
        if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
            return false;


        VkColorSpaceKHR imageColorSpace;                                        /* VK_FORMAT_R8G8B8A8_UNORM */
        if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.format, imageColorSpace))
            return false;

        if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.format, imageColorSpace }, swapchain.size, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
            return false;


        if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
            return false;
    }

    for (size_t i = 0; i < swapchain.images.size(); ++i)
    {
        VkImageView* imageView = new VkImageView;
        swapchain.imageViews.emplace_back(imageView);
        if (!CreateImageView(logicalDevice, swapchain.images[i], VK_IMAGE_VIEW_TYPE_2D, swapchain.format, VK_IMAGE_ASPECT_COLOR_BIT, *swapchain.imageViews.back()))
        {
            return false;
        }
        swapchain.ImageViewsRaw.push_back(*swapchain.imageViews.back());
    }

    if (!UpdateUniformBuffer(swapchain, physicalDevice, logicalDevice, uniformBuffer, graphicsQueue, framesResources))
        return false;

    return true;
}

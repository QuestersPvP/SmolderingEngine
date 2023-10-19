
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Common/Common.h"
#include "Rendering/Instances/InstancesAndDevices.h"
#include "Rendering/Application/WindowCreation.h"
#include "Rendering/ImagePresentation/SwapChain.h"
#include "Rendering/BuffersAndPools/CommandBufferAndPool.h"
#include "Rendering/RenderPass/RenderPass.h"

using namespace SmolderingEngine;

int main()
{
    // TODO: Clean up main
    LIBRARY_TYPE vulkanLibrary;
    VkInstance instance;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    //uint32_t graphicsQueueFamilyIndex = 0;
    //uint32_t presentQueueFamilyIndex = 0;
    QueueParameters graphicsQueue;
    QueueParameters presentQueue;

    std::vector<VkPhysicalDevice> physicalDevices;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    std::vector<char const*> instanceExtensions;

    WindowParameters windowParams = GenerateApplicationWindowParameters();
    VkSurfaceKHR presentationSurface;

    //VkFormat swapchainImageFormat;
    //VkExtent2D swapchainImageSize;
    SwapchainParameters swapchain;
    swapchain.handle = VK_NULL_HANDLE;
    VkSwapchainKHR oldSwapchain = std::move(swapchain.handle);
    //std::vector<VkImage> swapchainImages;
    VkSemaphore imageAcquiredSemaphore = VK_NULL_HANDLE;
    VkSemaphore readyToPresentSemaphore = VK_NULL_HANDLE;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFence drawingFence = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer;
    VkFramebuffer framebuffer;
    std::vector<FrameResources> framesResources;
    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImagesMemory;

    // For testing
    bool setUp = true;

    // Connecting to vulkan-1.dll
    if (!ConnectWithVulkanLoaderLibrary(vulkanLibrary))
        setUp = false;

    // Loading macros
    if (!LoadFunctionExportedFromVulkanLoaderLibrary(vulkanLibrary))
        setUp = false;

    // Loading macros
    if (!LoadGlobalLevelFunctions()) 
        setUp = false;

    // Create vulkan instance
    if (!CreateVulkanInstance(instanceExtensions, "Smouldering Engine", instance))
        setUp = false;

    // loading macros based off of enabled extensions
    if (!LoadInstanceLevelFunctions(instance, instanceExtensions))
        setUp = false;

    // Create presentation surface
    if (!CreatePresentationSurface(instance, windowParams, presentationSurface))
        setUp = false;

    // Create logical device
    if (!EnumerateAvailablePhysicalDevices(instance, physicalDevices))
        setUp = false;

    //ChoosePhysicalAndLogicalDevices(physicalDevices, physicalDevice, logicalDevice, graphicsQueueFamilyIndex, presentQueueFamilyIndex, presentationSurface, graphicsQueue, presentQueue);
    for (auto& _physicalDevice : physicalDevices)
    {
        // Check for a device that supports graphics operations. 
        if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(_physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueue.familyIndex)) 
            continue;

        if (!SelectQueueFamilyThatSupportsPresentationToGivenSurface(_physicalDevice, presentationSurface, presentQueue.familyIndex)) 
            continue;
        
        std::vector<QueueInfo> requestedQueues = { { graphicsQueue.familyIndex, { 1.0f } } };
        if (graphicsQueue.familyIndex != presentQueue.familyIndex) 
        {
            requestedQueues.push_back({ presentQueue.familyIndex, { 1.0f } });
        }

        VkDevice _logicalDevice;
        std::vector<char const*> deviceExtensions;
        if (!CreateLogicalDevice(_physicalDevice, requestedQueues, deviceExtensions, nullptr, _logicalDevice))
        {
            continue;
        }
        else 
        {
            if (!LoadDeviceLevelFunctions(_logicalDevice, deviceExtensions))
            {
                continue;
            }
            physicalDevice = _physicalDevice;
            logicalDevice = std::move(_logicalDevice);
            GetDeviceQueue(logicalDevice, graphicsQueue.familyIndex, 0, graphicsQueue.handle);
            GetDeviceQueue(logicalDevice, presentQueue.familyIndex, 0, presentQueue.handle);
            break;
        }
    }

    if (!logicalDevice)
        setUp = false;

    // Create a semaphore using vkCreateSemaphore
    VkSemaphoreCreateInfo semaphoreCreateInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    if (!CreateCommandPool(logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueue.familyIndex, commandPool))
        setUp = false;

    for (uint32_t i = 0; i < 3; i++)
    {
        std::vector<VkCommandBuffer> _commandBuffer;
        VkSemaphore image_acquired_semaphore;
        VkSemaphore ready_to_present_semaphore;
        VkFence drawing_finished_fence;
        VkImageView depth_attachment;
        VkFramebuffer tempIdea;


        if (!AllocateCommandBuffers(logicalDevice, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, _commandBuffer)) {
            return false;
        }
        if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &image_acquired_semaphore) != VK_SUCCESS) {
            return false;
        }
        if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &ready_to_present_semaphore) != VK_SUCCESS) {
            return false;
        }
        if (!CreateFence(logicalDevice, true, drawing_finished_fence)) {
            return false;
        }

        framesResources.emplace_back(_commandBuffer[0], image_acquired_semaphore, ready_to_present_semaphore, drawing_finished_fence, depth_attachment, tempIdea);

        //framesResources.emplace_back(
        //    _commandBuffer[0],
        //    image_acquired_semaphore,
        //    ready_to_present_semaphore,
        //    drawing_finished_fence,
        //    depth_attachment,
        //    VkFramebuffer()
        //);
    }

#pragma region Swapchain Creation

    swapchain.ImageViewsRaw.clear();
    swapchain.imageViews.clear();
    swapchain.images.clear();

    // Choose presentation mode VK_PRESENT_MODE_MAILBOX_KHR is our preference to avoid screen tearing. 
    VkPresentModeKHR desiredPresentMode;
    if (!SelectDesiredPresentationMode(physicalDevice, presentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, desiredPresentMode))
        setUp = false;

    // Get capabilities.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (!GetCapabilitiesOfPresentationSurface(physicalDevice, presentationSurface, surfaceCapabilities))
        setUp = false;

    uint32_t numberOfImages;
    if (!SelectNumberOfSwapchainImages(surfaceCapabilities, numberOfImages))
        setUp = false;

    bool skip = false;
    if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchain.size))
        setUp = false;

    if ((0 == swapchain.size.width) || (0 == swapchain.size.height))
        skip = true;

    if (!skip)
    {
        VkImageUsageFlags imageUsage;
        if (!SelectDesiredUsageScenariosOfSwapchainImages(surfaceCapabilities, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, imageUsage))
            setUp = false;


        VkSurfaceTransformFlagBitsKHR surfaceTransform;
        if (!SelectTransformationOfSwapchainImagesInSwapChain(surfaceCapabilities, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, surfaceTransform))
            setUp = false;


        VkColorSpaceKHR imageColorSpace;
        if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchain.format, imageColorSpace))
            setUp = false;

        if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchain.format, imageColorSpace }, swapchain.size, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain.handle))
            setUp = false;


        if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain.handle, swapchain.images))
            setUp = false;
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
    
    // When we want to use depth buffering, we need to use a depth attachment
    // It must have the same size as the swapchain, so we need to recreate it along with the swapchain
    depthImages.clear();
    depthImagesMemory.clear();
    
    for (uint32_t i = 0; i < 3; ++i) 
    {
        depthImages.emplace_back(VkImage());
        depthImagesMemory.emplace_back(VkDeviceMemory());
    
        if (!Create2DImageAndView(physicalDevice, logicalDevice, VK_FORMAT_D16_UNORM, swapchain.size, 1, 1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, depthImages.back(), depthImagesMemory.back(),
            framesResources[i].depthAttachment)) 
        {
            return false;
        }
    }
#pragma endregion

    std::vector<VkCommandBuffer> commandBuffers;
    if (!AllocateCommandBuffers(logicalDevice, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, commandBuffers))
        setUp = false;
    
    commandBuffer = commandBuffers[0];
    
    // Drawing synchronization
    if (!CreateFence(logicalDevice, true, drawingFence))
        setUp = false;

    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAcquiredSemaphore) != VK_SUCCESS)
        setUp = false;

    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &readyToPresentSemaphore) != VK_SUCCESS)
        setUp = false;

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
        setUp = false;

    if (setUp)
    {
        // Show the window (assuming windows OS)
        ShowWindow(windowParams.HWnd, SW_SHOWNORMAL);
        UpdateWindow(windowParams.HWnd);

        MSG message;
        while (setUp)
        {
            if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
            {
                // TODO: add event handeling (e.g. x button clicked, resized, etc.)

                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                // Rendering

                if (!WaitForFences(logicalDevice, { drawingFence }, false, 5000000000))
                    setUp = false;

                if (!ResetFences(logicalDevice, { drawingFence }))
                    setUp = false;

                uint32_t imageIndex;
                if (!AcquireSwapchainImage(logicalDevice, swapchain.handle, imageAcquiredSemaphore, VK_NULL_HANDLE, imageIndex))
                    setUp = false;

                if (!CreateFramebuffer(logicalDevice, renderPass, { *swapchain.imageViews[imageIndex] }, swapchain.size.width, swapchain.size.height, 1, framebuffer))
                    setUp = false;

                if (!BeginCommandBufferRecordingOperation(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr))
                    setUp = false;

                if (presentQueue.familyIndex != graphicsQueue.familyIndex) 
                {
                    ImageTransition imageTransitionBeforeDrawing =
                    {
                      swapchain.images[imageIndex],                 // VkImage              Image
                      VK_ACCESS_MEMORY_READ_BIT,                    // VkAccessFlags        CurrentAccess
                      VK_ACCESS_MEMORY_READ_BIT,                    // VkAccessFlags        NewAccess
                      VK_IMAGE_LAYOUT_UNDEFINED,                    // VkImageLayout        CurrentLayout
                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,     // VkImageLayout        NewLayout
                      presentQueue.familyIndex,                     // uint32_t             CurrentQueueFamily
                      graphicsQueue.familyIndex,                    // uint32_t             NewQueueFamily
                      VK_IMAGE_ASPECT_COLOR_BIT                     // VkImageAspectFlags   Aspect
                    };
                    SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { imageTransitionBeforeDrawing });
                }

                BeginRenderPass(commandBuffer, renderPass, framebuffer, { { 0, 0 }, swapchain.size }, { { 0.2f, 0.5f, 0.8f, 1.0f } }, VK_SUBPASS_CONTENTS_INLINE);

                EndRenderPass(commandBuffer);

                if (presentQueue.familyIndex != graphicsQueue.familyIndex)
                {
                    ImageTransition imageTransitionBeforePresent = 
                    {
                      swapchain.images[imageIndex],                 // VkImage              Image
                      VK_ACCESS_MEMORY_READ_BIT,                    // VkAccessFlags        CurrentAccess
                      VK_ACCESS_MEMORY_READ_BIT,                    // VkAccessFlags        NewAccess
                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,              // VkImageLayout        CurrentLayout
                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,              // VkImageLayout        NewLayout
                      graphicsQueue.familyIndex,                    // uint32_t             CurrentQueueFamily
                      presentQueue.familyIndex,                     // uint32_t             NewQueueFamily
                      VK_IMAGE_ASPECT_COLOR_BIT                     // VkImageAspectFlags   Aspect
                    };
                    SetImageMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, { imageTransitionBeforePresent });
                }

                if (!EndCommandBufferRecordingOperation(commandBuffer))
                    return false;

                WaitSemaphoreInfo waitSemaphoreInfo = 
                {
                  imageAcquiredSemaphore,               // VkSemaphore            Semaphore
                  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT    // VkPipelineStageFlags   WaitingStage
                };

                if (!SubmitCommandBuffersToQueue(graphicsQueue.handle, { waitSemaphoreInfo }, { commandBuffer }, { readyToPresentSemaphore }, drawingFence))
                    return false;

                PresentInfo presentInfo = 
                {
                  swapchain.handle,     // VkSwapchainKHR   Swapchain
                  imageIndex           // uint32_t         ImageIndex
                };

                if (!PresentImage(presentQueue.handle, { readyToPresentSemaphore }, { presentInfo }))
                    setUp = false;
            }
        }
    }

    return 0;
}

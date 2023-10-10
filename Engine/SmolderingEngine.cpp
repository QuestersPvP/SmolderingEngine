
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Common/Common.h"
#include "Rendering/Instances/InstancesAndDevices.h"
#include "Rendering/Application/WindowCreation.h"
#include "Rendering/ImagePresentation/SwapChain.h"

using namespace SmolderingEngine;

int main()
{
    // TODO: Clean up main
    LIBRARY_TYPE vulkanLibrary;
    VkInstance instance;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    uint32_t graphicsQueueFamilyIndex = 0;
    uint32_t presentQueueFamilyIndex = 0;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    std::vector<VkPhysicalDevice> physicalDevices;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    std::vector<char const*> instanceExtensions;

    WindowParameters windowParams = GenerateApplicationWindowParameters();
    VkSurfaceKHR presentationSurface;

    VkFormat swapchainImageFormat;
    VkExtent2D swapchainImageSize;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSwapchainKHR oldSwapchain = std::move(swapchain);
    std::vector<VkImage> swapchainImages;
    VkSemaphore imageAcquiredSemaphore = VK_NULL_HANDLE;
    VkSemaphore readyToPresentSemaphore = VK_NULL_HANDLE;

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
        if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(_physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex)) {
            continue;
        }

        if (!SelectQueueFamilyThatSupportsPresentationToGivenSurface(_physicalDevice, presentationSurface, presentQueueFamilyIndex)) {
            continue;
        }

        std::vector<QueueInfo> requestedQueues = { { graphicsQueueFamilyIndex, { 1.0f } } };
        if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) 
        {
            requestedQueues.push_back({ presentQueueFamilyIndex, { 1.0f } });
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
            GetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, graphicsQueue);
            GetDeviceQueue(logicalDevice, presentQueueFamilyIndex, 0, presentQueue);
            break;
        }
    }

    if (!logicalDevice)
        setUp = false;

#pragma region Swapchain Creation
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
    if (!ChooseSizeOfSwapchainImages(surfaceCapabilities, swapchainImageSize))
        setUp = false;

    if ((0 == swapchainImageSize.width) || (0 == swapchainImageSize.height))
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
        if (!SelectFormatOfSwapchainImages(physicalDevice, presentationSurface, { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, swapchainImageFormat, imageColorSpace))
            setUp = false;

        if (!CreateSwapchain(logicalDevice, presentationSurface, numberOfImages, { swapchainImageFormat, imageColorSpace }, swapchainImageSize, imageUsage, surfaceTransform, desiredPresentMode, oldSwapchain, swapchain))
            setUp = false;


        if (!GetHandlesOfSwapchainImages(logicalDevice, swapchain, swapchainImages))
            setUp = false;
    }

    // Create a semaphore using vkCreateSemaphore
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAcquiredSemaphore) != VK_SUCCESS)
        setUp = false;

    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &readyToPresentSemaphore) != VK_SUCCESS)
        setUp = false;
#pragma endregion

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

                uint32_t image_index;
                if (!AcquireSwapchainImage(logicalDevice, swapchain, imageAcquiredSemaphore, VK_NULL_HANDLE, image_index)) 
                    setUp = false;
                

                PresentInfo present_info = 
                {
                  swapchain,   // VkSwapchainKHR   Swapchain
                  image_index   // uint32_t         ImageIndex
                };

                if (!PresentImage(presentQueue, { readyToPresentSemaphore }, { present_info })) 
                    setUp = false;
                

                setUp = true;
            }
        }
    }

    return 0;
}

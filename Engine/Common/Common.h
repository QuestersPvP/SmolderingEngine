#pragma once

// TODO: Make file to hold #defines
// Vulkan library type for Windows
#define LIBRARY_TYPE HMODULE
// Wide character string to help load vulkan-1.dll
#define VULKAN_DLL_NAME "vulkan-1.dll"
#define WINDOW_NAME "Smoldering Engine"
#define WINDOW_TITLE "Smoldering Engine"

// OS include
#include <Windows.h>

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <thread>
#include <cmath>
#include <functional>
#include <memory>

#include "../Common/VulkanFunctions.h"

// TODO: Make file to store all headers in one location
//#include "../Rendering/Instances/InstancesAndDevices.h"

namespace SmolderingEngine
{
    // TODO: make a place to store all structs
    // Struct for Windows platform application windows
    struct WindowParameters
    {
        HINSTANCE           HInstance;
        HWND                HWnd;
    };

    // Defines a swapchain from which we want to present an image.
    struct PresentInfo 
    {
        VkSwapchainKHR      swapchain;
        uint32_t            imageIndex;
    };

    // Provide a semaphone that the hardware should wait for and at which pipeline stage we should wait.
    struct WaitSemaphoreInfo
    {
        VkSemaphore           Semaphore;
        VkPipelineStageFlags  WaitingStage;
    };

    struct SwapchainParameters 
    {
        VkSwapchainKHR              handle;
        VkFormat                    format;
        VkExtent2D                  size;
        std::vector<VkImage>        images;
        std::vector<VkImageView>    imageViews;
    };

    struct SubpassParameters 
    {
        VkPipelineBindPoint                     pipelineType;
        std::vector<VkAttachmentReference>      inputAttachments;
        std::vector<VkAttachmentReference>      colorAttachments;
        std::vector<VkAttachmentReference>      resolveAttachments;
        VkAttachmentReference const*            depthStencilAttachment;
        std::vector<uint32_t>                   preserveAttachments;
    };
    
    // Store information about queues we want to request for a logical device
    // Stores a family index that we want the queues to be created with and
    // The total number of queues requested from this family w/ the list of 
    // Priorities assigned to them.
    struct QueueInfo
    {
        uint32_t           FamilyIndex;
        std::vector<float> Priorities;
    };

    // Extension availability check
    bool IsExtensionSupported(std::vector<VkExtensionProperties> const& _availableExtensions, char const* const _extension);
};

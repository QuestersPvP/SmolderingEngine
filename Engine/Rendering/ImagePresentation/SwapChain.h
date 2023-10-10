#pragma once

#include "../../Common/Common.h"

namespace SmolderingEngine
{
    bool SelectDesiredPresentationMode(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface,VkPresentModeKHR _desiredPresentMode,VkPresentModeKHR& _presentMode);
    bool GetCapabilitiesOfPresentationSurface(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, VkSurfaceCapabilitiesKHR& _surfaceCapabilities);
    bool SelectNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, uint32_t& _numberOfImages);
    bool ChooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, VkExtent2D& _sizeOfImages);
    bool SelectDesiredUsageScenariosOfSwapchainImages(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, VkImageUsageFlags _desiredUsages, VkImageUsageFlags& _imageUsage);
    bool SelectTransformationOfSwapchainImagesInSwapChain(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, VkSurfaceTransformFlagBitsKHR _desiredTransform, VkSurfaceTransformFlagBitsKHR& _surfaceTransform);
    bool SelectFormatOfSwapchainImages(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, VkSurfaceFormatKHR _desiredSurfaceFormat, VkFormat& _imageFormat,VkColorSpaceKHR& _imageColorSpace);
    bool CreateSwapchain(VkDevice _logicalDevice, VkSurfaceKHR _presentationSurface, uint32_t _imageCount, VkSurfaceFormatKHR _surfaceFormat, VkExtent2D _imageSize, VkImageUsageFlags _imageUsage,
        VkSurfaceTransformFlagBitsKHR _surfaceTransform, VkPresentModeKHR _presentMode, VkSwapchainKHR& _oldSwapchain, VkSwapchainKHR& _swapchain);
    bool GetHandlesOfSwapchainImages(VkDevice _logicalDevice, VkSwapchainKHR _swapchain, std::vector<VkImage>& _swapchainImages);
    bool AcquireSwapchainImage(VkDevice _logicalDevice, VkSwapchainKHR _swapchain, VkSemaphore _semaphore, VkFence _fence, uint32_t& _imageIndex);
    bool PresentImage(VkQueue _queue, std::vector<VkSemaphore> _renderingSemaphores, std::vector<PresentInfo> _imagesToPresent);
};
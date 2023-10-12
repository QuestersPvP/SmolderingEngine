#include "SwapChain.h"

namespace SmolderingEngine
{
	bool SelectDesiredPresentationMode(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, VkPresentModeKHR _desiredPresentMode, VkPresentModeKHR& _presentMode)
	{
        // Enumerate supported present modes on the current platform. vkGetPhysicalDeviceSurfacePresentModesKHR gives us the number of presentation modes.
        uint32_t presentModesCount = 0;
        if ((vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _presentationSurface, &presentModesCount, nullptr) != VK_SUCCESS ) || (0 == presentModesCount))
        {
            std::cout << "Could not get the number of supported present modes." << std::endl;
            return false;
        }

        // Call vkGetPhysicalDeviceSurfacePresentModesKHR again with a vector that has been prepared to hold them all.
        std::vector<VkPresentModeKHR> presentModes(presentModesCount);
        if ((vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _presentationSurface, &presentModesCount, presentModes.data()) != VK_SUCCESS) || (0 == presentModesCount))
        {
            std::cout << "Could not enumerate present modes." << std::endl;
            return false;
        }

        // Select present mode that best fits us.
        for (auto& currentPresentMode : presentModes) 
        {
            if (currentPresentMode == _desiredPresentMode)
            {
                _presentMode = _desiredPresentMode;
                return true;
            }
        }

        std::cout << "Desired present mode is not supported. Selecting default FIFO mode." << std::endl;
        for (auto& currentPresentMode : presentModes) 
        {
            if (currentPresentMode == VK_PRESENT_MODE_FIFO_KHR) 
            {
                _presentMode = VK_PRESENT_MODE_FIFO_KHR;
                return true;
            }
        }

        // Your driver sux.
        std::cout << "VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory for all drivers!" << std::endl;
        return false;
	}

    bool GetCapabilitiesOfPresentationSurface(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, VkSurfaceCapabilitiesKHR& _surfaceCapabilities)
    {
        // Call vkGetPhysicalDeviceSurfaceCapabilitiesKHR and it will store the infoz into _surfaceCapabilities.
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _presentationSurface, &_surfaceCapabilities) != VK_SUCCESS)
        {
            std::cout << "Could not get the capabilities of a presentation surface." << std::endl;
            return false;
        }

        return true;
    }

    bool SelectNumberOfSwapchainImages(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, uint32_t& _numberOfImages)
    {
        // Select 1 more image than the minImageCount. We do not want to use too much memory storing images.
        _numberOfImages = _surfaceCapabilities.minImageCount + 1;
        if ((_surfaceCapabilities.maxImageCount > 0) && (_numberOfImages > _surfaceCapabilities.maxImageCount))
        {
            _numberOfImages = _surfaceCapabilities.maxImageCount;
        }
        return true;
    }

    bool ChooseSizeOfSwapchainImages(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, VkExtent2D& _sizeOfImages)
    {
        // 0xFFFFFFFF tells us the windows size is determined by the size of the swap chain images.
        if (0xFFFFFFFF == _surfaceCapabilities.currentExtent.width)
        {
            // Basically all we do here is resize the swap chain to fit.
            _sizeOfImages = { 640, 480 };

            if (_sizeOfImages.width < _surfaceCapabilities.minImageExtent.width) 
            {
                _sizeOfImages.width = _surfaceCapabilities.minImageExtent.width;
            }
            else if (_sizeOfImages.width > _surfaceCapabilities.maxImageExtent.width) 
            {
                _sizeOfImages.width = _surfaceCapabilities.maxImageExtent.width;
            }

            if (_sizeOfImages.height < _surfaceCapabilities.minImageExtent.height) 
            {
                _sizeOfImages.height = _surfaceCapabilities.minImageExtent.height;
            }
            else if (_sizeOfImages.height > _surfaceCapabilities.maxImageExtent.height) 
            {
                _sizeOfImages.height = _surfaceCapabilities.maxImageExtent.height;
            }
        }
        else 
        {
            _sizeOfImages = _surfaceCapabilities.currentExtent;
        }

        return true;
    }

    bool SelectDesiredUsageScenariosOfSwapchainImages(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, VkImageUsageFlags _desiredUsages, VkImageUsageFlags& _imageUsage)
    {
        // Check what usages are supported using bits.
        _imageUsage = _desiredUsages & _surfaceCapabilities.supportedUsageFlags;

        return _desiredUsages == _imageUsage;
    }

    bool SelectTransformationOfSwapchainImagesInSwapChain(VkSurfaceCapabilitiesKHR const& _surfaceCapabilities, VkSurfaceTransformFlagBitsKHR _desiredTransform, VkSurfaceTransformFlagBitsKHR& _surfaceTransform)
    {
        // Here all we are doing is checking what transformations are available on a platform and selecting our desired or current transform.
        if (_surfaceCapabilities.supportedTransforms & _desiredTransform)
        {
            _surfaceTransform = _desiredTransform;
        }
        else 
        {
            _surfaceTransform = _surfaceCapabilities.currentTransform;
        }

        return true;
    }

    bool SelectFormatOfSwapchainImages(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, VkSurfaceFormatKHR _desiredSurfaceFormat, VkFormat& _imageFormat, VkColorSpaceKHR& _imageColorSpace)
    {
        // Enumerate supported formats
        uint32_t formatsCount = 0;

        // Get the number of supported format-color space pairs.
        if ((vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _presentationSurface, &formatsCount, nullptr) != VK_SUCCESS) || (0 == formatsCount))
        {
            std::cout << "Could not get the number of supported surface formats." << std::endl;
            return false;
        }

        // Populate a vector with the supported format-color space pairs.
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
        if ((vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _presentationSurface, &formatsCount, surfaceFormats.data()) != VK_SUCCESS) ||(0 == formatsCount))
        {
            std::cout << "Could not enumerate supported surface formats." << std::endl;
            return false;
        }

        // This check lets us know there are no restrictions on formatting, so we will just chool our ideal formats.
        if ((1 == surfaceFormats.size()) && (VK_FORMAT_UNDEFINED == surfaceFormats[0].format))
        {
            _imageFormat = _desiredSurfaceFormat.format;
            _imageColorSpace = _desiredSurfaceFormat.colorSpace;
            return true;
        }

        // If there are multiple we check if the formats are both supported fully.
        for (auto& surfaceFormat : surfaceFormats) 
        {
            if ((_desiredSurfaceFormat.format == surfaceFormat.format) && (_desiredSurfaceFormat.colorSpace == surfaceFormat.colorSpace))
            {
                _imageFormat = _desiredSurfaceFormat.format;
                _imageColorSpace = _desiredSurfaceFormat.colorSpace;
                return true;
            }
        }

        // Look for a member that has the same image format but a different color space.
        for (auto& surfaceFormat : surfaceFormats) 
        {
            if ((_desiredSurfaceFormat.format == surfaceFormat.format))
            {
                _imageFormat = _desiredSurfaceFormat.format;
                _imageColorSpace = surfaceFormat.colorSpace;
                std::cout << "Desired combination of format and colorspace is not supported. Selecting other colorspace." << std::endl;
                return true;
            }
        }

        // If all else fails just pick the first image format-color space pair.
        _imageFormat = surfaceFormats[0].format;
        _imageColorSpace = surfaceFormats[0].colorSpace;
        std::cout << "Desired format is not supported. Selecting available format - colorspace combination." << std::endl;
        return true;
    }

    bool CreateSwapchain(VkDevice _logicalDevice, VkSurfaceKHR _presentationSurface, uint32_t _imageCount, VkSurfaceFormatKHR _surfaceFormat, VkExtent2D _imageSize, VkImageUsageFlags _imageUsage,
        VkSurfaceTransformFlagBitsKHR _surfaceTransform, VkPresentModeKHR _presentMode, VkSwapchainKHR& _oldSwapchain, VkSwapchainKHR& _swapchain)
    {
        // Information needed to create the swapchain.
        VkSwapchainCreateInfoKHR swapchainCreateInfo = 
        {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                  sType
            nullptr,                                      // const void                     * pNext
            0,                                            // VkSwapchainCreateFlagsKHR        flags
            _presentationSurface,                         // VkSurfaceKHR                     surface
            _imageCount,                                  // uint32_t                         minImageCount
            _surfaceFormat.format,                        // VkFormat                         imageFormat
            _surfaceFormat.colorSpace,                    // VkColorSpaceKHR                  imageColorSpace
            _imageSize,                                   // VkExtent2D                       imageExtent
            1,                                            // uint32_t                         imageArrayLayers
            _imageUsage,                                  // VkImageUsageFlags                imageUsage
            VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                    imageSharingMode
            0,                                            // uint32_t                         queueFamilyIndexCount
            nullptr,                                      // const uint32_t                 * pQueueFamilyIndices
            _surfaceTransform,                            // VkSurfaceTransformFlagBitsKHR    preTransform
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR      compositeAlpha
            _presentMode,                                 // VkPresentModeKHR                 presentMode
            VK_TRUE,                                      // VkBool32                         clipped
            _oldSwapchain                                 // VkSwapchainKHR                   oldSwapchain
        };

        // Create the new swap chain.
        if ((vkCreateSwapchainKHR(_logicalDevice, &swapchainCreateInfo, nullptr, &_swapchain) != VK_SUCCESS) || (VK_NULL_HANDLE == _swapchain))
        {
            std::cout << "Could not create a swapchain." << std::endl;
            return false;
        }

        // If the old swapchain is not null delete it, we can only have one.
        if (_oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(_logicalDevice, _oldSwapchain, nullptr);
            _oldSwapchain = VK_NULL_HANDLE;
        }

        return true;
    }

    bool GetHandlesOfSwapchainImages(VkDevice _logicalDevice, VkSwapchainKHR _swapchain, std::vector<VkImage>& _swapchainImages)
    {
        // Get the total number of swap chain images. we need to do this because we specefied the MINIMUM amount of swapchain images,
        // We may recieve more than the MINIMUM number.
        uint32_t imagesCount = 0;
        if ((vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &imagesCount, nullptr) != VK_SUCCESS) || (0 == imagesCount)) 
        {
            std::cout << "Could not get the number of swapchain images." << std::endl;
            return false;
        }

        // Store all the images into the vector of images.
        _swapchainImages.resize(imagesCount);
        if ((vkGetSwapchainImagesKHR(_logicalDevice, _swapchain, &imagesCount, _swapchainImages.data()) != VK_SUCCESS) || (0 == imagesCount)) 
        {
            std::cout << "Could not enumerate swapchain images." << std::endl;
            return false;
        }

        return true;
    }

    bool AcquireSwapchainImage(VkDevice _logicalDevice, VkSwapchainKHR _swapchain, VkSemaphore _semaphore, VkFence _fence, uint32_t& _imageIndex)
    {
        // 2000000000 is the wait time in nanoseconds (e.g. 2 second limit) for the next image to be aquired.
        // This function just returns an index to the array of swapchain images. 
        VkResult result = vkAcquireNextImageKHR(_logicalDevice, _swapchain, 2000000000, _semaphore, _fence, &_imageIndex);
        switch (result) 
        {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            return true;
        default:
            return false;
        }
    }

    bool PresentImage(VkQueue _queue, std::vector<VkSemaphore> _renderingSemaphores, std::vector<PresentInfo> _imagesToPresent)
    {
        std::vector<VkSwapchainKHR> swapchains;
        std::vector<uint32_t> imageIndices;

        for (auto& imageToPresent : _imagesToPresent) 
        {
            swapchains.emplace_back(imageToPresent.swapchain);
            imageIndices.emplace_back(imageToPresent.imageIndex);
        }

        VkPresentInfoKHR presentInfo = 
        {
          VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                   // VkStructureType          sType
          nullptr,                                              // const void*              pNext
          static_cast<uint32_t>(_renderingSemaphores.size()),   // uint32_t                 waitSemaphoreCount
          _renderingSemaphores.data(),                          // const VkSemaphore      * pWaitSemaphores
          static_cast<uint32_t>(swapchains.size()),             // uint32_t                 swapchainCount
          swapchains.data(),                                    // const VkSwapchainKHR   * pSwapchains
          imageIndices.data(),                                  // const uint32_t         * pImageIndices
          nullptr                                               // VkResult*                pResults
        };

        VkResult result = vkQueuePresentKHR(_queue, &presentInfo);
        switch (result) 
        {
        case VK_SUCCESS:
            return true;
        default:
            return false;
        }
    }

    void DestroySwapchain(VkDevice _logicalDevice, VkSwapchainKHR& _swapchain)
    {
        if (_swapchain)
        {
            vkDestroySwapchainKHR(_logicalDevice, _swapchain, nullptr);
            _swapchain = VK_NULL_HANDLE;
        }
    }
};
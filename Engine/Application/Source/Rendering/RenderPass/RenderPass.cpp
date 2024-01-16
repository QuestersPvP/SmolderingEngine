#include "RenderPass.h"

namespace SE_Renderer
{
    bool CreatePhysicalAndLogicalDevice(std::vector<std::string>& _supportedInstanceExtensions, VkInstance& _vulkanInstance, VkPhysicalDevice& _physicalDevice,
                                        VkPhysicalDeviceProperties& _deviceProperties, VkPhysicalDeviceFeatures& _deviceFeatures, VkPhysicalDeviceMemoryProperties& _memoryProperties,
                                        VkPhysicalDeviceFeatures& _enabledFeatures, std::vector<VkQueueFamilyProperties>& _queueFamilyProperties, std::vector<std::string>& _supportedExtensions,
                                        VkQueueFlags& _requestedQueueTypes, QueueFamilyIndices& _queueFamilyIndices, VkDevice& _logicalDevice)
    {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Smoldering Engine"; 
        appInfo.pEngineName = "Smoldering Engine";
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
                    _supportedInstanceExtensions.push_back(_extension.extensionName);
                }
            }
        }

        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = NULL;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        if (instanceExtensions.size() > 0)
        {
            instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
            instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
        }

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &_vulkanInstance) != VK_SUCCESS)
        {
            std::cout << "Error creating vulkan instance" << std::endl;
            return false;
        }

        // Physical device
        uint32_t gpuCount = 0;

        // Get number of available physical devices
        if (vkEnumeratePhysicalDevices(_vulkanInstance, &gpuCount, nullptr) != VK_SUCCESS)
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
        if (vkEnumeratePhysicalDevices(_vulkanInstance, &gpuCount, physicalDevices.data()) != VK_SUCCESS)
        {
            std::cout << "Error enumerating physical devices" << std::endl;
            return false;
        }

        // GPU selection
        // TODO: RE-WORK DEVICE SELECTION
        uint32_t selectedDevice = 0;

        _physicalDevice = physicalDevices[selectedDevice];
        assert(_physicalDevice);
        // Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
        vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_deviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_memoryProperties);

        // Fill mode non solid is required for wireframe display
        if (_deviceFeatures.fillModeNonSolid)
        {
            _enabledFeatures.fillModeNonSolid = VK_TRUE;
        };

        // Wide lines must be present for line width > 1.0f
        if (_deviceFeatures.wideLines)
        {
            _enabledFeatures.wideLines = VK_TRUE;
        }


        // Queue family properties, used for setting up requested queues upon device creation
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
        assert(queueFamilyCount > 0);
        _queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, _queueFamilyProperties.data());

        // Get list of supported extensions
        extCount = 0;
        vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
        if (extCount > 0)
        {
            std::vector<VkExtensionProperties> extensions(extCount);
            if (vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
            {
                for (auto _ext : extensions)
                {
                    _supportedExtensions.push_back(_ext.extensionName);
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
        if (_requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
        {
            _queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, _queueFamilyProperties);
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = _queueFamilyIndices.graphics;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
        else
        {
            _queueFamilyIndices.graphics = 0;
        }

        // Dedicated compute queue
        if (_requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
        {
            _queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, _queueFamilyProperties);
            if (_queueFamilyIndices.compute != _queueFamilyIndices.graphics)
            {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = _queueFamilyIndices.compute;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else
        {
            // Else we use the same queue
            _queueFamilyIndices.compute = _queueFamilyIndices.graphics;
        }

        // Dedicated transfer queue
        if (_requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
        {
            _queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, _queueFamilyProperties);
            if ((_queueFamilyIndices.transfer != _queueFamilyIndices.graphics) && (_queueFamilyIndices.transfer != _queueFamilyIndices.compute))
            {
                // If transfer family index differs, we need an additional queue create info for the transfer queue
                VkDeviceQueueCreateInfo queueInfo{};
                queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfo.queueFamilyIndex = _queueFamilyIndices.transfer;
                queueInfo.queueCount = 1;
                queueInfo.pQueuePriorities = &defaultQueuePriority;
                queueCreateInfos.push_back(queueInfo);
            }
        }
        else
        {
            // Else we use the same queue
            _queueFamilyIndices.transfer = _queueFamilyIndices.graphics;
        }

        // Create the logical device representation
        std::vector<const char*> deviceExtensions;
        // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &_enabledFeatures;

        if (deviceExtensions.size() > 0)
        {
            for (const char* enabledExtension : deviceExtensions)
            {
                if (!(std::find(_supportedExtensions.begin(), _supportedExtensions.end(), enabledExtension) != _supportedExtensions.end()))
                {
                    std::cout << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
                }
            }

            deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        }

        if (vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice) != VK_SUCCESS)
        {
            std::cout << "Error creating logical device!" << std::endl;
            return false;
        }

        return true;
    }

    bool CreateSwapchain(VkSwapchainKHR& _swapChain, VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, uint32_t _width, uint32_t _height, uint32_t& _imageCount,
                         std::vector<VkImage>& _swapchainImages, std::vector<SwapChainBuffer>& _swapchainBuffers, VkDevice _logicalDevice, VkFormat _colorFormat, VkColorSpaceKHR _colorSpace)
    {
        // TODO: FIGURE OUT WHY THIS FUNCTION DOES NOT WORK??

        // Store the current swap chain handle so we can use it later on to ease up recreation
        VkSwapchainKHR oldSwapchain = _swapChain;

        // Get physical device surface properties and formats
        VkSurfaceCapabilitiesKHR surfCaps;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _presentationSurface, &surfCaps))
        {
            std::cout << "Error obtaining surface capabilities" << std::endl;
            return false;
        }

        // Get available present modes
        uint32_t presentModeCount;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _presentationSurface, &presentModeCount, NULL))
        {
            std::cout << "Error getting presentation modes" << std::endl;
            return false;
        }
        assert(presentModeCount > 0);

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _presentationSurface, &presentModeCount, presentModes.data()))
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
            swapchainExtent.width = _width;
            swapchainExtent.height = _height;
        }
        else
        {
            // If the surface size is defined, the swap chain size must match
            swapchainExtent = surfCaps.currentExtent;
            _width = surfCaps.currentExtent.width;
            _height = surfCaps.currentExtent.height;
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
        swapchainCI.surface = _presentationSurface;
        swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
        swapchainCI.imageFormat = _colorFormat;
        swapchainCI.imageColorSpace = _colorSpace;
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

        if (vkCreateSwapchainKHR(_logicalDevice, &swapchainCI, nullptr, &_swapChain) != VK_SUCCESS)
        {
            std::cout << "Error creating swapchain" << std::endl;
            return false;
        }

        // If an existing swap chain is re-created, destroy the old swap chain
        // This also cleans up all the presentable images
        if (oldSwapchain != VK_NULL_HANDLE)
        {
            for (uint32_t i = 0; i < _imageCount; i++)
            {
                vkDestroyImageView(_logicalDevice, _swapchainBuffers[i].view, nullptr);
            }
            vkDestroySwapchainKHR(_logicalDevice, oldSwapchain, nullptr);
        }
        if (vkGetSwapchainImagesKHR(_logicalDevice, _swapChain, &_imageCount, NULL))
        {
            std::cout << "Error getting swapchain images" << std::endl;
            return false;
        }

        // Get the swap chain images
        _swapchainImages.resize(_imageCount);
        if (vkGetSwapchainImagesKHR(_logicalDevice, _swapChain, &_imageCount, _swapchainImages.data()))
        {
            std::cout << "Error getting swapchain images" << std::endl;
            return false;
        }

        // Get the swap chain buffers containing the image and imageview
        _swapchainBuffers.resize(_imageCount);
        for (uint32_t i = 0; i < _imageCount; i++)
        {
            VkImageViewCreateInfo colorAttachmentView = {};
            colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            colorAttachmentView.pNext = NULL;
            colorAttachmentView.format = _colorFormat;
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

            _swapchainBuffers[i].image = _swapchainImages[i];

            colorAttachmentView.image = _swapchainBuffers[i].image;

            if (vkCreateImageView(_logicalDevice, &colorAttachmentView, nullptr, &_swapchainBuffers[i].view))
            {
                std::cout << "Error creating image views" << std::endl;
                return false;
            }
        }

        return true;
    }

    bool CreateDepthStencil(uint32_t _width, uint32_t _height, VkDevice _logicalDevice, DepthStencil* _depthStencil, VkPhysicalDeviceMemoryProperties* _memoryProperties, VkFormat* _depthFormat)
    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        imageCreateInfo.extent = { _width, _height, 1 };
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (vkCreateImage(_logicalDevice, &imageCreateInfo, nullptr, &_depthStencil->image) != VK_SUCCESS)
        {
            std::cout << "Could not create an image." << std::endl;
            return false;
        }

        VkMemoryRequirements memReqs{};
        vkGetImageMemoryRequirements(_logicalDevice, _depthStencil->image, &memReqs);

        VkMemoryAllocateInfo memAllloc{};
        memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllloc.allocationSize = memReqs.size;
        memAllloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *_memoryProperties);

        if (vkAllocateMemory(_logicalDevice, &memAllloc, nullptr, &_depthStencil->mem) != VK_SUCCESS)
        {
            std::cout << "Could not allocate memory!" << std::endl;
            return false;
        }
        if (vkBindImageMemory(_logicalDevice, _depthStencil->image, _depthStencil->mem, 0) != VK_SUCCESS)
        {
            std::cout << "Could not bind memory object to an image." << std::endl;
            return false;
        }

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.image = _depthStencil->image;
        imageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT; // DEPTH FORMAT
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
        if (*_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
            imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        if (vkCreateImageView(_logicalDevice, &imageViewCreateInfo, nullptr, &_depthStencil->view) != VK_SUCCESS)
        {
            std::cout << "Could not create an image view." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateSynchronizationPrimitives(std::vector<VkFence>& _waitFences, std::vector<VkCommandBuffer>& _drawCmdBuffers, VkDevice _logicalDevice)
    {
        // Wait fences to sync command buffer access
        VkFenceCreateInfo fenceCreateInfo = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
        _waitFences.resize(_drawCmdBuffers.size());
        for (auto& fence : _waitFences)
        {
            if (vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
            {
                std::cout << "Error creating a fence" << std::endl;
                return false;
            }
        }

        return true;
    }

    bool CreateFrameBuffer()
    {
        return false;
    }

    bool CreateCommandBuffers(std::vector<VkCommandBuffer>& _drawCmdBuffers, VkCommandPool _commandBufferCommandPool, uint32_t _imageCount, VkDevice _logicalDevice)
    {
        _drawCmdBuffers.resize(_imageCount);

        VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            commandBufferAllocateInfo(
                _commandBufferCommandPool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                static_cast<uint32_t>(_drawCmdBuffers.size()));

        if (vkAllocateCommandBuffers(_logicalDevice, &cmdBufAllocateInfo, _drawCmdBuffers.data()) != VK_SUCCESS)
        {
            std::cout << "Error creating command buffers" << std::endl;
            return false;
        }

        return true;
    }

    bool BuildCommandBuffers()
    {
        return false;
    }

    void DestroyCommandBuffers(VkDevice _logicalDevice, VkCommandPool _commandBufferCommandPool, std::vector<VkCommandBuffer>& _drawCmdBuffers)
    {
        vkFreeCommandBuffers(_logicalDevice, _commandBufferCommandPool, static_cast<uint32_t>(_drawCmdBuffers.size()), _drawCmdBuffers.data());
    }

    VkBool32 GetSupportedDepthFormat(VkPhysicalDevice _physicalDevice, VkFormat* _depthFormat)
    {
        // Since all depth formats may be optional, we need to find a suitable depth format to use
        // Start with the highest precision packed format
        std::vector<VkFormat> formatList = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };
    
        for (auto& format : formatList)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &formatProps);
            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                *_depthFormat = format;
                return true;
            }
        }
    
        return false;
    }
    
    uint32_t GetQueueFamilyIndex(VkQueueFlags _queueFlags, std::vector<VkQueueFamilyProperties>& _queueFamilyProperties)
    {
        // Dedicated queue for compute
        // Try to find a queue family index that supports compute but not graphics
        if ((_queueFlags & VK_QUEUE_COMPUTE_BIT) == _queueFlags)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
            {
                if ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                {
                    return i;
                }
            }
        }
    
        // Dedicated queue for transfer
        // Try to find a queue family index that supports transfer but not graphics and compute
        if ((_queueFlags & VK_QUEUE_TRANSFER_BIT) == _queueFlags)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
            {
                if ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
                {
                    return i;
                }
            }
        }
    
        // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
        for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++)
        {
            if ((_queueFamilyProperties[i].queueFlags & _queueFlags) == _queueFlags)
            {
                return i;
            }
        }
    
        throw std::runtime_error("Could not find a matching queue family index");
    }
    
    uint32_t GetMemoryType(uint32_t _typeBits, VkMemoryPropertyFlags _properties, VkPhysicalDeviceMemoryProperties& _memoryProperties, VkBool32* _memTypeFound)
    {
        for (uint32_t i = 0; i < _memoryProperties.memoryTypeCount; i++)
        {
            if ((_typeBits & 1) == 1)
            {
                if ((_memoryProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
                {
                    if (_memTypeFound)
                    {
                        *_memTypeFound = true;
                    }
                    return i;
                }
            }
            _typeBits >>= 1;
        }

        if (_memTypeFound)
        {
            *_memTypeFound = false;
            return 0;
        }
        else
        {
            throw std::runtime_error("Could not find a matching memory type");
        }
    }
    
    VkShaderModule LoadShaderModule(const char* _fileName, VkDevice _logicalDevice)
    {
        std::ifstream is(_fileName, std::ios::binary | std::ios::in | std::ios::ate);
    
        if (is.is_open())
        {
            size_t size = is.tellg();
            is.seekg(0, std::ios::beg);
            char* shaderCode = new char[size];
            is.read(shaderCode, size);
            is.close();
    
            assert(size > 0);
    
            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo moduleCreateInfo{};
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.codeSize = size;
            moduleCreateInfo.pCode = (uint32_t*)shaderCode;
    
            if (vkCreateShaderModule(_logicalDevice, &moduleCreateInfo, NULL, &shaderModule) != VK_SUCCESS)
            {
                std::cout << "Error creating shader module" << std::endl;
                delete[] shaderCode;
                return VK_NULL_HANDLE;
            }
    
            delete[] shaderCode;
    
            return shaderModule;
        }
        else
        {
            std::cout << "Error: Could not open shader file" << std::endl;
            return VK_NULL_HANDLE;
        }
    }

    // TODO: MAKE THIS FUNCTION INLINE
    VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice _logicalDevice)
    {
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
        shaderStage.module = LoadShaderModule(fileName.c_str(), _logicalDevice);
        shaderStage.pName = "main";
        return shaderStage;
    }
};
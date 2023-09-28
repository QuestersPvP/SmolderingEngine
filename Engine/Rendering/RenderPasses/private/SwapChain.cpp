#include "../public/SwapChain.h"

#include "../../Instances/public/VulkanRendering.h"

SwapChain::SwapChain()
{
}

SwapChain::~SwapChain()
{
}

VkSurfaceFormatKHR SwapChain::ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // Check to see if there is only 1 format and see if it is undefined or not.
    // This means there is no preferred format, so choose best one for us.
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    // Try to find a format that fits our format.
    for (const auto& availableFormat : availableFormats) 
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace ==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
    // This is our prefered display mode, pictures are put in a queue and displayed using FIFO
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes) 
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
        else if(availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            bestMode = availablePresentMode;

        return bestMode;
    }
}

VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    // If you want the resolution to be different between the picture and window make it std::numeric_limits<uint32_t>::max()
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    else 
    {
        VkExtent2D actualExtent = { 1280, 720 };

        // Try to set the window size to best match resolution, if not 1280x720 is the best we can do
        actualExtent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height,actualExtent.height));

        return actualExtent;
    }
}

void SwapChain::Create(VkSurfaceKHR surface)
{
    // Get device support details
    SwapChainSupportDetails swapChainSupportDetails = VulkanRendering::GetInstance()->GetDevice()->swapchainSupport;

    // Get the surface format, present mode, and extent
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupportDetails.surfaceFormats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupportDetails.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupportDetails.surfaceCapabilities);

    // Get the minimum image count to make swap chain
    uint32_t imageCount = swapChainSupportDetails.surfaceCapabilities.minImageCount;

    // Ensure imageCount does not exceed maxImageCount
    if (swapChainSupportDetails.surfaceCapabilities.maxImageCount > 0 && 
        imageCount > swapChainSupportDetails.surfaceCapabilities.maxImageCount)
    {
        imageCount = swapChainSupportDetails.surfaceCapabilities.maxImageCount;
    }

    // Create the swap chain populate it
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface; // Vulkan Surface
    createInfo.minImageCount = imageCount; // Number of swap chain images
    createInfo.imageFormat = surfaceFormat.format; // Format of images
    createInfo.imageColorSpace = surfaceFormat.colorSpace; // Color space of images
    createInfo.imageExtent = extent; // WxH of swap chain images
    createInfo.imageArrayLayers = 1; // This is 1 unless you are making a VR application. In that case make this 2.
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // How image will be used
    createInfo.preTransform = swapChainSupportDetails.surfaceCapabilities.currentTransform; // Transform applied to images
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Alpha blending mode
    createInfo.presentMode = presentMode; // Presentation mode
    createInfo.clipped = VK_TRUE; // Should obscured portions of images be shown
    createInfo.oldSwapchain = VK_NULL_HANDLE; // Old swap chain

    QueueFamilyIndices indices = VulkanRendering::GetInstance()->GetDevice()->GetQueueFamiliesIndicesOfCurrentDevice();
    uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) 
    {   // Image can be used across multiple queue families
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {   // Image is owned by one family, ownership must be transferred 
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    

    if (vkCreateSwapchainKHR(VulkanRendering::GetInstance()->GetDevice()->logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    // Retrieve swapchain images
    // Color images are created automatically
    vkGetSwapchainImagesKHR(VulkanRendering::GetInstance()->GetDevice()->logicalDevice, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(VulkanRendering::GetInstance()->GetDevice()->logicalDevice, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainImageExtent = extent;
}

void SwapChain::Destroy()
{
    // Destroy the swapchain
    vkDestroySwapchainKHR(VulkanRendering::GetInstance()->GetDevice()->logicalDevice, swapChain, nullptr);
}

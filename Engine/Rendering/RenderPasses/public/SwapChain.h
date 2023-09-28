#pragma once

#include <vulkan\vulkan.h> 
#include <vector> 
#include <set> 
#include <algorithm>  

class SwapChain
{
public:
    SwapChain();
    ~SwapChain();


    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainImageExtent;

    std::vector<VkImage> swapChainImages;

    VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void Create(VkSurfaceKHR surface);
    void Destroy();
};


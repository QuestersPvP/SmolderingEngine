#pragma once

#include "../Common/Common.h"
#include "../Rendering/Instances/InstancesAndDevices.h"
#include "../Rendering/Application/WindowCreation.h"
#include "../Rendering/ImagePresentation/SwapChain.h"
#include "../Rendering/BuffersAndPools/CommandBufferAndPool.h"
#include "../Rendering/RenderPass/RenderPass.h"

using namespace SmolderingEngine;

class Renderer
{
	/* Variables */

public:
    LIBRARY_TYPE vulkanLibrary;

    VkInstance instance = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR presentationSurface = VK_NULL_HANDLE;
    VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFence drawingFence = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkPipeline modelPipeline = VK_NULL_HANDLE;
	
    std::vector<char const*> instanceExtensions;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<FrameResources> framesResources;
    std::vector<VkPhysicalDevice> physicalDevices;

    QueueParameters graphicsQueue;
    QueueParameters computeQueue;
    QueueParameters presentQueue;
    SwapchainParameters swapchain;
    Mesh model;

    //TODO: FIX INDEX
    uint32_t frame_index = 0;

    /* Functions */

public:
	bool InitRendererClass(const WindowParameters& _window);
	bool UpdateRendererClass();
    void ShutdownRendererClass();
};


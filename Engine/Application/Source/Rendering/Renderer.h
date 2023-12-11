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

    /* Vulkan */
    VkSubmitInfo submitInfo = {};
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineCache pipelineCache;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkFormat depthFormat;
    VkDescriptorSet descriptorSet;

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
    //VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkPipeline modelPipeline = VK_NULL_HANDLE;
	
    /* Vectors */
    std::vector<VkCommandBuffer> drawCommandBuffers;
    std::vector<VkFence> waitFences;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkShaderModule> shaderModules;

    std::vector<char const*> instanceExtensions;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<FrameResources> framesResources;
    std::vector<VkPhysicalDevice> physicalDevices;

    /* Struct section */
    Semaphores semaphores;
    DepthStencil depthStencil;
    Buffer uniformBuffer;
    Indices modelIndicies;
    Vertices modelVerticies;

    QueueParameters graphicsQueue;
    QueueParameters computeQueue;
    QueueParameters presentQueue;
    SwapchainParameters swapchain;
    Mesh model;

    //TODO: FIX INDEX
    uint32_t frame_index = 0;

private:
    bool applicationReadyToRender = true;
    float translationXValue = 0.0f;
    float translationYValue = 0.0f;
    float translationZValue = -3.0f;

    /* Functions */

public:
	bool InitRendererClass(const WindowParameters& _window);
	bool UpdateRendererClass();
    void ShutdownRendererClass();

    bool ResizeWindow();

    const bool GetApplicationReadyToRender() { return applicationReadyToRender; }
    const void SetApplicationReadyToRender(bool _shouldRender) { applicationReadyToRender = _shouldRender; }

    const float GetTranslattionXValue() { return translationXValue; }
    const void SetTranslationXValue(float _setValue) { translationXValue = _setValue; }
    const void AddToTranslationXValue(float _addValue) { translationXValue += _addValue; }

    const float GetTranslattionYValue() { return translationYValue; }
    const void SetTranslationYValue(float _setValue) { translationYValue = _setValue; }
    const void AddToTranslationYValue(float _addValue) { translationYValue += _addValue; }

    const float GetTranslattionZValue() { return translationZValue; }
    const void SetTranslationZValue(float _setValue) { translationZValue = _setValue; }
    const void AddToTranslationZValue(float _addValue) { translationZValue += _addValue; }

private:
    void UpdateModelPositions();
};


#pragma once

//#include "../Common/Common.h"
//#include "../Rendering/Instances/InstancesAndDevices.h"
//#include "../Rendering/Application/WindowCreation.h"
//#include "../Rendering/ImagePresentation/SwapChain.h"
//#include "../Rendering/BuffersAndPools/CommandBufferAndPool.h"
//#include "../Rendering/RenderPass/RenderPass.h"
//#include "../../VulkanglTFModel.h"
//#include "Camera/Camera.h"
//#include "vulkan/vulkan.h"

#include "../Utilities/Includes/ApplicationIncludes.h"

using namespace SE_Renderer;

class Renderer
{
	/* Variables */

public:
    //LIBRARY_TYPE vulkanLibrary;

    /* Vulkan */
    //VkSubmitInfo submitInfo = {};
    //VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //VkPipelineCache pipelineCache;
    //VkPhysicalDeviceMemoryProperties memoryProperties;
    //VkFormat depthFormat;
    //VkDescriptorSet descriptorSet;

    //VkInstance instance = VK_NULL_HANDLE;
    //VkDevice logicalDevice = VK_NULL_HANDLE;
    //VkSurfaceKHR presentationSurface = VK_NULL_HANDLE;
    //VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
    //VkRenderPass renderPass = VK_NULL_HANDLE;
    //VkFence drawingFence = VK_NULL_HANDLE;
    //VkCommandPool commandPool = VK_NULL_HANDLE;
    //VkFramebuffer framebuffer = VK_NULL_HANDLE;
    //VkBuffer vertexBuffer = VK_NULL_HANDLE;
    //VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    //VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    //VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    //VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    ////VkBuffer uniformBuffer = VK_NULL_HANDLE;
    //VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    //VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    //VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    //VkPipeline modelPipeline = VK_NULL_HANDLE;
	
    ///* Vectors */
    //std::vector<VkCommandBuffer> drawCommandBuffers;
    //std::vector<VkFence> waitFences;
    //std::vector<VkFramebuffer> frameBuffers;
    //std::vector<VkShaderModule> shaderModules;

    //std::vector<char const*> instanceExtensions;
    //std::vector<VkDescriptorSet> descriptorSets;
    //std::vector<FrameResources> framesResources;
    //std::vector<VkPhysicalDevice> physicalDevices;

    ///* Struct section */
    //Semaphores semaphores;
    //DepthStencil depthStencil;
    //Buffer uniformBuffer;
    //Indices modelIndicies;
    //Vertices modelVerticies;

    //QueueParameters graphicsQueue;
    //QueueParameters computeQueue;
    //QueueParameters presentQueue;
    //SwapchainParameters swapchain;
    //Mesh model;

    std::string title = "Smoldering Engine";
    std::string name = "Smoldering Engine";
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;
    Camera camera;
    VkQueueFlags requestedQueueTypes = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    VkSurfaceKHR surface;
    // Vulkan instance, stores all per-application states
    VkInstance instance;
    std::vector<std::string> supportedInstanceExtensions;
    // Stores physical device properties (for e.g. checking device limits)
    //VkPhysicalDeviceProperties deviceProperties;
    // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
    //VkPhysicalDeviceFeatures deviceFeatures;
    // Stores all available memory (type) properties for the physical device
    //VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    /** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
    VkPhysicalDeviceFeatures enabledFeatures{};
    /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
    //std::vector<const char*> enabledDeviceExtensions;
    //std::vector<const char*> enabledInstanceExtensions;
    /** @brief Optional pNext structure for passing extension structures to device creation */
    //void* deviceCreatepNextChain = nullptr;
    // Handle to the device graphics queue that command buffers are submitted to
    VkQueue queue;
    // Depth buffer format (selected during Vulkan initialization)
    VkFormat depthFormat;
    // Command buffer pool
    VkCommandPool commandBufferCommandPool;
    /** @brief Pipeline stages used to wait at for graphics queue submissions */
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // Contains command buffers and semaphores to be presented to the queue
    VkSubmitInfo submitInfo;
    // Command buffers used for rendering
    std::vector<VkCommandBuffer> drawCmdBuffers;
    // Global render pass for frame buffer writes
    VkRenderPass renderPass = VK_NULL_HANDLE;
    // List of available frame buffers (same as number of swap chain images)
    std::vector<VkFramebuffer>frameBuffers;
    // Active frame buffer index
    uint32_t currentBuffer = 0;
    // Descriptor set pool
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    // List of shader modules created (stored for cleanup)
    std::vector<VkShaderModule> shaderModules;
    // Pipeline cache object
    VkPipelineCache pipelineCache;
    // Wraps the swap chain to present images (framebuffers) to the windowing system
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    uint32_t imageCount;
    std::vector<VkImage> images;
    std::vector<SwapChainBuffer> buffers;
    uint32_t queueNodeIndex = UINT32_MAX;
    SynchronizationSemaphores semaphores;
    std::vector<VkFence> waitFences;
    uint32_t width = 1280;
    uint32_t height = 720;
    /** @brief Physical device representation */
    VkPhysicalDevice physicalDevice;
    /** @brief Logical device representation (application's view of the device) */
    VkDevice logicalDevice;
    /** @brief Properties of the physical device including limits that the application can check against */
    VkPhysicalDeviceProperties properties;
    /** @brief Features of the physical device that an application can use to check if a feature is supported */
    VkPhysicalDeviceFeatures features;
    /** @brief Memory types and heaps of the physical device */
    VkPhysicalDeviceMemoryProperties memoryProperties;
    /** @brief Queue family properties of the physical device */
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    /** @brief List of extensions supported by the device */
    std::vector<std::string> supportedExtensions;
    /** @brief Default command pool for the graphics queue family index */
    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
    /** @brief Contains queue family indices */
    QueueFamilyIndices queueFamilyIndices;
    VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    DepthStencil depthStencil;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor;
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    void* mapped = nullptr;
    /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
    VkBufferUsageFlags usageFlags;
    /** @brief Memory property flags to be filled by external source at buffer creation (to query at some later point) */
    VkMemoryPropertyFlags memoryPropertyFlags;
    vkglTF::Model model;
    UBOVS uboVS;
    uint32_t frame_index = 0;

private:
    bool applicationReadyToRender = false;
    //float translationXValue = 0.0f;
    //float translationYValue = 0.0f;
    //float translationZValue = -3.0f;

    /* Functions */

public:
	bool InitRendererClass(const WindowParameters& _window);
	bool UpdateRendererClass();
    void ShutdownRendererClass();

    bool ResizeWindow(uint32_t _width, uint32_t _height);

    const bool GetApplicationReadyToRender() { return applicationReadyToRender; }
    const void SetApplicationReadyToRender(bool _shouldRender) { applicationReadyToRender = _shouldRender; }

    //const float GetTranslattionXValue() { return translationXValue; }
    //const void SetTranslationXValue(float _setValue) { translationXValue = _setValue; }
    //const void AddToTranslationXValue(float _addValue) { translationXValue += _addValue; }

    //const float GetTranslattionYValue() { return translationYValue; }
    //const void SetTranslationYValue(float _setValue) { translationYValue = _setValue; }
    //const void AddToTranslationYValue(float _addValue) { translationYValue += _addValue; }

    //const float GetTranslattionZValue() { return translationZValue; }
    //const void SetTranslationZValue(float _setValue) { translationZValue = _setValue; }
    //const void AddToTranslationZValue(float _addValue) { translationZValue += _addValue; }

private:
    void UpdateModelPositions();
};


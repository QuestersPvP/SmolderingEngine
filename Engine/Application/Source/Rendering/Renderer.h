#pragma once

#include "Utilities/Includes/ApplicationIncludes.h"
#include "Objects/GameObjects/GameObject.h"

using namespace SE_Renderer;

class Renderer
{
	/* Variables */
public:
    // Camera that is attatched to the GameObject we are rendering
    Camera camera;
    // Object we want to render, holds a vkglTF::Model
    GameObject cube;

    VkQueueFlags requestedQueueTypes = (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    UBOVS uboVS;

    uint32_t currentBuffer = 0;
    /* Keeps track of the current swapchain image index */
    uint32_t frame_index = 0;
    uint32_t width = 1280;
    uint32_t height = 720;
    void* mapped = nullptr;

    /* VkTypedef*/
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkQueue copyQueue = VK_NULL_HANDLE;
    VkCommandPool commandBufferCommandPool = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;

    uint32_t imageCount = 0;
    VkBufferUsageFlags usageFlags;
    VkMemoryPropertyFlags memoryPropertyFlags;
    VkDeviceSize size = 0;
    VkDeviceSize alignment = 0;
    uint32_t queueNodeIndex = UINT32_MAX;

    /* VkStructs */
    VkPhysicalDeviceFeatures enabledFeatures{};
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkSubmitInfo submitInfo;
    VkDescriptorBufferInfo descriptor;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    /* Project Structs */
    DepthStencil depthStencil;
    SynchronizationSemaphores semaphores;
    QueueFamilyIndices queueFamilyIndices;
    SEVertices vertices;
    SEIndices indices;
    SEDescriptorSetLayouts descriptorSetLayouts;
    ShaderData shaderData;

    /* VkEnums */
    VkFormat depthFormat;
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;

    /* Vectors */
    std::vector<VkCommandBuffer> drawCmdBuffers;
    std::vector<VkFramebuffer>frameBuffers;
    std::vector<VkShaderModule> shaderModules;
    std::vector<VkImage> images;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<VkFence> waitFences;

    std::vector<std::string> supportedExtensions;
    std::vector<std::string> supportedInstanceExtensions;

    std::vector<SwapChainBuffer> buffers;
    std::vector<SEImage> modelImages;
    std::vector<SETexture> textures;
    std::vector<SEMaterial> materials;
    std::vector<SENode*> nodes;

    /* Union */
    // Color we clear the screen too
    VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

private:
    bool applicationReadyToRender = false;


    /* Functions */

public:
	bool InitRendererClass(const WindowParameters& _window);
	bool UpdateRendererClass();
    void ShutdownRendererClass();

    bool ResizeWindow(uint32_t _width, uint32_t _height);

    const bool GetApplicationReadyToRender() { return applicationReadyToRender; }
    const void SetApplicationReadyToRender(bool _shouldRender) { applicationReadyToRender = _shouldRender; }

private:
    void UpdateModelPositions();
};


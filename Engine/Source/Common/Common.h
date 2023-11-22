#pragma once

// TODO: Make file to hold #defines
// Vulkan library type for Windows
#define LIBRARY_TYPE HMODULE
// Wide character string to help load vulkan-1.dll
#define VULKAN_DLL_NAME L"vulkan-1.dll"
#define WINDOW_NAME L"Smoldering Engine"
#define WINDOW_TITLE L"Smoldering Engine"

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

    struct QueueParameters 
    {
        VkQueue   handle;
        uint32_t  familyIndex;
    };

    struct SwapchainParameters 
    {
        VkSwapchainKHR              handle;
        VkFormat                    format;
        VkExtent2D                  size;
        std::vector<VkImage>        images;
        std::vector<VkImageView*>   imageViews;
        std::vector<VkImageView>    ImageViewsRaw;
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

    struct ImageTransition 
    {
        VkImage             image;
        VkAccessFlags       currentAccess;
        VkAccessFlags       newAccess;
        VkImageLayout       currentLayout;
        VkImageLayout       newLayout;
        uint32_t            currentQueueFamily;
        uint32_t            newQueueFamily;
        VkImageAspectFlags  aspect;
    };

    struct BufferTransition 
    {
        VkBuffer        buffer;
        VkAccessFlags   currentAccess;
        VkAccessFlags   newAccess;
        uint32_t        currentQueueFamily;
        uint32_t        newQueueFamily;
    };

    struct VertexBufferParameters 
    {
        VkBuffer      buffer;
        VkDeviceSize  memoryOffset;
    };

    struct ShaderStageParameters 
    {
        VkShaderStageFlagBits           shaderStage;
        VkShaderModule                  shaderModule;
        char const*                     entryPointName;
        VkSpecializationInfo const*     specializationInfo;
    };

    struct ViewportInfo 
    {
        std::vector<VkViewport>   viewports;
        std::vector<VkRect2D>     scissors;
    };

    struct Mesh 
    {
        std::vector<float>  data;

        struct Part 
        {
            uint32_t        vertexOffset;
            uint32_t        vertexCount;
        };

        std::vector<Part>   parts;
    };

    struct ImageDescriptorInfo 
    {
        VkDescriptorSet                     targetDescriptorSet;
        uint32_t                            targetDescriptorBinding;
        uint32_t                            targetArrayElement;
        VkDescriptorType                    targetDescriptorType;
        std::vector<VkDescriptorImageInfo>  imageInfos;
    };

    struct BufferDescriptorInfo 
    {
        VkDescriptorSet                     targetDescriptorSet;
        uint32_t                            targetDescriptorBinding;
        uint32_t                            targetArrayElement;
        VkDescriptorType                    targetDescriptorType;
        std::vector<VkDescriptorBufferInfo> bufferInfos;
    };

    struct TexelBufferDescriptorInfo 
    {
        VkDescriptorSet             targetDescriptorSet;
        uint32_t                    targetDescriptorBinding;
        uint32_t                    targetArrayElement;
        VkDescriptorType            targetDescriptorType;
        std::vector<VkBufferView>   texelBufferViews;
    };

    struct CopyDescriptorInfo 
    {
        VkDescriptorSet targetDescriptorSet;
        uint32_t        targetDescriptorBinding;
        uint32_t        targetArrayElement;
        VkDescriptorSet sourceDescriptorSet;
        uint32_t        sourceDescriptorBinding;
        uint32_t        sourceArrayElement;
        uint32_t        descriptorCount;
    };

    struct FrameResources 
    {
        VkCommandBuffer commandBuffer;
        VkSemaphore     imageAcquiredSemaphore;
        VkSemaphore     readyToPresentSemaphore;
        VkFence         drawingFinishedFence;
        VkImageView     depthAttachment;
        VkFramebuffer   framebuffer;

        FrameResources(VkCommandBuffer& command_buffer,
            VkSemaphore& image_acquired_semaphore,
            VkSemaphore& ready_to_present_semaphore,
            VkFence& drawing_finished_fence,
            VkImageView& depth_attachment,
            VkFramebuffer& framebuffer) :
            commandBuffer(command_buffer),
            imageAcquiredSemaphore(std::move(image_acquired_semaphore)),
            readyToPresentSemaphore(std::move(ready_to_present_semaphore)),
            drawingFinishedFence(std::move(drawing_finished_fence)),
            depthAttachment(std::move(depth_attachment)),
            framebuffer(std::move(framebuffer)) {
        }

        FrameResources(FrameResources&& other) 
        {
            *this = std::move(other);
        }

        FrameResources& operator=(FrameResources&& other) 
        {
            if (this != &other) 
            {
                VkCommandBuffer command_buffer = commandBuffer;

                commandBuffer = other.commandBuffer;
                other.commandBuffer = command_buffer;
                imageAcquiredSemaphore = std::move(other.imageAcquiredSemaphore);
                readyToPresentSemaphore = std::move(other.readyToPresentSemaphore);
                drawingFinishedFence = std::move(other.drawingFinishedFence);
                depthAttachment = std::move(other.depthAttachment);
                framebuffer = std::move(other.framebuffer);
            }
            return *this;
        }

        FrameResources(FrameResources const&) = delete;
        FrameResources& operator=(FrameResources const&) = delete;
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

    //TODO: Math library
    using Vector3 = std::array<float, 3>;

    using Matrix4x4 = std::array<float, 16>;
    Matrix4x4 operator* (Matrix4x4 const& _left, Matrix4x4 const& _right);

    float Deg2Rad(float _value);
    Vector3 Normalize(Vector3 const& _vector);

    Matrix4x4 PrepareRotationMatrix(float _angle, Vector3 const& _axis, float _normalizeAxis = false);
    Matrix4x4 PrepareTranslationMatrix(float _x, float _y, float _z);  
    Matrix4x4 PreparePerspectiveProjectionMatrix(float _aspectRatio, float _fieldOfView, float _nearPlane, float _farPlane);

    // Extension availability check
    bool IsExtensionSupported(std::vector<VkExtensionProperties> const& _availableExtensions, char const* const _extension);
    bool GetBinaryFileContents(std::string const& _filename, std::vector<unsigned char>& _contents);
    bool Load3DModelFromObjFile(char const* _filename, bool _loadNormals, bool _loadTexcoords, bool _generateTangentSpaceVectors, bool _unify, Mesh& _mesh, uint32_t* _vertexStride = nullptr);
};

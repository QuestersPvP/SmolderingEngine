#include "RenderPass.h"

#include "../BuffersAndPools/CommandBufferAndPool.h"

namespace SmolderingEngine
{
    void SpecifySubpassDescriptions(std::vector<SubpassParameters> const& _subpassParameters, std::vector<VkSubpassDescription>& _subpassDescriptions)
    {
        _subpassDescriptions.clear();

        for (auto& subpassDescription : _subpassParameters)
        {
            _subpassDescriptions.push_back
            ({
                0,                                                                      // VkSubpassDescriptionFlags        flags
                subpassDescription.pipelineType,                                        // VkPipelineBindPoint              pipelineBindPoint
                static_cast<uint32_t>(subpassDescription.inputAttachments.size()),      // uint32_t                         inputAttachmentCount
                subpassDescription.inputAttachments.data(),                             // const VkAttachmentReference    * pInputAttachments
                static_cast<uint32_t>(subpassDescription.colorAttachments.size()),      // uint32_t                         colorAttachmentCount
                subpassDescription.colorAttachments.data(),                             // const VkAttachmentReference    * pColorAttachments
                subpassDescription.resolveAttachments.data(),                           // const VkAttachmentReference    * pResolveAttachments
                subpassDescription.depthStencilAttachment,                              // const VkAttachmentReference    * pDepthStencilAttachment
                static_cast<uint32_t>(subpassDescription.preserveAttachments.size()),   // uint32_t                         preserveAttachmentCount
                subpassDescription.preserveAttachments.data()                           // const uint32_t                 * pPreserveAttachments
            });
        }
    }

    bool CreateRenderPass(VkDevice _logicalDevice, std::vector<VkAttachmentDescription> const& _attachmentsDescriptions, std::vector<SubpassParameters> const& _subpassParameters,
        std::vector<VkSubpassDependency> const& _subpassDependencies, VkRenderPass& _renderPass)
	{
        std::vector<VkSubpassDescription> subpassDescriptions;
        SpecifySubpassDescriptions(_subpassParameters, subpassDescriptions);

        VkRenderPassCreateInfo render_pass_create_info = 
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,                // VkStructureType                    sType
            nullptr,                                                  // const void                       * pNext
            0,                                                        // VkRenderPassCreateFlags            flags
            static_cast<uint32_t>(_attachmentsDescriptions.size()),   // uint32_t                           attachmentCount
            _attachmentsDescriptions.data(),                          // const VkAttachmentDescription    * pAttachments
            static_cast<uint32_t>(subpassDescriptions.size()),        // uint32_t                           subpassCount
            subpassDescriptions.data(),                               // const VkSubpassDescription       * pSubpasses
            static_cast<uint32_t>(_subpassDependencies.size()),       // uint32_t                           dependencyCount
            _subpassDependencies.data()                               // const VkSubpassDependency        * pDependencies
        };

        if (vkCreateRenderPass(_logicalDevice, &render_pass_create_info, nullptr, &_renderPass) != VK_SUCCESS)
        {
            std::cout << "Could not create a render pass." << std::endl;
            return false;
        }

        return true;
	}

    void BeginRenderPass(VkCommandBuffer _commandBuffer, VkRenderPass _renderPass, VkFramebuffer _framebuffer, VkRect2D _renderArea, std::vector<VkClearValue> const& _clearValues, VkSubpassContents _subpassContents)
    {
        VkRenderPassBeginInfo renderPassBeginInfo = 
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,       // VkStructureType        sType
            nullptr,                                        // const void           * pNext
            _renderPass,                                    // VkRenderPass           renderPass
            _framebuffer,                                   // VkFramebuffer          framebuffer
            _renderArea,                                    // VkRect2D               renderArea
            static_cast<uint32_t>(_clearValues.size()),     // uint32_t               clearValueCount
            _clearValues.data()                             // const VkClearValue   * pClearValues
        };

        vkCmdBeginRenderPass(_commandBuffer, &renderPassBeginInfo, _subpassContents);
    }

    void EndRenderPass(VkCommandBuffer _commandBuffer)
    {
        vkCmdEndRenderPass(_commandBuffer);
    }

    bool CreateFramebuffer(VkDevice _logicalDevice, VkRenderPass _renderPass, std::vector<VkImageView> const& _attachments, uint32_t _width, uint32_t _height,
        uint32_t _layers, VkFramebuffer& _framebuffer)
    {
        VkFramebufferCreateInfo framebufferCreateInfo = 
        {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,      // VkStructureType              sType
            nullptr,                                        // const void                 * pNext
            0,                                              // VkFramebufferCreateFlags     flags
            _renderPass,                                    // VkRenderPass                 renderPass
            static_cast<uint32_t>(_attachments.size()),     // uint32_t                     attachmentCount
            _attachments.data(),                            // const VkImageView          * pAttachments
            _width,                                         // uint32_t                     width
            _height,                                        // uint32_t                     height
            _layers                                         // uint32_t                     layers
        };

        if (vkCreateFramebuffer(_logicalDevice, &framebufferCreateInfo, nullptr, &_framebuffer) != VK_SUCCESS) 
        {
            std::cout << "Could not create a framebuffer." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateImage(VkDevice _logicalDevice, VkImageType _type, VkFormat _format, VkExtent3D _size, uint32_t _numMipmaps, uint32_t _numLayers, VkSampleCountFlagBits _samples,
        VkImageUsageFlags _usageScenarios, bool _cubemap, VkImage& _image)
    {
        VkImageCreateInfo imageCreateInfo = 
        {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                    // VkStructureType          sType
            nullptr,                                                // const void             * pNext
            _cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,    // VkImageCreateFlags       flags
            _type,                                                  // VkImageType              imageType
            _format,                                                // VkFormat                 format
            _size,                                                  // VkExtent3D               extent
            _numMipmaps,                                            // uint32_t                 mipLevels
            _cubemap ? 6 * _numLayers : _numLayers,                 // uint32_t                 arrayLayers
            _samples,                                               // VkSampleCountFlagBits    samples
            VK_IMAGE_TILING_OPTIMAL,                                // VkImageTiling            tiling
            _usageScenarios,                                        // VkImageUsageFlags        usage
            VK_SHARING_MODE_EXCLUSIVE,                              // VkSharingMode            sharingMode
            0,                                                      // uint32_t                 queueFamilyIndexCount
            nullptr,                                                // const uint32_t         * pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED                               // VkImageLayout            initialLayout
        };

        if (vkCreateImage(_logicalDevice, &imageCreateInfo, nullptr, &_image) != VK_SUCCESS)
        {
            std::cout << "Could not create an image." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateImageView(VkDevice _logicalDevice, VkImage _image, VkImageViewType _viewType, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _imageView)
    {
        VkImageViewCreateInfo imageViewCreateInfo = 
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,       // VkStructureType            sType
            nullptr,                                        // const void               * pNext
            0,                                              // VkImageViewCreateFlags     flags
            _image,                                         // VkImage                    image
            _viewType,                                      // VkImageViewType            viewType
            _format,                                        // VkFormat                   format
            {                                               // VkComponentMapping         components
              VK_COMPONENT_SWIZZLE_IDENTITY,                // VkComponentSwizzle         r
              VK_COMPONENT_SWIZZLE_IDENTITY,                // VkComponentSwizzle         g
              VK_COMPONENT_SWIZZLE_IDENTITY,                // VkComponentSwizzle         b
              VK_COMPONENT_SWIZZLE_IDENTITY                 // VkComponentSwizzle         a
            },
            {                                               // VkImageSubresourceRange    subresourceRange
              _aspect,                                      // VkImageAspectFlags         aspectMask
              0,                                            // uint32_t                   baseMipLevel
              VK_REMAINING_MIP_LEVELS,                      // uint32_t                   levelCount
              0,                                            // uint32_t                   baseArrayLayer
              VK_REMAINING_ARRAY_LAYERS                     // uint32_t                   layerCount
            }
        };

        if (vkCreateImageView(_logicalDevice, &imageViewCreateInfo, nullptr, &_imageView) != VK_SUCCESS)
        {
            std::cout << "Could not create an image view." << std::endl;
            return false;
        }

        return true;
    }

    bool Create2DImageAndView(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkFormat _format, VkExtent2D _size, uint32_t _numMipmaps, uint32_t _numLayers,
        VkSampleCountFlagBits _samples, VkImageUsageFlags _usage, VkImageAspectFlags _aspect, VkImage& _image, VkDeviceMemory& _memoryObject, VkImageView& _imageView)
    {
        if (!CreateImage(_logicalDevice, VK_IMAGE_TYPE_2D, _format, { _size.width, _size.height, 1 }, _numMipmaps, _numLayers, _samples, _usage, false, _image))
            return false;

        if (!AllocateAndBindMemoryObjectToImage(_physicalDevice, _logicalDevice, _image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _memoryObject))
            return false;

        if (!CreateImageView(_logicalDevice, _image, VK_IMAGE_VIEW_TYPE_2D, _format, _aspect, _imageView))
            return false;

        return true;
    }

    bool CreateLayered2DImageWithCubemapView(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, uint32_t _size, uint32_t _numMipmaps, VkImageUsageFlags _usage,
        VkImageAspectFlags _aspect, VkImage& _image, VkDeviceMemory& _memoryObject, VkImageView& _imageView)
    {
        if (!CreateImage(_logicalDevice, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { _size, _size, 1 }, _numMipmaps, 6, VK_SAMPLE_COUNT_1_BIT, _usage, true, _image))
            return false;
        
        if (!AllocateAndBindMemoryObjectToImage(_physicalDevice, _logicalDevice, _image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _memoryObject))
            return false;

        if (!CreateImageView(_logicalDevice, _image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, _aspect, _imageView))
            return false;

        return true;
    }

    bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkImage _image, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject)
    {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &physicalDeviceMemoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(_logicalDevice, _image, &memoryRequirements);

        _memoryObject = VK_NULL_HANDLE;
        for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type)
        {
            if ((memoryRequirements.memoryTypeBits & (1 << type)) &&
                ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & _memoryProperties) == _memoryProperties))
            {

                VkMemoryAllocateInfo imageMemoryAllocateInfo = 
                {
                  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
                  nullptr,                                  // const void       * pNext
                  memoryRequirements.size,                 // VkDeviceSize       allocationSize
                  type                                      // uint32_t           memoryTypeIndex
                };

                if (vkAllocateMemory(_logicalDevice, &imageMemoryAllocateInfo, nullptr, &_memoryObject) == VK_SUCCESS)
                    break;
            }
        }

        if (VK_NULL_HANDLE == _memoryObject) 
        {
            std::cout << "Could not allocate memory for an image." << std::endl;
            return false;
        }

        if (vkBindImageMemory(_logicalDevice, _image, _memoryObject, 0) != VK_SUCCESS)
        {
            std::cout << "Could not bind memory object to an image." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateBuffer(VkDevice _logicalDevice, VkDeviceSize _size, VkBufferUsageFlags _usage, VkBuffer& _buffer)
    {
        VkBufferCreateInfo bufferCreateInfo =
        {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,       // VkStructureType        sType
            nullptr,                                    // const void           * pNext
            0,                                          // VkBufferCreateFlags    flags
            _size,                                      // VkDeviceSize           size
            _usage,                                     // VkBufferUsageFlags     usage
            VK_SHARING_MODE_EXCLUSIVE,                  // VkSharingMode          sharingMode
            0,                                          // uint32_t               queueFamilyIndexCount
            nullptr                                     // const uint32_t       * pQueueFamilyIndices
        };

        if (vkCreateBuffer(_logicalDevice, &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS)
        {
            std::cout << "Could not create a buffer." << std::endl;
            return false;
        }

        return true;
    }

    bool AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkBuffer _buffer, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject)
    {
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &physicalDeviceMemoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(_logicalDevice, _buffer, &memoryRequirements);

        _memoryObject = VK_NULL_HANDLE;
        for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type) 
        {
            if ((memoryRequirements.memoryTypeBits & (1 << type)) &&
                ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & _memoryProperties) == _memoryProperties)) 
            {
                VkMemoryAllocateInfo bufferMemoryAllocateInfo = 
                {
                  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
                  nullptr,                                  // const void       * pNext
                  memoryRequirements.size,                  // VkDeviceSize       allocationSize
                  type                                      // uint32_t           memoryTypeIndex
                };
                
                if (vkAllocateMemory(_logicalDevice, &bufferMemoryAllocateInfo, nullptr, &_memoryObject) == VK_SUCCESS)
                    break;
            }
        }

        if (VK_NULL_HANDLE == _memoryObject) 
        {
            std::cout << "Could not allocate memory for a buffer." << std::endl;
            return false;
        }

        if (vkBindBufferMemory(_logicalDevice, _buffer, _memoryObject, 0) != VK_SUCCESS)
        {
            std::cout << "Could not bind memory object to a buffer." << std::endl;
            return false;
        }

        return true;
    }

    void SetBufferMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<BufferTransition> _bufferTransitions)
    {
        std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;

        for (auto& bufferTransition : _bufferTransitions)
        {
            bufferMemoryBarriers.push_back({
              VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,      // VkStructureType    sType
              nullptr,                                      // const void       * pNext
              bufferTransition.currentAccess,               // VkAccessFlags      srcAccessMask
              bufferTransition.newAccess,                   // VkAccessFlags      dstAccessMask
              bufferTransition.currentQueueFamily,          // uint32_t           srcQueueFamilyIndex
              bufferTransition.newQueueFamily,              // uint32_t           dstQueueFamilyIndex
              bufferTransition.buffer,                      // VkBuffer           buffer
              0,                                            // VkDeviceSize       offset
              VK_WHOLE_SIZE                                 // VkDeviceSize       size
            });
        }

        if (bufferMemoryBarriers.size() > 0)
            vkCmdPipelineBarrier(_commandBuffer, _generatingStages, _consumingStages, 0, 0, nullptr, static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(), 0, nullptr);
    }

    bool CreateBufferView(VkDevice _logicalDevice, VkBuffer _buffer, VkFormat _format, VkDeviceSize _memoryOffset, VkDeviceSize _memoryRange, VkBufferView& _bufferView)
    {
        VkBufferViewCreateInfo bufferViewCreateInfo = 
        {
            VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,      // VkStructureType            sType
            nullptr,                                        // const void               * pNext
            0,                                              // VkBufferViewCreateFlags    flags
            _buffer,                                        // VkBuffer                   buffer
            _format,                                        // VkFormat                   format
            _memoryOffset,                                  // VkDeviceSize               offset
            _memoryRange                                    // VkDeviceSize               range
        };

        if (vkCreateBufferView(_logicalDevice, &bufferViewCreateInfo, nullptr, &_bufferView) != VK_SUCCESS)
        {
            std::cout << "Could not creat buffer view." << std::endl;
            return false;
        }

        return true;
    }

    void CopyDataBetweenBuffers(VkCommandBuffer _commandBuffer, VkBuffer _sourceBuffer, VkBuffer _destinationBuffer, std::vector<VkBufferCopy> _regions)
    {
        if (_regions.size() > 0)
            vkCmdCopyBuffer(_commandBuffer, _sourceBuffer, _destinationBuffer, static_cast<uint32_t>(_regions.size()), _regions.data());
    }

    void CopyDataFromBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _sourceBuffer, VkImage _destinationImage, VkImageLayout _imageLayout, std::vector<VkBufferImageCopy> _regions)
    {
        if (_regions.size() > 0)
            vkCmdCopyBufferToImage(_commandBuffer, _sourceBuffer, _destinationImage, _imageLayout, static_cast<uint32_t>(_regions.size()), _regions.data());
    }

    void CopyDataFromImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _sourceImage, VkImageLayout _imageLayout, VkBuffer _destinationBuffer, std::vector<VkBufferImageCopy> _regions)
    {
        if (_regions.size() > 0)
            vkCmdCopyImageToBuffer(_commandBuffer, _sourceImage, _imageLayout, _destinationBuffer, static_cast<uint32_t>(_regions.size()), _regions.data());
    }

    bool UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _dataSize, void* _data, VkBuffer _destinationBuffer,
        VkDeviceSize _destinationOffset, VkAccessFlags _destinationBufferCurrentAccess, VkAccessFlags _destinationBufferNewAccess, VkPipelineStageFlags _destinationBufferGeneratingStages,
        VkPipelineStageFlags _destinationBufferConsumingStages, VkQueue _queue, VkCommandBuffer _commandBuffer, std::vector<VkSemaphore> _signalSemaphores)
    {
        VkBuffer stagingBuffer;
        if (!CreateBuffer(_logicalDevice, _dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer)) 
            return false;

        VkDeviceMemory memoryObject;
        if (!AllocateAndBindMemoryObjectToBuffer(_physicalDevice, _logicalDevice, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryObject))
            return false;

        if (!MapUpdateAndUnmapHostVisibleMemory(_logicalDevice, memoryObject, 0, _dataSize, _data, true, nullptr))
            return false;

        if (!BeginCommandBufferRecordingOperation(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr))
            return false;

        SetBufferMemoryBarrier(_commandBuffer, _destinationBufferGeneratingStages, VK_PIPELINE_STAGE_TRANSFER_BIT, { { _destinationBuffer, _destinationBufferCurrentAccess, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });

        CopyDataBetweenBuffers(_commandBuffer, stagingBuffer, _destinationBuffer, { { 0, _destinationOffset, _dataSize } });

        SetBufferMemoryBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, _destinationBufferConsumingStages, { { _destinationBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, _destinationBufferNewAccess, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });

        if (!EndCommandBufferRecordingOperation(_commandBuffer))
            return false;

        VkFence fence;
        if (!CreateFence(_logicalDevice, false, fence))
            return false;

        if (!SubmitCommandBuffersToQueue(_queue, {}, { _commandBuffer }, _signalSemaphores, fence))
            return false;

        if (!WaitForFences(_logicalDevice, { fence }, VK_FALSE, 500000000))
            return false;

        return true;
    }

    bool UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _dataSize, void* _data, VkImage _destinationImage,
        VkImageSubresourceLayers _destinationImageSubresource, VkOffset3D _destinationImageOffset, VkExtent3D _destinationImageSize, VkImageLayout _destinationImageCurrentLayout,
        VkImageLayout _destinationImageNewLayout, VkAccessFlags _destinationImageCurrentAccess, VkAccessFlags _destinationImageNewAccess, VkImageAspectFlags _destinationImageAspect,
        VkPipelineStageFlags _destinationImageGeneratingStages, VkPipelineStageFlags _destinationImageConsumingStages, VkQueue _queue, VkCommandBuffer _commandBuffer, std::vector<VkSemaphore> _signalSemaphores)
    {
        VkBuffer stagingBuffer;
        if (!CreateBuffer(_logicalDevice, _dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer))
            return false;

        VkDeviceMemory memoryObject;
        if (!AllocateAndBindMemoryObjectToBuffer(_physicalDevice, _logicalDevice, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryObject))
            return false;

        if (!MapUpdateAndUnmapHostVisibleMemory(_logicalDevice, memoryObject, 0, _dataSize, _data, true, nullptr))
            return false;

        if (!BeginCommandBufferRecordingOperation(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr))
            return false;

        SetImageMemoryBarrier(_commandBuffer, _destinationImageGeneratingStages, VK_PIPELINE_STAGE_TRANSFER_BIT,
            {
                {
                  _destinationImage,                            // VkImage            Image
                  _destinationImageCurrentAccess,               // VkAccessFlags      CurrentAccess
                  VK_ACCESS_TRANSFER_WRITE_BIT,                 // VkAccessFlags      NewAccess
                  _destinationImageCurrentLayout,               // VkImageLayout      CurrentLayout
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,         // VkImageLayout      NewLayout
                  VK_QUEUE_FAMILY_IGNORED,                      // uint32_t           CurrentQueueFamily
                  VK_QUEUE_FAMILY_IGNORED,                      // uint32_t           NewQueueFamily
                  _destinationImageAspect                       // VkImageAspectFlags Aspect
                } 
            });

        CopyDataFromBufferToImage(_commandBuffer, stagingBuffer, _destinationImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            {
                {
                  0,                                        // VkDeviceSize               bufferOffset
                  0,                                        // uint32_t                   bufferRowLength
                  0,                                        // uint32_t                   bufferImageHeight
                  _destinationImageSubresource,             // VkImageSubresourceLayers   imageSubresource
                  _destinationImageOffset,                  // VkOffset3D                 imageOffset
                  _destinationImageSize,                    // VkExtent3D                 imageExtent
                } 
            });

        SetImageMemoryBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, _destinationImageConsumingStages,
            {
                {
                  _destinationImage,                        // VkImage            Image
                  VK_ACCESS_TRANSFER_WRITE_BIT,             // VkAccessFlags      CurrentAccess
                  _destinationImageNewAccess,             // VkAccessFlags      NewAccess
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     // VkImageLayout      CurrentLayout
                  _destinationImageNewLayout,             // VkImageLayout      NewLayout
                  VK_QUEUE_FAMILY_IGNORED,                  // uint32_t           CurrentQueueFamily
                  VK_QUEUE_FAMILY_IGNORED,                  // uint32_t           NewQueueFamily
                  _destinationImageAspect                  // VkImageAspectFlags Aspect
                } 
            });

        if (!EndCommandBufferRecordingOperation(_commandBuffer))
            return false;

        VkFence fence;
        if (!CreateFence(_logicalDevice, false, fence))
            return false;

        if (!SubmitCommandBuffersToQueue(_queue, {}, { _commandBuffer }, _signalSemaphores, fence))
            return false;

        if (!WaitForFences(_logicalDevice, { fence }, VK_FALSE, 500000000))
            return false;

        return true;
    }

    void DrawGeometry(VkCommandBuffer _commandBuffer, uint32_t _vertexCount, uint32_t _instanceCount, uint32_t _firstVertex, uint32_t _firstInstance)
    {
        vkCmdDraw(_commandBuffer, _vertexCount, _instanceCount, _firstVertex, _firstInstance);
    }

    void BindVertexBuffers(VkCommandBuffer _commandBuffer, uint32_t _firstBinding, std::vector<VertexBufferParameters> const& _buffersParameters)
    {
        if (_buffersParameters.size() > 0)
        {
            std::vector<VkBuffer> buffers;
            std::vector<VkDeviceSize> offsets;
            for (auto& bufferParameters : _buffersParameters)
            {
                buffers.push_back(bufferParameters.buffer);
                offsets.push_back(bufferParameters.memoryOffset);
            }
            vkCmdBindVertexBuffers(_commandBuffer, _firstBinding, static_cast<uint32_t>(_buffersParameters.size()), buffers.data(), offsets.data());
        }
    }

    bool CreateShaderModule(VkDevice _logicalDevice, std::vector<unsigned char> const& _sourceCode, VkShaderModule& _shaderModule)
    {
        VkShaderModuleCreateInfo shader_module_create_info = 
        {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,                // VkStructureType              sType
            nullptr,                                                    // const void                 * pNext
            0,                                                          // VkShaderModuleCreateFlags    flags
            _sourceCode.size(),                                         // size_t                       codeSize
            reinterpret_cast<uint32_t const*>(_sourceCode.data())       // const uint32_t             * pCode
        };

        if (vkCreateShaderModule(_logicalDevice, &shader_module_create_info, nullptr, &_shaderModule) != VK_SUCCESS)
        {
            std::cout << "Could not create a shader module." << std::endl;
            return false;
        }

        return true;
    }

    void SpecifyPipelineShaderStages(std::vector<ShaderStageParameters> const& _shaderStageParams, std::vector<VkPipelineShaderStageCreateInfo>& _shaderStageCreateInfos)
    {
        _shaderStageCreateInfos.clear();

        for (auto& shaderStage : _shaderStageParams)
        {
            _shaderStageCreateInfos.push_back
            ({
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // VkStructureType                    sType
                nullptr,                                              // const void                       * pNext
                0,                                                    // VkPipelineShaderStageCreateFlags   flags
                shaderStage.shaderStage,                             // VkShaderStageFlagBits              stage
                shaderStage.shaderModule,                            // VkShaderModule                     module
                shaderStage.entryPointName,                          // const char                       * pName
                shaderStage.specializationInfo                       // const VkSpecializationInfo       * pSpecializationInfo
            });
        }
    }

    void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const& _bindingDescriptions, std::vector<VkVertexInputAttributeDescription> const& _attributeDescriptions,
        VkPipelineVertexInputStateCreateInfo& _vertexInputStateCreateInfo)
    {
        _vertexInputStateCreateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                           sType
            nullptr,                                                      // const void                              * pNext
            0,                                                            // VkPipelineVertexInputStateCreateFlags     flags
            static_cast<uint32_t>(_bindingDescriptions.size()),           // uint32_t                                  vertexBindingDescriptionCount
            _bindingDescriptions.data(),                                  // const VkVertexInputBindingDescription   * pVertexBindingDescriptions
            static_cast<uint32_t>(_attributeDescriptions.size()),         // uint32_t                                  vertexAttributeDescriptionCount
            _attributeDescriptions.data()                                 // const VkVertexInputAttributeDescription * pVertexAttributeDescriptions
        };
    }

    void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology _topology, bool _primitiveRestartEnable, VkPipelineInputAssemblyStateCreateInfo& _inputAssemblyStateCreateInfo)
    {
        _inputAssemblyStateCreateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    // VkStructureType                           sType
            nullptr,                                                        // const void                              * pNext
            0,                                                              // VkPipelineInputAssemblyStateCreateFlags   flags
            _topology,                                                      // VkPrimitiveTopology                       topology
            _primitiveRestartEnable                                         // VkBool32                                  primitiveRestartEnable
        };
    }

    void SpecifyPipelineViewportAndScissorTestState(ViewportInfo const& _viewportInfos, VkPipelineViewportStateCreateInfo& _viewportStateCreateInfo)
    {
        uint32_t viewportCount = static_cast<uint32_t>(_viewportInfos.viewports.size());
        uint32_t scissorCount = static_cast<uint32_t>(_viewportInfos.scissors.size());
        
        _viewportStateCreateInfo =
        {
          VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,    // VkStructureType                      sType
          nullptr,                                                  // const void                         * pNext
          0,                                                        // VkPipelineViewportStateCreateFlags   flags
          viewportCount,                                            // uint32_t                             viewportCount
          _viewportInfos.viewports.data(),                          // const VkViewport                   * pViewports
          scissorCount,                                             // uint32_t                             scissorCount
          _viewportInfos.scissors.data()                            // const VkRect2D                     * pScissors
        };
    }

    void SpecifyPipelineRasterizationState(bool _depthClampEnable, bool _rasterizerDiscardEnable, VkPolygonMode _polygonMode, VkCullModeFlags _cullingMode, VkFrontFace _frontFace,
        bool _depthBiasEnable, float _depthBiasConstantFactor, float _depthBiasClamp, float _depthBiasSlopeFactor, float _lineWidth, VkPipelineRasterizationStateCreateInfo& _rasterizationStateCreateInfo)
    {
        _rasterizationStateCreateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                            sType
            nullptr,                                                    // const void                               * pNext
            0,                                                          // VkPipelineRasterizationStateCreateFlags    flags
            _depthClampEnable,                                          // VkBool32                                   depthClampEnable
            _rasterizerDiscardEnable,                                   // VkBool32                                   rasterizerDiscardEnable
            _polygonMode,                                               // VkPolygonMode                              polygonMode
            _cullingMode,                                               // VkCullModeFlags                            cullMode
            _frontFace,                                                 // VkFrontFace                                frontFace
            _depthBiasEnable,                                           // VkBool32                                   depthBiasEnable
            _depthBiasConstantFactor,                                   // float                                      depthBiasConstantFactor
            _depthBiasClamp,                                            // float                                      depthBiasClamp
            _depthBiasSlopeFactor,                                      // float                                      depthBiasSlopeFactor
            _lineWidth                                                  // float                                      lineWidth
        };
    }

    void SpecifyPipelineMultisampleState(VkSampleCountFlagBits _sampleCount, bool _perSampleShadingEnable, float _minSampleShading, VkSampleMask const* _sampleMasks,
        bool _alphaToCoverageEnable, bool _alphaToOneEnable, VkPipelineMultisampleStateCreateInfo& _multisampleStateCreateInfo)
    {
        _multisampleStateCreateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,   // VkStructureType                          sType
            nullptr,                                                    // const void                             * pNext
            0,                                                          // VkPipelineMultisampleStateCreateFlags    flags
            _sampleCount,                                               // VkSampleCountFlagBits                    rasterizationSamples
            _perSampleShadingEnable,                                    // VkBool32                                 sampleShadingEnable
            _minSampleShading,                                          // float                                    minSampleShading
            _sampleMasks,                                               // const VkSampleMask                     * pSampleMask
            _alphaToCoverageEnable,                                     // VkBool32                                 alphaToCoverageEnable
            _alphaToOneEnable                                           // VkBool32                                 alphaToOneEnable
        };
    }

    void SpecifyPipelineBlendState(bool _logicOpEnable, VkLogicOp _logicOp, std::vector<VkPipelineColorBlendAttachmentState> const& _attachmentBlendStates,
        std::array<float, 4> const& _blendConstants, VkPipelineColorBlendStateCreateInfo& _blendStateCreateInfo)
    {
        _blendStateCreateInfo = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                              sType
            nullptr,                                                    // const void                                 * pNext
            0,                                                          // VkPipelineColorBlendStateCreateFlags         flags
            _logicOpEnable,                                             // VkBool32                                     logicOpEnable
            _logicOp,                                                   // VkLogicOp                                    logicOp
            static_cast<uint32_t>(_attachmentBlendStates.size()),       // uint32_t                                     attachmentCount
            _attachmentBlendStates.data(),                              // const VkPipelineColorBlendAttachmentState  * pAttachments
            {                                                           // float                                        blendConstants[4]
              _blendConstants[0],
              _blendConstants[1],
              _blendConstants[2],
              _blendConstants[3]
            }
        };
    }

    void SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const& _dynamicStates, VkPipelineDynamicStateCreateInfo& _dynamicStateCreateInfo)
    {
        _dynamicStateCreateInfo =
        {
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,     // VkStructureType                      sType
            nullptr,                                                  // const void                         * pNext
            0,                                                        // VkPipelineDynamicStateCreateFlags    flags
            static_cast<uint32_t>(_dynamicStates.size()),             // uint32_t                             dynamicStateCount
            _dynamicStates.data()                                     // const VkDynamicState               * pDynamicStates
        };
    }

    bool CreatePipelineLayout(VkDevice _logicalDevice, std::vector<VkDescriptorSetLayout> const& _descriptorSetLayouts,
        std::vector<VkPushConstantRange> const& _pushConstantRanges, VkPipelineLayout& _pipelineLayout)
    {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,          // VkStructureType                  sType
            nullptr,                                                // const void                     * pNext
            0,                                                      // VkPipelineLayoutCreateFlags      flags
            static_cast<uint32_t>(_descriptorSetLayouts.size()),    // uint32_t                         setLayoutCount
            _descriptorSetLayouts.data(),                           // const VkDescriptorSetLayout    * pSetLayouts
            static_cast<uint32_t>(_pushConstantRanges.size()),      // uint32_t                         pushConstantRangeCount
            _pushConstantRanges.data()                              // const VkPushConstantRange      * pPushConstantRanges
        };

        if (vkCreatePipelineLayout(_logicalDevice, &pipeline_layout_create_info, nullptr, &_pipelineLayout) != VK_SUCCESS)
        {
            std::cout << "Could not create pipeline layout." << std::endl;
            return false;
        }

        return true;
    }

    void SpecifyGraphicsPipelineCreationParameters(VkPipelineCreateFlags _additionalOptions, std::vector<VkPipelineShaderStageCreateInfo> const& _shaderStageCreateInfos,
        VkPipelineVertexInputStateCreateInfo const& _vertexInputStateCreateInfo, VkPipelineInputAssemblyStateCreateInfo const& _inputAssemblyStateCreateInfo,
        VkPipelineTessellationStateCreateInfo const* _tessellationStateCreateInfo, VkPipelineViewportStateCreateInfo const* _viewportStateCreateInfo,
        VkPipelineRasterizationStateCreateInfo const& _rasterizationStateCreateInfo, VkPipelineMultisampleStateCreateInfo const* _multisampleStateCreateInfo,
        VkPipelineDepthStencilStateCreateInfo const* _depthAndStencilStateCreateInfo, VkPipelineColorBlendStateCreateInfo const* _blendStateCreateInfo,
        VkPipelineDynamicStateCreateInfo const* _dynamicStateCreateInfo, VkPipelineLayout _pipelineLayout, VkRenderPass _renderPass, uint32_t _subpass,
        VkPipeline _basePipelineHandle, int32_t _basePipelineIndex, VkGraphicsPipelineCreateInfo& _graphicsPipelineCreateInfo)
    {
        _graphicsPipelineCreateInfo =
        {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                sType
            nullptr,                                                    // const void                                   * pNext
            _additionalOptions,                                         // VkPipelineCreateFlags                          flags
            static_cast<uint32_t>(_shaderStageCreateInfos.size()),      // uint32_t                                       stageCount
            _shaderStageCreateInfos.data(),                             // const VkPipelineShaderStageCreateInfo        * pStages
            &_vertexInputStateCreateInfo,                               // const VkPipelineVertexInputStateCreateInfo   * pVertexInputState
            &_inputAssemblyStateCreateInfo,                             // const VkPipelineInputAssemblyStateCreateInfo * pInputAssemblyState
            _tessellationStateCreateInfo,                               // const VkPipelineTessellationStateCreateInfo  * pTessellationState
            _viewportStateCreateInfo,                                   // const VkPipelineViewportStateCreateInfo      * pViewportState
            &_rasterizationStateCreateInfo,                             // const VkPipelineRasterizationStateCreateInfo * pRasterizationState
            _multisampleStateCreateInfo,                                // const VkPipelineMultisampleStateCreateInfo   * pMultisampleState
            _depthAndStencilStateCreateInfo,                            // const VkPipelineDepthStencilStateCreateInfo  * pDepthStencilState
            _blendStateCreateInfo,                                      // const VkPipelineColorBlendStateCreateInfo    * pColorBlendState
            _dynamicStateCreateInfo,                                    // const VkPipelineDynamicStateCreateInfo       * pDynamicState
            _pipelineLayout,                                            // VkPipelineLayout                               layout
            _renderPass,                                                // VkRenderPass                                   renderPass
            _subpass,                                                   // uint32_t                                       subpass
            _basePipelineHandle,                                        // VkPipeline                                     basePipelineHandle
            _basePipelineIndex                                          // int32_t                                        basePipelineIndex
        };
    }

    bool CreateGraphicsPipelines(VkDevice _logicalDevice, std::vector<VkGraphicsPipelineCreateInfo> const& _graphicsPipelineCreateInfos,
        VkPipelineCache _pipelineCache, std::vector<VkPipeline>& _graphicsPipelines)
    {
        if (_graphicsPipelineCreateInfos.size() > 0)
        {
            _graphicsPipelines.resize(_graphicsPipelineCreateInfos.size());
            if (vkCreateGraphicsPipelines(_logicalDevice, _pipelineCache, static_cast<uint32_t>(_graphicsPipelineCreateInfos.size()), _graphicsPipelineCreateInfos.data(), nullptr, _graphicsPipelines.data()) != VK_SUCCESS)
            {
                std::cout << "Could not create a graphics pipeline." << std::endl;
                return false;
            }

            return true;
        }

        return false;
    }

    void BindPipelineObject(VkCommandBuffer _commandBuffer, VkPipelineBindPoint _pipelineType, VkPipeline _pipeline)
    {
        vkCmdBindPipeline(_commandBuffer, _pipelineType, _pipeline);
    }

    void SetViewportStateDynamically(VkCommandBuffer _commandBuffer, uint32_t _firstViewport, std::vector<VkViewport> const& _viewports)
    {
        vkCmdSetViewport(_commandBuffer, _firstViewport, static_cast<uint32_t>(_viewports.size()), _viewports.data());
    }

    void SetScissorStateDynamically(VkCommandBuffer _commandBuffer, uint32_t _firstScissor, std::vector<VkRect2D> const& _scissors)
    {
        vkCmdSetScissor(_commandBuffer, _firstScissor, static_cast<uint32_t>(_scissors.size()), _scissors.data());
    }

    void SetImageMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<ImageTransition> _imageTransitions)
    {

        std::vector<VkImageMemoryBarrier> imageMemoryBarriers;

        for (auto& imageTransition : _imageTransitions) 
        {
            imageMemoryBarriers.push_back
            ({
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType
                nullptr,                                    // const void               * pNext
                imageTransition.currentAccess,              // VkAccessFlags              srcAccessMask
                imageTransition.newAccess,                  // VkAccessFlags              dstAccessMask
                imageTransition.currentLayout,              // VkImageLayout              oldLayout
                imageTransition.newLayout,                  // VkImageLayout              newLayout
                imageTransition.currentQueueFamily,         // uint32_t                   srcQueueFamilyIndex
                imageTransition.newQueueFamily,             // uint32_t                   dstQueueFamilyIndex
                imageTransition.image,                      // VkImage                    image
                {                                           // VkImageSubresourceRange    subresourceRange
                  imageTransition.aspect,                   // VkImageAspectFlags         aspectMask
                  0,                                        // uint32_t                   baseMipLevel
                  VK_REMAINING_MIP_LEVELS,                  // uint32_t                   levelCount
                  0,                                        // uint32_t                   baseArrayLayer
                  VK_REMAINING_ARRAY_LAYERS                 // uint32_t                   layerCount
                }
            });
        }

        if (imageMemoryBarriers.size() > 0) 
            vkCmdPipelineBarrier(_commandBuffer, _generatingStages, _consumingStages, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(imageMemoryBarriers.size()), imageMemoryBarriers.data());
    }
    bool MapUpdateAndUnmapHostVisibleMemory(VkDevice _logicalDevice, VkDeviceMemory _memoryObject, VkDeviceSize _offset, VkDeviceSize _dataSize,
        void* _data, bool _unmap, void** _pointer)
    {
        void* localPointer;

        if (vkMapMemory(_logicalDevice, _memoryObject, _offset, _dataSize, 0, &localPointer) != VK_SUCCESS)
        {
            std::cout << "Could not map memory object." << std::endl;
            return false;
        }

        std::memcpy(localPointer, _data, static_cast<size_t>(_dataSize));

        std::vector<VkMappedMemoryRange> memoryRanges = 
        {
            {
              VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,    // VkStructureType    sType
              nullptr,                                  // const void       * pNext
              _memoryObject,                            // VkDeviceMemory     memory
              _offset,                                  // VkDeviceSize       offset
              VK_WHOLE_SIZE                             // VkDeviceSize       size
            }
        };

        if (vkFlushMappedMemoryRanges(_logicalDevice, static_cast<uint32_t>(memoryRanges.size()), memoryRanges.data()) != VK_SUCCESS)
        {
            std::cout << "Could not flush mapped memory." << std::endl;
            return false;
        }

        if (_unmap) 
        {
            vkUnmapMemory(_logicalDevice, _memoryObject);
        }
        else if (nullptr != _pointer) 
        {
            *_pointer = localPointer;
        }

        return true;
    }
};
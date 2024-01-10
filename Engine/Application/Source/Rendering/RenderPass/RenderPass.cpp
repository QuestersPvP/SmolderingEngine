#include "RenderPass.h"
//#include "../BuffersAndPools/CommandBufferAndPool.h"

namespace SE_Renderer
{
//    void SpecifySubpassDescriptions(std::vector<SubpassParameters> const& _subpassParameters, std::vector<VkSubpassDescription>& _subpassDescriptions)
//    {
//        _subpassDescriptions.clear();
//
//        for (auto& subpassDescription : _subpassParameters)
//        {
//            _subpassDescriptions.push_back
//            ({
//                0,                                                                      // VkSubpassDescriptionFlags        flags
//                subpassDescription.pipelineType,                                        // VkPipelineBindPoint              pipelineBindPoint
//                static_cast<uint32_t>(subpassDescription.inputAttachments.size()),      // uint32_t                         inputAttachmentCount
//                subpassDescription.inputAttachments.data(),                             // const VkAttachmentReference    * pInputAttachments
//                static_cast<uint32_t>(subpassDescription.colorAttachments.size()),      // uint32_t                         colorAttachmentCount
//                subpassDescription.colorAttachments.data(),                             // const VkAttachmentReference    * pColorAttachments
//                subpassDescription.resolveAttachments.data(),                           // const VkAttachmentReference    * pResolveAttachments
//                subpassDescription.depthStencilAttachment,                              // const VkAttachmentReference    * pDepthStencilAttachment
//                static_cast<uint32_t>(subpassDescription.preserveAttachments.size()),   // uint32_t                         preserveAttachmentCount
//                subpassDescription.preserveAttachments.data()                           // const uint32_t                 * pPreserveAttachments
//            });
//        }
//    }
//
//    bool CreateRenderPass(VkDevice _logicalDevice, std::vector<VkAttachmentDescription> const& _attachmentsDescriptions, std::vector<SubpassParameters> const& _subpassParameters,
//        std::vector<VkSubpassDependency> const& _subpassDependencies, VkRenderPass& _renderPass)
//	{
//        std::vector<VkSubpassDescription> subpassDescriptions;
//        SpecifySubpassDescriptions(_subpassParameters, subpassDescriptions);
//
//        VkRenderPassCreateInfo renderPassCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,                // VkStructureType                    sType
//            nullptr,                                                  // const void                       * pNext
//            0,                                                        // VkRenderPassCreateFlags            flags
//            static_cast<uint32_t>(_attachmentsDescriptions.size()),   // uint32_t                           attachmentCount
//            _attachmentsDescriptions.data(),                          // const VkAttachmentDescription    * pAttachments
//            static_cast<uint32_t>(subpassDescriptions.size()),        // uint32_t                           subpassCount
//            subpassDescriptions.data(),                               // const VkSubpassDescription       * pSubpasses
//            static_cast<uint32_t>(_subpassDependencies.size()),       // uint32_t                           dependencyCount
//            _subpassDependencies.data()                               // const VkSubpassDependency        * pDependencies
//        };
//
//        if (vkCreateRenderPass(_logicalDevice, &renderPassCreateInfo, nullptr, &_renderPass) != VK_SUCCESS)
//        {
//            std::cout << "Could not create a render pass." << std::endl;
//            return false;
//        }
//
//        return true;
//	}
//
//    void BeginRenderPass(VkCommandBuffer _commandBuffer, VkRenderPass _renderPass, VkFramebuffer _framebuffer, VkRect2D _renderArea, std::vector<VkClearValue> const& _clearValues, VkSubpassContents _subpassContents)
//    {
//        VkRenderPassBeginInfo renderPassBeginInfo = 
//        {
//            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,       // VkStructureType        sType
//            nullptr,                                        // const void           * pNext
//            _renderPass,                                    // VkRenderPass           renderPass
//            _framebuffer,                                   // VkFramebuffer          framebuffer
//            _renderArea,                                    // VkRect2D               renderArea
//            static_cast<uint32_t>(_clearValues.size()),     // uint32_t               clearValueCount
//            _clearValues.data()                             // const VkClearValue   * pClearValues
//        };
//
//        vkCmdBeginRenderPass(_commandBuffer, &renderPassBeginInfo, _subpassContents);
//    }
//
//    void EndRenderPass(VkCommandBuffer _commandBuffer)
//    {
//        vkCmdEndRenderPass(_commandBuffer);
//    }
//
//    bool CreateFramebuffer(VkDevice _logicalDevice, VkRenderPass _renderPass, std::vector<VkImageView> const& _attachments, uint32_t _width, uint32_t _height,
//        uint32_t _layers, VkFramebuffer& _framebuffer)
//    {
//        VkFramebufferCreateInfo framebufferCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,      // VkStructureType              sType
//            nullptr,                                        // const void                 * pNext
//            0,                                              // VkFramebufferCreateFlags     flags
//            _renderPass,                                    // VkRenderPass                 renderPass
//            static_cast<uint32_t>(_attachments.size()),     // uint32_t                     attachmentCount
//            _attachments.data(),                            // const VkImageView          * pAttachments
//            _width,                                         // uint32_t                     width
//            _height,                                        // uint32_t                     height
//            _layers                                         // uint32_t                     layers
//        };
//
//        if (vkCreateFramebuffer(_logicalDevice, &framebufferCreateInfo, nullptr, &_framebuffer) != VK_SUCCESS) 
//        {
//            std::cout << "Could not create a framebuffer." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool CreateImage(VkDevice _logicalDevice, VkImageType _type, VkFormat _format, VkExtent3D _size, uint32_t _numMipmaps, uint32_t _numLayers, VkSampleCountFlagBits _samples,
//        VkImageUsageFlags _usageScenarios, bool _cubemap, VkImage& _image)
//    {
//        VkImageCreateInfo imageCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                    // VkStructureType          sType
//            nullptr,                                                // const void             * pNext
//            _cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,    // VkImageCreateFlags       flags
//            _type,                                                  // VkImageType              imageType
//            _format,                                                // VkFormat                 format
//            _size,                                                  // VkExtent3D               extent
//            _numMipmaps,                                            // uint32_t                 mipLevels
//            _cubemap ? 6 * _numLayers : _numLayers,                 // uint32_t                 arrayLayers
//            _samples,                                               // VkSampleCountFlagBits    samples
//            VK_IMAGE_TILING_OPTIMAL,                                // VkImageTiling            tiling
//            _usageScenarios,                                        // VkImageUsageFlags        usage
//            VK_SHARING_MODE_EXCLUSIVE,                              // VkSharingMode            sharingMode
//            0,                                                      // uint32_t                 queueFamilyIndexCount
//            nullptr,                                                // const uint32_t         * pQueueFamilyIndices
//            VK_IMAGE_LAYOUT_UNDEFINED                               // VkImageLayout            initialLayout
//        };
//
//        if (vkCreateImage(_logicalDevice, &imageCreateInfo, nullptr, &_image) != VK_SUCCESS)
//        {
//            std::cout << "Could not create an image." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool CreateImageView(VkDevice _logicalDevice, VkImage _image, VkImageViewType _viewType, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _imageView)
//    {
//        VkImageViewCreateInfo imageViewCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,       // VkStructureType            sType
//            nullptr,                                        // const void               * pNext
//            0,                                              // VkImageViewCreateFlags     flags
//            _image,                                         // VkImage                    image
//            _viewType,                                      // VkImageViewType            viewType
//            _format,                                        // VkFormat                   format
//            {                                               // VkComponentMapping         components
//              VK_COMPONENT_SWIZZLE_IDENTITY,                // VkComponentSwizzle         r
//              VK_COMPONENT_SWIZZLE_IDENTITY,                // VkComponentSwizzle         g
//              VK_COMPONENT_SWIZZLE_IDENTITY,                // VkComponentSwizzle         b
//              VK_COMPONENT_SWIZZLE_IDENTITY                 // VkComponentSwizzle         a
//            },
//            {                                               // VkImageSubresourceRange    subresourceRange
//              _aspect,                                      // VkImageAspectFlags         aspectMask
//              0,                                            // uint32_t                   baseMipLevel
//              VK_REMAINING_MIP_LEVELS,                      // uint32_t                   levelCount
//              0,                                            // uint32_t                   baseArrayLayer
//              VK_REMAINING_ARRAY_LAYERS                     // uint32_t                   layerCount
//            }
//        };
//
//        if (vkCreateImageView(_logicalDevice, &imageViewCreateInfo, nullptr, &_imageView) != VK_SUCCESS)
//        {
//            std::cout << "Could not create an image view." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool Create2DImageAndView(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkFormat _format, VkExtent2D _size, uint32_t _numMipmaps, uint32_t _numLayers,
//        VkSampleCountFlagBits _samples, VkImageUsageFlags _usage, VkImageAspectFlags _aspect, VkImage& _image, VkDeviceMemory& _memoryObject, VkImageView& _imageView)
//    {
//        if (!CreateImage(_logicalDevice, VK_IMAGE_TYPE_2D, _format, { _size.width, _size.height, 1 }, _numMipmaps, _numLayers, _samples, _usage, false, _image))
//            return false;
//
//        if (!AllocateAndBindMemoryObjectToImage(_physicalDevice, _logicalDevice, _image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _memoryObject))
//            return false;
//
//        if (!CreateImageView(_logicalDevice, _image, VK_IMAGE_VIEW_TYPE_2D, _format, _aspect, _imageView))
//            return false;
//
//        return true;
//    }
//
//    bool CreateLayered2DImageWithCubemapView(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, uint32_t _size, uint32_t _numMipmaps, VkImageUsageFlags _usage,
//        VkImageAspectFlags _aspect, VkImage& _image, VkDeviceMemory& _memoryObject, VkImageView& _imageView)
//    {
//        if (!CreateImage(_logicalDevice, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, { _size, _size, 1 }, _numMipmaps, 6, VK_SAMPLE_COUNT_1_BIT, _usage, true, _image))
//            return false;
//        
//        if (!AllocateAndBindMemoryObjectToImage(_physicalDevice, _logicalDevice, _image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _memoryObject))
//            return false;
//
//        if (!CreateImageView(_logicalDevice, _image, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM, _aspect, _imageView))
//            return false;
//
//        return true;
//    }
//
//    bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkImage _image, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject)
//    {
//        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
//        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &physicalDeviceMemoryProperties);
//
//        VkMemoryRequirements memoryRequirements;
//        vkGetImageMemoryRequirements(_logicalDevice, _image, &memoryRequirements);
//
//        _memoryObject = VK_NULL_HANDLE;
//        for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type)
//        {
//            if ((memoryRequirements.memoryTypeBits & (1 << type)) &&
//                ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & _memoryProperties) == _memoryProperties))
//            {
//
//                VkMemoryAllocateInfo imageMemoryAllocateInfo = 
//                {
//                  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
//                  nullptr,                                  // const void       * pNext
//                  memoryRequirements.size,                 // VkDeviceSize       allocationSize
//                  type                                      // uint32_t           memoryTypeIndex
//                };
//
//                if (vkAllocateMemory(_logicalDevice, &imageMemoryAllocateInfo, nullptr, &_memoryObject) == VK_SUCCESS)
//                    break;
//            }
//        }
//
//        if (VK_NULL_HANDLE == _memoryObject) 
//        {
//            std::cout << "Could not allocate memory for an image." << std::endl;
//            return false;
//        }
//
//        if (vkBindImageMemory(_logicalDevice, _image, _memoryObject, 0) != VK_SUCCESS)
//        {
//            std::cout << "Could not bind memory object to an image." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool CreateBuffer(VkDevice _logicalDevice, VkDeviceSize _size, VkBufferUsageFlags _usage, VkBuffer& _buffer)
//    {
//        VkBufferCreateInfo bufferCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,       // VkStructureType        sType
//            nullptr,                                    // const void           * pNext
//            0,                                          // VkBufferCreateFlags    flags
//            _size,                                      // VkDeviceSize           size
//            _usage,                                     // VkBufferUsageFlags     usage
//            VK_SHARING_MODE_EXCLUSIVE,                  // VkSharingMode          sharingMode
//            0,                                          // uint32_t               queueFamilyIndexCount
//            nullptr                                     // const uint32_t       * pQueueFamilyIndices
//        };
//
//        if (vkCreateBuffer(_logicalDevice, &bufferCreateInfo, nullptr, &_buffer) != VK_SUCCESS)
//        {
//            std::cout << "Could not create a buffer." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkBuffer _buffer, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject)
//    {
//        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
//        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &physicalDeviceMemoryProperties);
//
//        VkMemoryRequirements memoryRequirements;
//        vkGetBufferMemoryRequirements(_logicalDevice, _buffer, &memoryRequirements);
//
//        _memoryObject = VK_NULL_HANDLE;
//        for (uint32_t type = 0; type < physicalDeviceMemoryProperties.memoryTypeCount; ++type) 
//        {
//            if ((memoryRequirements.memoryTypeBits & (1 << type)) &&
//                ((physicalDeviceMemoryProperties.memoryTypes[type].propertyFlags & _memoryProperties) == _memoryProperties)) 
//            {
//                VkMemoryAllocateInfo bufferMemoryAllocateInfo = 
//                {
//                  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
//                  nullptr,                                  // const void       * pNext
//                  memoryRequirements.size,                  // VkDeviceSize       allocationSize
//                  type                                      // uint32_t           memoryTypeIndex
//                };
//                
//                if (vkAllocateMemory(_logicalDevice, &bufferMemoryAllocateInfo, nullptr, &_memoryObject) == VK_SUCCESS)
//                    break;
//            }
//        }
//
//        if (VK_NULL_HANDLE == _memoryObject) 
//        {
//            std::cout << "Could not allocate memory for a buffer." << std::endl;
//            return false;
//        }
//
//        if (vkBindBufferMemory(_logicalDevice, _buffer, _memoryObject, 0) != VK_SUCCESS)
//        {
//            std::cout << "Could not bind memory object to a buffer." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    void SetBufferMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<BufferTransition> _bufferTransitions)
//    {
//        std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
//
//        for (auto& bufferTransition : _bufferTransitions)
//        {
//            bufferMemoryBarriers.push_back({
//              VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,      // VkStructureType    sType
//              nullptr,                                      // const void       * pNext
//              bufferTransition.currentAccess,               // VkAccessFlags      srcAccessMask
//              bufferTransition.newAccess,                   // VkAccessFlags      dstAccessMask
//              bufferTransition.currentQueueFamily,          // uint32_t           srcQueueFamilyIndex
//              bufferTransition.newQueueFamily,              // uint32_t           dstQueueFamilyIndex
//              bufferTransition.buffer,                      // VkBuffer           buffer
//              0,                                            // VkDeviceSize       offset
//              VK_WHOLE_SIZE                                 // VkDeviceSize       size
//            });
//        }
//
//        if (bufferMemoryBarriers.size() > 0)
//            vkCmdPipelineBarrier(_commandBuffer, _generatingStages, _consumingStages, 0, 0, nullptr, static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(), 0, nullptr);
//    }
//
//    bool CreateBufferView(VkDevice _logicalDevice, VkBuffer _buffer, VkFormat _format, VkDeviceSize _memoryOffset, VkDeviceSize _memoryRange, VkBufferView& _bufferView)
//    {
//        VkBufferViewCreateInfo bufferViewCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,      // VkStructureType            sType
//            nullptr,                                        // const void               * pNext
//            0,                                              // VkBufferViewCreateFlags    flags
//            _buffer,                                        // VkBuffer                   buffer
//            _format,                                        // VkFormat                   format
//            _memoryOffset,                                  // VkDeviceSize               offset
//            _memoryRange                                    // VkDeviceSize               range
//        };
//
//        if (vkCreateBufferView(_logicalDevice, &bufferViewCreateInfo, nullptr, &_bufferView) != VK_SUCCESS)
//        {
//            std::cout << "Could not creat buffer view." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    void CopyDataBetweenBuffers(VkCommandBuffer _commandBuffer, VkBuffer _sourceBuffer, VkBuffer _destinationBuffer, std::vector<VkBufferCopy> _regions)
//    {
//        if (_regions.size() > 0)
//            vkCmdCopyBuffer(_commandBuffer, _sourceBuffer, _destinationBuffer, static_cast<uint32_t>(_regions.size()), _regions.data());
//    }
//
//    void CopyDataFromBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _sourceBuffer, VkImage _destinationImage, VkImageLayout _imageLayout, std::vector<VkBufferImageCopy> _regions)
//    {
//        if (_regions.size() > 0)
//            vkCmdCopyBufferToImage(_commandBuffer, _sourceBuffer, _destinationImage, _imageLayout, static_cast<uint32_t>(_regions.size()), _regions.data());
//    }
//
//    void CopyDataFromImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _sourceImage, VkImageLayout _imageLayout, VkBuffer _destinationBuffer, std::vector<VkBufferImageCopy> _regions)
//    {
//        if (_regions.size() > 0)
//            vkCmdCopyImageToBuffer(_commandBuffer, _sourceImage, _imageLayout, _destinationBuffer, static_cast<uint32_t>(_regions.size()), _regions.data());
//    }
//
//    bool UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _dataSize, void* _data, VkBuffer _destinationBuffer,
//        VkDeviceSize _destinationOffset, VkAccessFlags _destinationBufferCurrentAccess, VkAccessFlags _destinationBufferNewAccess, VkPipelineStageFlags _destinationBufferGeneratingStages,
//        VkPipelineStageFlags _destinationBufferConsumingStages, VkQueue _queue, VkCommandBuffer _commandBuffer, std::vector<VkSemaphore> _signalSemaphores)
//    {
//        VkBuffer stagingBuffer;
//        if (!CreateBuffer(_logicalDevice, _dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer)) 
//            return false;
//
//        VkDeviceMemory memoryObject;
//        if (!AllocateAndBindMemoryObjectToBuffer(_physicalDevice, _logicalDevice, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryObject))
//            return false;
//
//        if (!MapUpdateAndUnmapHostVisibleMemory(_logicalDevice, memoryObject, 0, _dataSize, _data, true, nullptr))
//            return false;
//
//        if (!BeginCommandBufferRecordingOperation(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr))
//            return false;
//
//        SetBufferMemoryBarrier(_commandBuffer, _destinationBufferGeneratingStages, VK_PIPELINE_STAGE_TRANSFER_BIT, { { _destinationBuffer, _destinationBufferCurrentAccess, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });
//
//        CopyDataBetweenBuffers(_commandBuffer, stagingBuffer, _destinationBuffer, { { 0, _destinationOffset, _dataSize } });
//
//        SetBufferMemoryBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, _destinationBufferConsumingStages, { { _destinationBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, _destinationBufferNewAccess, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });
//
//        if (!EndCommandBufferRecordingOperation(_commandBuffer))
//            return false;
//
//        VkFence fence;
//        if (!CreateFence(_logicalDevice, false, fence))
//            return false;
//
//        if (!SubmitCommandBuffersToQueue(_queue, {}, { _commandBuffer }, _signalSemaphores, fence))
//            return false;
//
//        if (!WaitForFences(_logicalDevice, { fence }, VK_FALSE, 500000000))
//            return false;
//
//        return true;
//    }
//
//    bool UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _dataSize, void* _data, VkImage _destinationImage,
//        VkImageSubresourceLayers _destinationImageSubresource, VkOffset3D _destinationImageOffset, VkExtent3D _destinationImageSize, VkImageLayout _destinationImageCurrentLayout,
//        VkImageLayout _destinationImageNewLayout, VkAccessFlags _destinationImageCurrentAccess, VkAccessFlags _destinationImageNewAccess, VkImageAspectFlags _destinationImageAspect,
//        VkPipelineStageFlags _destinationImageGeneratingStages, VkPipelineStageFlags _destinationImageConsumingStages, VkQueue _queue, VkCommandBuffer _commandBuffer, std::vector<VkSemaphore> _signalSemaphores)
//    {
//        VkBuffer stagingBuffer;
//        if (!CreateBuffer(_logicalDevice, _dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer))
//            return false;
//
//        VkDeviceMemory memoryObject;
//        if (!AllocateAndBindMemoryObjectToBuffer(_physicalDevice, _logicalDevice, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryObject))
//            return false;
//
//        if (!MapUpdateAndUnmapHostVisibleMemory(_logicalDevice, memoryObject, 0, _dataSize, _data, true, nullptr))
//            return false;
//
//        if (!BeginCommandBufferRecordingOperation(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr))
//            return false;
//
//        SetImageMemoryBarrier(_commandBuffer, _destinationImageGeneratingStages, VK_PIPELINE_STAGE_TRANSFER_BIT,
//            {
//                {
//                  _destinationImage,                            // VkImage            Image
//                  _destinationImageCurrentAccess,               // VkAccessFlags      CurrentAccess
//                  VK_ACCESS_TRANSFER_WRITE_BIT,                 // VkAccessFlags      NewAccess
//                  _destinationImageCurrentLayout,               // VkImageLayout      CurrentLayout
//                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,         // VkImageLayout      NewLayout
//                  VK_QUEUE_FAMILY_IGNORED,                      // uint32_t           CurrentQueueFamily
//                  VK_QUEUE_FAMILY_IGNORED,                      // uint32_t           NewQueueFamily
//                  _destinationImageAspect                       // VkImageAspectFlags Aspect
//                } 
//            });
//
//        CopyDataFromBufferToImage(_commandBuffer, stagingBuffer, _destinationImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//            {
//                {
//                  0,                                        // VkDeviceSize               bufferOffset
//                  0,                                        // uint32_t                   bufferRowLength
//                  0,                                        // uint32_t                   bufferImageHeight
//                  _destinationImageSubresource,             // VkImageSubresourceLayers   imageSubresource
//                  _destinationImageOffset,                  // VkOffset3D                 imageOffset
//                  _destinationImageSize,                    // VkExtent3D                 imageExtent
//                } 
//            });
//
//        SetImageMemoryBarrier(_commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, _destinationImageConsumingStages,
//            {
//                {
//                  _destinationImage,                        // VkImage            Image
//                  VK_ACCESS_TRANSFER_WRITE_BIT,             // VkAccessFlags      CurrentAccess
//                  _destinationImageNewAccess,             // VkAccessFlags      NewAccess
//                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     // VkImageLayout      CurrentLayout
//                  _destinationImageNewLayout,             // VkImageLayout      NewLayout
//                  VK_QUEUE_FAMILY_IGNORED,                  // uint32_t           CurrentQueueFamily
//                  VK_QUEUE_FAMILY_IGNORED,                  // uint32_t           NewQueueFamily
//                  _destinationImageAspect                  // VkImageAspectFlags Aspect
//                } 
//            });
//
//        if (!EndCommandBufferRecordingOperation(_commandBuffer))
//            return false;
//
//        VkFence fence;
//        if (!CreateFence(_logicalDevice, false, fence))
//            return false;
//
//        if (!SubmitCommandBuffersToQueue(_queue, {}, { _commandBuffer }, _signalSemaphores, fence))
//            return false;
//
//        if (!WaitForFences(_logicalDevice, { fence }, VK_FALSE, 500000000))
//            return false;
//
//        return true;
//    }
//
//    void DrawGeometry(VkCommandBuffer _commandBuffer, uint32_t _vertexCount, uint32_t _instanceCount, uint32_t _firstVertex, uint32_t _firstInstance)
//    {
//        vkCmdDraw(_commandBuffer, _vertexCount, _instanceCount, _firstVertex, _firstInstance);
//    }
//
//    void BindVertexBuffers(VkCommandBuffer _commandBuffer, uint32_t _firstBinding, std::vector<VertexBufferParameters> const& _buffersParameters)
//    {
//        if (_buffersParameters.size() > 0)
//        {
//            std::vector<VkBuffer> buffers;
//            std::vector<VkDeviceSize> offsets;
//            for (auto& bufferParameters : _buffersParameters)
//            {
//                buffers.push_back(bufferParameters.buffer);
//                offsets.push_back(bufferParameters.memoryOffset);
//            }
//            vkCmdBindVertexBuffers(_commandBuffer, _firstBinding, static_cast<uint32_t>(_buffersParameters.size()), buffers.data(), offsets.data());
//        }
//    }
//
//    bool CreateShaderModule(VkDevice _logicalDevice, std::vector<unsigned char> const& _sourceCode, VkShaderModule& _shaderModule)
//    {
//        VkShaderModuleCreateInfo shaderModuleCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,                // VkStructureType              sType
//            nullptr,                                                    // const void                 * pNext
//            0,                                                          // VkShaderModuleCreateFlags    flags
//            _sourceCode.size(),                                         // size_t                       codeSize
//            reinterpret_cast<uint32_t const*>(_sourceCode.data())       // const uint32_t             * pCode
//        };
//
//        if (vkCreateShaderModule(_logicalDevice, &shaderModuleCreateInfo, nullptr, &_shaderModule) != VK_SUCCESS)
//        {
//            std::cout << "Could not create a shader module." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    void SpecifyPipelineShaderStages(std::vector<ShaderStageParameters> const& _shaderStageParams, std::vector<VkPipelineShaderStageCreateInfo>& _shaderStageCreateInfos)
//    {
//        _shaderStageCreateInfos.clear();
//
//        for (auto& shaderStage : _shaderStageParams)
//        {
//            _shaderStageCreateInfos.push_back
//            ({
//                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,  // VkStructureType                    sType
//                nullptr,                                              // const void                       * pNext
//                0,                                                    // VkPipelineShaderStageCreateFlags   flags
//                shaderStage.shaderStage,                             // VkShaderStageFlagBits              stage
//                shaderStage.shaderModule,                            // VkShaderModule                     module
//                shaderStage.entryPointName,                          // const char                       * pName
//                shaderStage.specializationInfo                       // const VkSpecializationInfo       * pSpecializationInfo
//            });
//        }
//    }
//
//    void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const& _bindingDescriptions, std::vector<VkVertexInputAttributeDescription> const& _attributeDescriptions,
//        VkPipelineVertexInputStateCreateInfo& _vertexInputStateCreateInfo)
//    {
//        _vertexInputStateCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                           sType
//            nullptr,                                                      // const void                              * pNext
//            0,                                                            // VkPipelineVertexInputStateCreateFlags     flags
//            static_cast<uint32_t>(_bindingDescriptions.size()),           // uint32_t                                  vertexBindingDescriptionCount
//            _bindingDescriptions.data(),                                  // const VkVertexInputBindingDescription   * pVertexBindingDescriptions
//            static_cast<uint32_t>(_attributeDescriptions.size()),         // uint32_t                                  vertexAttributeDescriptionCount
//            _attributeDescriptions.data()                                 // const VkVertexInputAttributeDescription * pVertexAttributeDescriptions
//        };
//    }
//
//    void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology _topology, bool _primitiveRestartEnable, VkPipelineInputAssemblyStateCreateInfo& _inputAssemblyStateCreateInfo)
//    {
//        _inputAssemblyStateCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    // VkStructureType                           sType
//            nullptr,                                                        // const void                              * pNext
//            0,                                                              // VkPipelineInputAssemblyStateCreateFlags   flags
//            _topology,                                                      // VkPrimitiveTopology                       topology
//            _primitiveRestartEnable                                         // VkBool32                                  primitiveRestartEnable
//        };
//    }
//
//    void SpecifyPipelineViewportAndScissorTestState(ViewportInfo const& _viewportInfos, VkPipelineViewportStateCreateInfo& _viewportStateCreateInfo)
//    {
//        uint32_t viewportCount = static_cast<uint32_t>(_viewportInfos.viewports.size());
//        uint32_t scissorCount = static_cast<uint32_t>(_viewportInfos.scissors.size());
//        
//        _viewportStateCreateInfo =
//        {
//          VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,    // VkStructureType                      sType
//          nullptr,                                                  // const void                         * pNext
//          0,                                                        // VkPipelineViewportStateCreateFlags   flags
//          viewportCount,                                            // uint32_t                             viewportCount
//          _viewportInfos.viewports.data(),                          // const VkViewport                   * pViewports
//          scissorCount,                                             // uint32_t                             scissorCount
//          _viewportInfos.scissors.data()                            // const VkRect2D                     * pScissors
//        };
//    }
//
//    void SpecifyPipelineRasterizationState(bool _depthClampEnable, bool _rasterizerDiscardEnable, VkPolygonMode _polygonMode, VkCullModeFlags _cullingMode, VkFrontFace _frontFace,
//        bool _depthBiasEnable, float _depthBiasConstantFactor, float _depthBiasClamp, float _depthBiasSlopeFactor, float _lineWidth, VkPipelineRasterizationStateCreateInfo& _rasterizationStateCreateInfo)
//    {
//        _rasterizationStateCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                            sType
//            nullptr,                                                    // const void                               * pNext
//            0,                                                          // VkPipelineRasterizationStateCreateFlags    flags
//            _depthClampEnable,                                          // VkBool32                                   depthClampEnable
//            _rasterizerDiscardEnable,                                   // VkBool32                                   rasterizerDiscardEnable
//            _polygonMode,                                               // VkPolygonMode                              polygonMode
//            _cullingMode,                                               // VkCullModeFlags                            cullMode
//            _frontFace,                                                 // VkFrontFace                                frontFace
//            _depthBiasEnable,                                           // VkBool32                                   depthBiasEnable
//            _depthBiasConstantFactor,                                   // float                                      depthBiasConstantFactor
//            _depthBiasClamp,                                            // float                                      depthBiasClamp
//            _depthBiasSlopeFactor,                                      // float                                      depthBiasSlopeFactor
//            _lineWidth                                                  // float                                      lineWidth
//        };
//    }
//
//    void SpecifyPipelineMultisampleState(VkSampleCountFlagBits _sampleCount, bool _perSampleShadingEnable, float _minSampleShading, VkSampleMask const* _sampleMasks,
//        bool _alphaToCoverageEnable, bool _alphaToOneEnable, VkPipelineMultisampleStateCreateInfo& _multisampleStateCreateInfo)
//    {
//        _multisampleStateCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,   // VkStructureType                          sType
//            nullptr,                                                    // const void                             * pNext
//            0,                                                          // VkPipelineMultisampleStateCreateFlags    flags
//            _sampleCount,                                               // VkSampleCountFlagBits                    rasterizationSamples
//            _perSampleShadingEnable,                                    // VkBool32                                 sampleShadingEnable
//            _minSampleShading,                                          // float                                    minSampleShading
//            _sampleMasks,                                               // const VkSampleMask                     * pSampleMask
//            _alphaToCoverageEnable,                                     // VkBool32                                 alphaToCoverageEnable
//            _alphaToOneEnable                                           // VkBool32                                 alphaToOneEnable
//        };
//    }
//
//    void SpecifyPipelineBlendState(bool _logicOpEnable, VkLogicOp _logicOp, std::vector<VkPipelineColorBlendAttachmentState> const& _attachmentBlendStates,
//        std::array<float, 4> const& _blendConstants, VkPipelineColorBlendStateCreateInfo& _blendStateCreateInfo)
//    {
//        _blendStateCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                              sType
//            nullptr,                                                    // const void                                 * pNext
//            0,                                                          // VkPipelineColorBlendStateCreateFlags         flags
//            _logicOpEnable,                                             // VkBool32                                     logicOpEnable
//            _logicOp,                                                   // VkLogicOp                                    logicOp
//            static_cast<uint32_t>(_attachmentBlendStates.size()),       // uint32_t                                     attachmentCount
//            _attachmentBlendStates.data(),                              // const VkPipelineColorBlendAttachmentState  * pAttachments
//            {                                                           // float                                        blendConstants[4]
//              _blendConstants[0],
//              _blendConstants[1],
//              _blendConstants[2],
//              _blendConstants[3]
//            }
//        };
//    }
//
//    void SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const& _dynamicStates, VkPipelineDynamicStateCreateInfo& _dynamicStateCreateInfo)
//    {
//        _dynamicStateCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,     // VkStructureType                      sType
//            nullptr,                                                  // const void                         * pNext
//            0,                                                        // VkPipelineDynamicStateCreateFlags    flags
//            static_cast<uint32_t>(_dynamicStates.size()),             // uint32_t                             dynamicStateCount
//            _dynamicStates.data()                                     // const VkDynamicState               * pDynamicStates
//        };
//    }
//
//    bool CreatePipelineLayout(VkDevice _logicalDevice, std::vector<VkDescriptorSetLayout> const& _descriptorSetLayouts,
//        std::vector<VkPushConstantRange> const& _pushConstantRanges, VkPipelineLayout& _pipelineLayout)
//    {
//        VkPipelineLayoutCreateInfo pipeline_layout_create_info = 
//        {
//            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,          // VkStructureType                  sType
//            nullptr,                                                // const void                     * pNext
//            0,                                                      // VkPipelineLayoutCreateFlags      flags
//            static_cast<uint32_t>(_descriptorSetLayouts.size()),    // uint32_t                         setLayoutCount
//            _descriptorSetLayouts.data(),                           // const VkDescriptorSetLayout    * pSetLayouts
//            static_cast<uint32_t>(_pushConstantRanges.size()),      // uint32_t                         pushConstantRangeCount
//            _pushConstantRanges.data()                              // const VkPushConstantRange      * pPushConstantRanges
//        };
//
//        if (vkCreatePipelineLayout(_logicalDevice, &pipeline_layout_create_info, nullptr, &_pipelineLayout) != VK_SUCCESS)
//        {
//            std::cout << "Could not create pipeline layout." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    void SpecifyGraphicsPipelineCreationParameters(VkPipelineCreateFlags _additionalOptions, std::vector<VkPipelineShaderStageCreateInfo> const& _shaderStageCreateInfos,
//        VkPipelineVertexInputStateCreateInfo const& _vertexInputStateCreateInfo, VkPipelineInputAssemblyStateCreateInfo const& _inputAssemblyStateCreateInfo,
//        VkPipelineTessellationStateCreateInfo const* _tessellationStateCreateInfo, VkPipelineViewportStateCreateInfo const* _viewportStateCreateInfo,
//        VkPipelineRasterizationStateCreateInfo const& _rasterizationStateCreateInfo, VkPipelineMultisampleStateCreateInfo const* _multisampleStateCreateInfo,
//        VkPipelineDepthStencilStateCreateInfo const* _depthAndStencilStateCreateInfo, VkPipelineColorBlendStateCreateInfo const* _blendStateCreateInfo,
//        VkPipelineDynamicStateCreateInfo const* _dynamicStateCreateInfo, VkPipelineLayout _pipelineLayout, VkRenderPass _renderPass, uint32_t _subpass,
//        VkPipeline _basePipelineHandle, int32_t _basePipelineIndex, VkGraphicsPipelineCreateInfo& _graphicsPipelineCreateInfo)
//    {
//        _graphicsPipelineCreateInfo =
//        {
//            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                sType
//            nullptr,                                                    // const void                                   * pNext
//            _additionalOptions,                                         // VkPipelineCreateFlags                          flags
//            static_cast<uint32_t>(_shaderStageCreateInfos.size()),      // uint32_t                                       stageCount
//            _shaderStageCreateInfos.data(),                             // const VkPipelineShaderStageCreateInfo        * pStages
//            &_vertexInputStateCreateInfo,                               // const VkPipelineVertexInputStateCreateInfo   * pVertexInputState
//            &_inputAssemblyStateCreateInfo,                             // const VkPipelineInputAssemblyStateCreateInfo * pInputAssemblyState
//            _tessellationStateCreateInfo,                               // const VkPipelineTessellationStateCreateInfo  * pTessellationState
//            _viewportStateCreateInfo,                                   // const VkPipelineViewportStateCreateInfo      * pViewportState
//            &_rasterizationStateCreateInfo,                             // const VkPipelineRasterizationStateCreateInfo * pRasterizationState
//            _multisampleStateCreateInfo,                                // const VkPipelineMultisampleStateCreateInfo   * pMultisampleState
//            _depthAndStencilStateCreateInfo,                            // const VkPipelineDepthStencilStateCreateInfo  * pDepthStencilState
//            _blendStateCreateInfo,                                      // const VkPipelineColorBlendStateCreateInfo    * pColorBlendState
//            _dynamicStateCreateInfo,                                    // const VkPipelineDynamicStateCreateInfo       * pDynamicState
//            _pipelineLayout,                                            // VkPipelineLayout                               layout
//            _renderPass,                                                // VkRenderPass                                   renderPass
//            _subpass,                                                   // uint32_t                                       subpass
//            _basePipelineHandle,                                        // VkPipeline                                     basePipelineHandle
//            _basePipelineIndex                                          // int32_t                                        basePipelineIndex
//        };
//    }
//
//    bool CreateGraphicsPipelines(VkDevice _logicalDevice, std::vector<VkGraphicsPipelineCreateInfo> const& _graphicsPipelineCreateInfos,
//        VkPipelineCache _pipelineCache, std::vector<VkPipeline>& _graphicsPipelines)
//    {
//        if (_graphicsPipelineCreateInfos.size() > 0)
//        {
//            _graphicsPipelines.resize(_graphicsPipelineCreateInfos.size());
//            if (vkCreateGraphicsPipelines(_logicalDevice, _pipelineCache, static_cast<uint32_t>(_graphicsPipelineCreateInfos.size()), _graphicsPipelineCreateInfos.data(), nullptr, _graphicsPipelines.data()) != VK_SUCCESS)
//            {
//                std::cout << "Could not create a graphics pipeline." << std::endl;
//                return false;
//            }
//
//            return true;
//        }
//
//        return false;
//    }
//
//    void BindPipelineObject(VkCommandBuffer _commandBuffer, VkPipelineBindPoint _pipelineType, VkPipeline _pipeline)
//    {
//        vkCmdBindPipeline(_commandBuffer, _pipelineType, _pipeline);
//    }
//
//    void SetViewportStateDynamically(VkCommandBuffer _commandBuffer, uint32_t _firstViewport, std::vector<VkViewport> const& _viewports)
//    {
//        vkCmdSetViewport(_commandBuffer, _firstViewport, static_cast<uint32_t>(_viewports.size()), _viewports.data());
//    }
//
//    void SetScissorStateDynamically(VkCommandBuffer _commandBuffer, uint32_t _firstScissor, std::vector<VkRect2D> const& _scissors)
//    {
//        vkCmdSetScissor(_commandBuffer, _firstScissor, static_cast<uint32_t>(_scissors.size()), _scissors.data());
//    }
//
//    bool CreateUniformBuffer(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _size, VkBufferUsageFlags _usage, VkBuffer& _uniformBuffer, VkDeviceMemory& _memoryObject)
//    {
//        if (!CreateBuffer(_logicalDevice, _size, _usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, _uniformBuffer))
//            return false;
//
//        if (!AllocateAndBindMemoryObjectToBuffer(_physicalDevice, _logicalDevice, _uniformBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _memoryObject))
//            return false;
//
//        return true;
//    }
//
//    bool UpdateUniformBuffer(SwapchainParameters _swapchain, VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkBuffer _uniformBuffer,
//        QueueParameters _graphicsQueue, std::vector<FrameResources> const& _framesResources, Mesh& _model)
//    {
//        if (!UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice, _logicalDevice, sizeof(_model.modelViewMatrix[0]) * _model.modelViewMatrix.size(),
//            &_model.modelViewMatrix[0], _uniformBuffer, 0, 0, VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
//            _graphicsQueue.handle, _framesResources.front().commandBuffer, {}))
//            return false;
//
//        if (!UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(_physicalDevice, _logicalDevice, sizeof(_model.perspectiveMatrix[0]) * _model.perspectiveMatrix.size(),
//            &_model.perspectiveMatrix[0], _uniformBuffer, 16 * sizeof(float), 0, VK_ACCESS_UNIFORM_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
//            _graphicsQueue.handle, _framesResources.front().commandBuffer, {}))
//            return false;
//
//        return true;
//    }
//
//    void SetImageMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<ImageTransition> _imageTransitions)
//    {
//
//        std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
//
//        for (auto& imageTransition : _imageTransitions) 
//        {
//            imageMemoryBarriers.push_back
//            ({
//                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType
//                nullptr,                                    // const void               * pNext
//                imageTransition.currentAccess,              // VkAccessFlags              srcAccessMask
//                imageTransition.newAccess,                  // VkAccessFlags              dstAccessMask
//                imageTransition.currentLayout,              // VkImageLayout              oldLayout
//                imageTransition.newLayout,                  // VkImageLayout              newLayout
//                imageTransition.currentQueueFamily,         // uint32_t                   srcQueueFamilyIndex
//                imageTransition.newQueueFamily,             // uint32_t                   dstQueueFamilyIndex
//                imageTransition.image,                      // VkImage                    image
//                {                                           // VkImageSubresourceRange    subresourceRange
//                  imageTransition.aspect,                   // VkImageAspectFlags         aspectMask
//                  0,                                        // uint32_t                   baseMipLevel
//                  VK_REMAINING_MIP_LEVELS,                  // uint32_t                   levelCount
//                  0,                                        // uint32_t                   baseArrayLayer
//                  VK_REMAINING_ARRAY_LAYERS                 // uint32_t                   layerCount
//                }
//            });
//        }
//
//        if (imageMemoryBarriers.size() > 0) 
//            vkCmdPipelineBarrier(_commandBuffer, _generatingStages, _consumingStages, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(imageMemoryBarriers.size()), imageMemoryBarriers.data());
//    }
//    bool MapUpdateAndUnmapHostVisibleMemory(VkDevice _logicalDevice, VkDeviceMemory _memoryObject, VkDeviceSize _offset, VkDeviceSize _dataSize,
//        void* _data, bool _unmap, void** _pointer)
//    {
//        void* localPointer;
//
//        if (vkMapMemory(_logicalDevice, _memoryObject, _offset, _dataSize, 0, &localPointer) != VK_SUCCESS)
//        {
//            std::cout << "Could not map memory object." << std::endl;
//            return false;
//        }
//
//        std::memcpy(localPointer, _data, static_cast<size_t>(_dataSize));
//
//        std::vector<VkMappedMemoryRange> memoryRanges = 
//        {
//            {
//              VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,    // VkStructureType    sType
//              nullptr,                                  // const void       * pNext
//              _memoryObject,                            // VkDeviceMemory     memory
//              _offset,                                  // VkDeviceSize       offset
//              VK_WHOLE_SIZE                             // VkDeviceSize       size
//            }
//        };
//
//        if (vkFlushMappedMemoryRanges(_logicalDevice, static_cast<uint32_t>(memoryRanges.size()), memoryRanges.data()) != VK_SUCCESS)
//        {
//            std::cout << "Could not flush mapped memory." << std::endl;
//            return false;
//        }
//
//        if (_unmap) 
//        {
//            vkUnmapMemory(_logicalDevice, _memoryObject);
//        }
//        else if (nullptr != _pointer) 
//        {
//            *_pointer = localPointer;
//        }
//
//        return true;
//    }
//
//    bool CreateDescriptorSetLayout(VkDevice _logicalDevice, std::vector<VkDescriptorSetLayoutBinding> const& _bindings, VkDescriptorSetLayout& _descriptorSetLayout)
//    {
//        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // VkStructureType                      sType
//            nullptr,                                              // const void                         * pNext
//            0,                                                    // VkDescriptorSetLayoutCreateFlags     flags
//            static_cast<uint32_t>(_bindings.size()),               // uint32_t                             bindingCount
//            _bindings.data()                                       // const VkDescriptorSetLayoutBinding * pBindings
//        };
//
//        if (vkCreateDescriptorSetLayout(_logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
//        {
//            std::cout << "Could not create a layout for descriptor sets." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool CreateDescriptorPool(VkDevice _logicalDevice, bool _freeIndividualSets, uint32_t _maxSetsCount, std::vector<VkDescriptorPoolSize> const& _descriptorTypes, VkDescriptorPool& _descriptorPool)
//    {
//        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = 
//        {
//            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,                  // VkStructureType                sType
//            nullptr,                                                        // const void                   * pNext
//            _freeIndividualSets ?                                           // VkDescriptorPoolCreateFlags    flags
//              VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0u,
//            _maxSetsCount,                                                  // uint32_t                       maxSets
//            static_cast<uint32_t>(_descriptorTypes.size()),                 // uint32_t                       poolSizeCount
//            _descriptorTypes.data()                                         // const VkDescriptorPoolSize   * pPoolSizes
//        };
//
//        if (vkCreateDescriptorPool(_logicalDevice, &descriptorPoolCreateInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
//        {
//            std::cout << "Could not create a descriptor pool." << std::endl;
//            return false;
//        }
//
//        return true;
//    }
//
//    bool AllocateDescriptorSets(VkDevice _logicalDevice, VkDescriptorPool _descriptorPool, std::vector<VkDescriptorSetLayout> const& _descriptorSetLayout, std::vector<VkDescriptorSet>& _descriptorSets)
//    {
//        if (_descriptorSetLayout.size() > 0) 
//        {
//            VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = 
//            {
//                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,         // VkStructureType                  sType
//                nullptr,                                                // const void                     * pNext
//                _descriptorPool,                                        // VkDescriptorPool                 descriptorPool
//                static_cast<uint32_t>(_descriptorSetLayout.size()),     // uint32_t                         descriptorSetCount
//                _descriptorSetLayout.data()                             // const VkDescriptorSetLayout    * pSetLayouts
//            };
//
//            _descriptorSets.resize(_descriptorSetLayout.size());
//
//            if (vkAllocateDescriptorSets(_logicalDevice, &descriptorSetAllocateInfo, _descriptorSets.data()) != VK_SUCCESS)
//            {
//                std::cout << "Could not allocate descriptor sets." << std::endl;
//                return false;
//            }
//
//            return true;
//        }
//
//        return false;
//    }
//
//    void UpdateDescriptorSets(VkDevice _logicalDevice, std::vector<ImageDescriptorInfo> const& _imageDescriptorInfos, std::vector<BufferDescriptorInfo> const& _bufferDescriptorInfos,
//        std::vector<TexelBufferDescriptorInfo> const& _texelBufferDescriptorInfos, std::vector<CopyDescriptorInfo> const& _copyDescriptorOnfos)
//    {
//        std::vector<VkWriteDescriptorSet> writeDescriptors;
//        std::vector<VkCopyDescriptorSet> copyDescriptors;
//
//        // image descriptors
//        for (auto& imageDescriptor : _imageDescriptorInfos) 
//        {
//            writeDescriptors.push_back
//            ({
//                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
//                nullptr,                                                                // const void                     * pNext
//                imageDescriptor.targetDescriptorSet,                                    // VkDescriptorSet                  dstSet
//                imageDescriptor.targetDescriptorBinding,                                // uint32_t                         dstBinding
//                imageDescriptor.targetArrayElement,                                     // uint32_t                         dstArrayElement
//                static_cast<uint32_t>(imageDescriptor.imageInfos.size()),               // uint32_t                         descriptorCount
//                imageDescriptor.targetDescriptorType,                                   // VkDescriptorType                 descriptorType
//                imageDescriptor.imageInfos.data(),                                      // const VkDescriptorImageInfo    * pImageInfo
//                nullptr,                                                                // const VkDescriptorBufferInfo   * pBufferInfo
//                nullptr                                                                 // const VkBufferView             * pTexelBufferView
//            });
//        }
//
//        // buffer descriptors
//        for (auto& bufferDescriptor : _bufferDescriptorInfos) 
//        {
//            writeDescriptors.push_back
//            ({
//                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                 // VkStructureType                  sType
//                nullptr,                                                                // const void                     * pNext
//                bufferDescriptor.targetDescriptorSet,                                   // VkDescriptorSet                  dstSet
//                bufferDescriptor.targetDescriptorBinding,                               // uint32_t                         dstBinding
//                bufferDescriptor.targetArrayElement,                                    // uint32_t                         dstArrayElement
//                static_cast<uint32_t>(bufferDescriptor.bufferInfos.size()),             // uint32_t                         descriptorCount
//                bufferDescriptor.targetDescriptorType,                                  // VkDescriptorType                 descriptorType
//                nullptr,                                                                // const VkDescriptorImageInfo    * pImageInfo
//                bufferDescriptor.bufferInfos.data(),                                    // const VkDescriptorBufferInfo   * pBufferInfo
//                nullptr                                                                 // const VkBufferView             * pTexelBufferView
//            });
//        }
//
//        // texel buffer descriptors
//        for (auto& texelBufferDescriptor : _texelBufferDescriptorInfos) 
//        {
//            writeDescriptors.push_back
//            ({
//              VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,                                   // VkStructureType                  sType
//              nullptr,                                                                  // const void                     * pNext
//              texelBufferDescriptor.targetDescriptorSet,                                // VkDescriptorSet                  dstSet
//              texelBufferDescriptor.targetDescriptorBinding,                            // uint32_t                         dstBinding
//              texelBufferDescriptor.targetArrayElement,                                 // uint32_t                         dstArrayElement
//              static_cast<uint32_t>(texelBufferDescriptor.texelBufferViews.size()),     // uint32_t                         descriptorCount
//              texelBufferDescriptor.targetDescriptorType,                               // VkDescriptorType                 descriptorType
//              nullptr,                                                                  // const VkDescriptorImageInfo    * pImageInfo
//              nullptr,                                                                  // const VkDescriptorBufferInfo   * pBufferInfo
//              texelBufferDescriptor.texelBufferViews.data()                             // const VkBufferView             * pTexelBufferView
//            });
//        }
//
//        // copy descriptors
//        for (auto& copyDescriptor : _copyDescriptorOnfos) 
//        {
//            copyDescriptors.push_back
//            ({
//                VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,                                  // VkStructureType    sType
//                nullptr,                                                                // const void       * pNext
//                copyDescriptor.sourceDescriptorSet,                                    // VkDescriptorSet    srcSet
//                copyDescriptor.sourceDescriptorBinding,                                // uint32_t           srcBinding
//                copyDescriptor.sourceArrayElement,                                     // uint32_t           srcArrayElement
//                copyDescriptor.targetDescriptorSet,                                    // VkDescriptorSet    dstSet
//                copyDescriptor.targetDescriptorBinding,                                // uint32_t           dstBinding
//                copyDescriptor.targetArrayElement,                                     // uint32_t           dstArrayElement
//                copyDescriptor.descriptorCount                                         // uint32_t           descriptorCount
//            });
//        }
//
//        vkUpdateDescriptorSets(_logicalDevice, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), static_cast<uint32_t>(copyDescriptors.size()), copyDescriptors.data());
//    }
//    void BindDescriptorSets(VkCommandBuffer _commandBuffer, VkPipelineBindPoint _pipelineType, VkPipelineLayout _pipelineLayout, uint32_t _indexForFirstSet,
//        std::vector<VkDescriptorSet> const& _descriptorSets, std::vector<uint32_t> const& _dynamicOffsets)
//    {
//        vkCmdBindDescriptorSets(_commandBuffer, _pipelineType, _pipelineLayout, _indexForFirstSet,
//            static_cast<uint32_t>(_descriptorSets.size()), _descriptorSets.data(),
//            static_cast<uint32_t>(_dynamicOffsets.size()), _dynamicOffsets.data());
//    }
//
//
//    bool AllocateMemory(VkDevice _logicalDevice, Buffer& _buffer, uint32_t _size, uint32_t _type)
//    {
//        VkMemoryRequirements memoryRequirements;
//        vkGetBufferMemoryRequirements(_logicalDevice, _buffer.buffer, &memoryRequirements);
//
//        VkMemoryAllocateInfo imageMemoryAllocateInfo =
//        {
//          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
//          nullptr,                                  // const void       * pNext
//          memoryRequirements.size,                  // VkDeviceSize       allocationSize
//          _type                                     // uint32_t           memoryTypeIndex
//        };
//
//        if (vkAllocateMemory(_logicalDevice, &imageMemoryAllocateInfo, nullptr, &_buffer.memory) == VK_SUCCESS)
//            return false;
//
//        _buffer.alignment = memoryRequirements.alignment;
//        _buffer.size = _size;
//        _buffer.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//        _buffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
//
//        return true;
//    }
//
//    bool CreateGraphicsPipeline(VkDevice _logicalDevice, VkPipelineCache _pipelineCache, VkPipelineLayout _pipelineLayout, VkRenderPass _renderPass, VkPipeline& _pipeline)
//    {
//        //VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
//        //inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//        //inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//        //inputAssemblyState.flags = 0;
//        //inputAssemblyState.primitiveRestartEnable = VK_FALSE;
//
//        //VkPipelineRasterizationStateCreateInfo rasterizationState;
//        //rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//        //rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
//        //rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
//        //rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//        //rasterizationState.flags = 0;
//        //rasterizationState.depthClampEnable = VK_FALSE;
//        //rasterizationState.lineWidth = 1.0f;
//
//        //VkPipelineColorBlendAttachmentState blendAttachmentState;
//        //blendAttachmentState.colorWriteMask = 0xf;
//        //blendAttachmentState.blendEnable = VK_FALSE;
//
//        //VkPipelineColorBlendStateCreateInfo colorBlendState;
//        //colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//        //colorBlendState.attachmentCount = 1;
//        //colorBlendState.pAttachments = &blendAttachmentState;
//
//        //VkPipelineDepthStencilStateCreateInfo depthStencilState;
//        //depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//        //depthStencilState.depthTestEnable = VK_TRUE;
//        //depthStencilState.depthWriteEnable = VK_TRUE;
//        //depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//        //depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
//
//        //VkPipelineViewportStateCreateInfo viewportState;
//        //viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//        //viewportState.viewportCount = 1;
//        //viewportState.scissorCount = 1;
//        //viewportState.flags = 0;
//
//        //VkPipelineMultisampleStateCreateInfo multisampleState;
//        //multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//        //multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//        //multisampleState.flags = 0;
//
//        //std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
//        //VkPipelineDynamicStateCreateInfo dynamicState;
//        //dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//        //dynamicState.pDynamicStates = dynamicStateEnables.data();
//        //dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
//        //dynamicState.flags = 0;
//
//        //std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions =
//        //{
//        //    {
//        //      0,                            // uint32_t                     binding
//        //      6 * sizeof(float),            // uint32_t                     stride
//        //      VK_VERTEX_INPUT_RATE_VERTEX   // VkVertexInputRate            inputRate
//        //    }
//        //};
//
//        //std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
//        //{
//        //  {
//        //    0,                                                                        // uint32_t   location
//        //    0,                                                                        // uint32_t   binding
//        //    VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
//        //    0                                                                         // uint32_t   offset
//        //  },
//        //  {
//        //    1,                                                                        // uint32_t   location
//        //    0,                                                                        // uint32_t   binding
//        //    VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
//        //    3 * sizeof(float)                                                         // uint32_t   offset
//        //  }
//        //};
//
//        //VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
//        //SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, vertexAttributeDescriptions, vertexInputStateCreateInfo);
//
//        //std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
//
//        //VkGraphicsPipelineCreateInfo pipelineCI;
//        //pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//        //pipelineCI.layout = _pipelineLayout;
//        //pipelineCI.renderPass = _renderPass;
//        //pipelineCI.flags = 0;
//        //pipelineCI.basePipelineIndex = -1;
//        //pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
//        //pipelineCI.pInputAssemblyState = &inputAssemblyState;
//        //pipelineCI.pRasterizationState = &rasterizationState;
//        //pipelineCI.pColorBlendState = &colorBlendState;
//        //pipelineCI.pMultisampleState = &multisampleState;
//        //pipelineCI.pViewportState = &viewportState;
//        //pipelineCI.pDepthStencilState = &depthStencilState;
//        //pipelineCI.pDynamicState = &dynamicState;
//        //pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
//        //pipelineCI.pStages = shaderStages.data();
//        //pipelineCI.pVertexInputState = &vertexInputStateCreateInfo;
//
//        //// Create the graphics pipeline state objects
//
//        //// We are using this pipeline as the base for the other pipelines (derivatives)
//        //// Pipeline derivatives can be used for pipelines that share most of their state
//        //// Depending on the implementation this may result in better performance for pipeline
//        //// switching and faster creation time
//        //pipelineCI.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
//
//        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions =
//        {
//            {
//              0,                            // uint32_t                     binding
//              6 * sizeof(float),            // uint32_t                     stride
//              VK_VERTEX_INPUT_RATE_VERTEX   // VkVertexInputRate            inputRate
//            }
//        };
//        
//        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions =
//        {
//          {
//            0,                                                                        // uint32_t   location
//            0,                                                                        // uint32_t   binding
//            VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
//            0                                                                         // uint32_t   offset
//          },
//          {
//            1,                                                                        // uint32_t   location
//            0,                                                                        // uint32_t   binding
//            VK_FORMAT_R32G32B32_SFLOAT,                                               // VkFormat   format
//            3 * sizeof(float)                                                         // uint32_t   offset
//          }
//        };
//        
//        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
//        SpecifyPipelineVertexInputState(vertexInputBindingDescriptions, vertexAttributeDescriptions, vertexInputStateCreateInfo);
//        
//        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
//        VkPipelineRasterizationStateCreateInfo rasterizationState = pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
//        VkPipelineColorBlendAttachmentState blendAttachmentState = pipelineColorBlendAttachmentState(0xf, VK_FALSE);
//        VkPipelineColorBlendStateCreateInfo colorBlendState = pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
//        VkPipelineDepthStencilStateCreateInfo depthStencilState = pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
//        VkPipelineViewportStateCreateInfo viewportState = pipelineViewportStateCreateInfo(1, 1, 0);
//        VkPipelineMultisampleStateCreateInfo multisampleState = pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
//        std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH, };
//        VkPipelineDynamicStateCreateInfo dynamicState = pipelineDynamicStateCreateInfo(dynamicStateEnables);
//        
//        VkGraphicsPipelineCreateInfo pipelineCI = pipelineCreateInfo(_pipelineLayout, _renderPass);
//        
//        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
//        
//        pipelineCI.pInputAssemblyState = &inputAssemblyState;
//        pipelineCI.pRasterizationState = &rasterizationState;
//        pipelineCI.pColorBlendState = &colorBlendState;
//        pipelineCI.pMultisampleState = &multisampleState;
//        pipelineCI.pViewportState = &viewportState;
//        pipelineCI.pDepthStencilState = &depthStencilState;
//        pipelineCI.pDynamicState = &dynamicState;
//        pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
//        pipelineCI.pStages = shaderStages.data();
//        pipelineCI.pVertexInputState = &vertexInputStateCreateInfo;
//        
//        // Create the graphics pipeline state objects
//        
//        // We are using this pipeline as the base for the other pipelines (derivatives)
//        // Pipeline derivatives can be used for pipelines that share most of their state
//        // Depending on the implementation this may result in better performance for pipeline
//        // switching and faster creation time
//        pipelineCI.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
//        
//        
//        shaderStages[0] = LoadShader("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, _logicalDevice);
//        shaderStages[1] = LoadShader("S:/SmoulderingEngine/Engine/Application/Source/Other/Shaders/model.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, _logicalDevice);
//        
//        //shaderModules.push_back(shaderStages[0].module);
//        //shaderModules.push_back(shaderStages[1].module);
//
//        if (vkCreateGraphicsPipelines(_logicalDevice, _pipelineCache, 1, &pipelineCI, nullptr, &_pipeline) != VK_SUCCESS)
//        {
//            std::cout << "Error while creating graphics pipeline!" << std::endl;
//            return false;
//        }
//        return true;
//    }

//
//    void DestroyBuffer(VkDevice _logicalDevice, VkBuffer& _buffer)
//    {
//        if (_buffer != VK_NULL_HANDLE)
//        {
//            vkDestroyBuffer(_logicalDevice, _buffer, nullptr);
//            _buffer = VK_NULL_HANDLE;
//        }
//    }
//
//    void FreeMemoryObject(VkDevice _logicalDevice, VkDeviceMemory& _memoryObject)
//    {
//        if (_memoryObject != VK_NULL_HANDLE)
//        {
//            vkFreeMemory(_logicalDevice, _memoryObject, nullptr);
//            _memoryObject = VK_NULL_HANDLE;
//        }
//    }
//
//    void DestroyBufferView(VkDevice _logicalDevice, VkBufferView& _bufferView)
//    {
//        if (_bufferView != VK_NULL_HANDLE)
//        {
//            vkDestroyBufferView(_logicalDevice, _bufferView, nullptr);
//            _bufferView = VK_NULL_HANDLE;
//        }
//    }
//
//    void DestroyImage(VkDevice _logicalDevice, VkImage& _image)
//    {
//        if (_image != VK_NULL_HANDLE)
//        {
//            vkDestroyImage(_logicalDevice, _image, nullptr);
//            _image = VK_NULL_HANDLE;
//        }
//    }
//
//    void DestroyImageView(VkDevice _logicalDevice, VkImageView& _imageView)
//    {
//        if (_imageView != VK_NULL_HANDLE)
//        {
//            vkDestroyImageView(_logicalDevice, _imageView, nullptr);
//            _imageView = VK_NULL_HANDLE;
//        }
//    }
//
//    void DestroyRenderPass(VkDevice _logicalDevice, VkRenderPass& _renderPass)
//    {
//        if (_renderPass != VK_NULL_HANDLE)
//        {
//            vkDestroyRenderPass(_logicalDevice, _renderPass, nullptr);
//            _renderPass = VK_NULL_HANDLE;
//        }
//    }
//
//    void DestroyFramebuffer(VkDevice _logicalDevice, VkFramebuffer& _framebuffer)
//    {
//        if (_framebuffer != VK_NULL_HANDLE)
//        {
//            vkDestroyFramebuffer(_logicalDevice, _framebuffer, nullptr);
//            _framebuffer = VK_NULL_HANDLE;
//        }
//    }

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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
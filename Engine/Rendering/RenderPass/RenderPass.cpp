#include "RenderPass.h"

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
                subpassDescription.pipelineType,                                       // VkPipelineBindPoint              pipelineBindPoint
                static_cast<uint32_t>(subpassDescription.inputAttachments.size()),     // uint32_t                         inputAttachmentCount
                subpassDescription.inputAttachments.data(),                            // const VkAttachmentReference    * pInputAttachments
                static_cast<uint32_t>(subpassDescription.colorAttachments.size()),     // uint32_t                         colorAttachmentCount
                subpassDescription.colorAttachments.data(),                            // const VkAttachmentReference    * pColorAttachments
                subpassDescription.resolveAttachments.data(),                          // const VkAttachmentReference    * pResolveAttachments
                subpassDescription.depthStencilAttachment,                             // const VkAttachmentReference    * pDepthStencilAttachment
                static_cast<uint32_t>(subpassDescription.preserveAttachments.size()),  // uint32_t                         preserveAttachmentCount
                subpassDescription.preserveAttachments.data()                          // const uint32_t                 * pPreserveAttachments
            });
        }
    }

    bool CreateRenderPass(VkDevice _logicalDevice, std::vector<VkAttachmentDescription> const& _attachmentsDescriptions, std::vector<SubpassParameters> const& _subpassParameters,
        std::vector<VkSubpassDependency> const& _subpassDependencies, VkRenderPass& _renderPass)
	{
        //SpecifyAttachmentsDescriptions(_attachmentsDescriptions);

        std::vector<VkSubpassDescription> subpassDescriptions;
        SpecifySubpassDescriptions(_subpassParameters, subpassDescriptions);

        //SpecifyDependenciesBetweenSubpasses(_subpassDependencies);

        VkRenderPassCreateInfo render_pass_create_info = {
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
        VkImageCreateInfo image_create_info = 
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

        VkResult result = vkCreateImage(_logicalDevice, &image_create_info, nullptr, &_image);
        if (VK_SUCCESS != result) {
            std::cout << "Could not create an image." << std::endl;
            return false;
        }
        return true;
    }

    bool CreateImageView(VkDevice _logicalDevice, VkImage _image, VkImageViewType _viewType, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _imageView)
    {
        VkImageViewCreateInfo imageViewCreateInfo = 
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType
            nullptr,                                    // const void               * pNext
            0,                                          // VkImageViewCreateFlags     flags
            _image,                                      // VkImage                    image
            _viewType,                                  // VkImageViewType            viewType
            _format,                                     // VkFormat                   format
            {                                           // VkComponentMapping         components
              VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         r
              VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         g
              VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         b
              VK_COMPONENT_SWIZZLE_IDENTITY               // VkComponentSwizzle         a
            },
            {                                           // VkImageSubresourceRange    subresourceRange
              _aspect,                                     // VkImageAspectFlags         aspectMask
              0,                                          // uint32_t                   baseMipLevel
              VK_REMAINING_MIP_LEVELS,                    // uint32_t                   levelCount
              0,                                          // uint32_t                   baseArrayLayer
              VK_REMAINING_ARRAY_LAYERS                   // uint32_t                   layerCount
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
};
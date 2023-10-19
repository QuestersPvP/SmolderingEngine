#pragma once

#include "../../Common/Common.h"

namespace SmolderingEngine
{
    void SpecifySubpassDescriptions(std::vector<SubpassParameters> const& _subpassParameters, std::vector<VkSubpassDescription>& _subpassDescriptions);

    bool CreateRenderPass(VkDevice _logicalDevice, std::vector<VkAttachmentDescription> const& _attachmentsDescriptions, std::vector<SubpassParameters> const& _subpassParameters,
        std::vector<VkSubpassDependency> const& _subpassDependencies, VkRenderPass& _renderPass);
    void BeginRenderPass(VkCommandBuffer _commandBuffer, VkRenderPass _renderPass, VkFramebuffer _framebuffer, VkRect2D _renderArea,
        std::vector<VkClearValue> const& _clearValues, VkSubpassContents _subpassContents);
    void EndRenderPass(VkCommandBuffer _commandBuffer);

    bool CreateFramebuffer(VkDevice _logicalDevice, VkRenderPass _renderPass, std::vector<VkImageView> const& _attachments, uint32_t _width, uint32_t _height,
        uint32_t _layers, VkFramebuffer& _framebuffer);


    bool CreateImage(VkDevice _logicalDevice, VkImageType _type, VkFormat _format, VkExtent3D _size, uint32_t _numMipmaps, uint32_t _numLayers, VkSampleCountFlagBits _samples,
        VkImageUsageFlags _usageScenarios, bool _cubemap, VkImage& _image);
    bool CreateImageView(VkDevice _logicalDevice, VkImage _image, VkImageViewType _viewType, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _imageView);
    bool Create2DImageAndView(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkFormat _format, VkExtent2D _size, uint32_t _numMipmaps, uint32_t _numLayers,
        VkSampleCountFlagBits _samples, VkImageUsageFlags _usage, VkImageAspectFlags _aspect, VkImage& _image, VkDeviceMemory& _memoryObject, VkImageView& _imageView);
    bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkImage _image, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject);

    //TODO: move
    void SetImageMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<ImageTransition> _imageTransitions);
};
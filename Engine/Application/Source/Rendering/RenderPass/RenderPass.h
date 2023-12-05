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
    bool CreateLayered2DImageWithCubemapView(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, uint32_t _size, uint32_t _numMipmaps, VkImageUsageFlags _usage,
        VkImageAspectFlags _aspect, VkImage& _image, VkDeviceMemory& _memoryObject, VkImageView& _imageView);
    bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkImage _image, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject);
    void SetImageMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<ImageTransition> _imageTransitions);
    
    bool MapUpdateAndUnmapHostVisibleMemory(VkDevice _logicalDevice, VkDeviceMemory _memoryObject, VkDeviceSize _offset, VkDeviceSize _dataSize,
        void* _data, bool _unmap, void** _pointer);

    bool CreateBuffer(VkDevice _logicalDevice, VkDeviceSize _size, VkBufferUsageFlags _usage, VkBuffer& _buffer);
    bool AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkBuffer _buffer, VkMemoryPropertyFlagBits _memoryProperties, VkDeviceMemory& _memoryObject);
    void SetBufferMemoryBarrier(VkCommandBuffer _commandBuffer, VkPipelineStageFlags _generatingStages, VkPipelineStageFlags _consumingStages, std::vector<BufferTransition> _bufferTransitions);
    bool CreateBufferView(VkDevice _logicalDevice, VkBuffer _buffer, VkFormat _format, VkDeviceSize _memoryOffset, VkDeviceSize _memoryRange, VkBufferView& _bufferView);

    void CopyDataBetweenBuffers(VkCommandBuffer _commandBuffer, VkBuffer _sourceBuffer, VkBuffer _destinationBuffer, std::vector<VkBufferCopy> _regions);
    void CopyDataFromBufferToImage(VkCommandBuffer _commandBuffer, VkBuffer _sourceBuffer, VkImage _destinationImage, VkImageLayout _imageLayout, std::vector<VkBufferImageCopy> _regions);
    void CopyDataFromImageToBuffer(VkCommandBuffer _commandBuffer, VkImage _sourceImage, VkImageLayout _imageLayout, VkBuffer _destinationBuffer, std::vector<VkBufferImageCopy> _regions);

    bool UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _dataSize, void* _data, VkBuffer _destinationBuffer,
        VkDeviceSize _destinationOffset, VkAccessFlags _destinationBufferCurrentAccess, VkAccessFlags _destinationBufferNewAccess, VkPipelineStageFlags _destinationBufferGeneratingStages,
        VkPipelineStageFlags _destinationBufferConsumingStages, VkQueue _queue, VkCommandBuffer _commandBuffer, std::vector<VkSemaphore> _signalSemaphores);
    bool UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _dataSize, void* _data, VkImage _destinationImage,
        VkImageSubresourceLayers _destinationImageSubresource, VkOffset3D _destinationImageOffset, VkExtent3D _destinationImageSize, VkImageLayout _destinationImageCurrentLayout,
        VkImageLayout _destinationImageNewLayout, VkAccessFlags _destinationImageCurrentAccess, VkAccessFlags _destinationImageNewAccess, VkImageAspectFlags _destinationImageAspect,
        VkPipelineStageFlags _destinationImageGeneratingStages, VkPipelineStageFlags _destinationImageConsumingStages, VkQueue _queue, VkCommandBuffer _commandBuffer, std::vector<VkSemaphore> _signalSemaphores);

    void DrawGeometry(VkCommandBuffer _commandBuffer, uint32_t _vertexCount, uint32_t _instanceCount, uint32_t _firstVertex, uint32_t _firstInstance);
    void BindVertexBuffers(VkCommandBuffer _commandBuffer, uint32_t _firstBinding, std::vector<VertexBufferParameters> const& _buffersParameters);

    bool CreateShaderModule(VkDevice _logicalDevice, std::vector<unsigned char> const& _sourceCode, VkShaderModule& _shaderModule);
    void SpecifyPipelineShaderStages(std::vector<ShaderStageParameters> const& _shaderStageParams, std::vector<VkPipelineShaderStageCreateInfo>& _shaderStageCreateInfos);
    void SpecifyPipelineVertexInputState(std::vector<VkVertexInputBindingDescription> const& _bindingDescriptions, std::vector<VkVertexInputAttributeDescription> const& _attributeDescriptions,
        VkPipelineVertexInputStateCreateInfo& _vertexInputStateCreateInfo);
    void SpecifyPipelineInputAssemblyState(VkPrimitiveTopology _topology, bool _primitiveRestartEnable, VkPipelineInputAssemblyStateCreateInfo& _inputAssemblyStateCreateInfo);
    void SpecifyPipelineViewportAndScissorTestState(ViewportInfo const& _viewportInfos, VkPipelineViewportStateCreateInfo& _viewportStateCreateInfo);
    void SpecifyPipelineRasterizationState(bool _depthClampEnable, bool _rasterizerDiscardEnable, VkPolygonMode _polygonMode, VkCullModeFlags _cullingMode, VkFrontFace _frontFace,
        bool _depthBiasEnable, float _depthBiasConstantFactor, float _depthBiasClamp, float _depthBiasSlopeFactor, float _lineWidth, VkPipelineRasterizationStateCreateInfo& _rasterizationStateCreateInfo);
    void SpecifyPipelineMultisampleState(VkSampleCountFlagBits _sampleCount, bool _perSampleShadingEnable, float _minSampleShading, VkSampleMask const* _sampleMasks,
        bool _alphaToCoverageEnable, bool _alphaToOneEnable, VkPipelineMultisampleStateCreateInfo& _multisampleStateCreateInfo);
    void SpecifyPipelineBlendState(bool _logicOpEnable, VkLogicOp _logicOp, std::vector<VkPipelineColorBlendAttachmentState> const& _attachmentBlendStates,
        std::array<float, 4> const& _blendConstants, VkPipelineColorBlendStateCreateInfo& _blendStateCreateInfo);
    void SpecifyPipelineDynamicStates(std::vector<VkDynamicState> const& _dynamicStates, VkPipelineDynamicStateCreateInfo& _dynamicStateCreateInfo);
    bool CreatePipelineLayout(VkDevice _logicalDevice, std::vector<VkDescriptorSetLayout> const& _descriptorSetLayouts,
        std::vector<VkPushConstantRange> const& _pushConstantRanges, VkPipelineLayout& _pipelineLayout);
    void SpecifyGraphicsPipelineCreationParameters(VkPipelineCreateFlags _additionalOptions, std::vector<VkPipelineShaderStageCreateInfo> const& _shaderStageCreateInfos,
        VkPipelineVertexInputStateCreateInfo const& _vertexInputStateCreateInfo, VkPipelineInputAssemblyStateCreateInfo const& _inputAssemblyStateCreateInfo,
        VkPipelineTessellationStateCreateInfo const* _tessellationStateCreateInfo, VkPipelineViewportStateCreateInfo const* _viewportStateCreateInfo,
        VkPipelineRasterizationStateCreateInfo const& _rasterizationStateCreateInfo, VkPipelineMultisampleStateCreateInfo const* _multisampleStateCreateInfo,
        VkPipelineDepthStencilStateCreateInfo const* _depthAndStencilStateCreateInfo, VkPipelineColorBlendStateCreateInfo const* _blendStateCreateInfo,
        VkPipelineDynamicStateCreateInfo const* _dynamicStateCreateInfo, VkPipelineLayout _pipelineLayout, VkRenderPass _renderPass, uint32_t _subpass, 
        VkPipeline _basePipelineHandle, int32_t _basePipelineIndex, VkGraphicsPipelineCreateInfo& _graphicsPipelineCreateInfo);
    bool CreateGraphicsPipelines(VkDevice _logicalDevice, std::vector<VkGraphicsPipelineCreateInfo> const& _graphicsPipelineCreateInfos,
        VkPipelineCache _pipelineCache, std::vector<VkPipeline>& _graphicsPipelines);
    void BindPipelineObject(VkCommandBuffer _commandBuffer, VkPipelineBindPoint _pipelineType, VkPipeline _pipeline);
    void SetViewportStateDynamically(VkCommandBuffer _commandBuffer, uint32_t _firstViewport, std::vector<VkViewport> const& _viewports);
    void SetScissorStateDynamically(VkCommandBuffer _commandBuffer, uint32_t _firstScissor, std::vector<VkRect2D> const& _scissors);


    bool CreateUniformBuffer(VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkDeviceSize _size, VkBufferUsageFlags _usage, VkBuffer& _uniformBuffer, VkDeviceMemory& _memoryObject);
    bool UpdateUniformBuffer(SwapchainParameters _swapchain, VkPhysicalDevice _physicalDevice, VkDevice _logicalDevice, VkBuffer _uniformBuffer,
        QueueParameters _graphicsQueue, std::vector<FrameResources> const& _framesResources, Mesh& _model);

    bool CreateDescriptorSetLayout(VkDevice _logicalDevice, std::vector<VkDescriptorSetLayoutBinding> const& _bindings, VkDescriptorSetLayout& _descriptorSetLayout);  
    bool CreateDescriptorPool(VkDevice _logicalDevice, bool _freeIndividualSets, uint32_t _maxSetsCount, std::vector<VkDescriptorPoolSize> const& _descriptorTypes, VkDescriptorPool& _descriptorPool);
    bool AllocateDescriptorSets(VkDevice _logicalDevice, VkDescriptorPool _descriptorPool, std::vector<VkDescriptorSetLayout> const& _descriptorSetLayout, std::vector<VkDescriptorSet>& _descriptorSets);
    void UpdateDescriptorSets(VkDevice _logicalDevice, std::vector<ImageDescriptorInfo> const& _imageDescriptorInfos, std::vector<BufferDescriptorInfo> const& _bufferDescriptorInfos,
        std::vector<TexelBufferDescriptorInfo> const& _texelBufferDescriptorInfos, std::vector<CopyDescriptorInfo> const& _copyDescriptorOnfos);
    void BindDescriptorSets(VkCommandBuffer _commandBuffer, VkPipelineBindPoint _pipelineType, VkPipelineLayout _pipelineLayout, uint32_t _indexForFirstSet,
        std::vector<VkDescriptorSet> const& _descriptorSets, std::vector<uint32_t> const& _dynamicOffsets);

    void DestroyBuffer(VkDevice _logicalDevice, VkBuffer& _buffer);
    void FreeMemoryObject(VkDevice _logicalDevice, VkDeviceMemory& _memoryObject);
    void DestroyBufferView(VkDevice _logicalDevice, VkBufferView& _bufferView);
    void DestroyImage(VkDevice _logicalDevice, VkImage& _image);
    void DestroyImageView(VkDevice _logicalDevice, VkImageView& _imageView);
    void DestroyRenderPass(VkDevice _logicalDevice, VkRenderPass& _renderPass);
    void DestroyFramebuffer(VkDevice _logicalDevice, VkFramebuffer& _framebuffer);
};
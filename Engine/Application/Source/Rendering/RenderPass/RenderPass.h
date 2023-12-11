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

    uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties& memoryProperties, VkBool32* memTypeFound = nullptr);
    bool AllocateMemory(VkDevice _logicalDevice, Buffer& _buffer, uint32_t _size, uint32_t _type);
	bool CreateGraphicsPipeline(VkDevice _logicalDevice, VkPipelineCache _pipelineCache, VkPipelineLayout _pipelineLayout, VkRenderPass _renderPass, VkPipeline& _pipeline);
	VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage, VkDevice _logicalDevice);
	VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);

    void DestroyBuffer(VkDevice _logicalDevice, VkBuffer& _buffer);
    void FreeMemoryObject(VkDevice _logicalDevice, VkDeviceMemory& _memoryObject);
    void DestroyBufferView(VkDevice _logicalDevice, VkBufferView& _bufferView);
    void DestroyImage(VkDevice _logicalDevice, VkImage& _image);
    void DestroyImageView(VkDevice _logicalDevice, VkImageView& _imageView);
    void DestroyRenderPass(VkDevice _logicalDevice, VkRenderPass& _renderPass);
    void DestroyFramebuffer(VkDevice _logicalDevice, VkFramebuffer& _framebuffer);

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
	{
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		return pipelineVertexInputStateCreateInfo;
	}

	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
		const std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions,
		const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions
	)
	{
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptions.size());
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions.data();
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
		return pipelineVertexInputStateCreateInfo;
	}

	inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
		VkPrimitiveTopology topology,
		VkPipelineInputAssemblyStateCreateFlags flags,
		VkBool32 primitiveRestartEnable)
	{
		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
		pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyStateCreateInfo.topology = topology;
		pipelineInputAssemblyStateCreateInfo.flags = flags;
		pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
		return pipelineInputAssemblyStateCreateInfo;
	}

	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
		VkPolygonMode polygonMode,
		VkCullModeFlags cullMode,
		VkFrontFace frontFace,
		VkPipelineRasterizationStateCreateFlags flags = 0)
	{
		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
		pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
		pipelineRasterizationStateCreateInfo.cullMode = cullMode;
		pipelineRasterizationStateCreateInfo.frontFace = frontFace;
		pipelineRasterizationStateCreateInfo.flags = flags;
		pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
		return pipelineRasterizationStateCreateInfo;
	}

	inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
		VkColorComponentFlags colorWriteMask,
		VkBool32 blendEnable)
	{
		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
		pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
		pipelineColorBlendAttachmentState.blendEnable = blendEnable;
		return pipelineColorBlendAttachmentState;
	}

	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
		uint32_t attachmentCount,
		const VkPipelineColorBlendAttachmentState* pAttachments)
	{
		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
		pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
		pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
		return pipelineColorBlendStateCreateInfo;
	}

	inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
		VkBool32 depthTestEnable,
		VkBool32 depthWriteEnable,
		VkCompareOp depthCompareOp)
	{
		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
		pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
		pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
		pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		return pipelineDepthStencilStateCreateInfo;
	}

	inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
		uint32_t viewportCount,
		uint32_t scissorCount,
		VkPipelineViewportStateCreateFlags flags = 0)
	{
		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
		pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportStateCreateInfo.viewportCount = viewportCount;
		pipelineViewportStateCreateInfo.scissorCount = scissorCount;
		pipelineViewportStateCreateInfo.flags = flags;
		return pipelineViewportStateCreateInfo;
	}

	inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
		VkSampleCountFlagBits rasterizationSamples,
		VkPipelineMultisampleStateCreateFlags flags = 0)
	{
		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
		pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
		pipelineMultisampleStateCreateInfo.flags = flags;
		return pipelineMultisampleStateCreateInfo;
	}

	inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
		const std::vector<VkDynamicState>& pDynamicStates,
		VkPipelineDynamicStateCreateFlags flags = 0)
	{
		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
		pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
		pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
		pipelineDynamicStateCreateInfo.flags = flags;
		return pipelineDynamicStateCreateInfo;
	}

	inline VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
	{
		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
		pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
		return pipelineTessellationStateCreateInfo;
	}

    inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
        VkPipelineLayout layout,
        VkRenderPass renderPass,
        VkPipelineCreateFlags flags = 0)
    {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.layout = layout;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.flags = flags;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        return pipelineCreateInfo;
    }

	inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
		VkDescriptorType type,
		VkShaderStageFlags stageFlags,
		uint32_t binding,
		uint32_t descriptorCount = 1)
	{
		VkDescriptorSetLayoutBinding setLayoutBinding{};
		setLayoutBinding.descriptorType = type;
		setLayoutBinding.stageFlags = stageFlags;
		setLayoutBinding.binding = binding;
		setLayoutBinding.descriptorCount = descriptorCount;
		return setLayoutBinding;
	}

	inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
		const VkDescriptorSetLayoutBinding* pBindings,
		uint32_t bindingCount)
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pBindings = pBindings;
		descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
		return descriptorSetLayoutCreateInfo;
	}

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
		const VkDescriptorSetLayout* pSetLayouts,
		uint32_t setLayoutCount = 1)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
		pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
		return pipelineLayoutCreateInfo;
	}

	inline VkDescriptorPoolSize descriptorPoolSize(
		VkDescriptorType type,
		uint32_t descriptorCount)
	{
		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type = type;
		descriptorPoolSize.descriptorCount = descriptorCount;
		return descriptorPoolSize;
	}

	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(
		const std::vector<VkDescriptorPoolSize>& poolSizes,
		uint32_t maxSets)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		return descriptorPoolInfo;
	}

	inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(
		VkDescriptorPool descriptorPool,
		const VkDescriptorSetLayout* pSetLayouts,
		uint32_t descriptorSetCount)
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
		descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
		return descriptorSetAllocateInfo;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		VkDescriptorSet dstSet,
		VkDescriptorType type,
		uint32_t binding,
		VkDescriptorBufferInfo* bufferInfo,
		uint32_t descriptorCount = 1)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = dstSet;
		writeDescriptorSet.descriptorType = type;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.pBufferInfo = bufferInfo;
		writeDescriptorSet.descriptorCount = descriptorCount;
		return writeDescriptorSet;
	}

	inline VkCommandBufferBeginInfo commandBufferBeginInfo()
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		return cmdBufferBeginInfo;
	}

	inline VkRenderPassBeginInfo RenderPassBeginInfo()
	{
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		return renderPassBeginInfo;
	}

	inline VkViewport Viewport(
		float width,
		float height,
		float minDepth,
		float maxDepth)
	{
		VkViewport viewport{};
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		return viewport;
	}

	inline VkRect2D rect2D(
		int32_t width,
		int32_t height,
		int32_t offsetX,
		int32_t offsetY)
	{
		VkRect2D rect2D{};
		rect2D.extent.width = width;
		rect2D.extent.height = height;
		rect2D.offset.x = offsetX;
		rect2D.offset.y = offsetY;
		return rect2D;
	}

	inline VkBufferCreateInfo BufferCreateInfo(
		VkBufferUsageFlags usage,
		VkDeviceSize size)
	{
		VkBufferCreateInfo bufCreateInfo{};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufCreateInfo.usage = usage;
		bufCreateInfo.size = size;
		return bufCreateInfo;
	}

	inline VkMemoryAllocateInfo memoryAllocateInfo()
	{
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		return memAllocInfo;
	}

	inline VkMappedMemoryRange mappedMemoryRange()
	{
		VkMappedMemoryRange mappedMemoryRange{};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		return mappedMemoryRange;
	}

	inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(
		VkCommandPool commandPool,
		VkCommandBufferLevel level,
		uint32_t bufferCount)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.level = level;
		commandBufferAllocateInfo.commandBufferCount = bufferCount;
		return commandBufferAllocateInfo;
	}

	inline VkSubmitInfo submitInfo()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		return submitInfo;
	}

	inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0)
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = flags;
		return fenceCreateInfo;
	}
};
#include "Engine/Public/Rendering/SkyboxRenderer.h"

//// Third Party
//#include <stb_image.h>

// Project Includes
#include "Engine/Public/Rendering/Utilities.h"

#include "Engine/Public/Camera/Camera.h"

SkyboxRenderer::SkyboxRenderer()
{
	CreateSkyboxVertices();
}

SkyboxRenderer::SkyboxRenderer(std::string _fileLocation, std::vector<std::string> _fileNames, const VulkanResources& _resources)
	: vulkanResources(_resources)
{
	try
	{
		CreateSkyboxVertices();
		CreateCubemapTextureSampler();
		CreateCubemapDescriptorSetLayout();
		CreateCubemapDescriptorPool();
		CreateCubemapUniformBuffer();
		CreateVertexBuffer();
		CreateCubemapTextureImage(_fileLocation, _fileNames);
		CreateCubemapGraphicsPipeline();
	}
	catch (const std::runtime_error& error)
	{
		printf("Error: %s\n", error.what());
	}
}

void SkyboxRenderer::DestroySkyboxRenderer()
{

	delete(this);
}


void SkyboxRenderer::RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex)
{
	// Bind the skybox pipeline
	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapGraphicsPipeline);

	// Bind vertex buffer
	VkBuffer vertexBuffers[] = { skyboxVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);

	// Bind descriptor sets (using the skybox pipeline layout)
	vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipelineLayout,
		0, 1, &cubemapUBODescriptorSets[_imageIndex], 0, nullptr); // Set 0
	vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipelineLayout,
		1, 1, &cubemapSamplerDescriptorSet, 0, nullptr); // Set 1 remains the same if texture doesn't change

	// Draw the skybox cube
	vkCmdDraw(_commandBuffer, static_cast<uint32_t>(skyboxVertices.size()), 1, 0, 0);
}

void SkyboxRenderer::UpdateUniformBuffer(const Camera* _camera, uint32_t _imageIndex)
{
	UniformBufferObjectViewProjection ubo = {};
	ubo.view = _camera->uboViewProjection.view; // Remove translation
	ubo.projection = _camera->uboViewProjection.projection;
	ubo.projection[1][1] *= -1; // Flip Y coordinate since we are using GLM

	void* data;
	vkMapMemory(vulkanResources.logicalDevice, cubemapUniformBufferMemories[_imageIndex], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(vulkanResources.logicalDevice, cubemapUniformBufferMemories[_imageIndex]);
}

void SkyboxRenderer::CreateCubemapTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;

	if (vkCreateSampler(vulkanResources.logicalDevice, &samplerCreateInfo, nullptr, &cubemapTextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create cubemap sampler!");
	}
}

void SkyboxRenderer::CreateCubemapDescriptorSetLayout()
{
	// UBO Descriptor Set Layout (Set 0)
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Accessible in vertex shader
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo uboLayoutCreateInfo = {};
	uboLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	uboLayoutCreateInfo.bindingCount = 1;
	uboLayoutCreateInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(vulkanResources.logicalDevice, &uboLayoutCreateInfo, nullptr, &cubemapUBOSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create UBO descriptor set layout!");

	// Sampler Descriptor Set Layout (Set 1)
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Accessible in fragment shader
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo samplerLayoutCreateInfo = {};
	samplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	samplerLayoutCreateInfo.bindingCount = 1;
	samplerLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	if (vkCreateDescriptorSetLayout(vulkanResources.logicalDevice, &samplerLayoutCreateInfo, nullptr, &cubemapSamplerSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create sampler descriptor set layout!");
}

void SkyboxRenderer::CreateCubemapUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObjectViewProjection);

	cubemapUniformBuffers.resize(vulkanResources.swapchainImages.size());
	cubemapUniformBufferMemories.resize(vulkanResources.swapchainImages.size());

	for (size_t i = 0; i < vulkanResources.swapchainImages.size(); i++)
	{
		CreateBuffer(vulkanResources.physicalDevice, vulkanResources.logicalDevice, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&cubemapUniformBuffers[i], &cubemapUniformBufferMemories[i]);
	}
}

void SkyboxRenderer::CreateCubemapDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1; // Number of descriptors of this type

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;
	poolCreateInfo.maxSets = 1; // Maximum number of descriptor sets that can be allocated

	if (vkCreateDescriptorPool(vulkanResources.logicalDevice, &poolCreateInfo, nullptr, &cubemapSamplerDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");

	// Create UBO descriptor pool
	VkDescriptorPoolSize uboPoolSize = {};
	uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboPoolSize.descriptorCount = vulkanResources.swapchainImages.size();

	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = 1;

	std::array<VkDescriptorPoolSize, 2> poolSizes = { uboPoolSize, samplerPoolSize };

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(vulkanResources.swapchainImages.size() + 1); // +1 for the sampler descriptor set

	if (vkCreateDescriptorPool(vulkanResources.logicalDevice, &poolInfo, nullptr, &cubemapUBODescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create cubemap descriptor pool!");
}

void SkyboxRenderer::CreateCubemapGraphicsPipeline()
{
	// Read in shaders & create Shader Stage
	std::vector<char> cubemapVertexShaderCode = ReadFile(std::string(PROJECT_SOURCE_DIR) +
		"/SmolderingEngine/Engine/Shaders/Compiled/CubemapSkyboxVertexShader.vert.spv");
	std::vector<char> cubemapFragmentShaderCode = ReadFile(std::string(PROJECT_SOURCE_DIR) +
		"/SmolderingEngine/Engine/Shaders/Compiled/CubemapSkyboxFragmentShader.frag.spv");

	// Convert the SPIR-V code into shader modules
	VkShaderModule cubemapVertexShaderModule = CreateShaderModule(vulkanResources.logicalDevice, cubemapVertexShaderCode);
	VkShaderModule cubemapFragmentShaderModule = CreateShaderModule(vulkanResources.logicalDevice, cubemapFragmentShaderCode);

	VkPipelineShaderStageCreateInfo cubemapVertexShaderStageCreateInfo = {};
	cubemapVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	cubemapVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	cubemapVertexShaderStageCreateInfo.module = cubemapVertexShaderModule;
	cubemapVertexShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo cubemapFragmentShaderStageCreateInfo = {};
	cubemapFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	cubemapFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	cubemapFragmentShaderStageCreateInfo.module = cubemapFragmentShaderModule;
	cubemapFragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo cubemapShaderStages[] = { cubemapVertexShaderStageCreateInfo, cubemapFragmentShaderStageCreateInfo };

	// Get vertex input
	VkVertexInputBindingDescription cubemapBindingDescription = {};
	cubemapBindingDescription.binding = 0;
	cubemapBindingDescription.stride = sizeof(Vertex);
	cubemapBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription cubemapAttributeDescription = {};
	cubemapAttributeDescription.binding = 0;
	cubemapAttributeDescription.location = 0;
	cubemapAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	cubemapAttributeDescription.offset = offsetof(Vertex, position);

	VkPipelineVertexInputStateCreateInfo cubemapVertexInputCreateInfo = {};
	cubemapVertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	cubemapVertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	cubemapVertexInputCreateInfo.pVertexBindingDescriptions = &cubemapBindingDescription;
	cubemapVertexInputCreateInfo.vertexAttributeDescriptionCount = 1;
	cubemapVertexInputCreateInfo.pVertexAttributeDescriptions = &cubemapAttributeDescription;

	// Setup input assembly 
	VkPipelineInputAssemblyStateCreateInfo cubemapInputAssembly = {};
	cubemapInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	cubemapInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	cubemapInputAssembly.primitiveRestartEnable = VK_FALSE;

	// Define the Viewport and Scissor 
	VkViewport cubemapViewport = {};
	cubemapViewport.x = 0.0f;
	cubemapViewport.y = 0.0f;
	cubemapViewport.width = (float)vulkanResources.swapchainExtent.width;
	cubemapViewport.height = (float)vulkanResources.swapchainExtent.height;
	cubemapViewport.minDepth = 0.0f;
	cubemapViewport.maxDepth = 1.0f;

	VkRect2D cubemapScissor = {};
	cubemapScissor.offset = { 0,0 };
	cubemapScissor.extent = vulkanResources.swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &cubemapViewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &cubemapScissor;

	VkPipelineViewportStateCreateInfo cubemapViewportStateCreateInfo = {};
	cubemapViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	cubemapViewportStateCreateInfo.viewportCount = 1;
	cubemapViewportStateCreateInfo.pViewports = &cubemapViewport;
	cubemapViewportStateCreateInfo.scissorCount = 1;
	cubemapViewportStateCreateInfo.pScissors = &cubemapScissor;

	// Rasterization info
	VkPipelineRasterizationStateCreateInfo cubemapRasterizationCreateInfo = {};
	cubemapRasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	cubemapRasterizationCreateInfo.depthClampEnable = VK_FALSE;
	cubemapRasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	cubemapRasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	cubemapRasterizationCreateInfo.lineWidth = 1.0f;
	cubemapRasterizationCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	cubemapRasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	cubemapRasterizationCreateInfo.depthBiasEnable = VK_FALSE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo cubemapMultisampleCreateInfo = {};
	cubemapMultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	cubemapMultisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	cubemapMultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Setup color blending
	VkPipelineColorBlendAttachmentState cubemapColorBlendAttachment = {};
	cubemapColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	cubemapColorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo cubemapColorBlending = {};
	cubemapColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cubemapColorBlending.logicOpEnable = VK_FALSE;
	cubemapColorBlending.logicOp = VK_LOGIC_OP_COPY;
	cubemapColorBlending.attachmentCount = 1;
	cubemapColorBlending.pAttachments = &cubemapColorBlendAttachment;
	cubemapColorBlending.blendConstants[0] = 0.0f;
	cubemapColorBlending.blendConstants[1] = 0.0f;
	cubemapColorBlending.blendConstants[2] = 0.0f;
	cubemapColorBlending.blendConstants[3] = 0.0f;

	// Pipeline Layout
	std::array<VkDescriptorSetLayout, 2> cubemapDescriptorSetLayouts = { cubemapUBOSetLayout, cubemapSamplerSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(cubemapDescriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = cubemapDescriptorSetLayouts.data();

	if (vkCreatePipelineLayout(vulkanResources.logicalDevice, &pipelineLayoutInfo, nullptr, &cubemapPipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create cubemap pipeline layout!");

	// Depth stencil
	VkPipelineDepthStencilStateCreateInfo cubemapDepthStencilCreateInfo = {};
	cubemapDepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	cubemapDepthStencilCreateInfo.depthTestEnable = VK_TRUE;
	cubemapDepthStencilCreateInfo.depthWriteEnable = VK_FALSE;
	cubemapDepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	cubemapDepthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	cubemapDepthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	// Create pipeline
	VkGraphicsPipelineCreateInfo cubemapGraphicsPipelineCreateInfo = {};
	cubemapGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	cubemapGraphicsPipelineCreateInfo.stageCount = 2;
	cubemapGraphicsPipelineCreateInfo.pStages = cubemapShaderStages;
	cubemapGraphicsPipelineCreateInfo.pVertexInputState = &cubemapVertexInputCreateInfo;
	cubemapGraphicsPipelineCreateInfo.pInputAssemblyState = &cubemapInputAssembly;
	cubemapGraphicsPipelineCreateInfo.pViewportState = &cubemapViewportStateCreateInfo;
	cubemapGraphicsPipelineCreateInfo.pRasterizationState = &cubemapRasterizationCreateInfo;
	cubemapGraphicsPipelineCreateInfo.pMultisampleState = &cubemapMultisampleCreateInfo;
	cubemapGraphicsPipelineCreateInfo.pColorBlendState = &cubemapColorBlending;
	cubemapGraphicsPipelineCreateInfo.pDepthStencilState = &cubemapDepthStencilCreateInfo;
	cubemapGraphicsPipelineCreateInfo.layout = cubemapPipelineLayout;
	cubemapGraphicsPipelineCreateInfo.renderPass = vulkanResources.renderPass;
	cubemapGraphicsPipelineCreateInfo.subpass = 0;
	cubemapGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	cubemapGraphicsPipelineCreateInfo.basePipelineIndex = -1;

	VkResult result = vkCreateGraphicsPipelines(vulkanResources.logicalDevice, VK_NULL_HANDLE, 1, &cubemapGraphicsPipelineCreateInfo, nullptr, &cubemapGraphicsPipeline);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create cubemap graphics pipeline!");

	// Destroy the shader modules
	vkDestroyShaderModule(vulkanResources.logicalDevice, cubemapFragmentShaderModule, nullptr);
	vkDestroyShaderModule(vulkanResources.logicalDevice, cubemapVertexShaderModule, nullptr);
}

void SkyboxRenderer::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * skyboxVertices.size();

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(vulkanResources.physicalDevice, vulkanResources.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// Copy vertex data to staging buffer
	void* data;
	vkMapMemory(vulkanResources.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, skyboxVertices.data(), (size_t)bufferSize);
	vkUnmapMemory(vulkanResources.logicalDevice, stagingBufferMemory);

	// Create vertex buffer
	CreateBuffer(vulkanResources.physicalDevice, vulkanResources.logicalDevice, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&skyboxVertexBuffer, &skyboxVertexBufferMemory);

	// Copy data from staging buffer to vertex buffer
	CopyBuffer(vulkanResources.logicalDevice, vulkanResources.graphicsQueue, vulkanResources.graphicsCommandPool,
		stagingBuffer, skyboxVertexBuffer, bufferSize);

	// Clean up staging buffer
	vkDestroyBuffer(vulkanResources.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(vulkanResources.logicalDevice, stagingBufferMemory, nullptr);
}

void SkyboxRenderer::CreateCubemapTextureImage(std::string _fileLocation, std::vector<std::string> _fileNames)
{
	// Load all face images
	std::vector<stbi_uc*> faceData(6);
	int width, height, channels;
	VkDeviceSize imageSize;
	VkDeviceSize layerSize;
	VkDeviceSize totalSize = 0;

	stbi_set_flip_vertically_on_load(1);

	for (size_t i = 0; i < 6; ++i)
	{
		std::string imagePath = _fileLocation + _fileNames[i];
		faceData[i] = stbi_load(imagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!faceData[i])
		{
			throw std::runtime_error("Failed to load texture image!");
		}
		layerSize = width * height * 4; // Assuming 4 bytes per pixel (RGBA)
		totalSize += layerSize;
	}

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(vulkanResources.physicalDevice, vulkanResources.logicalDevice, totalSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// Copy face data into staging buffer
	void* data;
	vkMapMemory(vulkanResources.logicalDevice, stagingBufferMemory, 0, totalSize, 0, &data);
	VkDeviceSize offset = 0;
	for (size_t i = 0; i < 6; ++i)
	{
		layerSize = width * height * 4;
		memcpy(static_cast<char*>(data) + offset, faceData[i], static_cast<size_t>(layerSize));
		offset += layerSize;
		stbi_image_free(faceData[i]); // Free individual face data after copying
	}
	vkUnmapMemory(vulkanResources.logicalDevice, stagingBufferMemory);

	// Create cubemap image
	cubemapImage = CreateCubemapImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &cubemapImageMemory);

	// Transition image to TRANSFER_DST_OPTIMAL
	TransitionImageLayout(vulkanResources.logicalDevice, vulkanResources.graphicsQueue, vulkanResources.graphicsCommandPool,
		cubemapImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);

	// Copy buffer to image
	CopyBufferToCubemapImage(vulkanResources.logicalDevice, vulkanResources.graphicsQueue, vulkanResources.graphicsCommandPool,
		stagingBuffer, cubemapImage, width, height);

	// Transition image to SHADER_READ_ONLY_OPTIMAL
	TransitionImageLayout(vulkanResources.logicalDevice, vulkanResources.graphicsQueue, vulkanResources.graphicsCommandPool,
		cubemapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);

	// Create image view
	cubemapImageView = CreateCubemapImageView(cubemapImage, VK_FORMAT_R8G8B8A8_UNORM);

	// Create descriptor
	CreateCubemapTextureDescriptor(cubemapImageView);

	// Clean up staging buffer
	vkDestroyBuffer(vulkanResources.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(vulkanResources.logicalDevice, stagingBufferMemory, nullptr);
}

VkImage SkyboxRenderer::CreateCubemapImage(uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usageFlags, VkMemoryPropertyFlags _propertyFlags, VkDeviceMemory* _imageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					// Could be 1D, 2D, or 3D
	imageCreateInfo.extent.width = _width;
	imageCreateInfo.extent.height = _height;
	imageCreateInfo.extent.depth = 1;								// Depth of image is just 1, we do not have 3D aspect.
	imageCreateInfo.mipLevels = 1;									// Number of mipmap levels
	imageCreateInfo.arrayLayers = 6;								// Number of levels in image array
	imageCreateInfo.format = _format;
	imageCreateInfo.tiling = _tiling;								// How image data should be "tiled" (e.g. arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Layout of image data when created
	imageCreateInfo.usage = _usageFlags;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;				// Number of samples for milti-sampling
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// Image will not be shared between queues

	VkImage image;
	VkResult result = vkCreateImage(vulkanResources.logicalDevice, &imageCreateInfo, nullptr, &image);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create an image!");

	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vulkanResources.logicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocationInfo = {};
	memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocationInfo.allocationSize = memoryRequirements.size;
	memoryAllocationInfo.memoryTypeIndex = FindMemoryTypeIndex(vulkanResources.physicalDevice, memoryRequirements.memoryTypeBits, _propertyFlags);

	result = vkAllocateMemory(vulkanResources.logicalDevice, &memoryAllocationInfo, nullptr, _imageMemory);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate memory!");

	// Bind memory to the image
	vkBindImageMemory(vulkanResources.logicalDevice, image, *_imageMemory, 0);

	return image;
}

VkImageView SkyboxRenderer::CreateCubemapImageView(VkImage _image, VkFormat _format)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = _image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.format = _format;
	viewInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 6;

	VkImageView imageView;
	if (vkCreateImageView(vulkanResources.logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create cubemap image view!");
	}

	return imageView;
}

void SkyboxRenderer::CreateCubemapTextureDescriptor(VkImageView _cubemapImageView)
{
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = cubemapSamplerDescriptorPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = &cubemapSamplerSetLayout;

	if (vkAllocateDescriptorSets(vulkanResources.logicalDevice, &setAllocInfo, &cubemapSamplerDescriptorSet) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate cubemap descriptor set!");

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _cubemapImageView;
	imageInfo.sampler = cubemapTextureSampler; // Ensure you have created a sampler suitable for the cubemap

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = cubemapSamplerDescriptorSet;
	descriptorWrite.dstBinding = 0; // Adjust binding as per your descriptor set layout
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(vulkanResources.logicalDevice, 1, &descriptorWrite, 0, nullptr);

	// Descriptor Set for UBO
	cubemapUBODescriptorSets.resize(vulkanResources.swapchainImages.size());

	std::vector<VkDescriptorSetLayout> layouts(vulkanResources.swapchainImages.size(), cubemapUBOSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = cubemapUBODescriptorPool;
	allocInfo.descriptorSetCount = vulkanResources.swapchainImages.size();
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(vulkanResources.logicalDevice, &allocInfo, cubemapUBODescriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate cubemap uniform buffer descriptor sets!");

	for (size_t i = 0; i < vulkanResources.swapchainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = cubemapUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObjectViewProjection);

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = cubemapUBODescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(vulkanResources.logicalDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void SkyboxRenderer::CopyBufferToCubemapImage(VkDevice _logicalDevice, VkQueue _queue, VkCommandPool _commandPool,
	VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height)
{
	VkCommandBuffer commandBuffer = BeginCommandBuffer(vulkanResources.logicalDevice, _commandPool);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	VkDeviceSize layerSize = _width * _height * 4; // Assuming 4 bytes per pixel (RGBA)

	for (uint32_t face = 0; face < 6; ++face)
	{
		VkBufferImageCopy region = {};
		region.bufferOffset = layerSize * face;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = face;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { _width, _height, 1 };
		bufferCopyRegions.push_back(region);
	}

	vkCmdCopyBufferToImage(commandBuffer, _buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

	EndAndSubmitCommandBuffer(vulkanResources.logicalDevice, _commandPool, _queue, commandBuffer);
}

void SkyboxRenderer::CreateSkyboxVertices()
{
	skyboxVertices =
	{
		// Right face (+X)
		{ { 1.0f, -1.0f, -1.0f } },
		{ { 1.0f, -1.0f,  1.0f } },
		{ { 1.0f,  1.0f,  1.0f } },
		{ { 1.0f,  1.0f,  1.0f } },
		{ { 1.0f,  1.0f, -1.0f } },
		{ { 1.0f, -1.0f, -1.0f } },

		// Left face (-X)
		{ { -1.0f, -1.0f,  1.0f } },
		{ { -1.0f, -1.0f, -1.0f } },
		{ { -1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f,  1.0f } },
		{ { -1.0f, -1.0f,  1.0f } },

		// Top face (+Y)
		{ { -1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f,  1.0f } },
		{ { -1.0f,  1.0f,  1.0f } },
		{ { -1.0f,  1.0f, -1.0f } },

		// Bottom face (-Y)
		{ { -1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f,  1.0f } },
		{ {  1.0f, -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, -1.0f } },
		{ { -1.0f, -1.0f, -1.0f } },
		{ { -1.0f, -1.0f,  1.0f } },

		// Front face (+Z)
		{ { -1.0f, -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, -1.0f } },
		{ { -1.0f,  1.0f, -1.0f } },
		{ { -1.0f, -1.0f, -1.0f } },

		// Back face (-Z)
		{ {  1.0f, -1.0f,  1.0f } },
		{ { -1.0f, -1.0f,  1.0f } },
		{ { -1.0f,  1.0f,  1.0f } },
		{ { -1.0f,  1.0f,  1.0f } },
		{ {  1.0f,  1.0f,  1.0f } },
		{ {  1.0f, -1.0f,  1.0f } }
	};
}

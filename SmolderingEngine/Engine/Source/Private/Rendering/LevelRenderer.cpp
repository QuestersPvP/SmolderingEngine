#include "Engine/Source/Public/Rendering/LevelRenderer.h"

// Project Includes
#include "Engine/Source/EngineManager.h"
#include "Engine/Source/Public/EngineLevel/EngineLevelManager.h"
#include "Engine/Source/Public/Object/ObjectManager.h"

#include "Engine/Source/Public/Rendering/Utilities.h"
#include "Engine/Source/Public/Camera/Camera.h"

LevelRenderer::LevelRenderer(const VulkanResources* _resources)
	: vulkanResources(_resources)
{
	try
	{
		CreateDescriptorSetLayout();
		CreatePushConstantRange();
		CreateGraphicsPipeline();
		CreateTextureSampler();
		CreateUniformBuffers();
		CreateDescriptorPool();
		AllocateDescriptorSets();
	}
	catch (const std::runtime_error& error)
	{
		printf("Error: %s\n", error.what());
	}
}

void LevelRenderer::DestroyLevelRenderer()
{
	// Destroy texture image views, images, and memory
	DestroyAllRendererTextures();

	// Destroy the texture sampler
	vkDestroySampler(vulkanResources->logicalDevice, textureSampler, nullptr);

	// Destroy descriptor pools
	vkDestroyDescriptorPool(vulkanResources->logicalDevice, uboDescriptorPool, nullptr);
	vkDestroyDescriptorPool(vulkanResources->logicalDevice, samplerDescriptorPool, nullptr);

	// Destroy uniform buffers
	for (size_t i = 0; i < viewProjectionUniformBuffers.size(); i++) 
	{
		vkDestroyBuffer(vulkanResources->logicalDevice, viewProjectionUniformBuffers[i], nullptr);
		vkFreeMemory(vulkanResources->logicalDevice, viewProjectionUniformBufferMemory[i], nullptr);
	}

	// Destroy the graphics pipeline and its layout
	vkDestroyPipeline(vulkanResources->logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(vulkanResources->logicalDevice, graphicsPipelineLayout, nullptr);

	// Destroy descriptor set layouts
	vkDestroyDescriptorSetLayout(vulkanResources->logicalDevice, uboDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(vulkanResources->logicalDevice, samplerSetLayout, nullptr);

	delete(this);
}

void LevelRenderer::DestroyAllRendererTextures()
{
	// Wait until queues and all operations are done before cleaning up
	vkDeviceWaitIdle(vulkanResources->logicalDevice);

	// Destroy texture-related Vulkan objects for the current level
	for (size_t i = 0; i < textureImages.size(); i++)
	{
		vkDestroyImageView(vulkanResources->logicalDevice, textureImageViews[i], nullptr);
		vkDestroyImage(vulkanResources->logicalDevice, textureImages[i], nullptr);
		vkFreeMemory(vulkanResources->logicalDevice, textureImageMemory[i], nullptr);
	}

	textureImageViews.clear();
	textureImages.clear();
	textureImageMemory.clear();
}

void LevelRenderer::RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex)
{
	EngineManager* seEngineManager = EngineManager::GetEngineManager();
	
	if (seEngineManager == nullptr)
	{
		std::cout << "Fatal error: LevelRenderer::RecordToCommandBuffer - EngineManager is nullptr!" << std::endl;
		return;
	}

	// Bind the main graphics pipeline and its pipeline layout
	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	
	// Re-bind descriptor sets for the main pipeline
	vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout,
		0, 1, &uboDescriptorSets[_imageIndex], 0, nullptr);

	for (size_t i = 0; i < seEngineManager->GetEngineLevelManager()->GetObjectManager()->GetGameObjects().size(); i++)
	{
		MeshModel* tempModel = seEngineManager->GetEngineLevelManager()->GetObjectManager()->GetGameObjects()[i]->objectMeshModel;
		// Push constants to given shader stage directly
		vkCmdPushConstants(_commandBuffer, graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Model),
			&seEngineManager->GetEngineLevelManager()->GetObjectManager()->GetGameObjects()[i]->GetModel());

		for (size_t j = 0; j < tempModel->GetMeshCount(); j++)
		{
			// bind mesh vertex buffer
			VkBuffer vertexBuffers[] = { tempModel->GetMesh(j)->GetVertexBuffer() }; // Buffers to bind
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);

			// Bind mesh index buffer
			vkCmdBindIndexBuffer(_commandBuffer, tempModel->GetMesh(j)->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			// Bind descriptor sets
			if (tempModel->GetMesh(j)->GetTextureID() >= 0)
			{
				// TODO: MEMORY LEAK -> When loading new level, make sure to remove old sampler descriptor set!!
				std::array<VkDescriptorSet, 2> descriptorSetGroup = { uboDescriptorSets[_imageIndex],
					samplerDescriptorSets[tempModel->GetMesh(j)->GetTextureID()] };
				
				vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout,
					0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);
			}
			else
			{
				std::cout << "Error: Game mesh has no texture AND blank texture is not loaded." << std::endl;
			}

			// Execute the pipeline
			vkCmdDrawIndexed(_commandBuffer, tempModel->GetMesh(j)->GetIndexCount(), 1, 0, 0, 0);
		}
	}
}

void LevelRenderer::UpdateUniformBuffer(const Camera* _camera, uint32_t _imageIndex)
{
	// copy view projection data
	void* data;
	vkMapMemory(vulkanResources->logicalDevice, viewProjectionUniformBufferMemory[_imageIndex], 0, sizeof(UniformBufferObjectViewProjection), 0, &data);
	memcpy(data, &_camera->uboViewProjection, sizeof(UniformBufferObjectViewProjection));
	vkUnmapMemory(vulkanResources->logicalDevice, viewProjectionUniformBufferMemory[_imageIndex]);
}

void LevelRenderer::ResizeRenderer()
{
	vkDestroyPipelineLayout(vulkanResources->logicalDevice, graphicsPipelineLayout, nullptr);
	vkDestroyPipeline(vulkanResources->logicalDevice, graphicsPipeline, nullptr);

	CreateGraphicsPipeline();
}

void LevelRenderer::CreateDescriptorSetLayout()
{
	// View Projection binding info
	VkDescriptorSetLayoutBinding viewProjectionLayoutBinding = {};
	viewProjectionLayoutBinding.binding = 1;										// binding point in shader
	viewProjectionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	viewProjectionLayoutBinding.descriptorCount = 1;
	viewProjectionLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;			// it is bound in the vertex shader
	viewProjectionLayoutBinding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { viewProjectionLayoutBinding };

	// Create descripor set layout with given bindings
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();

	// Create descriptor set layout
	VkResult result = vkCreateDescriptorSetLayout(vulkanResources->logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &uboDescriptorSetLayout);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout!");

	// Texture binding info
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	// Create a Descriptor Set Layout with given bindings for texture
	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	// Create Descriptor Set Layout
	result = vkCreateDescriptorSetLayout(vulkanResources->logicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a Descriptor Set Layout!");
}

void LevelRenderer::CreatePushConstantRange()
{
	// Define push constant values
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	// push constant will be in vertex shader
	pushConstantRange.offset = 0;								// start at begining of data
	pushConstantRange.size = sizeof(Model);						// hold the size of the Models data
}

void LevelRenderer::CreateGraphicsPipeline()
{
#pragma region Shader Stage Creation
	// Read in SPIR-V code
	std::vector<char> vertexShaderCode = ReadFile(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Engine/Shaders/Compiled/Shader.vert.spv");
	std::vector<char> fragmentShaderCode = ReadFile(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Engine/Shaders/Compiled/Shader.frag.spv");

	// Convert the SPIR-V code into shader modules
	VkShaderModule VertexShaderModule = CreateShaderModule(vulkanResources->logicalDevice, vertexShaderCode);
	VkShaderModule FragmentShaderModule = CreateShaderModule(vulkanResources->logicalDevice, fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.module = VertexShaderModule;
	vertexShaderStageCreateInfo.pName = "main"; // run the "main" function in the shader	

	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageCreateInfo.module = FragmentShaderModule;
	fragmentShaderStageCreateInfo.pName = "main"; // run the "main" function in the shader

	VkPipelineShaderStageCreateInfo ShaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };
#pragma endregion

#pragma region Vertex Input
	// How the data for a single vertex is as a whole (position, color, texture coords, normals, etc.)
	VkVertexInputBindingDescription VertexBindingDescription = {};
	VertexBindingDescription.binding = 0;								// Can bind multiple streams of data
	VertexBindingDescription.stride = sizeof(Vertex);					// Size of vertex data
	VertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// How to move between data after each vertex
	// VK_VERTEX_INPUT_RATE_VERTEX = Move onto next vertex
	// VK_VERTEX_INPUT_RATE_INSTANCE = Move onto vertex for the next instance of this object

	// How the data for an attribute is defined within a vertex
	std::array<VkVertexInputAttributeDescription, 3> AttributeDescriptions;

	// Position attribute
	AttributeDescriptions[0].binding = 0;							// What binding the data set is at, should be same as above
	AttributeDescriptions[0].location = 0;							// Location in shader where data is read from
	AttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// Format the data will take / defines size of data
	AttributeDescriptions[0].offset = offsetof(Vertex, position);	// Where the attribute is defined in the data for a single vertex

	// Color attribute
	AttributeDescriptions[1].binding = 0;
	AttributeDescriptions[1].location = 1;
	AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	AttributeDescriptions[1].offset = offsetof(Vertex, color);

	// Texture attribute
	AttributeDescriptions[2].binding = 0;
	AttributeDescriptions[2].location = 2;
	AttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	AttributeDescriptions[2].offset = offsetof(Vertex, texture);

	VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = {};
	VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	VertexInputCreateInfo.pVertexBindingDescriptions = &VertexBindingDescription;		// List of vertex binding descriptions (data spacing and stride info)
	VertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescriptions.size());
	VertexInputCreateInfo.pVertexAttributeDescriptions = AttributeDescriptions.data();	// List of vertex attribute descriptions (data format and where to bind to/from)
#pragma endregion

#pragma region Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
#pragma endregion

#pragma region Viewport and Scissor
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)vulkanResources->swapchainExtent.width;
	viewport.height = (float)vulkanResources->swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = vulkanResources->swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;
#pragma endregion

	//// TODO: Dynamic States
	//std::vector<VkDynamicState> EnabledDynamicStates;
	//EnabledDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// Dynamic Viewport allows you to resize command buffer with vkCmdSetViewport();
	//EnabledDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);	// Dynamic Scissor allows you to resize command buffer with vkCmdSetScissor();

	//VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {};
	//DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(EnabledDynamicStates.size());
	//DynamicStateCreateInfo.pDynamicStates = EnabledDynamicStates.data();

#pragma region Rasterization Creation
	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
	rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;					// Requires DepthClamp = true on device features. in order to enable
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;				// Wether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer ouput
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;				// When a polygon is drawn, what do we do (e.g. we want if colored)
	rasterizationCreateInfo.lineWidth = 1.0f;								// How thick lines should be when drawn (if we want VK_POLYGON_MODE_LINE above)
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;				// Do not draw back side of triangles.
	rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Basically helps figure out what the backside of the triangle is.
	rasterizationCreateInfo.depthBiasClamp = VK_FALSE;						// If we should add depth bias to fragments (good for stopping "shadow acne")
#pragma endregion

#pragma region Multisampling
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// number of samples to use per fragment  
#pragma endregion

#pragma region Color Blending
	VkPipelineColorBlendAttachmentState ColorState = {};
	ColorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// Colors to apply blending to
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorState.blendEnable = VK_TRUE;
	// Blending uses equation (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old color)
	ColorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	ColorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	ColorState.colorBlendOp = VK_BLEND_OP_ADD;
	ColorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	ColorState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo = {};
	ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	ColorBlendingCreateInfo.attachmentCount = 1;
	ColorBlendingCreateInfo.pAttachments = &ColorState;
#pragma endregion

#pragma region Pipeline Layout
	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { uboDescriptorSetLayout, samplerSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	VkResult Result = vkCreatePipelineLayout(vulkanResources->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &graphicsPipelineLayout);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout");
#pragma endregion

	// Depth Stencil
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;				// enable checking depth to determine fragment write
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;				// enable writing to depth buffer (to replace old values)
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;		// we want things that are closer to be shown infront of things far away.
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;		// Should the depth value exist between two values
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	// Create Graphics Pipeline
	VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = {};
	GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	GraphicsPipelineCreateInfo.stageCount = 2;
	GraphicsPipelineCreateInfo.pStages = ShaderStages;
	GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputCreateInfo;
	GraphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	GraphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	GraphicsPipelineCreateInfo.pDynamicState = nullptr;
	GraphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	GraphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
	GraphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	GraphicsPipelineCreateInfo.layout = graphicsPipelineLayout;
	GraphicsPipelineCreateInfo.renderPass = vulkanResources->renderPass;
	GraphicsPipelineCreateInfo.subpass = 0;
	// Use if we want to create multiple pipelines deriving from eachother
	GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	GraphicsPipelineCreateInfo.basePipelineIndex = -1;

	Result = vkCreateGraphicsPipelines(vulkanResources->logicalDevice, VK_NULL_HANDLE, 1, &GraphicsPipelineCreateInfo, nullptr, &graphicsPipeline);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");

	// Destroy shader modules
	vkDestroyShaderModule(vulkanResources->logicalDevice, FragmentShaderModule, nullptr);
	vkDestroyShaderModule(vulkanResources->logicalDevice, VertexShaderModule, nullptr);
}

void LevelRenderer::CreateTextureSampler()
{
	// Sampler Creation Info
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;						// How to render when image is magnified on screen
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;						// How to render when image is minified on screen
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle texture wrap in U (x) direction
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle texture wrap in V (y) direction
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	// How to handle texture wrap in W (z) direction
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// Border beyond texture (only workds for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;				// Whether coords should be normalized (between 0 and 1)
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;		// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = 0.0f;								// Level of Details bias for mip level
	samplerCreateInfo.minLod = 0.0f;									// Minimum Level of Detail to pick mip level
	samplerCreateInfo.maxLod = 0.0f;									// Maximum Level of Detail to pick mip level
	samplerCreateInfo.anisotropyEnable = VK_TRUE;						// Enable Anisotropy
	samplerCreateInfo.maxAnisotropy = 16;								// Anisotropy sample level

	VkResult result = vkCreateSampler(vulkanResources->logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Filed to create a Texture Sampler!");
}

void LevelRenderer::CreateUniformBuffers()
{
	VkDeviceSize viewProjectionBufferSize = sizeof(UniformBufferObjectViewProjection);

	viewProjectionUniformBuffers.resize(vulkanResources->swapchainImages.size());
	viewProjectionUniformBufferMemory.resize(vulkanResources->swapchainImages.size());

	for (size_t i = 0; i < vulkanResources->swapchainImages.size(); i++)
	{
		CreateBuffer(vulkanResources->physicalDevice, vulkanResources->logicalDevice, viewProjectionBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &viewProjectionUniformBuffers[i], &viewProjectionUniformBufferMemory[i]);
	}
}

void LevelRenderer::CreateDescriptorPool()
{
	// type of descriptor and how many descriptors.
	// View projection pool
	VkDescriptorPoolSize viewProjectionDescriptorPoolSize = {};
	viewProjectionDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	viewProjectionDescriptorPoolSize.descriptorCount = static_cast<uint32_t>(viewProjectionUniformBuffers.size());	

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { viewProjectionDescriptorPoolSize };

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(vulkanResources->swapchainImages.size());
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	VkResult result = vkCreateDescriptorPool(vulkanResources->logicalDevice, &descriptorPoolCreateInfo, nullptr, &uboDescriptorPool);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a descriptor pool");

	// TODO: research array layers or texture atlases to optimize this
	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	result = vkCreateDescriptorPool(vulkanResources->logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a descriptor pool!");
}

void LevelRenderer::AllocateDescriptorSets()
{
	// resize descriptor set, the uniform buffers are linked
	uboDescriptorSets.resize(vulkanResources->swapchainImages.size());

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(vulkanResources->swapchainImages.size(), uboDescriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = uboDescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(vulkanResources->swapchainImages.size());
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

	VkResult result = vkAllocateDescriptorSets(vulkanResources->logicalDevice, &descriptorSetAllocateInfo, uboDescriptorSets.data());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate descriptor sets!");

	// Update all of descriptor set buffer bindings
	for (size_t i = 0; i < vulkanResources->swapchainImages.size(); i++)
	{
		// View projection descriptor
		// Buffer info and data offset info
		VkDescriptorBufferInfo viewProjectionBufferInfo = {};
		viewProjectionBufferInfo.buffer = viewProjectionUniformBuffers[i];						// buffer to get data from
		viewProjectionBufferInfo.offset = 0;													// any offset (e.g. skip any data)
		viewProjectionBufferInfo.range = sizeof(UniformBufferObjectViewProjection);				// bind everything (size of data)

		VkWriteDescriptorSet viewProjectionSetWrite = {};
		viewProjectionSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		viewProjectionSetWrite.dstSet = uboDescriptorSets[i];							// desriptor set to update
		viewProjectionSetWrite.dstBinding = 1;										// binding in shader to update
		viewProjectionSetWrite.dstArrayElement = 0;									// index to update
		viewProjectionSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewProjectionSetWrite.descriptorCount = 1;
		viewProjectionSetWrite.pBufferInfo = &viewProjectionBufferInfo;

		std::vector<VkWriteDescriptorSet> writeDescriptors = { viewProjectionSetWrite };

		// update the descriptor sets with the new buffer binding info
		vkUpdateDescriptorSets(vulkanResources->logicalDevice, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
	}
}

VkImage LevelRenderer::CreateImage(uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling,
	VkImageUsageFlags _usageFlags, VkMemoryPropertyFlags _propertyFlags, VkDeviceMemory* _imageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					// Could be 1D, 2D, or 3D
	imageCreateInfo.extent.width = _width;
	imageCreateInfo.extent.height = _height;
	imageCreateInfo.extent.depth = 1;								// Depth of image is just 1, we do not have 3D aspect.
	imageCreateInfo.mipLevels = 1;									// Number of mipmap levels
	imageCreateInfo.arrayLayers = 1;								// Number of levels in image array
	imageCreateInfo.format = _format;
	imageCreateInfo.tiling = _tiling;								// How image data should be "tiled" (e.g. arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Layout of image data when created
	imageCreateInfo.usage = _usageFlags;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;				// Number of samples for milti-sampling
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// Image will not be shared between queues

	VkImage image;
	VkResult result = vkCreateImage(vulkanResources->logicalDevice, &imageCreateInfo, nullptr, &image);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create an image!");

	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vulkanResources->logicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocationInfo = {};
	memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocationInfo.allocationSize = memoryRequirements.size;
	memoryAllocationInfo.memoryTypeIndex = FindMemoryTypeIndex(vulkanResources->physicalDevice, memoryRequirements.memoryTypeBits, _propertyFlags);

	result = vkAllocateMemory(vulkanResources->logicalDevice, &memoryAllocationInfo, nullptr, _imageMemory);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate memory!");

	// Bind memory to the image
	vkBindImageMemory(vulkanResources->logicalDevice, image, *_imageMemory, 0);

	return image;
}

VkImageView LevelRenderer::CreateImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags)
{
	VkImageViewCreateInfo ImageViewInfo = {};
	ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewInfo.image = _image;
	ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// Type of image to be displayed (2D, 3D, Cube Map (skybox), etc.)
	ImageViewInfo.format = _format;
	ImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// You can remap RGBA components to other RGBA values
	ImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInfo.subresourceRange.aspectMask = _aspectFlags;			// What aspect of image to view (color, depth, etc.)
	ImageViewInfo.subresourceRange.baseMipLevel = 0;					// Start mipmap level to view from					
	ImageViewInfo.subresourceRange.levelCount = 1;						// number of mipmap levels to view
	ImageViewInfo.subresourceRange.baseArrayLayer = 0;					// start array level to view from
	ImageViewInfo.subresourceRange.layerCount = 1;						// number of array levels to view

	VkImageView ImageView;
	VkResult Result = vkCreateImageView(vulkanResources->logicalDevice, &ImageViewInfo, nullptr, &ImageView);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create image views!");

	return ImageView;
}

stbi_uc* LevelRenderer::LoadTextureFile(std::string _fileName, int* _width, int* _height, VkDeviceSize* _imageSize)
{
	// Number of channels image uses
	int channels;

	// Load pixel data for image
	stbi_uc* image = stbi_load(_fileName.c_str(), _width, _height, &channels, STBI_rgb_alpha);

	if (!image)
	{
		throw std::runtime_error("Failed to load a Texture file! (" + _fileName + ")");
	}

	// Calculate image size using given and known data
	*_imageSize = *_width * *_height * 4; // *4 because RGBA

	return image;
}

int LevelRenderer::CreateTextureImage(std::string _fileName)
{
	// Load image file
	int width, height;
	VkDeviceSize imageSize;
	stbi_uc* imageData = LoadTextureFile(_fileName, &width, &height, &imageSize);

	// Create staging buffer to hold loaded data, ready to copy to device
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;
	CreateBuffer(vulkanResources->physicalDevice, vulkanResources->logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&imageStagingBuffer, &imageStagingBufferMemory);

	// Copy image data to staging buffer
	void* data;
	vkMapMemory(vulkanResources->logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(imageSize));
	vkUnmapMemory(vulkanResources->logicalDevice, imageStagingBufferMemory);

	// Free original image data
	stbi_image_free(imageData);

	// Create image to hold final texture
	VkImage texImage;
	VkDeviceMemory texImageMemory;
	texImage = CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);


	// COPY DATA TO IMAGE
	// Transition image to be DST for copy operation
	TransitionImageLayout(vulkanResources->logicalDevice, vulkanResources->graphicsQueue, vulkanResources->graphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

	// Copy image data
	CopyImageBuffer(vulkanResources->logicalDevice, vulkanResources->graphicsQueue, vulkanResources->graphicsCommandPool, imageStagingBuffer, texImage, width, height);

	// Transition image to be shader readable for shader usage
	TransitionImageLayout(vulkanResources->logicalDevice, vulkanResources->graphicsQueue, vulkanResources->graphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

	// Add texture data to vector for reference
	textureImages.push_back(texImage);
	textureImageMemory.push_back(texImageMemory);

	// Destroy staging buffers
	vkDestroyBuffer(vulkanResources->logicalDevice, imageStagingBuffer, nullptr);
	vkFreeMemory(vulkanResources->logicalDevice, imageStagingBufferMemory, nullptr);

	// Return index of new texture image
	return textureImages.size() - 1;
}

int LevelRenderer::CreateTexture(std::string _fileName)
{
	// Create Texture Image and get its location in array
	int textureImageLoc = CreateTextureImage(_fileName);

	// Create Image View and add to list
	VkImageView imageView = CreateImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textureImageViews.push_back(imageView);

	// Create Texture Descriptor
	int descriptorLoc = CreateTextureDescriptor(imageView);

	// Return location of set with texture
	return descriptorLoc;
}

int LevelRenderer::CreateTextureDescriptor(VkImageView _textureImage)
{
	VkDescriptorSet descriptorSet;

	// Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = samplerDescriptorPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = &samplerSetLayout;

	// Allocate Descriptor Sets
	VkResult result = vkAllocateDescriptorSets(vulkanResources->logicalDevice, &setAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Texture Descriptor Sets!");
	}

	// Texture Image Info
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// Image layout when in use
	imageInfo.imageView = _textureImage;									// Image to bind to set
	imageInfo.sampler = textureSampler;									// Sampler to use for set

	// Descriptor Write Info
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	// Update new descriptor set
	vkUpdateDescriptorSets(vulkanResources->logicalDevice, 1, &descriptorWrite, 0, nullptr);

	// Add descriptor set to list
	samplerDescriptorSets.push_back(descriptorSet);

	// Return descriptor set location
	return samplerDescriptorSets.size() - 1;
}

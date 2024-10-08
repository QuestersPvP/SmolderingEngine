#include "Engine/Source/Public/Rendering/Renderer.h"

// Project Includes
#include "Engine/Source/Public/Rendering/SkyboxRenderer.h"
#include "Engine/Source/Public/Rendering/EngineGUIRenderer.h"
#include "Engine/Source/Public/Rendering/LevelRenderer.h"

#include "Engine/Source/Public/Camera/Camera.h"
#include "Engine/Source/Public/EngineLevel/EngineLevelManager.h"

Renderer::Renderer(GLFWwindow* _window, Camera* _camera)
	: window(_window), seCamera(_camera)
{
	try
	{
		vulkanResources = new VulkanResources();
		CreateVulkanInstance();

		if (ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS)
			SetupDebugMessenger();

		CreateVulkanSurface();
		RetrievePhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateRenderpass();
		CreateDepthBufferImage();
		CreateFramebuffers();
		CreateCommandPool();
		AllocateCommandBuffers();
		CreateSynchronizationPrimatives();
	}
	catch (const std::runtime_error& error)
	{
		printf("Error: %s\n", error.what());
	}

	// --- CREATE SKYBOX RENDERER ---
	std::vector<std::string> imageNames =
	{ "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg", "posz.jpg", "negz.jpg" };
	//{"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
	std::string fileLocation =
		std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Skybox/mountain-skyboxes/Maskonaive2/";
	seSkyboxRenderer = new SkyboxRenderer(fileLocation, imageNames, vulkanResources);
	// --- CREATE SKYBOX RENDERER ---

	// --- CREATE LEVEL RENDERER ---
	seLevelRenderer = new LevelRenderer(vulkanResources);
	// --- CREATE LEVEL RENDERER ---

	// --- CREATE ENGINE GUI RENDERER ---
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(window, true);
	seEngineGUIRenderer = new EngineGUIRenderer(vulkanResources);
	// --- CREATE ENGINE GUI RENDERER ---
}

void Renderer::Draw()
{
	// Wait for given fence to signal/open from last draw call before continuing
	vkWaitForFences(vulkanResources->logicalDevice, 1, &drawFences[currentFrame], VK_TRUE , std::numeric_limits<uint64_t>::max());
	// Reset/close the fence again as we work on this new draw call.
	vkResetFences(vulkanResources->logicalDevice, 1, &drawFences[currentFrame]);

	// Process any GUI inputs prior to rendering
	seEngineGUIRenderer->ProcessEngineGUIInputs();

	// Aquire the next image we want to draw
	uint32_t ImageIndex;
	VkResult Result = vkAcquireNextImageKHR(vulkanResources->logicalDevice, vulkanResources->swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &ImageIndex);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire next image!");

	// Record commands for all renderers
	RecordCommands(ImageIndex);

	// Submit the command buffer we want to render
	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
	VkPipelineStageFlags WaitStages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &commandBuffers[ImageIndex];
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &renderingCompleteSemaphores[currentFrame];

	Result = vkQueueSubmit(vulkanResources->graphicsQueue, 1, &SubmitInfo, drawFences[currentFrame]);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to submit command buffer to queue!");

	// Present the image to the screen when it has signaled it has finished rendering
	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &renderingCompleteSemaphores[currentFrame];
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &vulkanResources->swapchain;
	PresentInfo.pImageIndices = &ImageIndex;

	Result = vkQueuePresentKHR(presentationQueue, &PresentInfo);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to present image!");

	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void Renderer::DestroyRenderer()
{
	// Wait until queues and all operations are done before cleaning up
	vkDeviceWaitIdle(vulkanResources->logicalDevice);

	// Destroy other renderers first
	seEngineGUIRenderer->DestroyEngineGUIRenderer();
	seLevelRenderer->DestroyLevelRenderer();
	seSkyboxRenderer->DestroySkyboxRenderer();

	// Destroy game objects 
	//seLevelManager->DestroyGameMeshes();

	// Destroy all general vulkan stuffz
	vkDestroyImageView(vulkanResources->logicalDevice, depthBufferImageView, nullptr);
	vkDestroyImage(vulkanResources->logicalDevice, depthBufferImage, nullptr);
	vkFreeMemory(vulkanResources->logicalDevice, depthBufferImageMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(vulkanResources->logicalDevice, renderingCompleteSemaphores[i], nullptr);
		vkDestroySemaphore(vulkanResources->logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(vulkanResources->logicalDevice, drawFences[i], nullptr);
	}

	vkDestroyCommandPool(vulkanResources->logicalDevice, vulkanResources->graphicsCommandPool, nullptr);

	for (auto framebuffer : swapchainFramebuffers)
		vkDestroyFramebuffer(vulkanResources->logicalDevice, framebuffer, nullptr);

	vkDestroyRenderPass(vulkanResources->logicalDevice, vulkanResources->renderPass, nullptr);

	for (auto image : vulkanResources->swapchainImages)
		vkDestroyImageView(vulkanResources->logicalDevice, image.imageView, nullptr);

	vkDestroySwapchainKHR(vulkanResources->logicalDevice, vulkanResources->swapchain, nullptr);
	vkDestroySurfaceKHR(vulkanResources->vulkanInstance, vulkanSurface, nullptr);
	vkDestroyDevice(vulkanResources->logicalDevice, nullptr);

	if (ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS)
		DestroyDebugUtilsMessengerEXT(vulkanResources->vulkanInstance, debugMessenger, nullptr);

	vkDestroyInstance(vulkanResources->vulkanInstance, nullptr);

	// Finally destroy the vulkanResources and this class
	delete(vulkanResources);
	delete(this);
}

void Renderer::CreateVulkanInstance()
{
	// Info about app itself, most is for debugging / devs.
	VkApplicationInfo ApplicationInfo = {};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pEngineName = "Smoldering Engine";
	ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

	if (ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS)
	{
		// Check that validation layers are supported
		if (!CheckValidationLayerSupport())
			throw std::runtime_error("Validation layers that were requested are not available on your PC!");
	}

	// Hold instance extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> InstanceExtensions = std::vector<const char*>();
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
		InstanceExtensions.push_back(glfwExtensions[i]);

	// Push back debug utils extension so we can handle callbacks
	InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	
	// Check all the instance extensions are supported
	if (!CheckInstanceExtensionSupport(&InstanceExtensions))
		throw std::runtime_error("vkInstance does not support required instance extensions!");

	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &ApplicationInfo;
	if (ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS)
	{
		CreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		CreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		CreateInfo.enabledLayerCount = 0;
		CreateInfo.ppEnabledLayerNames = nullptr;
	}
	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	CreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();

	// For debugging support
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	PopulateDebugMessengerCreateInfo(debugCreateInfo);
	CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	if (vkCreateInstance(&CreateInfo, nullptr, &vulkanResources->vulkanInstance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create a vulkanResources->vulkanInstance");
}

void Renderer::CreateLogicalDevice()
{
	QueueFamilyIndicies Indicies = GetQueueFamilies(vulkanResources->physicalDevice);


	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfoVector;
	std::set<int> QueueFamilyIndicies = { Indicies.graphicsFamily, Indicies.presentationFamily };

	for (int QueueFamilyIndex : QueueFamilyIndicies)
	{
		VkDeviceQueueCreateInfo QueueCreateInfo = {};
		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;					// Index of family to create queue for
		QueueCreateInfo.queueCount = 1;											// number of queues to create
		float Priority = 1.0f;
		QueueCreateInfo.pQueuePriorities = &Priority;							// Incase of multiple queues 1.0 = highest priority

		QueueCreateInfoVector.push_back(QueueCreateInfo);
	}

	VkDeviceCreateInfo DeviceCreateInfo = {};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfoVector.size());
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfoVector.data();
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	DeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	// Features on physical device that the logical device will use.
	VkPhysicalDeviceFeatures PhysicalDeviceFeatures = {};
	PhysicalDeviceFeatures.samplerAnisotropy = VK_TRUE;		// Enable Anisotropy
	DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;

	VkResult Result = vkCreateDevice(vulkanResources->physicalDevice, &DeviceCreateInfo, nullptr, &vulkanResources->logicalDevice);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device");

	// Get access to the Queues we just created while making the logical device
	vkGetDeviceQueue(vulkanResources->logicalDevice, Indicies.graphicsFamily, 0, &vulkanResources->graphicsQueue);
	vkGetDeviceQueue(vulkanResources->logicalDevice, Indicies.presentationFamily, 0, &presentationQueue);
}

void Renderer::CreateVulkanSurface()
{
	// GLFW handles creating a surface that is specific to the OS of the PC.
	VkResult Result = glfwCreateWindowSurface(vulkanResources->vulkanInstance, window, nullptr, &vulkanSurface);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a presentation surface!");
}

void Renderer::CreateSwapChain()
{
	SwapchainDetails SwapchainInfo = GetSwapchainDetails(vulkanResources->physicalDevice);

	// Find best surface values for swapchain before trying to create it
	VkSurfaceFormatKHR SurfaceFormat = ChooseBestSurfaceFormat(SwapchainInfo.surfaceFormats);
	VkPresentModeKHR PresentationMode = ChooseBestPresentationMode(SwapchainInfo.presentationModes);
	VkExtent2D SurfaceExtent = ChooseSwapChainExtent(SwapchainInfo.surfaceCapabilities);

	// How many images are in the swap chain
	uint32_t ImageCount = SwapchainInfo.surfaceCapabilities.minImageCount + 1; // +1 because we want atleast triple buffering
	if (SwapchainInfo.surfaceCapabilities.maxImageCount > 0 // if 0 then limitless image count
		&& ImageCount > SwapchainInfo.surfaceCapabilities.maxImageCount)
		ImageCount = SwapchainInfo.surfaceCapabilities.maxImageCount;

	VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.surface = vulkanSurface;
	SwapchainCreateInfo.minImageCount = ImageCount;
	SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageExtent = SurfaceExtent;
	SwapchainCreateInfo.imageArrayLayers = 1;												// Number of layers for each image in swap chain
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;					// We are attatching color to the images.
	
	// Fill information out about Queue families
	QueueFamilyIndicies Indicies = GetQueueFamilies(vulkanResources->physicalDevice);
	if (Indicies.graphicsFamily != Indicies.presentationFamily)
	{
		uint32_t QueueFamilyIndicies[] = { (uint32_t)Indicies.graphicsFamily,
										   (uint32_t)Indicies.presentationFamily };

		// If families are different then swapchain must let images be shared
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapchainCreateInfo.queueFamilyIndexCount = 2;
		SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndicies;
	}
	else
	{
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SwapchainCreateInfo.queueFamilyIndexCount = 1;
		uint32_t QueueFamilyIndex = static_cast<uint32_t>(Indicies.graphicsFamily);
		SwapchainCreateInfo.pQueueFamilyIndices = &QueueFamilyIndex;
		//SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	SwapchainCreateInfo.preTransform = SwapchainInfo.surfaceCapabilities.currentTransform;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;					// How to handle blending other windows over this application
	SwapchainCreateInfo.presentMode = PresentationMode;
	SwapchainCreateInfo.clipped = VK_TRUE;													// Wether to clip parts of image behind other windows / off screen.
	SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult Result = vkCreateSwapchainKHR(vulkanResources->logicalDevice, &SwapchainCreateInfo, nullptr, &vulkanResources->swapchain);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a valid swapchain!");

	std::cout << std::endl << std::endl <<"^ ^ ^ ^ Ignore errors above here, Will fix them some year ^ ^ ^ ^" << std::endl << std::endl << std::endl;

	// Store for later use
	swapchainImageFormat = SurfaceFormat.format;
	vulkanResources->swapchainExtent = SurfaceExtent;

	// Get the Swapchain image count now that we made the swapchain
	uint32_t SwapChainImageCount = 0;
	vkGetSwapchainImagesKHR(vulkanResources->logicalDevice, vulkanResources->swapchain, &SwapChainImageCount, nullptr);

	std::vector<VkImage> Images(SwapChainImageCount);
	vkGetSwapchainImagesKHR(vulkanResources->logicalDevice, vulkanResources->swapchain, &SwapChainImageCount, Images.data());

	for (VkImage Image : Images)
	{
		SwapchainImage SESwapchainImage = {};
		SESwapchainImage.image = Image;
		SESwapchainImage.imageView = CreateImageView(Image, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Add image to swapchain image vector
		vulkanResources->swapchainImages.push_back(SESwapchainImage);
	}
}

void Renderer::CreateRenderpass()
{
	// Color attachment of the renderpass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;		// Number of samples to write for multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// Framebuffer data will be stored as an image but images can be given different layouts for optimal use.
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Depth attachment of renderpass
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,  VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderpasscreateinfo
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Attachment reference for depth stencil
	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	// Determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> subpassDependencies;
	// conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after ->
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// Transition must happen before ->
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after ->
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// Transition must happen before ->
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> renderPassAttachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(vulkanResources->logicalDevice, &renderPassCreateInfo, nullptr, &vulkanResources->renderPass);
	
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create renderpass!");
}

void Renderer::CreateDepthBufferImage()
{
	// Get supported formats for depth buffer
	VkFormat depthAttachmentFormat = ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,  VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	
	// Create depth buffer image
	depthBufferImage = CreateImage(vulkanResources->swapchainExtent.width, vulkanResources->swapchainExtent.height, depthAttachmentFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory);

	depthBufferImageView = CreateImageView(depthBufferImage, depthAttachmentFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::CreateFramebuffers()
{
	swapchainFramebuffers.resize(vulkanResources->swapchainImages.size());

	for (size_t i = 0; i < swapchainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
		vulkanResources->swapchainImages[i].imageView,
		depthBufferImageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = vulkanResources->renderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = vulkanResources->swapchainExtent.width;
		framebufferCreateInfo.height = vulkanResources->swapchainExtent.height;
		framebufferCreateInfo.layers = 1;
		
		VkResult result = vkCreateFramebuffer(vulkanResources->logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create a frame buffer!");
	}
}

void Renderer::AllocateCommandBuffers()
{
	commandBuffers.resize(swapchainFramebuffers.size());

	/*
	VkStructureType         sType;
    const void*             pNext;
    VkCommandPool           commandPool;
    VkCommandBufferLevel    level;
    uint32_t                commandBufferCount;
	*/
	VkCommandBufferAllocateInfo CommandBufferAllocationInfo = {};
	CommandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocationInfo.commandPool = vulkanResources->graphicsCommandPool;
	CommandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// Primary are buffers you submit directly to queues, secondary are executed by a primary buffers by using VkCmdExecuteCommands.
	CommandBufferAllocationInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	VkResult Result = vkAllocateCommandBuffers(vulkanResources->logicalDevice, &CommandBufferAllocationInfo, commandBuffers.data());

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!");
}

void Renderer::CreateCommandPool()
{
	QueueFamilyIndicies familyIndicies = GetQueueFamilies(vulkanResources->physicalDevice);

	/*
	VkStructureType             sType;
    const void*                 pNext;
    VkCommandPoolCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
	*/
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	// resets the pools anytime a command buffer begins recording
	commandPoolInfo.queueFamilyIndex = familyIndicies.graphicsFamily;	// Queue family type that buffers from this command pool will use
	
	VkResult result = vkCreateCommandPool(vulkanResources->logicalDevice, &commandPoolInfo, nullptr, &vulkanResources->graphicsCommandPool);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a command pool!");
}

void Renderer::CreateSynchronizationPrimatives()
{
	imageAvailableSemaphores.resize(MAX_FRAME_DRAWS);
	renderingCompleteSemaphores.resize(MAX_FRAME_DRAWS);
	drawFences.resize(MAX_FRAME_DRAWS);

	VkFenceCreateInfo FenceCreateInfo = {};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(vulkanResources->logicalDevice, &SemaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(vulkanResources->logicalDevice, &SemaphoreCreateInfo, nullptr, &renderingCompleteSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(vulkanResources->logicalDevice, &FenceCreateInfo, nullptr, &drawFences[i]))
			throw std::runtime_error("Failed to create a semaphore or fence!");
	}
}

void Renderer::RetrievePhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vulkanResources->vulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("Could not find physical devices that support Vulkan!");
	}

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(vulkanResources->vulkanInstance, &deviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (CheckForBestPhysicalDevice(device))
		{
			vulkanResources->physicalDevice = device;
			break;
		}
	}

	// Get properties of our device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(vulkanResources->physicalDevice, &deviceProperties);

	// get min uniform buffer offset alignment
	//minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}

bool Renderer::CheckInstanceExtensionSupport(std::vector<const char*>* InExtensionsToCheck)
{
	// Get all vulkan extensions our PC can support
	uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);

	std::vector<VkExtensionProperties> ExtensionProperties(ExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, ExtensionProperties.data());

	// Check to make sure we can support all the extensions we requested.
	for (const auto& ExtensionCheck : *InExtensionsToCheck)
	{
		bool ExtensionFound = false;
		for (const auto& Extension : ExtensionProperties)
		{
			ExtensionFound = true;
			break;
		}

		if (!ExtensionFound)
			return false;
	}

	return true;
}

bool Renderer::CheckForBestPhysicalDevice(VkPhysicalDevice InPhysicalDevice)
{
	/*
	// Information about the device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	*/
	
	// Information about what the device can do (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(InPhysicalDevice, &deviceFeatures);

	// TODO: BETTER CHECK FOR DEVICE FEATURES!

	// check for graphics queue families
	QueueFamilyIndicies Indicies = GetQueueFamilies(InPhysicalDevice);

	// check for requested extensions
	bool ExtensionsSupported = CheckDeviceExtentionSupport(InPhysicalDevice);

	// check swapchain support
	SwapchainDetails SwapchainInfo = GetSwapchainDetails(InPhysicalDevice);
	bool IsSwapChainValid = !SwapchainInfo.presentationModes.empty() && !SwapchainInfo.surfaceFormats.empty();

	return Indicies.IsValid() && ExtensionsSupported && IsSwapChainValid && deviceFeatures.samplerAnisotropy;
}

bool Renderer::CheckDeviceExtentionSupport(VkPhysicalDevice InPhysicalDevice)
{
	uint32_t ExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(InPhysicalDevice, nullptr, &ExtensionCount, nullptr);
	
	if (ExtensionCount == 0)
		return false;
	
	std::vector<VkExtensionProperties> Extensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(InPhysicalDevice, nullptr, &ExtensionCount, Extensions.data());

	for (const auto& DeviceExtension : deviceExtensions)
	{
		bool HasExtension = false;
		
		for (const auto& Extension : Extensions)
		{

			if (strcmp(DeviceExtension, Extension.extensionName) == 0)
			{
				HasExtension = true;
				break;
			}
		}

		if (!HasExtension)
			return false;
	}

	return true;
}

bool Renderer::CheckValidationLayerSupport()
{
	// Check all layers that are supported on this PC
	uint32_t LayerCount;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	// Check that the validation layers we requested are within the list of layers supported.
	for (const char* LayerName : validationLayers) 
	{
		bool LayerFound = false;

		for (const auto& LayerProperties : AvailableLayers) 
		{
			if (strcmp(LayerName, LayerProperties.layerName) == 0) 
			{
				LayerFound = true;
				break;
			}
		}

		if (!LayerFound) 
			return false;
	}

	return true;
}

VkImage Renderer::CreateImage(uint32_t inWidth, uint32_t inHeight, VkFormat inFormat, VkImageTiling inTiling,
	VkImageUsageFlags inUsageFlags, VkMemoryPropertyFlags inPropertyFlags, VkDeviceMemory* outImageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					// Could be 1D, 2D, or 3D
	imageCreateInfo.extent.width = inWidth;
	imageCreateInfo.extent.height = inHeight;
	imageCreateInfo.extent.depth = 1;								// Depth of image is just 1, we do not have 3D aspect.
	imageCreateInfo.mipLevels = 1;									// Number of mipmap levels
	imageCreateInfo.arrayLayers = 1;								// Number of levels in image array
	imageCreateInfo.format = inFormat;
	imageCreateInfo.tiling = inTiling;								// How image data should be "tiled" (e.g. arranged for optimal reading)
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Layout of image data when created
	imageCreateInfo.usage = inUsageFlags;
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
	memoryAllocationInfo.memoryTypeIndex = FindMemoryTypeIndex(vulkanResources->physicalDevice, memoryRequirements.memoryTypeBits, inPropertyFlags);
	
	result = vkAllocateMemory(vulkanResources->logicalDevice, &memoryAllocationInfo, nullptr, outImageMemory);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate memory!");

	// Bind memory to the image
	vkBindImageMemory(vulkanResources->logicalDevice, image, *outImageMemory, 0);

	return image;
}

VkImageView Renderer::CreateImageView(VkImage InImage, VkFormat InFormat, VkImageAspectFlags InAspectFlags)
{
	/*
	VkStructureType            sType;
    const void*                pNext;
    VkImageViewCreateFlags     flags;
    VkImage                    image;
    VkImageViewType            viewType;
    VkFormat                   format;
    VkComponentMapping         components;
    VkImageSubresourceRange    subresourceRange;
	*/
	VkImageViewCreateInfo ImageViewInfo = {};
	ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ImageViewInfo.image = InImage;
	ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// Type of image to be displayed (2D, 3D, Cube Map (skybox), etc.)
	ImageViewInfo.format = InFormat;
	ImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// You can remap RGBA components to other RGBA values
	ImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ImageViewInfo.subresourceRange.aspectMask = InAspectFlags;			// What aspect of image to view (color, depth, etc.)
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

void Renderer::RecordCommands(uint32_t _imageIndex)
{
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Start recording
	VkResult result = vkBeginCommandBuffer(commandBuffers[_imageIndex],&bufferBeginInfo);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to start recording a command buffer!");

	// Info on how to begin a render pass
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = vulkanResources->renderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = vulkanResources->swapchainExtent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.6f, 0.6f, 0.4f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();				// Clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.framebuffer = swapchainFramebuffers[_imageIndex];

	// start the render pass
	vkCmdBeginRenderPass(commandBuffers[_imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	/*
	The order we want to draw is:
	1- Skybox
	2- Level
	3- GUI
	*/
	seSkyboxRenderer->UpdateUniformBuffer(seCamera, _imageIndex);
	seSkyboxRenderer->RecordToCommandBuffer(commandBuffers[_imageIndex], _imageIndex);

	seLevelRenderer->UpdateUniformBuffer(seCamera, _imageIndex);
	seLevelRenderer->RecordToCommandBuffer(commandBuffers[_imageIndex], _imageIndex);

	seEngineGUIRenderer->RecordToCommandBuffer(commandBuffers[_imageIndex], _imageIndex);

	vkCmdEndRenderPass(commandBuffers[_imageIndex]);

	// Stop recording
	result = vkEndCommandBuffer(commandBuffers[_imageIndex]);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to stop recording a command buffer!");
}

VkFormat Renderer::ChooseSupportedFormat(const std::vector<VkFormat>& inFormats, VkImageTiling inTiling, VkFormatFeatureFlags inFeatureFlags)
{
	for (VkFormat format : inFormats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(vulkanResources->physicalDevice, format, &properties);

		// Depending on tiling choice check for different bit flag
		if (inTiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & inFeatureFlags) == inFeatureFlags)
		{
			return format;
		}
		else if (inTiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & inFeatureFlags) == inFeatureFlags)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find a matching format");

	return VkFormat();
}

VkSurfaceFormatKHR Renderer::ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& InSurfaceFormats)
{
	// If VK_FORMAT_UNDEFINED then ALL formats are available so return the preferred one.
	if (InSurfaceFormats.size() == 1 && InSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	// If possible I want to use VK_FORMAT_R8G8B8A8_UNORM color format with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR color space.
	for (const auto& Format : InSurfaceFormats)
	{
		if (Format.format == VK_FORMAT_R8G8B8A8_UNORM && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return Format;
	}

	// If not possible I can try for VK_FORMAT_B8G8R8A8_UNORM format
	for (const auto& Format : InSurfaceFormats)
	{
		if (Format.format == VK_FORMAT_B8G8R8A8_UNORM && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return Format;
	}

	return InSurfaceFormats[0];
}

VkPresentModeKHR Renderer::ChooseBestPresentationMode(const std::vector<VkPresentModeKHR> InPresentationMode)
{
	// Mailbox presentation mode is the best
	for (const auto& PresentationMode : InPresentationMode)
	{
		if (PresentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return PresentationMode;
	}

	// Otherwise use FIFO, it must be supported.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& InSurfaceCapabilities)
{
	// If current extent is at numeric limits then extent can vary
	if (InSurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return InSurfaceCapabilities.currentExtent;
	}
	else
	{
		// Otherwise we need to set size of window manually
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D NewExtent = {};
		NewExtent.width = static_cast<uint32_t>(width);
		NewExtent.height = static_cast<uint32_t>(height);

		// Make sure we are not larger than the max or min set by surface
		NewExtent.width = std::max(InSurfaceCapabilities.minImageExtent.width, std::min(InSurfaceCapabilities.maxImageExtent.width, NewExtent.width));
		NewExtent.height = std::max(InSurfaceCapabilities.minImageExtent.height, std::min(InSurfaceCapabilities.maxImageExtent.height, NewExtent.height));

		return NewExtent;
	}
}

QueueFamilyIndicies Renderer::GetQueueFamilies(VkPhysicalDevice InPhysicalDevice)
{
	QueueFamilyIndicies Indicies;

	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueFamilyCount, nullptr);

	/*
	VkQueueFlags    queueFlags;
    uint32_t        queueCount;
    uint32_t        timestampValidBits;
    VkExtent3D      minImageTransferGranularity;
	*/
	std::vector<VkQueueFamilyProperties> QueueFamilyList(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(InPhysicalDevice, &QueueFamilyCount, QueueFamilyList.data());

	// Go through each queue family and make sure it has atleast 1 of the required queue types.
	int Index = 0;
	for (const auto& QueueFamily : QueueFamilyList)
	{
		if (QueueFamily.queueCount > 0 && QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			Indicies.graphicsFamily = Index;
		}

		// Check if the Queue Family supports presentation
		VkBool32 PresentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(InPhysicalDevice, Index, vulkanSurface, &PresentationSupport);

		if (QueueFamily.queueCount > 0 && PresentationSupport)
			Indicies.presentationFamily = Index;

		// Check if queue family indicies are valid and then stop searching.
		if (Indicies.IsValid())
			break;

		Index++;
	}

	return Indicies;
}

SwapchainDetails Renderer::GetSwapchainDetails(VkPhysicalDevice InPhysicalDevice)
{
	SwapchainDetails SwapChainInfo;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InPhysicalDevice, vulkanSurface, &SwapChainInfo.surfaceCapabilities);

	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, vulkanSurface, &FormatCount, nullptr);

	if (FormatCount != 0)
	{
		SwapChainInfo.surfaceFormats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, vulkanSurface, &FormatCount, SwapChainInfo.surfaceFormats.data());
	}

	uint32_t PresentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, vulkanSurface, &PresentationCount, nullptr);

	if (PresentationCount != 0)
	{
		SwapChainInfo.presentationModes.resize(PresentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, vulkanSurface, &PresentationCount, SwapChainInfo.presentationModes.data());
	}


	return SwapChainInfo;
}

void Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo)
{
	CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	CreateInfo.pfnUserCallback = DebugCallback;
}

void Renderer::SetupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	VkResult Result = CreateDebugUtilsMessengerEXT(vulkanResources->vulkanInstance, &createInfo, nullptr, &debugMessenger);

	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

void Renderer::ResizeRenderer(int inWidth, int inHeight)
{
	// Wait for commands to finish
	vkDeviceWaitIdle(vulkanResources->logicalDevice);

	// Destroy Depth Buffer stuff
	vkDestroyImageView(vulkanResources->logicalDevice, depthBufferImageView, nullptr);
	vkDestroyImage(vulkanResources->logicalDevice, depthBufferImage, nullptr);
	vkFreeMemory(vulkanResources->logicalDevice, depthBufferImageMemory, nullptr);

	// Destroy Frame Buffers
	for (auto framebuffer : swapchainFramebuffers)
		vkDestroyFramebuffer(vulkanResources->logicalDevice, framebuffer, nullptr);

	// Destroy the Swapchain Images
	for (auto image : vulkanResources->swapchainImages)
		vkDestroyImageView(vulkanResources->logicalDevice, image.imageView, nullptr);

	// Destroy the Swapchain
	vkDestroySwapchainKHR(vulkanResources->logicalDevice, vulkanResources->swapchain, nullptr);

	// Clear vectors that are now holding nullptrs
	vulkanResources->swapchainImages.clear();
	swapchainFramebuffers.clear();

	// re-create them all
	CreateSwapChain();
	CreateDepthBufferImage();
	CreateFramebuffers();

	// Do the same for the other renderers 
	seSkyboxRenderer->ResizeRenderer();
	seLevelRenderer->ResizeRenderer();

	// NOTE: seEngineGUIRenderer->Resize() Is not needed, ImGUI is smart AF
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	}

	return VK_FALSE;
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		func(instance, debugMessenger, pAllocator);
	}
}

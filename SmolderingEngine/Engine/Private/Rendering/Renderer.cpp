#include "Public/Rendering/Renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

int Renderer::InitRenderer(GLFWwindow* InWindow)
{
	Window = InWindow;

	try
	{
		CreateVulkanInstance();
		SetupDebugMessenger();
		CreateVulkanSurface();
		GetPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateRenderpass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandPool();
		AllocateCommandBuffers();
		RecordCommands();
		CreateSynchronizationPrimatives();
	}
	catch(const std::runtime_error& error)
	{
		printf("Error: %s\n", error.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void Renderer::Draw()
{
	// Wait for given fence to signal/open from last draw call before continuing
	vkWaitForFences(Devices.LogicalDevice, 1, &DrawFences[CurrentFrame], VK_TRUE , std::numeric_limits<uint64_t>::max());
	// Reset/close the fence again as we work on this new draw call.
	vkResetFences(Devices.LogicalDevice, 1, &DrawFences[CurrentFrame]);

	// Aquire the next image we want to draw
	uint32_t ImageIndex;
	VkResult Result = vkAcquireNextImageKHR(Devices.LogicalDevice, Swapchain, std::numeric_limits<uint64_t>::max(), ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire next image!");

	// Submit the command buffer we want to render
	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = &ImageAvailableSemaphores[CurrentFrame];
	VkPipelineStageFlags WaitStages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	SubmitInfo.pWaitDstStageMask = WaitStages;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffers[ImageIndex];
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &RenderingCompleteSemaphores[CurrentFrame];

	Result = vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, DrawFences[CurrentFrame]);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to submit command buffer to queue!");

	// Present the image to the screen when it has signaled it has finished rendering
	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &RenderingCompleteSemaphores[CurrentFrame];
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &Swapchain;
	PresentInfo.pImageIndices = &ImageIndex;

	Result = vkQueuePresentKHR(PresentationQueue, &PresentInfo);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to present image!");

	CurrentFrame = (CurrentFrame + 1) % MAX_FRAME_DRAWS;
}

void Renderer::DestroyRenderer()
{
	vkDeviceWaitIdle(Devices.LogicalDevice); // Wait until queues and all operations are done before cleaning up

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(Devices.LogicalDevice, RenderingCompleteSemaphores[i], nullptr);
		vkDestroySemaphore(Devices.LogicalDevice, ImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Devices.LogicalDevice, DrawFences[i], nullptr);
	}
	vkDestroyCommandPool(Devices.LogicalDevice, GraphicsCommandPool, nullptr);
	for (auto Framebuffer : SwapchainFramebuffers)
		vkDestroyFramebuffer(Devices.LogicalDevice, Framebuffer, nullptr);
	vkDestroyPipeline(Devices.LogicalDevice, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Devices.LogicalDevice, PipelineLayout, nullptr);
	vkDestroyRenderPass(Devices.LogicalDevice, RenderPass, nullptr);
	for (auto Image : SwapchainImages)
		vkDestroyImageView(Devices.LogicalDevice, Image.ImageView, nullptr);
	vkDestroySwapchainKHR(Devices.LogicalDevice, Swapchain, nullptr);
	vkDestroySurfaceKHR(VulkanInstance, VulkanSurface, nullptr);
	vkDestroyDevice(Devices.LogicalDevice, nullptr);
	DestroyDebugUtilsMessengerEXT(VulkanInstance, DebugMessenger, nullptr);
	vkDestroyInstance(VulkanInstance, nullptr);
}

void Renderer::CreateVulkanInstance()
{
	// Info about app itself, most is for debugging / devs.
	/*
	VkStructureType    sType;
    const void*        pNext;
    const char*        pApplicationName;
    uint32_t           applicationVersion;
    const char*        pEngineName;
    uint32_t           engineVersion;
    uint32_t           apiVersion;
	*/
	VkApplicationInfo ApplicationInfo = {};
	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pEngineName = "Smoldering Engine";
	ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3; // TODO: ANY ISSUES MAY BE CAUSED BY API_VERSION_1_3

	// Check that validation layers are supported
	if (!CheckValidationLayerSupport())
		throw std::runtime_error("Validation layers that were requested are not available on your PC!");

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

	/*
	VkStructureType             sType;
    const void*                 pNext;
    VkInstanceCreateFlags       flags;
    const VkApplicationInfo*    pApplicationInfo;
    uint32_t                    enabledLayerCount;
    const char* const*          ppEnabledLayerNames;
    uint32_t                    enabledExtensionCount;
    const char* const*          ppEnabledExtensionNames;
	*/
	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &ApplicationInfo;
	CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	CreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();

	// For debugging support
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	PopulateDebugMessengerCreateInfo(debugCreateInfo);
	CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	if (vkCreateInstance(&CreateInfo, nullptr, &VulkanInstance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create a VulkanInstance");
}

void Renderer::CreateLogicalDevice()
{
	QueueFamilyIndicies Indicies = GetQueueFamilies(Devices.PhysicalDevice);


	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfoVector;
	std::set<int> QueueFamilyIndicies = { Indicies.GraphicsFamily, Indicies.PresentationFamily };

	for (int QueueFamilyIndex : QueueFamilyIndicies)
	{
		/*
		VkStructureType             sType;
		const void*                 pNext;
		VkDeviceQueueCreateFlags    flags;
		uint32_t                    queueFamilyIndex;
		uint32_t                    queueCount;
		const float*                pQueuePriorities;
		*/
		VkDeviceQueueCreateInfo QueueCreateInfo = {};
		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;					// Index of family to create queue for
		QueueCreateInfo.queueCount = 1;											// number of queues to create
		float Priority = 1.0f;
		QueueCreateInfo.pQueuePriorities = &Priority;							// Incase of multiple queues 1.0 = highest priority

		QueueCreateInfoVector.push_back(QueueCreateInfo);
	}

	/*
	VkStructureType                    sType;
    const void*                        pNext;
    VkDeviceCreateFlags                flags;
    uint32_t                           queueCreateInfoCount;
    const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
    uint32_t                           enabledLayerCount;	// NOT NEEDED
    const char* const*                 ppEnabledLayerNames; // NOT NEEDED
    uint32_t                           enabledExtensionCount;
    const char* const*                 ppEnabledExtensionNames;
    const VkPhysicalDeviceFeatures*    pEnabledFeatures;
	*/
	VkDeviceCreateInfo DeviceCreateInfo = {};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfoVector.size());
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfoVector.data();
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	// Features on physical device that the logical device will use.
	VkPhysicalDeviceFeatures PhysicalDeviceFeatures = {};
	DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;

	VkResult Result = vkCreateDevice(Devices.PhysicalDevice, &DeviceCreateInfo, nullptr, &Devices.LogicalDevice);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device");

	// Get access to the Queues we just created while making the logical device
	vkGetDeviceQueue(Devices.LogicalDevice, Indicies.GraphicsFamily, 0, &GraphicsQueue);
	vkGetDeviceQueue(Devices.LogicalDevice, Indicies.PresentationFamily, 0, &PresentationQueue);
}

void Renderer::CreateVulkanSurface()
{
	// GLFW handles creating a surface that is specific to the OS of the PC.
	VkResult Result = glfwCreateWindowSurface(VulkanInstance, Window, nullptr, &VulkanSurface);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a presentation surface!");
}

void Renderer::CreateSwapChain()
{
	SwapchainDetails SwapchainInfo = GetSwapchainDetails(Devices.PhysicalDevice);

	// Find best surface values for swapchain before trying to create it
	VkSurfaceFormatKHR SurfaceFormat = ChooseBestSurfaceFormat(SwapchainInfo.SurfaceFormats);
	VkPresentModeKHR PresentationMode = ChooseBestPresentationMode(SwapchainInfo.PresentationModes);
	VkExtent2D SurfaceExtent = ChooseSwapChainExtent(SwapchainInfo.SurfaceCapabilities);

	// How many images are in the swap chain
	uint32_t ImageCount = SwapchainInfo.SurfaceCapabilities.minImageCount + 1; // +1 because we want atleast triple buffering
	if (SwapchainInfo.SurfaceCapabilities.maxImageCount > 0 // if 0 then limitless image count
		&& ImageCount > SwapchainInfo.SurfaceCapabilities.maxImageCount)
		ImageCount = SwapchainInfo.SurfaceCapabilities.maxImageCount;

	/*
	VkStructureType                  sType;
    const void*                      pNext;
    VkSwapchainCreateFlagsKHR        flags;
    VkSurfaceKHR                     surface;
    uint32_t                         minImageCount;
    VkFormat                         imageFormat;
    VkColorSpaceKHR                  imageColorSpace;
    VkExtent2D                       imageExtent;
    uint32_t                         imageArrayLayers;
    VkImageUsageFlags                imageUsage;
    VkSharingMode                    imageSharingMode;
    uint32_t                         queueFamilyIndexCount;
    const uint32_t*                  pQueueFamilyIndices;
    VkSurfaceTransformFlagBitsKHR    preTransform;
    VkCompositeAlphaFlagBitsKHR      compositeAlpha;
    VkPresentModeKHR                 presentMode;
    VkBool32                         clipped;
    VkSwapchainKHR                   oldSwapchain;
	*/
	VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
	SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainCreateInfo.surface = VulkanSurface;
	SwapchainCreateInfo.minImageCount = ImageCount;
	SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
	SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
	SwapchainCreateInfo.imageExtent = SurfaceExtent;
	SwapchainCreateInfo.imageArrayLayers = 1;												// Number of layers for each image in swap chain
	SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;					// We are attatching color to the images.
	
	// Fill information out about Queue families
	QueueFamilyIndicies Indicies = GetQueueFamilies(Devices.PhysicalDevice);
	if (Indicies.GraphicsFamily != Indicies.PresentationFamily)
	{
		uint32_t QueueFamilyIndicies[] = { (uint32_t)Indicies.GraphicsFamily,
										   (uint32_t)Indicies.PresentationFamily };

		// If families are different then swapchain must let images be shared
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		SwapchainCreateInfo.queueFamilyIndexCount = 2;
		SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndicies;
	}
	else
	{
		SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SwapchainCreateInfo.queueFamilyIndexCount = 1;
		uint32_t QueueFamilyIndex = static_cast<uint32_t>(Indicies.GraphicsFamily);
		SwapchainCreateInfo.pQueueFamilyIndices = &QueueFamilyIndex;
		//SwapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	SwapchainCreateInfo.preTransform = SwapchainInfo.SurfaceCapabilities.currentTransform;
	SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;					// How to handle blending other windows over this application
	SwapchainCreateInfo.presentMode = PresentationMode;
	SwapchainCreateInfo.clipped = VK_TRUE;													// Wether to clip parts of image behind other windows / off screen.
	SwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	std::cout << std::endl << std::endl << "v v v v Ignore these 4 errors, Will fix that shit some year v v v v" << std::endl << std::endl << std::endl;

	VkResult Result = vkCreateSwapchainKHR(Devices.LogicalDevice, &SwapchainCreateInfo, nullptr, &Swapchain);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a valid swapchain!");

	std::cout << std::endl << std::endl <<"^ ^ ^ ^ Ignore these 4 errors, Will fix that shit some year ^ ^ ^ ^" << std::endl << std::endl << std::endl;

	// Store for later use
	SwapchainImageFormat = SurfaceFormat.format;
	SwapchainExtent = SurfaceExtent;

	// Get the Swapchain image count now that we made the swapchain
	uint32_t SwapChainImageCount = 0;
	vkGetSwapchainImagesKHR(Devices.LogicalDevice, Swapchain, &SwapChainImageCount, nullptr);

	std::vector<VkImage> Images(SwapChainImageCount);
	vkGetSwapchainImagesKHR(Devices.LogicalDevice, Swapchain, &SwapChainImageCount, Images.data());

	for (VkImage Image : Images)
	{
		SwapchainImage SESwapchainImage = {};
		SESwapchainImage.Image = Image;
		SESwapchainImage.ImageView = CreateImageView(Image, SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Add image to swapchain image vector
		SwapchainImages.push_back(SESwapchainImage);
	}
}

void Renderer::CreateRenderpass()
{
	// Color attatchment of the renderpass
	/*
	VkAttachmentDescriptionFlags    flags;
    VkFormat                        format;
    VkSampleCountFlagBits           samples;
    VkAttachmentLoadOp              loadOp;
    VkAttachmentStoreOp             storeOp;
    VkAttachmentLoadOp              stencilLoadOp;
    VkAttachmentStoreOp             stencilStoreOp;
    VkImageLayout                   initialLayout;
    VkImageLayout                   finalLayout;
	*/
	VkAttachmentDescription ColorAttachment = {};
	ColorAttachment.format = SwapchainImageFormat;
	ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;		// Number of samples to write for multisampling
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// Framebuffer data will be stored as an image but images can be given different layouts for optimal use.
	ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Attatchment reference uses an attachment index that refers to index in the attachment list passed to renderpasscreateinfo
	VkAttachmentReference ColorAttachmentReference = {};
	ColorAttachmentReference.attachment = 0;
	ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	/*
	VkSubpassDescriptionFlags       flags;
    VkPipelineBindPoint             pipelineBindPoint;
    uint32_t                        inputAttachmentCount;
    const VkAttachmentReference*    pInputAttachments;
    uint32_t                        colorAttachmentCount;
    const VkAttachmentReference*    pColorAttachments;
    const VkAttachmentReference*    pResolveAttachments;
    const VkAttachmentReference*    pDepthStencilAttachment;
    uint32_t                        preserveAttachmentCount;
    const uint32_t*                 pPreserveAttachments;
	*/
	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentReference;

	// Determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 2> SubpassDependencies;
	// conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after ->
	SubpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	SubpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// Transition must happen before ->
	SubpassDependencies[0].dstSubpass = 0;
	SubpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	SubpassDependencies[0].dependencyFlags = 0;

	// conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after ->
	SubpassDependencies[1].srcSubpass = 0;
	SubpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// Transition must happen before ->
	SubpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	SubpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	SubpassDependencies[1].dependencyFlags = 0;

	/*
	VkStructureType                   sType;
    const void*                       pNext;
    VkRenderPassCreateFlags           flags;
    uint32_t                          attachmentCount;
    const VkAttachmentDescription*    pAttachments;
    uint32_t                          subpassCount;
    const VkSubpassDescription*       pSubpasses;
    uint32_t                          dependencyCount;
    const VkSubpassDependency*        pDependencies;
	*/
	VkRenderPassCreateInfo RenderPassCreateInfo = {};
	RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassCreateInfo.attachmentCount = 1;
	RenderPassCreateInfo.pAttachments = &ColorAttachment;
	RenderPassCreateInfo.subpassCount = 1;
	RenderPassCreateInfo.pSubpasses = &Subpass;
	RenderPassCreateInfo.dependencyCount = static_cast<uint32_t>(SubpassDependencies.size());
	RenderPassCreateInfo.pDependencies = SubpassDependencies.data();

	VkResult Result = vkCreateRenderPass(Devices.LogicalDevice, &RenderPassCreateInfo, nullptr, &RenderPass);
	
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create renderpass!");
}

void Renderer::CreateGraphicsPipeline()
{
#pragma region Shader Stage Creation
	// Read in SPIR-V code
	std::vector<char> VertexShaderCode = ReadFile(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Engine/Shaders/vert.spv");
	std::vector<char> FragmentShaderCode = ReadFile(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Engine/Shaders/frag.spv");

	// Convert the SPIR-V code into shader modules
	VkShaderModule VertexShaderModule = CreateShaderModule(VertexShaderCode);
	VkShaderModule FragmentShaderModule = CreateShaderModule(FragmentShaderCode);

	/*
	VkStructureType                     sType;
	const void*                         pNext;
	VkPipelineShaderStageCreateFlags    flags;
	VkShaderStageFlagBits               stage;
	VkShaderModule                      module;
	const char*                         pName;
	const VkSpecializationInfo*         pSpecializationInfo;
	*/
	VkPipelineShaderStageCreateInfo VertexShaderStageCreateInfo = {};
	VertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCreateInfo.module = VertexShaderModule;
	VertexShaderStageCreateInfo.pName = "main"; // run the "main" function in the shader	

	VkPipelineShaderStageCreateInfo FragmentShaderStageCreateInfo = {};
	FragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCreateInfo.module = FragmentShaderModule;
	FragmentShaderStageCreateInfo.pName = "main"; // run the "main" function in the shader

	VkPipelineShaderStageCreateInfo ShaderStages[] = { VertexShaderStageCreateInfo, FragmentShaderStageCreateInfo };
#pragma endregion

#pragma region Vertex Input
	/*
VkStructureType                             sType;
const void*                                 pNext;
VkPipelineVertexInputStateCreateFlags       flags;
uint32_t                                    vertexBindingDescriptionCount;
const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
uint32_t                                    vertexAttributeDescriptionCount;
const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
*/
	VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = {};
	VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	VertexInputCreateInfo.pVertexBindingDescriptions = nullptr;		// List of vertex binding descriptions (data spacing and stride info)
	VertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	VertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;	// List of vertex attribute descriptions (data format and where to bind to/from)  
#pragma endregion

#pragma region Input Assembly
	/*
	VkStructureType                            sType;
	const void*                                pNext;
	VkPipelineInputAssemblyStateCreateFlags    flags;
	VkPrimitiveTopology                        topology;
	VkBool32                                   primitiveRestartEnable;
	*/
	VkPipelineInputAssemblyStateCreateInfo InputAssembly = {};
	InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssembly.primitiveRestartEnable = VK_FALSE;
#pragma endregion

#pragma region Viewport and Scissor
	VkViewport Viewport = {};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapchainExtent.width;
	Viewport.height = (float)SwapchainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	VkRect2D Scissor = {};
	Scissor.offset = { 0,0 };
	Scissor.extent = SwapchainExtent;

	/*
	VkStructureType                       sType;
	const void*                           pNext;
	VkPipelineViewportStateCreateFlags    flags;
	uint32_t                              viewportCount;
	const VkViewport*                     pViewports;
	uint32_t                              scissorCount;
	const VkRect2D*                       pScissors;
	*/
	VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = {};
	ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportStateCreateInfo.viewportCount = 1;
	ViewportStateCreateInfo.pViewports = &Viewport;
	ViewportStateCreateInfo.scissorCount = 1;
	ViewportStateCreateInfo.pScissors = &Scissor;
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
					/*
	VkStructureType                            sType;
	const void*                                pNext;
	VkPipelineRasterizationStateCreateFlags    flags;
	VkBool32                                   depthClampEnable;
	VkBool32                                   rasterizerDiscardEnable;
	VkPolygonMode                              polygonMode;
	VkCullModeFlags                            cullMode;
	VkFrontFace                                frontFace;
	VkBool32                                   depthBiasEnable;
	float                                      depthBiasConstantFactor;
	float                                      depthBiasClamp;
	float                                      depthBiasSlopeFactor;
	float                                      lineWidth;
	*/
	VkPipelineRasterizationStateCreateInfo RasterizationCreateInfo = {};
	RasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizationCreateInfo.depthClampEnable = VK_FALSE;			// Requires DepthClamp = true on device features. in order to enable
	RasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;		// Wether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer ouput
	RasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;		// When a polygon is drawn, what do we do (e.g. we want if colored)
	RasterizationCreateInfo.lineWidth = 1.0f;						// How thick lines should be when drawn (if we want VK_POLYGON_MODE_LINE above)
	RasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;		// Do not draw back side of triangles.
	RasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;	// Basically helps figure out what the backside of the triangle is.
	RasterizationCreateInfo.depthBiasClamp = VK_FALSE;				// If we should add depth bias to fragments (good for stopping "shadow acne")
#pragma endregion

#pragma region Multisampling
	/*
	VkStructureType                          sType;
    const void*                              pNext;
    VkPipelineMultisampleStateCreateFlags    flags;
    VkSampleCountFlagBits                    rasterizationSamples;
    VkBool32                                 sampleShadingEnable;
    float                                    minSampleShading;
    const VkSampleMask*                      pSampleMask;
    VkBool32                                 alphaToCoverageEnable;
    VkBool32                                 alphaToOneEnable;
	*/
	VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo = {};
	MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// number of samples to use per fragment  
#pragma endregion

#pragma region Color Blending
	/*
	VkBool32                 blendEnable;
    VkBlendFactor            srcColorBlendFactor;
    VkBlendFactor            dstColorBlendFactor;
    VkBlendOp                colorBlendOp;
    VkBlendFactor            srcAlphaBlendFactor;
    VkBlendFactor            dstAlphaBlendFactor;
    VkBlendOp                alphaBlendOp;
    VkColorComponentFlags    colorWriteMask;
	*/
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

	/*
	VkStructureType                               sType;
    const void*                                   pNext;
    VkPipelineColorBlendStateCreateFlags          flags;
    VkBool32                                      logicOpEnable;
    VkLogicOp                                     logicOp;
    uint32_t                                      attachmentCount;
    const VkPipelineColorBlendAttachmentState*    pAttachments;
    float                                         blendConstants[4];
	*/
	VkPipelineColorBlendStateCreateInfo ColorBlendingCreateInfo = {};
	ColorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendingCreateInfo.logicOpEnable = VK_FALSE;
	ColorBlendingCreateInfo.attachmentCount = 1;
	ColorBlendingCreateInfo.pAttachments = &ColorState;
#pragma endregion

#pragma region Pipeline Layout
	VkPipelineLayoutCreateInfo PipelineCreateInfo = {};
	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineCreateInfo.setLayoutCount = 0;
	PipelineCreateInfo.pSetLayouts = nullptr;
	PipelineCreateInfo.pushConstantRangeCount = 0;
	PipelineCreateInfo.pPushConstantRanges = nullptr;

	VkResult Result = vkCreatePipelineLayout(Devices.LogicalDevice, &PipelineCreateInfo, nullptr, &PipelineLayout);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout");
#pragma endregion
	
	// TODO: Depth Stencil

	// Create Graphics Pipeline
	VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo = {};
	GraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	GraphicsPipelineCreateInfo.stageCount = 2;
	GraphicsPipelineCreateInfo.pStages = ShaderStages;
	GraphicsPipelineCreateInfo.pVertexInputState = &VertexInputCreateInfo;
	GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssembly;
	GraphicsPipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
	GraphicsPipelineCreateInfo.pDynamicState = nullptr;
	GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationCreateInfo;
	GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleCreateInfo;
	GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
	GraphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	GraphicsPipelineCreateInfo.layout = PipelineLayout;
	GraphicsPipelineCreateInfo.renderPass = RenderPass;
	GraphicsPipelineCreateInfo.subpass = 0;
	// Use if we want to create multiple pipelines deriving from eachother
	GraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	GraphicsPipelineCreateInfo.basePipelineIndex = -1;

	Result = vkCreateGraphicsPipelines(Devices.LogicalDevice, VK_NULL_HANDLE, 1, &GraphicsPipelineCreateInfo, nullptr, &GraphicsPipeline);
	
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");

	// Destroy shader modules
	vkDestroyShaderModule(Devices.LogicalDevice, FragmentShaderModule, nullptr);
	vkDestroyShaderModule(Devices.LogicalDevice, VertexShaderModule, nullptr);
}

void Renderer::CreateFramebuffers()
{
	SwapchainFramebuffers.resize(SwapchainImages.size());

	for (size_t i = 0; i < SwapchainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 1> Attachments = {
		SwapchainImages[i].ImageView
		};

		/*
		VkStructureType             sType;
		const void*                 pNext;
		VkFramebufferCreateFlags    flags;
		VkRenderPass                renderPass;
		uint32_t                    attachmentCount;
		const VkImageView*          pAttachments;
		uint32_t                    width;
		uint32_t                    height;
		uint32_t                    layers;
		*/
		VkFramebufferCreateInfo FramebufferCreateInfo = {};
		FramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FramebufferCreateInfo.renderPass = RenderPass;
		FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
		FramebufferCreateInfo.pAttachments = Attachments.data();
		FramebufferCreateInfo.width = SwapchainExtent.width;
		FramebufferCreateInfo.height = SwapchainExtent.height;
		FramebufferCreateInfo.layers = 1;
		
		VkResult Result = vkCreateFramebuffer(Devices.LogicalDevice, &FramebufferCreateInfo, nullptr, &SwapchainFramebuffers[i]);

		if (Result != VK_SUCCESS)
			throw std::runtime_error("Failed to create a frame buffer!");
	}
}

void Renderer::AllocateCommandBuffers()
{
	CommandBuffers.resize(SwapchainFramebuffers.size());

	/*
	VkStructureType         sType;
    const void*             pNext;
    VkCommandPool           commandPool;
    VkCommandBufferLevel    level;
    uint32_t                commandBufferCount;
	*/
	VkCommandBufferAllocateInfo CommandBufferAllocationInfo = {};
	CommandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocationInfo.commandPool = GraphicsCommandPool;
	CommandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// Primary are buffers you submit directly to queues, secondary are executed by a primary buffers by using VkCmdExecuteCommands.
	CommandBufferAllocationInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

	VkResult Result = vkAllocateCommandBuffers(Devices.LogicalDevice, &CommandBufferAllocationInfo, CommandBuffers.data());

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!");
}

void Renderer::CreateCommandPool()
{
	QueueFamilyIndicies FamilyIndicies = GetQueueFamilies(Devices.PhysicalDevice);

	/*
	VkStructureType             sType;
    const void*                 pNext;
    VkCommandPoolCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
	*/
	VkCommandPoolCreateInfo CommandPoolInfo = {};
	CommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolInfo.queueFamilyIndex = FamilyIndicies.GraphicsFamily;	// Queue family type that buffers from this command pool will use
	
	VkResult Result = vkCreateCommandPool(Devices.LogicalDevice, &CommandPoolInfo, nullptr, &GraphicsCommandPool);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a command pool!");
}

void Renderer::CreateSynchronizationPrimatives()
{
	ImageAvailableSemaphores.resize(MAX_FRAME_DRAWS);
	RenderingCompleteSemaphores.resize(MAX_FRAME_DRAWS);
	DrawFences.resize(MAX_FRAME_DRAWS);

	VkFenceCreateInfo FenceCreateInfo = {};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(Devices.LogicalDevice, &SemaphoreCreateInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(Devices.LogicalDevice, &SemaphoreCreateInfo, nullptr, &RenderingCompleteSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(Devices.LogicalDevice, &FenceCreateInfo, nullptr, &DrawFences[i]))
			throw std::runtime_error("Failed to create a semaphore or fence!");
	}
}

void Renderer::GetPhysicalDevice()
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(VulkanInstance, &DeviceCount, nullptr);

	if (DeviceCount == 0)
	{
		throw std::runtime_error("Could not find physical devices that support Vulkan!");
	}

	std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
	vkEnumeratePhysicalDevices(VulkanInstance, &DeviceCount, PhysicalDevices.data());

	for (const auto& Device : PhysicalDevices)
	{
		if (CheckForBestPhysicalDevice(Device))
		{
			Devices.PhysicalDevice = Device;
			break;
		}
	}
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
	// check for graphics queue families
	QueueFamilyIndicies Indicies = GetQueueFamilies(InPhysicalDevice);

	// check for requested extensions
	bool ExtensionsSupported = CheckDeviceExtentionSupport(InPhysicalDevice);

	// check swapchain support
	SwapchainDetails SwapchainInfo = GetSwapchainDetails(InPhysicalDevice);
	bool IsSwapChainValid = !SwapchainInfo.PresentationModes.empty() && !SwapchainInfo.SurfaceFormats.empty();

	return Indicies.IsValid() && ExtensionsSupported && IsSwapChainValid;
}

bool Renderer::CheckDeviceExtentionSupport(VkPhysicalDevice InPhysicalDevice)
{
	uint32_t ExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(InPhysicalDevice, nullptr, &ExtensionCount, nullptr);
	
	if (ExtensionCount == 0)
		return false;
	
	std::vector<VkExtensionProperties> Extensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(InPhysicalDevice, nullptr, &ExtensionCount, Extensions.data());

	for (const auto& DeviceExtension : DeviceExtensions)
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
	for (const char* LayerName : ValidationLayers) 
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
	VkResult Result = vkCreateImageView(Devices.LogicalDevice, &ImageViewInfo, nullptr, &ImageView);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create image views!");

	return ImageView;
}

VkShaderModule Renderer::CreateShaderModule(const std::vector<char>& InCode)
{
	/*
	VkStructureType              sType;
    const void*                  pNext;
    VkShaderModuleCreateFlags    flags;
    size_t                       codeSize;
    const uint32_t*              pCode;
	*/
	VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
	ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCreateInfo.codeSize = InCode.size();
	ShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(InCode.data());

	VkShaderModule ShaderModule;
	VkResult Result = vkCreateShaderModule(Devices.LogicalDevice, &ShaderModuleCreateInfo, nullptr, &ShaderModule);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");

	return ShaderModule;
}

void Renderer::RecordCommands()
{
	VkCommandBufferBeginInfo BufferBeginInfo = {};
	BufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Info on how to begin a render pass (for graphical applications)
	VkRenderPassBeginInfo RenderPassBeginInfo = {};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.renderPass = RenderPass;
	RenderPassBeginInfo.renderArea.offset = { 0, 0 };
	RenderPassBeginInfo.renderArea.extent = SwapchainExtent;
	VkClearValue ClearValues[] = { {0.6f, 0.6f, 0.4f, 1.0f} };
	RenderPassBeginInfo.pClearValues = ClearValues;				// Clear values, only 1 currently because we only have 1 attachment
	RenderPassBeginInfo.clearValueCount = 1;

	for (size_t i = 0; i < CommandBuffers.size(); i++)
	{
		// Start recording
		VkResult Result = vkBeginCommandBuffer(CommandBuffers[i],&BufferBeginInfo);
		if (Result != VK_SUCCESS)
			throw std::runtime_error("Failed to start recording a command buffer!");

		// start the render pass
		RenderPassBeginInfo.framebuffer = SwapchainFramebuffers[i];
		vkCmdBeginRenderPass(CommandBuffers[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind the pipeline to render pass 
		vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
		// you can bind more stuff here

		// Execute the pipeline
		vkCmdDraw(CommandBuffers[i], 3, 1, 0, 0);
		// you can draw more stuff here also

		vkCmdEndRenderPass(CommandBuffers[i]);

		// Stop recording
		Result = vkEndCommandBuffer(CommandBuffers[i]);
		if (Result != VK_SUCCESS)
			throw std::runtime_error("Failed to stop recording a command buffer!");
	}
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
		glfwGetFramebufferSize(Window, &width, &height);

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
			Indicies.GraphicsFamily = Index;
		}

		// Check if the Queue Family supports presentation
		VkBool32 PresentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(InPhysicalDevice, Index, VulkanSurface, &PresentationSupport);

		if (QueueFamily.queueCount > 0 && PresentationSupport)
			Indicies.PresentationFamily = Index;

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

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InPhysicalDevice, VulkanSurface, &SwapChainInfo.SurfaceCapabilities);

	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, VulkanSurface, &FormatCount, nullptr);

	if (FormatCount != 0)
	{
		SwapChainInfo.SurfaceFormats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, VulkanSurface, &FormatCount, SwapChainInfo.SurfaceFormats.data());
	}

	uint32_t PresentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, VulkanSurface, &PresentationCount, nullptr);

	if (PresentationCount != 0)
	{
		SwapChainInfo.PresentationModes.resize(PresentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, VulkanSurface, &PresentationCount, SwapChainInfo.PresentationModes.data());
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

	VkResult Result = CreateDebugUtilsMessengerEXT(VulkanInstance, &createInfo, nullptr, &DebugMessenger);

	if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger!");
	}
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

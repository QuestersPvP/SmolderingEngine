#include "Engine/Public/Rendering/Renderer.h"

// Project Includes
#include "Engine/Public/Camera/Camera.h"
#include "Engine/Public/EngineLevel/EngineLevelManager.h"


void Renderer::UpdateModelPosition(int inModelId, glm::mat4 inModelMatrix, float inRotation)
{
	// TODO: FIX THIS SHIT - should not even be in renderer
	if (inModelId >= SEGame->gameObjects.size() || inModelId > MAX_OBJECTS) return;

	//SEGame->gameObjects[inModelId]->ApplyLocalYRotation(inRotation);
	SEGame->gameObjects[inModelId]->SetModel(inModelMatrix);

	//if (SEGame->gameObjects[inModelId]->HasChildObjects())
	//	SEGame->gameObjects[inModelId]->ApplyLocalRotation(inRotation);

	//SEGame->gameObjects[1]->objectMesh.SetModel(inModelMatrix * SEGame->gameObjects[1]->objectMesh.GetModel().modelMatrix);

	//SEGame->gameObjects[inModelId]->objectMesh->SetModel(inModelMatrix);
}

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

int Renderer::InitRenderer(GLFWwindow* inWindow, Game* inGame, class Camera* inCamera)
{
	Window = inWindow;
	SEGame = inGame;
	seCamera = inCamera;

	try
	{
		CreateVulkanInstance();

		if (ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS)
			SetupDebugMessenger();

		CreateVulkanSurface();
		RetrievePhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateRenderpass();
		CreateDescriptorSetLayout();
		CreatePushConstantRange();
		CreateGraphicsPipeline();
		CreateDepthBufferImage();
		CreateFramebuffers();
		CreateCommandPool();
		AllocateCommandBuffers();
		CreateTextureSampler();
		//AllocateDynamicBufferTransferSpace();
		CreateUniformBuffers();
		CreateDescriptorPool();
		AllocateDescriptorSets();
		CreateSynchronizationPrimatives();

		//// TODO: STOP HARDCODING THIS
		//std::string fileLoc = (std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Textures/BlankTexture.jpg");
		//int blankTexture = CreateTexture(fileLoc);

		//// Matrix creation									//FOV						// Aspect ratio									// near, far plane
		//uboViewProjection.projection = glm::perspective(glm::radians(45.0f), (float)SwapchainExtent.width / (float)SwapchainExtent.height, 0.1f, 1000.f);
		////uboViewProjection.projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.1f, 100.0f);
		//										// where camera is				// where we are looking			// Y is up
		//uboViewProjection.view = glm::lookAt(glm::vec3(0.0f, 20.0f, -50.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		//// invert the Y (Vulkan treats Y as downwards, so if we invert it then Y is up.)
		//uboViewProjection.projection[1][1] *= -1;

		// Mesh creation
		//SEGame->LoadMeshes(Devices.PhysicalDevice, Devices.LogicalDevice, GraphicsQueue, GraphicsCommandPool);

		/*
		std::string filePath = std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Models/House/Heilig_Grab_Kapelle_C.obj";
		SEGame->LoadMeshModel(Devices.PhysicalDevice, Devices.LogicalDevice, GraphicsQueue, GraphicsCommandPool, filePath, this);

		// model was rotated strange - this is just to fix that for now.
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		SEGame->gameObjects[0]->SetModel(modelMatrix);
		*/

		//for (size_t i = 0; i < SEGame->gameObjects.size(); i++)
		//{
		//	if (!SEGame->gameObjects[i]->objectMesh.GetTextureFilePath().empty() &&				// not empty
		//		SEGame->gameObjects[i]->objectMesh.GetTextureFilePath() != "BlankTexture.jpg")	// not blank texture
		//	{
		//		SEGame->gameObjects[i]->objectMesh.SetTextureID(CreateTexture(SEGame->gameObjects[i]->objectMesh.GetTextureFilePath()));
		//		SEGame->gameObjects[i]->SetUseTexture(1);
		//	}
		//	else
		//	{
		//		SEGame->gameObjects[i]->objectMesh.SetTextureID(blankTexture);
		//		SEGame->gameObjects[i]->SetUseTexture(0);
		//	}
		//}

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
	if (shouldLoadNewLevel)
	{
		levelManager->LoadNewScene();
		shouldLoadNewLevel = false;
	}
	if (shouldSaveLevel)
	{
		// get where they want to save and replace all the \\ with / so it works in file explorer
		std::string fileName = levelManager->SaveFileExplorer();
		std::replace(fileName.begin(), fileName.end(), '\\', '/');


		levelManager->SaveLevel(fileName);
		shouldSaveLevel = false;
	}

	// Wait for given fence to signal/open from last draw call before continuing
	vkWaitForFences(Devices.LogicalDevice, 1, &DrawFences[CurrentFrame], VK_TRUE , std::numeric_limits<uint64_t>::max());
	// Reset/close the fence again as we work on this new draw call.
	vkResetFences(Devices.LogicalDevice, 1, &DrawFences[CurrentFrame]);

	// Aquire the next image we want to draw
	uint32_t ImageIndex;
	VkResult Result = vkAcquireNextImageKHR(Devices.LogicalDevice, Swapchain, std::numeric_limits<uint64_t>::max(), ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);
	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire next image!");

	RecordCommands(ImageIndex);
	UpdateUniformBuffers(ImageIndex);

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

	//_aligned_free(modelTransferSpace);

	//  --- Destroy ImGui --- 
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(Devices.LogicalDevice, imguiDescriptorPool, nullptr);
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	//  --- Destroy ImGui --- 

	// --- Destroy GameObject Meshes ---
	levelManager->DestroyGameMeshes();
	// --- Destroy GameObject Meshes ---

	vkDestroyDescriptorPool(Devices.LogicalDevice, samplerDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(Devices.LogicalDevice, samplerSetLayout, nullptr);
	vkDestroySampler(Devices.LogicalDevice, textureSampler, nullptr);

	for (size_t i = 0; i < textureImages.size(); i++)
	{
		vkDestroyImageView(Devices.LogicalDevice, textureImageViews[i], nullptr);
		vkDestroyImage(Devices.LogicalDevice, textureImages[i], nullptr);
		vkFreeMemory(Devices.LogicalDevice, textureImageMemory[i], nullptr);
	}

	vkDestroyImageView(Devices.LogicalDevice, depthBufferImageView, nullptr);
	vkDestroyImage(Devices.LogicalDevice, depthBufferImage, nullptr);
	vkFreeMemory(Devices.LogicalDevice, depthBufferImageMemory, nullptr);

	vkDestroyDescriptorPool(Devices.LogicalDevice, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(Devices.LogicalDevice, descriptorSetLayout, nullptr);
	for (size_t i = 0; i < SwapchainImages.size(); i++)
	{
		vkDestroyBuffer(Devices.LogicalDevice, viewProjectionUniformBuffers[i], nullptr);
		vkFreeMemory(Devices.LogicalDevice, viewProjectionUniformBufferMemory[i], nullptr);		
		//vkDestroyBuffer(Devices.LogicalDevice, modelDynamicUniformBuffers[i], nullptr);
		//vkFreeMemory(Devices.LogicalDevice, modelDynamicUniformBufferMemory[i], nullptr);
	}

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(Devices.LogicalDevice, RenderingCompleteSemaphores[i], nullptr);
		vkDestroySemaphore(Devices.LogicalDevice, ImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Devices.LogicalDevice, DrawFences[i], nullptr);
	}
	vkDestroyCommandPool(Devices.LogicalDevice, GraphicsCommandPool, nullptr);
	for (auto framebuffer : SwapchainFramebuffers)
		vkDestroyFramebuffer(Devices.LogicalDevice, framebuffer, nullptr);
	vkDestroyPipeline(Devices.LogicalDevice, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Devices.LogicalDevice, PipelineLayout, nullptr);
	vkDestroyRenderPass(Devices.LogicalDevice, RenderPass, nullptr);
	for (auto image : SwapchainImages)
		vkDestroyImageView(Devices.LogicalDevice, image.imageView, nullptr);
	vkDestroySwapchainKHR(Devices.LogicalDevice, Swapchain, nullptr);
	vkDestroySurfaceKHR(VulkanInstance, VulkanSurface, nullptr);
	vkDestroyDevice(Devices.LogicalDevice, nullptr);

	if (ENABLE_VULKAN_DEBUG_VALIDATION_LAYERS)
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

	if (vkCreateInstance(&CreateInfo, nullptr, &VulkanInstance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create a VulkanInstance");
}

void Renderer::CreateLogicalDevice()
{
	QueueFamilyIndicies Indicies = GetQueueFamilies(Devices.PhysicalDevice);


	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfoVector;
	std::set<int> QueueFamilyIndicies = { Indicies.graphicsFamily, Indicies.presentationFamily };

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
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	DeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	// Features on physical device that the logical device will use.
	VkPhysicalDeviceFeatures PhysicalDeviceFeatures = {};
	PhysicalDeviceFeatures.samplerAnisotropy = VK_TRUE;		// Enable Anisotropy
	DeviceCreateInfo.pEnabledFeatures = &PhysicalDeviceFeatures;

	VkResult Result = vkCreateDevice(Devices.PhysicalDevice, &DeviceCreateInfo, nullptr, &Devices.LogicalDevice);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device");

	// Get access to the Queues we just created while making the logical device
	vkGetDeviceQueue(Devices.LogicalDevice, Indicies.graphicsFamily, 0, &GraphicsQueue);
	vkGetDeviceQueue(Devices.LogicalDevice, Indicies.presentationFamily, 0, &PresentationQueue);
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
	VkSurfaceFormatKHR SurfaceFormat = ChooseBestSurfaceFormat(SwapchainInfo.surfaceFormats);
	VkPresentModeKHR PresentationMode = ChooseBestPresentationMode(SwapchainInfo.presentationModes);
	VkExtent2D SurfaceExtent = ChooseSwapChainExtent(SwapchainInfo.surfaceCapabilities);

	// How many images are in the swap chain
	uint32_t ImageCount = SwapchainInfo.surfaceCapabilities.minImageCount + 1; // +1 because we want atleast triple buffering
	if (SwapchainInfo.surfaceCapabilities.maxImageCount > 0 // if 0 then limitless image count
		&& ImageCount > SwapchainInfo.surfaceCapabilities.maxImageCount)
		ImageCount = SwapchainInfo.surfaceCapabilities.maxImageCount;

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

	//std::cout << std::endl << std::endl << "v v v v Ignore these 4 errors, Will fix that shit some year v v v v" << std::endl << std::endl << std::endl;

	VkResult Result = vkCreateSwapchainKHR(Devices.LogicalDevice, &SwapchainCreateInfo, nullptr, &Swapchain);

	if (Result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a valid swapchain!");

	std::cout << std::endl << std::endl <<"^ ^ ^ ^ Ignore errors above here, Will fix them some year ^ ^ ^ ^" << std::endl << std::endl << std::endl;

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
		SESwapchainImage.image = Image;
		SESwapchainImage.imageView = CreateImageView(Image, SwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Add image to swapchain image vector
		SwapchainImages.push_back(SESwapchainImage);
	}
}

void Renderer::CreateRenderpass()
{
	// Color attachment of the renderpass
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
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = SwapchainImageFormat;
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
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(Devices.LogicalDevice, &renderPassCreateInfo, nullptr, &RenderPass);
	
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create renderpass!");
}

void Renderer::CreateDescriptorSetLayout()
{
	// Model View Projection binding info
	VkDescriptorSetLayoutBinding viewProjectionLayoutBinding = {};
	viewProjectionLayoutBinding.binding = 0;										// binding point in shader
	viewProjectionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	viewProjectionLayoutBinding.descriptorCount = 1;
	viewProjectionLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;			// it is bound in the vertex shader
	viewProjectionLayoutBinding.pImmutableSamplers = nullptr;

	/*VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1;
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelLayoutBinding.descriptorCount = 1;
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelLayoutBinding.pImmutableSamplers = nullptr;*/

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { viewProjectionLayoutBinding/*, modelLayoutBinding*/ };

	// Create descripor set layout with given bindings
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();
	
	// Create descriptor set layout
	VkResult result = vkCreateDescriptorSetLayout(Devices.LogicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout!");

	// CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
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
	result = vkCreateDescriptorSetLayout(Devices.LogicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a Descriptor Set Layout!");
}

void Renderer::CreatePushConstantRange()
{
	// Define push constant values
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;	// push constant will be in vertex shader
	pushConstantRange.offset = 0;								// start at begining of data
	pushConstantRange.size = sizeof(Model);						// hold the size of the Models data
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
	VertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	VertexInputCreateInfo.pVertexBindingDescriptions = &VertexBindingDescription;		// List of vertex binding descriptions (data spacing and stride info)
	VertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescriptions.size());
	VertexInputCreateInfo.pVertexAttributeDescriptions = AttributeDescriptions.data();	// List of vertex attribute descriptions (data format and where to bind to/from)  
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
	RasterizationCreateInfo.depthClampEnable = VK_FALSE;					// Requires DepthClamp = true on device features. in order to enable
	RasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;				// Wether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer ouput
	RasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;				// When a polygon is drawn, what do we do (e.g. we want if colored)
	RasterizationCreateInfo.lineWidth = 1.0f;								// How thick lines should be when drawn (if we want VK_POLYGON_MODE_LINE above)
	RasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;				// Do not draw back side of triangles.
	RasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Basically helps figure out what the backside of the triangle is.
	RasterizationCreateInfo.depthBiasClamp = VK_FALSE;						// If we should add depth bias to fragments (good for stopping "shadow acne")
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
	std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { descriptorSetLayout, samplerSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	VkResult Result = vkCreatePipelineLayout(Devices.LogicalDevice, &pipelineLayoutCreateInfo, nullptr, &PipelineLayout);
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
	GraphicsPipelineCreateInfo.pInputAssemblyState = &InputAssembly;
	GraphicsPipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
	GraphicsPipelineCreateInfo.pDynamicState = nullptr;
	GraphicsPipelineCreateInfo.pRasterizationState = &RasterizationCreateInfo;
	GraphicsPipelineCreateInfo.pMultisampleState = &MultisampleCreateInfo;
	GraphicsPipelineCreateInfo.pColorBlendState = &ColorBlendingCreateInfo;
	GraphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
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

void Renderer::CreateDepthBufferImage()
{
	// Get supported formats for depth buffer
	VkFormat depthAttachmentFormat = ChooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,  VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	
	// Create depth buffer image
	depthBufferImage = CreateImage(SwapchainExtent.width, SwapchainExtent.height, depthAttachmentFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory);

	depthBufferImageView = CreateImageView(depthBufferImage, depthAttachmentFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::CreateFramebuffers()
{
	SwapchainFramebuffers.resize(SwapchainImages.size());

	for (size_t i = 0; i < SwapchainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
		SwapchainImages[i].imageView,
		depthBufferImageView
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
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = RenderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.width = SwapchainExtent.width;
		framebufferCreateInfo.height = SwapchainExtent.height;
		framebufferCreateInfo.layers = 1;
		
		VkResult result = vkCreateFramebuffer(Devices.LogicalDevice, &framebufferCreateInfo, nullptr, &SwapchainFramebuffers[i]);

		if (result != VK_SUCCESS)
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

//void Renderer::AllocateDynamicBufferTransferSpace()
//{
//	// Basically all models need to be aligned based off of minUniformBufferOffset. So for example if it is 32 but the object is
//	// 48 bits we would need to allocate 64 bits of memory for it. If the model is only 16 bits then we only need 32 bits for it etc.
//	modelUniformAlignment = (sizeof(Model) + minUniformBufferOffset) & ~(minUniformBufferOffset - 1);
//
//	// Create a chunk of memory to hold our dynamic buffer that is aligned to our modelUniformAlignment and holds a maximum number of objects (MAX_OBJECTS)
//	modelTransferSpace = (Model*)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
//}

void Renderer::CreateCommandPool()
{
	QueueFamilyIndicies familyIndicies = GetQueueFamilies(Devices.PhysicalDevice);

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
	
	VkResult result = vkCreateCommandPool(Devices.LogicalDevice, &commandPoolInfo, nullptr, &GraphicsCommandPool);

	if (result != VK_SUCCESS)
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

void Renderer::CreateUniformBuffers()
{
	VkDeviceSize viewProjectionBufferSize = sizeof(UniformBufferObjectViewProjection);
	//VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

	viewProjectionUniformBuffers.resize(SwapchainImages.size());
	viewProjectionUniformBufferMemory.resize(SwapchainImages.size());	
	
	//modelDynamicUniformBuffers.resize(SwapchainImages.size());
	//modelDynamicUniformBufferMemory.resize(SwapchainImages.size());

	for (size_t i = 0; i < SwapchainImages.size(); i++)
	{
		CreateBuffer(Devices.PhysicalDevice, Devices.LogicalDevice, viewProjectionBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &viewProjectionUniformBuffers[i], &viewProjectionUniformBufferMemory[i]);		
		
		//CreateBuffer(Devices.PhysicalDevice, Devices.LogicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,	// even though this is dynamic uniform buffer, it is treated same as uniform buffer.
		//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDynamicUniformBuffers[i], &modelDynamicUniformBufferMemory[i]);
	}
}

void Renderer::CreateDescriptorPool()
{
	// type of descriptor and how many descriptors.
	// View projection pool
	VkDescriptorPoolSize viewProjectionDescriptorPoolSize = {};
	viewProjectionDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	viewProjectionDescriptorPoolSize.descriptorCount = static_cast<uint32_t>(viewProjectionUniformBuffers.size());	
	
	//// Dynamic model pool
	//VkDescriptorPoolSize modelDescriptorPoolSize = {};
	//modelDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	//modelDescriptorPoolSize.descriptorCount = static_cast<uint32_t>(modelDynamicUniformBuffers.size());

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { viewProjectionDescriptorPoolSize/*, modelDescriptorPoolSize*/ };

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(SwapchainImages.size());
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	VkResult result = vkCreateDescriptorPool(Devices.LogicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a descriptor pool");

	// -- CREATE SAMPLER DESCRIPTOR POOL --
	// Texture sampler pool
	// TODO: research array layers or texture atlases to optimize this
	VkDescriptorPoolSize samplerPoolSize = {};
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
	samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerPoolCreateInfo.poolSizeCount = 1;
	samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	result = vkCreateDescriptorPool(Devices.LogicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create a descriptor pool!");
}

void Renderer::AllocateDescriptorSets()
{
	// resize descriptor set, the uniform buffers are linked
	descriptorSets.resize(SwapchainImages.size());

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(SwapchainImages.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(SwapchainImages.size());
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();
	
	VkResult result = vkAllocateDescriptorSets(Devices.LogicalDevice, &descriptorSetAllocateInfo, descriptorSets.data());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate descriptor sets!");

	// Update all of descriptor set buffer bindings
	for (size_t i = 0; i < SwapchainImages.size(); i++)
	{
		// View projection descriptor
		// Buffer info and data offset info
		VkDescriptorBufferInfo viewProjectionBufferInfo = {};
		viewProjectionBufferInfo.buffer = viewProjectionUniformBuffers[i];						// buffer to get data from
		viewProjectionBufferInfo.offset = 0;													// any offset (e.g. skip any data)
		viewProjectionBufferInfo.range = sizeof(seCamera->uboViewProjection);					// bind everything (size of data)

		VkWriteDescriptorSet viewProjectionSetWrite = {};
		viewProjectionSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		viewProjectionSetWrite.dstSet = descriptorSets[i];							// desriptor set to update
		viewProjectionSetWrite.dstBinding = 0;										// binding in shader to update
		viewProjectionSetWrite.dstArrayElement = 0;									// index to update
		viewProjectionSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewProjectionSetWrite.descriptorCount = 1;
		viewProjectionSetWrite.pBufferInfo = &viewProjectionBufferInfo;

		//// Model descriptor
		//VkDescriptorBufferInfo modelBufferInfo = {};
		//modelBufferInfo.buffer = modelDynamicUniformBuffers[i];
		//modelBufferInfo.offset = 0;
		//modelBufferInfo.range = modelUniformAlignment;

		//VkWriteDescriptorSet modelSetWrite = {};
		//modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//modelSetWrite.dstSet = descriptorSets[i];
		//modelSetWrite.dstBinding = 1;
		//modelSetWrite.dstArrayElement = 0;
		//modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		//modelSetWrite.descriptorCount = 1;
		//modelSetWrite.pBufferInfo = &modelBufferInfo;

		std::vector<VkWriteDescriptorSet> writeDescriptors = { viewProjectionSetWrite/*, modelSetWrite*/ };

		// update the descriptor sets with the new buffer binding info
		vkUpdateDescriptorSets(Devices.LogicalDevice, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
	}
}

void Renderer::RetrievePhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(VulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("Could not find physical devices that support Vulkan!");
	}

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(VulkanInstance, &deviceCount, physicalDevices.data());

	for (const auto& device : physicalDevices)
	{
		if (CheckForBestPhysicalDevice(device))
		{
			Devices.PhysicalDevice = device;
			break;
		}
	}

	// Get properties of our device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(Devices.PhysicalDevice, &deviceProperties);

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
	VkResult result = vkCreateImage(Devices.LogicalDevice, &imageCreateInfo, nullptr, &image);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create an image!");

	// Get memory requirements for a type of image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(Devices.LogicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocationInfo = {};
	memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocationInfo.allocationSize = memoryRequirements.size;
	memoryAllocationInfo.memoryTypeIndex = FindMemoryTypeIndex(Devices.PhysicalDevice, memoryRequirements.memoryTypeBits, inPropertyFlags);
	
	result = vkAllocateMemory(Devices.LogicalDevice, &memoryAllocationInfo, nullptr, outImageMemory);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate memory!");

	// Bind memory to the image
	vkBindImageMemory(Devices.LogicalDevice, image, *outImageMemory, 0);

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

void Renderer::RecordCommands(uint32_t inImageIndex)
{
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Info on how to begin a render pass (for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = RenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = SwapchainExtent;

	std::array<VkClearValue, 2> clearValues = {}; 
	clearValues[0].color = {0.6f, 0.6f, 0.4f, 1.0f};
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();				// Clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());


	// Start recording
	VkResult result = vkBeginCommandBuffer(CommandBuffers[inImageIndex],&bufferBeginInfo);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to start recording a command buffer!");

	// start the render pass
	renderPassBeginInfo.framebuffer = SwapchainFramebuffers[inImageIndex];
	vkCmdBeginRenderPass(CommandBuffers[inImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Bind the pipeline to render pass 
	vkCmdBindPipeline(CommandBuffers[inImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
	
	// Bind more stuff here

	for (size_t i = 0; i < SEGame->gameObjects.size(); i++)
	{		
		MeshModel tempModel = SEGame->gameObjects[i]->objectMeshModel;
		// Push constants to given shader stage directly
		vkCmdPushConstants(CommandBuffers[inImageIndex], PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Model), &SEGame->gameObjects[i]->GetModel());

		for (size_t j = 0; j < tempModel.GetMeshCount(); j++)
		{
			// bind mesh vertex buffer
			VkBuffer vertexBuffers[] = { tempModel.GetMesh(j)->GetVertexBuffer() }; // Buffers to bind
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(CommandBuffers[inImageIndex], 0, 1, vertexBuffers, offsets);
			
			// Bind mesh index buffer
			vkCmdBindIndexBuffer(CommandBuffers[inImageIndex], tempModel.GetMesh(j)->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
			
			// Bind descriptor sets
			if (tempModel.GetMesh(j)->GetTextureID() >= 0)
			{
				// TODO: MEMORY LEAK -> When loading new level, make sure to remove old sampler descriptor set!!
				std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSets[inImageIndex],
					samplerDescriptorSets[tempModel.GetMesh(j)->GetTextureID()] };
			
				vkCmdBindDescriptorSets(CommandBuffers[inImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout,
					0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);
			}
			else
			{
				std::cout << "Error: Game mesh has no texture AND blank texture is not loaded." << std::endl;
			}
			
			// Execute the pipeline
			vkCmdDrawIndexed(CommandBuffers[inImageIndex], tempModel.GetMesh(j)->GetIndexCount(), 1, 0, 0, 0);
		}
	}
	
	// you can draw more stuff here also

	// --- ImGui rendering ---
	// TODO: ADD IMGUI RENDERING TO AN ENGINE GUI CLASS
	
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)SwapchainExtent.width, (float)SwapchainExtent.height);

	// Insert your ImGui code here

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save Level"))
			{
				shouldSaveLevel = true;
			}
			if (ImGui::MenuItem("Load Level"))
			{
				shouldLoadNewLevel = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


	ImGui::Begin("Edit Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Input field for the object ID
	ImGui::Text("Object ID");
	ImGui::InputInt("ID", &currentSelectedModelID);

	if (currentSelectedModelID < SEGame->gameObjects.size())
	{
		glm::mat4 modelMat = SEGame->gameObjects[currentSelectedModelID]->GetModel().modelMatrix;

		float position[3] = { modelMat[3].x, modelMat[3].y, modelMat[3].z };
		// Input fields for the position
		ImGui::Text("Object Position");
		ImGui::InputFloat3("##Position", position);

		if (modelMat[3].x != position[0] || modelMat[3].y != position[1] || modelMat[3].z != position[2])
		{
			modelMat[3].x = position[0];
			modelMat[3].y = position[1];
			modelMat[3].z = position[2];

			SEGame->gameObjects[currentSelectedModelID]->SetModel(modelMat);
		}
	}

	ImGui::End();

	// Render ImGui's draw data into the command buffer
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffers[inImageIndex]);
	// --- End ImGui rendering ---


	vkCmdEndRenderPass(CommandBuffers[inImageIndex]);

	// Stop recording
	result = vkEndCommandBuffer(CommandBuffers[inImageIndex]);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to stop recording a command buffer!");
}

void Renderer::UpdateUniformBuffers(uint32_t inImageIndex)
{
	// copy view projection data
	void* data;
	vkMapMemory(Devices.LogicalDevice, viewProjectionUniformBufferMemory[inImageIndex], 0, sizeof(UniformBufferObjectViewProjection), 0, &data);
	memcpy(data, &seCamera->uboViewProjection, sizeof(UniformBufferObjectViewProjection));
	vkUnmapMemory(Devices.LogicalDevice, viewProjectionUniformBufferMemory[inImageIndex]);

	//// Future - Ensure gamemeshes.size() is less than MAX_OBJECTS
	//// copy model data
	//for (size_t i = 0; i < SEGame.GameMeshes.size(); i++)
	//{
	//	// get the memory address at which this models memory will be updated at within the modelTransferSpace
	//	Model* model = (Model*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment));
	//	// update the data.
	//	*model = SEGame.GameMeshes[i].GetModel();
	//}

	//// copy the model data to the model buffer.
	//vkMapMemory(Devices.LogicalDevice, modelDynamicUniformBufferMemory[inImageIndex], 0, modelUniformAlignment * SEGame.GameMeshes.size(), 0, &data);
	//memcpy(data, modelTransferSpace, modelUniformAlignment * SEGame.GameMeshes.size());
	//vkUnmapMemory(Devices.LogicalDevice, modelDynamicUniformBufferMemory[inImageIndex]);
}

VkFormat Renderer::ChooseSupportedFormat(const std::vector<VkFormat>& inFormats, VkImageTiling inTiling, VkFormatFeatureFlags inFeatureFlags)
{
	for (VkFormat format : inFormats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(Devices.PhysicalDevice, format, &properties);

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
			Indicies.graphicsFamily = Index;
		}

		// Check if the Queue Family supports presentation
		VkBool32 PresentationSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(InPhysicalDevice, Index, VulkanSurface, &PresentationSupport);

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

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InPhysicalDevice, VulkanSurface, &SwapChainInfo.surfaceCapabilities);

	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, VulkanSurface, &FormatCount, nullptr);

	if (FormatCount != 0)
	{
		SwapChainInfo.surfaceFormats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(InPhysicalDevice, VulkanSurface, &FormatCount, SwapChainInfo.surfaceFormats.data());
	}

	uint32_t PresentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, VulkanSurface, &PresentationCount, nullptr);

	if (PresentationCount != 0)
	{
		SwapChainInfo.presentationModes.resize(PresentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(InPhysicalDevice, VulkanSurface, &PresentationCount, SwapChainInfo.presentationModes.data());
	}


	return SwapChainInfo;
}

void Renderer::CreateTextureSampler()
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

	VkResult result = vkCreateSampler(Devices.LogicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Filed to create a Texture Sampler!");
}

int Renderer::CreateTextureImage(std::string inFileName)
{
	// Load image file
	int width, height;
	VkDeviceSize imageSize;
	stbi_uc* imageData = LoadTextureFile(inFileName, &width, &height, &imageSize);

	// Create staging buffer to hold loaded data, ready to copy to device
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;
	CreateBuffer(Devices.PhysicalDevice, Devices.LogicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&imageStagingBuffer, &imageStagingBufferMemory);

	// Copy image data to staging buffer
	void* data;
	vkMapMemory(Devices.LogicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(imageSize));
	vkUnmapMemory(Devices.LogicalDevice, imageStagingBufferMemory);

	// Free original image data
	stbi_image_free(imageData);

	// Create image to hold final texture
	VkImage texImage;
	VkDeviceMemory texImageMemory;
	texImage = CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);


	// COPY DATA TO IMAGE
	// Transition image to be DST for copy operation
	TransitionImageLayout(Devices.LogicalDevice, GraphicsQueue, GraphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Copy image data
	CopyImageBuffer(Devices.LogicalDevice, GraphicsQueue, GraphicsCommandPool, imageStagingBuffer, texImage, width, height);

	// Transition image to be shader readable for shader usage
	TransitionImageLayout(Devices.LogicalDevice, GraphicsQueue, GraphicsCommandPool,
		texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Add texture data to vector for reference
	textureImages.push_back(texImage);
	textureImageMemory.push_back(texImageMemory);

	// Destroy staging buffers
	vkDestroyBuffer(Devices.LogicalDevice, imageStagingBuffer, nullptr);
	vkFreeMemory(Devices.LogicalDevice, imageStagingBufferMemory, nullptr);

	// Return index of new texture image
	return textureImages.size() - 1;
}

int Renderer::CreateTexture(std::string inFileName)
{
	// Create Texture Image and get its location in array
	int textureImageLoc = CreateTextureImage(inFileName);

	// Create Image View and add to list
	VkImageView imageView = CreateImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	textureImageViews.push_back(imageView);

	// Create Texture Descriptor
	int descriptorLoc = CreateTextureDescriptor(imageView);

	// Return location of set with texture
	return descriptorLoc;
}

int Renderer::CreateTextureDescriptor(VkImageView inTextureImage)
{
	VkDescriptorSet descriptorSet;

	// Descriptor Set Allocation Info
	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = samplerDescriptorPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = &samplerSetLayout;

	// Allocate Descriptor Sets
	VkResult result = vkAllocateDescriptorSets(Devices.LogicalDevice, &setAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Texture Descriptor Sets!");
	}

	// Texture Image Info
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// Image layout when in use
	imageInfo.imageView = inTextureImage;									// Image to bind to set
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
	vkUpdateDescriptorSets(Devices.LogicalDevice, 1, &descriptorWrite, 0, nullptr);

	// Add descriptor set to list
	samplerDescriptorSets.push_back(descriptorSet);

	// Return descriptor set location
	return samplerDescriptorSets.size() - 1;
}

stbi_uc* Renderer::LoadTextureFile(std::string inFileName, int* inWidth, int* inHeight, VkDeviceSize* inImageSize)
{
	// Number of channels image uses
	int channels;

	// Load pixel data for image
	stbi_uc* image = stbi_load(inFileName.c_str(), inWidth, inHeight, &channels, STBI_rgb_alpha);

	if (!image)
	{
		throw std::runtime_error("Failed to load a Texture file! (" + inFileName + ")");
	}

	// Calculate image size using given and known data
	*inImageSize = *inWidth * *inHeight * 4; // *4 because RGBA

	return image;
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

bool Renderer::InitImGuiForVulkan()
{
	// Create a descriptor pool for ImGui
	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allows freeing individual descriptor sets
	poolInfo.maxSets = 1000 * static_cast<uint32_t>(std::size(poolSizes)); // Number of descriptor sets
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(Devices.LogicalDevice, &poolInfo, nullptr, &imguiDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ImGui descriptor pool!");

	// Initialize ImGui for Vulkan
	ImGui_ImplVulkan_InitInfo imGuiCreateInfo = {};
	imGuiCreateInfo.Instance = VulkanInstance;
	imGuiCreateInfo.PhysicalDevice = Devices.PhysicalDevice;
	imGuiCreateInfo.Device = Devices.LogicalDevice;

	QueueFamilyIndicies indicies = GetQueueFamilies(Devices.PhysicalDevice);
	imGuiCreateInfo.QueueFamily = indicies.graphicsFamily;

	imGuiCreateInfo.Queue = GraphicsQueue;
	imGuiCreateInfo.PipelineCache = nullptr;
	imGuiCreateInfo.DescriptorPool = imguiDescriptorPool;
	imGuiCreateInfo.Allocator = nullptr;
	imGuiCreateInfo.MinImageCount = static_cast<uint32_t>(SwapchainImages.size());
	imGuiCreateInfo.ImageCount = static_cast<uint32_t>(SwapchainImages.size());
	imGuiCreateInfo.CheckVkResultFn = nullptr;				// TODO: Setup debug callback function for ImGui
	imGuiCreateInfo.RenderPass = RenderPass;

	
	bool result = ImGui_ImplVulkan_Init(&imGuiCreateInfo);

	//if (!result == EXIT_FAILURE) // check ! because if it succeeds ImGui_ImplVulkan_Init returns 1 and that = EXIT_FAILURE.
	//	return EXIT_FAILURE;

	return !result; // check ! because if it succeeds ImGui_ImplVulkan_Init returns 1 and that = EXIT_FAILURE.
}

void Renderer::ImGuiResultCheck(VkResult inError)
{
	if (inError != VK_SUCCESS)
	{
		std::cerr << "imGui error: " << inError << std::endl;
	}
}

void Renderer::ResizeRenderer(int inWidth, int inHeight)
{
	// Wait for commands to finish
	vkDeviceWaitIdle(Devices.LogicalDevice);

	// Destroy Depth Buffer stuff
	vkDestroyImageView(Devices.LogicalDevice, depthBufferImageView, nullptr);
	vkDestroyImage(Devices.LogicalDevice, depthBufferImage, nullptr);
	vkFreeMemory(Devices.LogicalDevice, depthBufferImageMemory, nullptr);
	// Destroy Frame Buffers
	for (auto framebuffer : SwapchainFramebuffers)
		vkDestroyFramebuffer(Devices.LogicalDevice, framebuffer, nullptr);
	// Destroy the Graphics Pipeline
	vkDestroyPipeline(Devices.LogicalDevice, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Devices.LogicalDevice, PipelineLayout, nullptr);
	// Destroy the Swapchain Images
	for (auto image : SwapchainImages)
		vkDestroyImageView(Devices.LogicalDevice, image.imageView, nullptr);
	// Destroy the Swapchain
	vkDestroySwapchainKHR(Devices.LogicalDevice, Swapchain, nullptr);

	// Clear vectors that are now holding nullptrs
	SwapchainImages.clear();
	SwapchainFramebuffers.clear();

	// re-create them all
	CreateSwapChain();
	CreateGraphicsPipeline();
	CreateDepthBufferImage();
	CreateFramebuffers();
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

void Renderer::DestroyAllRendererTextures()
{
	// Wait until queues and all operations are done before cleaning up
	vkDeviceWaitIdle(Devices.LogicalDevice);

	// Destroy texture-related Vulkan objects for the current level
	for (size_t i = 0; i < textureImages.size(); i++)
	{
		vkDestroyImageView(Devices.LogicalDevice, textureImageViews[i], nullptr);
		vkDestroyImage(Devices.LogicalDevice, textureImages[i], nullptr);
		vkFreeMemory(Devices.LogicalDevice, textureImageMemory[i], nullptr);
	}

	textureImageViews.clear();
	textureImages.clear();
	textureImageMemory.clear();
}

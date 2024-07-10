#include "../../Public/Rendering/Renderer.h"

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
	}
	catch(const std::runtime_error& error)
	{
		printf("Error: %s\n", error.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void Renderer::DestroyRenderer()
{
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
	QueueFamilyIndicies Indicies = GetQueueFamilies(InPhysicalDevice);
	bool ExtensionsSupported = CheckDeviceExtentionSupport(InPhysicalDevice);

	return Indicies.IsValid() && ExtensionsSupported;
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


#include "InstancesAndDevices.h"
#include "../Application/WindowCreation.h"

namespace SmolderingEngine
{
#pragma region Function Macros

    bool CreateVulkanInstanceAndFunctions(std::vector<char const*>& _extensions, LIBRARY_TYPE& _vulkanLibrary, VkInstance& _instance)
    {   
        // Connecting to vulkan-1.dll
        if (!ConnectWithVulkanLoaderLibrary(_vulkanLibrary))
            return false;

        // Loading macros
        if (!LoadFunctionExportedFromVulkanLoaderLibrary(_vulkanLibrary))
            return false;

        // Loading macros
        if (!LoadGlobalLevelFunctions())
            return false;

        // Create vulkan instance
        if (!CreateVulkanInstance(_extensions, "Smouldering Engine", _instance))
            return false;

        // loading macros based off of enabled extensions
        if (!LoadInstanceLevelFunctions(_instance, _extensions))
            return false;

        return true;
    }

    bool ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE& _vulkanLibrary)
    {
        _vulkanLibrary = LoadLibrary(VULKAN_DLL_NAME);

        if (_vulkanLibrary == nullptr)
        {
            std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
            return false;
        }
        return true;
    }

    bool LoadFunctionExportedFromVulkanLoaderLibrary(LIBRARY_TYPE const& _vulkanLibrary)
    {
        // Defines a macro called LoadFunction that represents vkGetInstanceProcAddr()
        #define LoadFunction GetProcAddress 
        
        // We then create a macro that loads a vulkan function from the loader library,
        // It casts the result into a PFN_GetInstanceProcAddr type and stores it into 
        // vkGetInstanceProcAddr and checks if it has succeeded or not. 
        #define EXPORTED_VULKAN_FUNCTION(name)                                  \
        name = (PFN_##name)LoadFunction(_vulkanLibrary, #name);                 \
        if(name == nullptr)                                                     \
        {                                                                       \
            std::cout << "Could not load exported Vulkan function named: "      \
            #name << std::endl;                                                 \
            return false;                                                       \
        }                                                                       \

        // This seems strange, this is needed however for the line 
        // #define EXPORTED_VULKAN_FUNCTION(name) to link to the .inl file
        // and load the EXPORTED_VULKAN_FUNCTIONs
        #include "../../Common/VulkanFunctionsList.inl"

        return true;
    }

    bool LoadGlobalLevelFunctions()
    {
        // Function macro that takes function name and gives it to vkGetInstanceProcAddr.
        // It will try to load the function and if it fails nullptr is returned.
        #define GLOBAL_LEVEL_VULKAN_FUNCTION(name)                              \
        name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name);               \
        if( name == nullptr )                                                   \
        {                                                                       \
          std::cout << "Could not load global level Vulkan function named: "    \
          #name << std::endl;                                                   \
          return false;                                                         \
        }                                                                       \

        // This seems strange, this is needed however for the line 
        // #define GLOBAL_LEVEL_VULKAN_FUNCTION(name) to link to the .inl file
        // and load the GLOBAL_LEVEL_VULKAN_FUNCTIONs
        #include "../../Common/VulkanFunctionsList.inl"

        return true;
    }

    bool LoadInstanceLevelFunctions(VkInstance _instance, std::vector<char const*> const& _enabledExtensions)
    {
        // This function macro calls a vkGetInstanceProcAddr() function and passed in
        // a VkInstance, we can only load functions that work properly AFTER the instance
        // object is created.
        #define INSTANCE_LEVEL_VULKAN_FUNCTION(name)                                \
        name = (PFN_##name)vkGetInstanceProcAddr(_instance, #name);                 \
        if(name == nullptr)                                                         \
        {                                                                           \
          std::cout << "Could not load instance-level Vulkan function named: "      \
          #name << std::endl;                                                       \
          return false;                                                             \
        }                                                                           \

        // Load instance-level functions from enabled extensions ONLY, we must
        // know what extensions are enabled and what functions come from them.
        #define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)      \
        for(auto& enabledExtension : _enabledExtensions)                            \
        {                                                                           \
          if( std::string(enabledExtension) == std::string(extension))              \
          {                                                                         \
            name = (PFN_##name)vkGetInstanceProcAddr(_instance, #name);             \
            if(name == nullptr)                                                     \
            {                                                                       \
              std::cout << "Could not load instance-level Vulkan function named: "  \
              #name << std::endl;                                                   \
              return false;                                                         \
            }                                                                       \
          }                                                                         \
        }                                                                           \

        #include "../../Common/VulkanFunctionsList.inl"

        return true;
    }

    bool LoadDeviceLevelFunctions(VkDevice _logicalDevice, std::vector<char const*> const& _enabledExtensions)
    {
        // Load core Vulkan API device-level functions.
        // For each DEVICE_LEVEL_VULKAN_FUNCTION it tries to load a procedure.
        #define DEVICE_LEVEL_VULKAN_FUNCTION(name)                                  \
        name = (PFN_##name)vkGetDeviceProcAddr(_logicalDevice, #name);              \
        if(name == nullptr)                                                         \
        {                                                                           \
          std::cout << "Could not load device-level Vulkan function named: "        \
            #name << std::endl;                                                     \
          return false;                                                             \
        }                                                                           \

        // Load device-level functions from enabled extensions.
        // For each enabled extension it compares the name of the enabled extension
        // Inside of the vector to the name of the extension specified for the function.
        #define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)        \
        for(auto& enabledExtension : _enabledExtensions)                            \
        {                                                                           \
          if(std::string(enabledExtension) == std::string(extension))               \
          {                                                                         \
            name = (PFN_##name)vkGetDeviceProcAddr(_logicalDevice, #name);          \
            if(name == nullptr)                                                     \
            {                                                                       \
              std::cout << "Could not load device-level Vulkan function named: "    \
                #name << std::endl;                                                 \
              return false;                                                         \
            }                                                                       \
          }                                                                         \
        }                                                                           \

        #include "../../Common/VulkanFunctionsList.inl"

        return true;
    }

#pragma endregion

#pragma region Vulkan Instance

    bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties>& _availableExtensions)
    {
        // Make a variable and read in the amount of extensions this platform supports
        uint32_t extensionsCount = 0;
        if ((vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr) != VK_SUCCESS) || (extensionsCount == 0))
        {
            std::cout << "Could not get the number of instance extensions." << std::endl;
            return false;
        }

        // Now that we know how many extensions we need we resize the vector and get the extension properties.
        _availableExtensions.resize(extensionsCount);
        if ((vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, _availableExtensions.data()) != VK_SUCCESS) || (extensionsCount == 0))
        {
            std::cout << "Could not enumerate instance extensions." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateVulkanInstance(std::vector<char const*>& _desiredExtensions, char const* const _applicationName, VkInstance& _instance)
    {
        // Presentation surface extension for windows platforms.
        _desiredExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
        _desiredExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

        // Create a vector of instance-level extensions we want to enable.
        std::vector<VkExtensionProperties> availableExtensions;
        if (!CheckAvailableInstanceExtensions(availableExtensions))
            return false;

        // Check to see if the extensions we want to enable are supported on the hardware.
        for (auto& extension : _desiredExtensions)
        {
            if (!IsExtensionSupported(availableExtensions, extension))
            {
                std::cout << "Extension named '" << extension << "' is not supported by an Instance object." << std::endl;
                return false;
            }
        }

        // Make a variable that has our applications information
        VkApplicationInfo applicationInfo =
        {
          VK_STRUCTURE_TYPE_APPLICATION_INFO,                   // VkStructureType           sType
          nullptr,                                              // const void              * pNext
          _applicationName,                                     // const char              * pApplicationName
          VK_MAKE_API_VERSION(0, 0, 1, 0),                      // uint32_t                  applicationVersion
          "Smoldering Engine",                                  // const char              * pEngineName
          VK_MAKE_API_VERSION(0, 0, 1, 0),                      // uint32_t                  engineVersion
          VK_MAKE_API_VERSION(0, 1, 0, 0)                       // uint32_t                  apiVersion
        };

        // Variable that stores the parameters used to create an instance.
        VkInstanceCreateInfo instanceCreateInfo =
        {
          VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,               // VkStructureType           sType
          nullptr,                                              // const void              * pNext
          0,                                                    // VkInstanceCreateFlags     flags
          &applicationInfo,                                     // const VkApplicationInfo * pApplicationInfo
          0,                                                    // uint32_t                  enabledLayerCount
          nullptr,                                              // const char * const      * ppEnabledLayerNames
          static_cast<uint32_t>(_desiredExtensions.size()),     // uint32_t                  enabledExtensionCount
          _desiredExtensions.data()                             // const char * const      * ppEnabledExtensionNames
        };

        // Now we will try to create an instance object with this information.
        if ((vkCreateInstance(&instanceCreateInfo, nullptr, &_instance) != VK_SUCCESS) || (_instance == VK_NULL_HANDLE))
        {
            std::cout << "Could not create Vulkan instance." << std::endl;
            return false;
        }

        return true;
    }

#pragma endregion

#pragma region Physical / Logical Device

    bool EnumerateAvailablePhysicalDevices(VkInstance _instance, std::vector<VkPhysicalDevice>& _availableDevices)
    {
        // Here we check how many physical devices are available on the computer using vkEnumeratePhysicalDevices.
        uint32_t devicesCount = 0;
        if ((vkEnumeratePhysicalDevices(_instance, &devicesCount, nullptr) != VK_SUCCESS) || (devicesCount == 0))
        {
            std::cout << "Could not get the number of available physical devices." << std::endl;
            return false;
        }

        // Now that we know how many devices we can have first resize the vector of devices
        // and try to get the handles of the physical devices by passing in the resized vector.
        _availableDevices.resize(devicesCount);
        if ((vkEnumeratePhysicalDevices(_instance, &devicesCount, _availableDevices.data()) != VK_SUCCESS) || (devicesCount == 0))
        {
            std::cout << "Could not enumerate physical devices." << std::endl;
            return false;
        }

        return true;
    }

    bool CheckAvailableDeviceExtensions(VkPhysicalDevice _physicalDevice, std::vector<VkExtensionProperties>& _availableExtensions)
    {
        // Check how many extensions are supported on a physical device.
        uint32_t extensionsCount = 0;
        if ((vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionsCount, nullptr) != VK_SUCCESS) || (extensionsCount == 0))
        {
            std::cout << "Could not get the number of device extensions." << std::endl;
            return false;
        }

        // Now that we know the number of extensions supported on this device resize the vector to hold the properties
        // and pass it back into vkEnumerateDeviceExtensionProperties to get them filled in. 
        _availableExtensions.resize(extensionsCount);
        if ((vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionsCount, _availableExtensions.data()) != VK_SUCCESS) || (extensionsCount == 0))
        {
            std::cout << "Could not enumerate device extensions." << std::endl;
            return false;
        }

        return true;
    }

    void GetFeaturesAndPropertiesOfPhysicalDevice(VkPhysicalDevice _physicalDevice, VkPhysicalDeviceFeatures& _deviceFeatures, VkPhysicalDeviceProperties& _deviceProperties)
    {
        // This function gives us information such as device names, driver versions, and supported Vulkan versions.
        // It can also tell us limitations of the device.
        vkGetPhysicalDeviceFeatures(_physicalDevice, &_deviceFeatures);

        // This function will give us features that may be supported by the hardware but are not
        // Required for Vulkan to run, these features need to be specifically enabled for use. 
        vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
    }

    bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice _physicalDevice, std::vector<VkQueueFamilyProperties>& _queueFamilies)
    {
        // First we will get the queueFamiliesCount on the _physicalDevice.
        uint32_t queueFamiliesCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamiliesCount, nullptr);
        if (queueFamiliesCount == 0) {
            std::cout << "Could not get the number of queue families." << std::endl;
            return false;
        }

        // Now that we know how many queue families there are resize the vector to fit them all,
        // then store the properties of the queue families inside of the vector.
        _queueFamilies.resize(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamiliesCount, _queueFamilies.data());
        if (queueFamiliesCount == 0) {
            std::cout << "Could not acquire properties of queue families." << std::endl;
            return false;
        }

        return true;
    }

    bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice _physicalDevice, VkQueueFlags _desiredCapabilities, uint32_t& _queueFamilyIndex)
    {
        // Get the properties of the queue families on a physical device and check if they are available.
        std::vector<VkQueueFamilyProperties> queueFamilies;
        if (!CheckAvailableQueueFamiliesAndTheirProperties(_physicalDevice, queueFamilies))
            return false;

        // Check every element of the queueFamilies vector for supported operations. 
        for (uint32_t index = 0; index < static_cast<uint32_t>(queueFamilies.size()); index++)
        {
            if ((queueFamilies[index].queueCount > 0) && ((queueFamilies[index].queueFlags & _desiredCapabilities) == _desiredCapabilities))
            {
                _queueFamilyIndex = index;
                return true;
            }
        }

        return false;
    }

    bool CreateLogicalDevice(VkPhysicalDevice _physicalDevice, std::vector<QueueInfo> _queueInfo, std::vector<char const*>& _desiredExtensions, VkPhysicalDeviceFeatures* _desiredFeatures, VkDevice& _logicalDevice)
    {
        _desiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        // First get all the extensions supported by this physical device
        std::vector<VkExtensionProperties> availableExtensions;
        if (!CheckAvailableDeviceExtensions(_physicalDevice, availableExtensions))
            return false;

        // Then we check to make sure the extensions are supported so we can make a logical device. 
        for (auto& extension : _desiredExtensions)
        {
            if (!IsExtensionSupported(availableExtensions, extension))
            {
                std::cout << "Extension named '" << extension << "' is not supported by a physical device." << std::endl;
                return false;
            }
        }

        // Next we make a vector queueCreateInfo that contains information about queues and family queues
        // That we want to request for the logical device. 
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
        for (auto& info : _queueInfo)
        {
            queueCreateInfo.push_back
            ({
              VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,       // VkStructureType                  sType
              nullptr,                                          // const void                     * pNext
              0,                                                // VkDeviceQueueCreateFlags         flags
              info.FamilyIndex,                                 // uint32_t                         queueFamilyIndex
              static_cast<uint32_t>(info.Priorities.size()),    // uint32_t                         queueCount
              info.Priorities.data()                            // const float                    * pQueuePriorities
                });
        };

        // Now that we have the queueCreateInfo we provide this information to deviceCreateInfo,
        // DeviceCreateInfo will store information about the number of different queue families in which we
        // Will be requesting queues for a logical device, names of enabled layers, extensions we want to enable,
        // And features we wish to use. 
        VkDeviceCreateInfo deviceCreateInfo =
        {
          VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,                 // VkStructureType                  sType
          nullptr,                                              // const void                     * pNext
          0,                                                    // VkDeviceCreateFlags              flags
          static_cast<uint32_t>(queueCreateInfo.size()),        // uint32_t                         queueCreateInfoCount
          queueCreateInfo.data(),                               // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
          0,                                                    // uint32_t                         enabledLayerCount
          nullptr,                                              // const char * const             * ppEnabledLayerNames
          static_cast<uint32_t>(_desiredExtensions.size()),     // uint32_t                         enabledExtensionCount
          _desiredExtensions.data(),                            // const char * const             * ppEnabledExtensionNames
          _desiredFeatures                                      // const VkPhysicalDeviceFeatures * pEnabledFeatures
        };

        // Lastly we provide deviceCreateInfo to vkCreateDevice which creates a logical device. 
        if ((vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice) != VK_SUCCESS) || (_logicalDevice == VK_NULL_HANDLE))
        {
            std::cout << "Could not create logical device." << std::endl;
            return false;
        }

        return true;
    }

    void GetDeviceQueue(VkDevice _logicalDevice, uint32_t _queueFamilyIndex, uint32_t _queueIndex, VkQueue& _queue)
    {
        vkGetDeviceQueue(_logicalDevice, _queueFamilyIndex, _queueIndex, &_queue);
    }

    bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(VkInstance _instance, VkDevice& _logicalDevice, VkQueue& _graphicsQueue, VkQueue& _computeQueue)
    {
        // Get handles to physical devices on the computer.
        std::vector<VkPhysicalDevice> physicalDevices;
        EnumerateAvailablePhysicalDevices(_instance, physicalDevices);

        // loop through all available physical devices 
        for (auto& physicalDevice : physicalDevices)
        {
            // Get the features and properties to get information.
            VkPhysicalDeviceFeatures deviceFeatures;
            VkPhysicalDeviceProperties deviceProperties;
            GetFeaturesAndPropertiesOfPhysicalDevice(physicalDevice, deviceFeatures, deviceProperties);

            // Does this device support geometry shaders?
            if (!deviceFeatures.geometryShader)
            {
                continue;
            }
            else
            {
                // If they are reset all other members of features list. 
                deviceFeatures = {};
                deviceFeatures.geometryShader = VK_TRUE;
            }

            // check if the physical device exposes queue families that support graphic operations.
            uint32_t graphicsQueueFamilyIndex;
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(physicalDevice, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex))
            {
                continue;
            }

            // check if the physical device exposes queue families that support compute operations.
            uint32_t computeQueueFamilyIndex;
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(physicalDevice, VK_QUEUE_COMPUTE_BIT, computeQueueFamilyIndex))
            {
                continue;
            }

            // make a list of queue families that we want to request queues from and assign priorities to them.
            // If the graphics and queue families have the same index only request one queue from one queue family,
            // If they are different then request two queues, one from the graphics and one from the compute families. 
            std::vector<QueueInfo> requestedQueues = { {graphicsQueueFamilyIndex, {1.0f}} };
            if (graphicsQueueFamilyIndex != computeQueueFamilyIndex)
            {
                requestedQueues.push_back({ computeQueueFamilyIndex, {1.0f} });
            }

            // Try to make a logical device.
            std::vector<char const*> deviceExtensions;
            if (!CreateLogicalDevice(physicalDevice, requestedQueues, deviceExtensions, &deviceFeatures, _logicalDevice))
            {
                continue;
            }
            else
            {
                // Since we have a logical device load device functions.
                if (!LoadDeviceLevelFunctions(_logicalDevice, {}))
                {
                    return false;
                }
                // Get the queues.
                GetDeviceQueue(_logicalDevice, graphicsQueueFamilyIndex, 0, _graphicsQueue);
                GetDeviceQueue(_logicalDevice, computeQueueFamilyIndex, 0, _computeQueue);
                return true;
            }
        }
        return false;
    }

    bool ChoosePhysicalAndLogicalDevices(VkInstance _instance, std::vector<VkPhysicalDevice> _physicalDevices, VkPhysicalDevice& _physicalDevice, VkDevice& _logicalDevice,
        uint32_t& _graphicsQueueFamilyIndex, uint32_t& _presentQueueFamilyIndex, VkSurfaceKHR _presentationSurface, VkQueue& _graphicsQueue, VkQueue& _presentQueue)
    {
        // Create logical device
        if (!EnumerateAvailablePhysicalDevices(_instance, _physicalDevices))
            return false;

        // Basically just loop through physical devices to see what one is best.
        for (auto& physicalDevice : _physicalDevices)
        {
            if (!SelectIndexOfQueueFamilyWithDesiredCapabilities(physicalDevice, VK_QUEUE_GRAPHICS_BIT, _graphicsQueueFamilyIndex))
                continue;

            if (!SelectQueueFamilyThatSupportsPresentationToGivenSurface(physicalDevice, _presentationSurface, _presentQueueFamilyIndex))
                continue;

            std::vector<QueueInfo> requestedQueues = { { _graphicsQueueFamilyIndex, { 1.0f } } };
            if (_graphicsQueueFamilyIndex != _presentQueueFamilyIndex)
            {
                requestedQueues.push_back({ _presentQueueFamilyIndex, { 1.0f } });
            }

            VkDevice logicalDevice;
            std::vector<char const*> deviceExtensions;
            if (!CreateLogicalDevice(physicalDevice, requestedQueues, deviceExtensions, nullptr, logicalDevice))
            {
                continue;
            }
            else
            {
                // Once we can create a logical device load functions, set devices, get queues, and return.
                if (!LoadDeviceLevelFunctions(logicalDevice, deviceExtensions))
                {
                    continue;
                }
                _physicalDevice = physicalDevice;
                _logicalDevice = std::move(logicalDevice);
                GetDeviceQueue(_logicalDevice, _graphicsQueueFamilyIndex, 0, _graphicsQueue);
                GetDeviceQueue(_logicalDevice, _presentQueueFamilyIndex, 0, _presentQueue);
                break;
            }
        }

        if (!_logicalDevice)
            return false;

        return true;
    }

#pragma endregion

#pragma region Clean-Up

    void DestroyLogicalDevice(VkDevice& _logicalDevice)
    {
        // Check to make sure we have a valid logical device
        // Then destroy it and set it to null.
        if (_logicalDevice)
        {
            vkDestroyDevice(_logicalDevice, nullptr);
            _logicalDevice = VK_NULL_HANDLE;
        }
    }

    void DestroyVulkanInstance(VkInstance& _instance)
    {
        // Check to make sure we have a valid instance
        // Then destroy it and set it to null.
        if (_instance)
        {
            vkDestroyInstance(_instance, nullptr);
            _instance = VK_NULL_HANDLE;
        }
    }

    void ReleaseVulkanLoaderLibrary(LIBRARY_TYPE& _vulkanLibrary)
    {
        // Check to make sure we have a valid library
        // Then set the library to null.
        if (_vulkanLibrary)
        {
            // FreeLibrary is for windows platforms
            FreeLibrary(_vulkanLibrary);
            _vulkanLibrary = nullptr;
        }
    }

#pragma endregion

};

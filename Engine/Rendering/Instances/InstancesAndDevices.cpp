
#include "InstancesAndDevices.h"

namespace SmoulderingEngine
{
    bool ConnectWithVulkanLoaderLibrary(LIBRARY_TYPE& vulkan_library)
    {
        vulkan_library = LoadLibrary(VULKAN_DLL_NAME);

        if (vulkan_library == nullptr) 
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

    bool LoadInstanceLevelFunctions(VkInstance instance, std::vector<char const*> const& enabled_extensions)
    {
        // This function macro calls a vkGetInstanceProcAddr() function and passed in
        // a VkInstance, we can only load functions that work properly AFTER the instance
        // object is created.
        #define INSTANCE_LEVEL_VULKAN_FUNCTION(name)                                \
        name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);                  \
        if(name == nullptr)                                                         \
        {                                                                           \
          std::cout << "Could not load instance-level Vulkan function named: "      \
          #name << std::endl;                                                       \
          return false;                                                             \
        }                                                                           \
        
        // Load instance-level functions from enabled extensions ONLY, we must
        // know what extensions are enabled and what functions come from them.
        #define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(name, extension)      \
        for(auto& enabled_extension : enabled_extensions)                           \
        {                                                                           \
          if( std::string(enabled_extension) == std::string(extension))             \
          {                                                                         \
            name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);              \
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

    bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties>& _availableExtensions)
    {
        // Make a variable and read in the amount of extensions this platform supports
        uint32_t extensions_count = 0;
        if ((vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr) != VK_SUCCESS) || (extensions_count == 0)) 
        {
            std::cout << "Could not get the number of instance extensions." << std::endl;
            return false;
        }

        // Now that we know how many extensions we need we resize the vector and get the extension properties.
        _availableExtensions.resize(extensions_count);
        if ((vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, _availableExtensions.data()) != VK_SUCCESS) ||(extensions_count == 0))
        {
            std::cout << "Could not enumerate instance extensions." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateVulkanInstance(std::vector<char const*> const& desired_extensions, char const* const application_name, VkInstance& instance)
    {
        std::vector<VkExtensionProperties> available_extensions;
        if (!CheckAvailableInstanceExtensions(available_extensions)) 
            return false;

        for (auto& extension : desired_extensions) 
        {
            if (!IsExtensionSupported(available_extensions, extension)) 
            {
                std::cout << "Extension named '" << extension << "' is not supported by an Instance object." << std::endl;
                return false;
            }
        }

        VkApplicationInfo application_info = 
        {
          VK_STRUCTURE_TYPE_APPLICATION_INFO,                   // VkStructureType           sType
          nullptr,                                              // const void              * pNext
          application_name,                                     // const char              * pApplicationName
          VK_MAKE_API_VERSION(0, 0, 1, 0),                      // uint32_t                  applicationVersion
          "Smouldering Engine",                                 // const char              * pEngineName
          VK_MAKE_API_VERSION(0, 0, 1, 0),                      // uint32_t                  engineVersion
          VK_MAKE_API_VERSION(0, 1, 0, 0)                       // uint32_t                  apiVersion
        };

        VkInstanceCreateInfo instance_create_info = 
        {
          VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,               // VkStructureType           sType
          nullptr,                                              // const void              * pNext
          0,                                                    // VkInstanceCreateFlags     flags
          &application_info,                                    // const VkApplicationInfo * pApplicationInfo
          0,                                                    // uint32_t                  enabledLayerCount
          nullptr,                                              // const char * const      * ppEnabledLayerNames
          static_cast<uint32_t>(desired_extensions.size()),     // uint32_t                  enabledExtensionCount
          desired_extensions.data()                             // const char * const      * ppEnabledExtensionNames
        };

        if ((vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS) || (instance == VK_NULL_HANDLE))
        {
            std::cout << "Could not create Vulkan instance." << std::endl;
            return false;
        }

        return true;
    }

};

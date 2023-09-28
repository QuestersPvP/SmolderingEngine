
#include "InstancesAndDevices.h"

namespace SmoulderingEngine
{
    bool LoadFunctionExportedFromVulkanLoaderLibrary(LIBRARY_TYPE const& vulkan_library)
    {
        #define LoadFunction GetProcAddress 

        #define EXPORTED_VULKAN_FUNCTION( name )                          \
        name = (PFN_##name)LoadFunction( vulkan_library, #name );         \
        if( name == nullptr ) {                                           \
          std::cout << "Could not load exported Vulkan function named: "  \
            #name << std::endl;                                           \
          return false;                                                   \
        }

        return true;
    }

};

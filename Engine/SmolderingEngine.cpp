
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Common/Common.h"

using namespace SmolderingEngine;

int main()
{
    // TODO: throw this stuff somewhere other than main 
    LIBRARY_TYPE VulkanLibrary;
    VkInstance instance;
    VkDevice logicDevice;
    VkQueue graphicsQueue;
    VkQueue computeQueue;

    // For testing
    bool setUp = true;

    if (!ConnectWithVulkanLoaderLibrary(VulkanLibrary))
        setUp = false;

    if (!LoadFunctionExportedFromVulkanLoaderLibrary(VulkanLibrary))
        setUp = false;

    if (!LoadGlobalLevelFunctions()) 
        setUp = false;

    if (!CreateVulkanInstance({}, "Smouldering Engine", instance))
        setUp = false;

    if (!LoadInstanceLevelFunctions(instance, {}))
        setUp = false;

    while (setUp)
    {

    }

    return 0;
}

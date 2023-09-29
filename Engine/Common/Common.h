#pragma once

// TODO: Make file to hold #defines
// Vulkan library type for Windows
#define LIBRARY_TYPE HMODULE
// Wide character string to help load vulkan-1.dll
#define VULKAN_DLL_NAME L"vulkan-1.dll"

// OS include
#include <Windows.h>

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <thread>
#include <cmath>
#include <functional>
#include <memory>

#include "../Common/VulkanFunctions.h"

// TODO: Make file to store all headers in one location
#include "../Rendering/Instances/InstancesAndDevices.h"

namespace SmoulderingEngine
{
    // OS-specific parameters
    struct WindowParameters
    {
        HINSTANCE          HInstance;
        HWND               HWnd;
    };

    // Extension availability check
    bool IsExtensionSupported(std::vector<VkExtensionProperties> const& available_extensions, char const* const extension);
};

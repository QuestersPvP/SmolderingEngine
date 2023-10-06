
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Common/Common.h"
#include "Rendering/Instances/InstancesAndDevices.h"

using namespace SmolderingEngine;

int main()
{
    // TODO: Clean up main
    // TODO: throw this stuff somewhere other than main 
    LIBRARY_TYPE VulkanLibrary;
    VkInstance instance;
    VkDevice logicDevice;
    VkQueue graphicsQueue;
    VkQueue computeQueue;

#pragma region Window Creation

    // To create the window that you can see,
    // Ignore if you are using a library like Glfw.
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.lpszClassName = WINDOW_NAME;

    if (!RegisterClassEx(&windowClass)) 
    {
        return 0;
    }

    HWND windowHandle = CreateWindow
    (
        WINDOW_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!windowHandle)
    {
        return 0;
    }

#pragma endregion

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

    if (!CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(instance, logicDevice, graphicsQueue, computeQueue))
        setUp = false;

    if (setUp)
    {
        // Show the window (assuming windows OS)
        ShowWindow(windowHandle, SW_SHOW);
        UpdateWindow(windowHandle);

        MSG message;
        while (setUp)
        {
            if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
            {
                // TODO: add event handeling (e.g. x button clicked, resized, etc.)

                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                // Do whatever you want (e.g. render stuff)
                std::cout << "rendering..." << std::endl;
            }
        }
    }

    return 0;
}

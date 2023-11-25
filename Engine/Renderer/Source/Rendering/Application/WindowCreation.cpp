
#include "WindowCreation.h"

#include "../Instances/InstancesAndDevices.h"

namespace SmolderingEngine
{
    LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_LBUTTONDOWN:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 1);
            break;
        case WM_LBUTTONUP:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 0);
            break;
        case WM_RBUTTONDOWN:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 1);
            break;
        case WM_RBUTTONUP:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 0);
            break;
        case WM_MOUSEMOVE:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_MOVE, LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOUSEWHEEL:
            PostMessage(hWnd, USER_MESSAGE_MOUSE_WHEEL, HIWORD(wParam), 0);
            break;
        case WM_SIZE:
        case WM_EXITSIZEMOVE:
            PostMessage(hWnd, USER_MESSAGE_RESIZE, wParam, lParam);
            break;
        case WM_KEYDOWN:
            if (VK_ESCAPE == wParam) {
                PostMessage(hWnd, USER_MESSAGE_QUIT, wParam, lParam);
            }
            break;
        case WM_CLOSE:
            PostMessage(hWnd, USER_MESSAGE_QUIT, wParam, lParam);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    WindowParameters GenerateApplicationWindowParameters()
    {
        WindowParameters windowParams;

        windowParams.HInstance = GetModuleHandle(nullptr);

        WNDCLASSEX windowClass = 
        {
            sizeof(WNDCLASSEX),                 // UINT         cbSize
            CS_HREDRAW | CS_VREDRAW,            // UINT         style
            WindowProcedure,                    // WNDPROC      lpfnWndProc
            0,                                  // int          cbClsExtra
            0,                                  // int          cbWndExtra
            windowParams.HInstance,            // HINSTANCE    hInstance
            nullptr,                            // HICON        hIcon
            LoadCursor(nullptr, IDC_ARROW),     // HCURSOR      hCursor
            (HBRUSH)(COLOR_WINDOW + 1),         // HBRUSH       hbrBackground
            nullptr,                            // LPCSTR       lpszMenuName
            WINDOW_NAME,                        // LPCSTR       lpszClassName
            nullptr                             // HICON        hIconSm
        };

        if (!RegisterClassEx(&windowClass))
        {
            std::cout << "Error Registering Class" << std::endl;
        }

        //HWND windowHandle = CreateWindow
        //(
        //    WINDOW_NAME,
        //    WINDOW_TITLE,
        //    WS_OVERLAPPEDWINDOW,
        //    CW_USEDEFAULT,
        //    CW_USEDEFAULT,
        //    1280,
        //    720,
        //    NULL,
        //    NULL,
        //    GetModuleHandle(NULL),
        //    NULL
        //);

        windowParams.HWnd = CreateWindow(WINDOW_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, windowParams.HInstance, nullptr);
        if (!windowParams.HWnd)
        {
            std::cout << "Error Creating Window" << std::endl;
        }

        return windowParams;
    }

    bool CreatePresentationSurface(VkInstance _instance, WindowParameters _windowParameters, VkSurfaceKHR& _presentationSurface)
	{
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = 
        {
          VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,      // VkStructureType                 sType
          nullptr,                                              // const void                    * pNext
          0,                                                    // VkWin32SurfaceCreateFlagsKHR    flags
          _windowParameters.HInstance,                          // HINSTANCE                       hinstance
          _windowParameters.HWnd                                // HWND                            hwnd
        };

        if ((vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &_presentationSurface) != VK_SUCCESS) || (VK_NULL_HANDLE == _presentationSurface))
        {
            std::cout << "Could not create presentation surface." << std::endl;
            return false;
        }

        return true;
	}

    void DestroyPresentationSurface(VkInstance _instance, VkSurfaceKHR& _presentationSurface)
    {
        if (_presentationSurface)
        {
            vkDestroySurfaceKHR(_instance, _presentationSurface, nullptr);
            _presentationSurface = VK_NULL_HANDLE;
        }
    }

    bool SelectQueueFamilyThatSupportsPresentationToGivenSurface(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, uint32_t& _queueFamilyIndex)
    {
        // Check what queue families are exposed by a physical device.
        std::vector<VkQueueFamilyProperties> queueFamilies;
        if (!CheckAvailableQueueFamiliesAndTheirProperties(_physicalDevice, queueFamilies))
            return false;

        // iterate over all queue families and check if they support image presentation.
        for (uint32_t index = 0; index < static_cast<uint32_t>(queueFamilies.size()); ++index) 
        {
            VkBool32 presentationSupported = VK_FALSE;
            if ((vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, index, _presentationSurface, &presentationSupported) == VK_SUCCESS) && (presentationSupported == VK_TRUE))
            {
                _queueFamilyIndex = index;
                return true;
            }
        }

        return false;
    }
};

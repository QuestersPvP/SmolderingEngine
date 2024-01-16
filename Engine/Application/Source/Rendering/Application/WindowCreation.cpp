
#include "WindowCreation.h"

namespace SE_Renderer
{
    LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
    {
        switch (message) 
        {
        case WM_SIZE:
        case WM_EXITSIZEMOVE:
            PostMessage(hWnd, USERMESSAGE_RESIZE, wParam, lParam);
            break;
        case WM_KEYDOWN:
            switch (wParam)
            {
            case 'W':
                PostMessage(hWnd, KEYDOWN_W, 0, 0);
                break;
            case 'A':
                PostMessage(hWnd, KEYDOWN_A, 0, 0);
                break;
            case 'S':
                PostMessage(hWnd, KEYDOWN_S, 0, 0);
                break;
            case 'D':
                PostMessage(hWnd, KEYDOWN_D, 0, 0);
                break;
            case VK_ESCAPE:
                PostMessage(hWnd, KEYDOWN_ESC, 0, 0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
                break;
            }
            break;
        case WM_KEYUP:
            switch (wParam)
            {
            case 'W':
                PostMessage(hWnd, KEYUP_W, 0, 0);
                break;
            case 'A':
                PostMessage(hWnd, KEYUP_A, 0, 0);
                break;
            case 'S':
                PostMessage(hWnd, KEYUP_S, 0, 0);
                break;
            case 'D':
                PostMessage(hWnd, KEYUP_D, 0, 0);
                break;
            case VK_ESCAPE:
                PostMessage(hWnd, KEYUP_ESC, 0, 0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
                break;
            }
            break;
            break;
        case WM_CLOSE:
            PostMessage(hWnd, USERMESSAGE_QUIT, wParam, lParam);
            break;
        case WM_MOUSEMOVE:
            PostMessage(hWnd, USERMESSAGE_MOUSEMOVE, wParam, lParam);
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
            windowParams.HInstance,             // HINSTANCE    hInstance
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
};

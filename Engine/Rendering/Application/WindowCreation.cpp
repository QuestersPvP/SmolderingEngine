#include "WindowCreation.h"

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
        WindowParameters _windowParams;

        _windowParams.HInstance = GetModuleHandle(nullptr);

        WNDCLASSEX window_class = 
        {
            sizeof(WNDCLASSEX),                 // UINT         cbSize
            CS_HREDRAW | CS_VREDRAW,            // UINT         style
            WindowProcedure,                    // WNDPROC      lpfnWndProc
            0,                                  // int          cbClsExtra
            0,                                  // int          cbWndExtra
            _windowParams.HInstance,            // HINSTANCE    hInstance
            nullptr,                            // HICON        hIcon
            LoadCursor(nullptr, IDC_ARROW),     // HCURSOR      hCursor
            (HBRUSH)(COLOR_WINDOW + 1),         // HBRUSH       hbrBackground
            nullptr,                            // LPCSTR       lpszMenuName
            WINDOW_NAME,                        // LPCSTR       lpszClassName
            nullptr                             // HICON        hIconSm
        };

        if (!RegisterClassEx(&window_class)) 
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

        _windowParams.HWnd = CreateWindow(WINDOW_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, _windowParams.HInstance, nullptr);
        if (!_windowParams.HWnd)
        {
            std::cout << "Error Creating Window" << std::endl;
        }

        return _windowParams;
    }

    bool CreatePresentationSurface(VkInstance _instance, WindowParameters _windowParameters, VkSurfaceKHR& _presentationSurface)
	{
        VkWin32SurfaceCreateInfoKHR surface_create_info = 
        {
          VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,      // VkStructureType                 sType
          nullptr,                                              // const void                    * pNext
          0,                                                    // VkWin32SurfaceCreateFlagsKHR    flags
          _windowParameters.HInstance,                          // HINSTANCE                       hinstance
          _windowParameters.HWnd                                // HWND                            hwnd
        };

        if ((vkCreateWin32SurfaceKHR(_instance, &surface_create_info, nullptr, &_presentationSurface) != VK_SUCCESS) || (VK_NULL_HANDLE == _presentationSurface))
        {
            std::cout << "Could not create presentation surface." << std::endl;
            return false;
        }

        return true;
	}
};

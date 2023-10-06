#pragma once

#include "../../Common/Common.h"

namespace SmolderingEngine
{
    enum UserMessage 
    {
        USER_MESSAGE_RESIZE = WM_USER + 1,
        USER_MESSAGE_QUIT,
        USER_MESSAGE_MOUSE_CLICK,
        USER_MESSAGE_MOUSE_MOVE,
        USER_MESSAGE_MOUSE_WHEEL
    };

    WindowParameters GenerateApplicationWindowParameters();
    bool CreatePresentationSurface(VkInstance _instance, WindowParameters _windowParameters, VkSurfaceKHR& _presentationSurface);
};


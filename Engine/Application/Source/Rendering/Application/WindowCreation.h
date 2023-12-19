#pragma once

// TODO: MOVE THIS WHOLE CLASS INTO WINDOW

#include "../../Common/Common.h"

namespace SmolderingEngine
{
    enum UserMessage 
    {
        // Application events
        USERMESSAGE_RESIZE = WM_USER + 1,
        USERMESSAGE_KEYDOWN,
        USERMESSAGE_KEYUP,
        USERMESSAGE_QUIT,

        // Keydown events
        KEYDOWN_W,
        KEYDOWN_A,
        KEYDOWN_S,
        KEYDOWN_D,
        KEYDOWN_ESC,

        // Keyup events
        KEYUP_W,
        KEYUP_A,
        KEYUP_S,
        KEYUP_D,
        KEYUP_ESC
    };



    WindowParameters GenerateApplicationWindowParameters();
    bool CreatePresentationSurface(VkInstance _instance, WindowParameters _windowParameters, VkSurfaceKHR& _presentationSurface);
    void DestroyPresentationSurface(VkInstance _instance, VkSurfaceKHR& _presentationSurface);

    bool SelectQueueFamilyThatSupportsPresentationToGivenSurface(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _presentationSurface, uint32_t& _queueFamilyIndex);
};


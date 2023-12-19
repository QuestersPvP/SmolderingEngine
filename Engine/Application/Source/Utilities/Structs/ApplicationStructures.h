#pragma once

#include "../Includes/ApplicationIncludes.h"

/* Window */

struct WindowParameters
{
    HINSTANCE           HInstance;
    HWND                HWnd;
};

/* Rendering */

struct SynchronizationSemaphores
{
    // Swap chain image presentation
    VkSemaphore presentComplete;
    // Command buffer submission and execution
    VkSemaphore renderComplete;
};

struct QueueFamilyIndices
{
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
};

struct DepthStencil
{
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct SwapChainBuffer
{
    VkImage         image;
    VkImageView     view;
};

/* ?? Need to look into this more ?? */
// Same uniform buffer layout as shader
struct UBOVS
{
    glm::mat4 projection;
    glm::mat4 modelView;
    glm::vec4 lightPos = glm::vec4(0.0f, 2.0f, 1.0f, 0.0f);
};
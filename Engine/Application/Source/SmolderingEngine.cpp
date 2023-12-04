
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Rendering/Renderer.h"
#include "Window/WindowManager.h"

int main()
{
    WindowManager window;       // Handles the window.
    Renderer engineRenderer;    // Handles rendering stuff.

    // Initialization 
    window.InitWindowClass();
    engineRenderer.InitRendererClass(window.GetWindow());

    // Update the application
    while (window.GetApplicationRunStatus())
    {
        if (window.UpdateWindowClass(engineRenderer) && engineRenderer.GetApplicationReadyToRender())
        {
            engineRenderer.UpdateRendererClass();
        }
    }

    // Clean-up
    engineRenderer.ShutdownRendererClass();

    return 0;
}

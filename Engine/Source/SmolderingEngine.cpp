
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Rendering/Renderer.h"
#include "Window/WindowManager.h"

int main()
{
    Renderer engineRenderer;
    WindowManager window;

    // Initialization 
    window.InitWindowClass();
    engineRenderer.InitRendererClass(window.GetWindow());

    // Update the application
    while (true)
    {
        window.UpdateWindowClass();
        engineRenderer.UpdateRendererClass();
    }

    // Clean-up
    engineRenderer.ShutdownRendererClass();

    return 0;
}

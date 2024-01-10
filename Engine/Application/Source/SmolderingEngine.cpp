
/*
Note: Currently SmoulderingEngine only supports Windows operating system, it is assumed you are
running this project on a Windows based platform. 
*/

#include "Rendering/Renderer.h"
#include "Window/WindowManager.h"

int main()
{
    // time stuff
    std::chrono::high_resolution_clock::time_point tickStart = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point tickEnd = std::chrono::high_resolution_clock::now();
    float timeBetweenFrames = 0.0f;

    WindowManager window;       // Handles the window.
    Renderer engineRenderer;    // Handles rendering stuff.

    // Initialization 
    window.InitWindowClass();
    engineRenderer.InitRendererClass(window.GetWindow());

    // Update the application
    while (window.GetApplicationRunStatus())
    {
        tickStart = std::chrono::high_resolution_clock::now();

        if (window.UpdateWindowClass(engineRenderer) && engineRenderer.GetApplicationReadyToRender())
        {
            // Convert time into seconds
            timeBetweenFrames = std::chrono::duration_cast<std::chrono::duration<float>>(tickStart-tickEnd).count();

            // update engine
            engineRenderer.UpdateRendererClass();
            window.UpdateInput(engineRenderer, timeBetweenFrames);
        }

        tickEnd = std::chrono::high_resolution_clock::now();
    }

    // Clean-up
    engineRenderer.ShutdownRendererClass();

    return 0;
}

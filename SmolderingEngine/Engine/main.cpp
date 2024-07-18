// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Standard Library
#include <stdexcept>
#include <vector>
#include <iostream>

// Project Includes
#include "Engine/Public/Rendering/Renderer.h"


// TODO: make a class to handle window operations / input
#pragma region WINDOW STUFFz
GLFWwindow* Window;

void InitWindow(std::string InWindowName = "Smoldering Engine", const int InWidth = 800, const int InHeight = 600)
{
	// init glfw
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// TODO: make window able to be resizable
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	Window = glfwCreateWindow(InWidth, InHeight, InWindowName.c_str(), nullptr, nullptr);

}
#pragma endregion

int main()
{
	Game SEGame;
	Renderer SERenderer;

	// Setup the window
	InitWindow("Smoldering Engine", 800, 600);

	// Setup the renderer
	if (SERenderer.InitRenderer(Window, SEGame) == EXIT_FAILURE)
		return EXIT_FAILURE;

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();
		SERenderer.Draw();
	}

	// Destroys all Renderer resources and the Game meshes
	SERenderer.DestroyRenderer();

	// Destroy GLFW window / GLFW
	glfwDestroyWindow(Window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
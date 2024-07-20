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

	float deltaTime = 0.0f, lastTime = 0.0f , modelRotation = 0.0f;

	while (!glfwWindowShouldClose(Window))
	{
		glfwPollEvents();

		// TODO: MOVE THIS
		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
		modelRotation += 10.0f * deltaTime;
		if (modelRotation > 360.0f)
			modelRotation -= 360.0f;
		SERenderer.UpdateModelPosition(glm::rotate(glm::mat4(1.0f), glm::radians(modelRotation), glm::vec3(0.0f, 0.0f, 1.0f)));


		SERenderer.Draw();
	}

	// Destroys all Renderer resources and the Game meshes
	SERenderer.DestroyRenderer();

	// Destroy GLFW window / GLFW
	glfwDestroyWindow(Window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
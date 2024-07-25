// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Standard Library
#include <stdexcept>
#include <vector>
#include <iostream>

// Project Includes
#include "Engine/Public/Rendering/Renderer.h"

Renderer* seRenderer;
Game* seGame;
GLFWwindow* seWindow;

float deltaTime = 0.0f;
float lastTime = 0.0f;

float modelY = 0.0f;
float xMovementSpeed = 100.0f;
float modelX = 0.0f;


// TODO: make a class to handle window operations / input
#pragma region WINDOW STUFFz

void InitWindow(std::string InWindowName = "Smoldering Engine", const int InWidth = 800, const int InHeight = 600)
{
	// init glfw
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// TODO: make window able to be resizable
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	seWindow = glfwCreateWindow(InWidth, InHeight, InWindowName.c_str(), nullptr, nullptr);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) 
	{
		/*static Game* gameInstance = nullptr;*/
		if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D) 
		{
			//if (!gameInstance) 
			//{
			//	// Find Game instance, assuming it's a global variable or you have a method to get it.
			//	// This is a placeholder; you need to adjust it based on how you manage the Game instance.
			//	gameInstance = static_cast<Game*>(glfwGetWindowUserPointer(window));
			//}
			glm::mat4 modelMatrix(1.0f);

			switch (key) {
			case GLFW_KEY_W: // Jump
				break;
			case GLFW_KEY_A: // Go left
				modelX += -xMovementSpeed * deltaTime;
				modelMatrix = glm::translate(modelMatrix, glm::vec3(modelX, 0.0f, 0.0f));
				seRenderer->UpdateModelPosition(seGame->GameMeshes.size() - 1, modelMatrix);
				break;
			case GLFW_KEY_D: // Go right
				modelX += xMovementSpeed * deltaTime;
				modelMatrix = glm::translate(modelMatrix, glm::vec3(modelX, 0.0f, 0.0f));
				seRenderer->UpdateModelPosition(seGame->GameMeshes.size() - 1, modelMatrix);
				break;
			default:
				break;
			}
		}
	}
}
#pragma endregion

int main()
{
	seRenderer = new Renderer();
	seGame = new Game();

	// Setup the window
	InitWindow("Smoldering Engine", 800, 600);
	glfwSetKeyCallback(seWindow, key_callback);
	glfwSetWindowUserPointer(seWindow, &seGame);

	// Setup the renderer
	if (seRenderer->InitRenderer(seWindow, seGame) == EXIT_FAILURE)
		return EXIT_FAILURE;

	while (!glfwWindowShouldClose(seWindow))
	{
		glfwPollEvents();

		// TODO: MOVE THIS
		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;
		//modelRotation += 0.05f * deltaTime;

		//if (modelRotation > 5.0f)
		//	modelRotation -= 5.0f;

		//glm::mat4 modelMatrix(1.0f);
		//modelMatrix = glm::rotate(modelMatrix, glm::radians(modelRotation), glm::vec3(0.0f, 0.0f, 1.0f));
		//modelMatrix = glm::translate(modelMatrix, glm::vec3(modelRotation, 0.0f, 0.0f));
		//seRenderer.UpdateModelPosition(0, modelMatrix);


		seRenderer->Draw();
	}

	// Destroys all Renderer resources and the Game meshes
	seRenderer->DestroyRenderer();

	// Destroy GLFW window / GLFW
	glfwDestroyWindow(seWindow);
	glfwTerminate();

	delete(seGame);
	delete(seRenderer);

	return EXIT_SUCCESS;
}
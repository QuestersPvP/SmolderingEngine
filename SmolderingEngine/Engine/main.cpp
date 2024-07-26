// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Standard Library
#include <stdexcept>
#include <vector>
#include <iostream>

// Project Includes
#include "Engine/Public/Rendering/Renderer.h"
#include "Engine/Public/Collision/CollisionManager.h"
#include "Game/Public/Game.h"

Renderer* seRenderer;
Game* seGame;
GLFWwindow* seWindow;

CollisionManager seCollision;

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
		if (key == GLFW_KEY_W || key == GLFW_KEY_SPACE || key == GLFW_KEY_A || key == GLFW_KEY_D)
		{
			//if (!gameInstance) 
			//{
			//	// Find Game instance, assuming it's a global variable or you have a method to get it.
			//	// This is a placeholder; you need to adjust it based on how you manage the Game instance.
			//	gameInstance = static_cast<Game*>(glfwGetWindowUserPointer(window));
			//}
			glm::mat4 modelMatrix(1.0f);

			switch (key) {
			case GLFW_KEY_W:		// Jump
				break;
			case GLFW_KEY_SPACE:	// Jump
				break;
			case GLFW_KEY_A:		// Go left
				modelX += -xMovementSpeed * deltaTime;
				modelMatrix = glm::translate(modelMatrix, glm::vec3(modelX, 0.0f, 0.0f));
				seRenderer->UpdateModelPosition(seGame->GameMeshes.size() - 1, modelMatrix);
				break;
			case GLFW_KEY_D:		// Go right
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
	glfwSetWindowUserPointer(seWindow, seGame);

	// Setup the renderer
	if (seRenderer->InitRenderer(seWindow, seGame) == EXIT_FAILURE)
		return EXIT_FAILURE;

	while (!glfwWindowShouldClose(seWindow))
	{
		// Calculate time since last frame
		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Check for window inputs
		glfwPollEvents();
		// Draw all objects
		seRenderer->Draw();
		// Check for any collisions
		seCollision.CheckForCollisions(seGame);

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
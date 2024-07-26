// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//chrono
#include <chrono>
#include <thread>

//include these for keyStates
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

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
float xMovementSpeed = 1.0f;
float yMovementSpeed = 1.0f;
float modelX = 0.0f;

//
std::unordered_map<int, bool> keyStates;

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

//always initialize
void initializeKeyStates() {
	keyStates[GLFW_KEY_W] = false;
	keyStates[GLFW_KEY_SPACE] = false;
	keyStates[GLFW_KEY_A] = false;
	keyStates[GLFW_KEY_D] = false;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//simple, if you are touching key it's true if not its false.
	if (action == GLFW_PRESS)
	{
		keyStates[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keyStates[key] = false;
	}
}

void processInput(float deltaTime) {
	//technically stupid but it happens so fast you cannot see the stupidity
	float movementX = 0.0f;
	float movementY = 0.0f;

	//every delta time it takes all key inputs and then applies them at the same time XD
	if (keyStates[GLFW_KEY_W]) {
		// Currently, no action for W
	}

	if (keyStates[GLFW_KEY_SPACE]) {
		movementY += yMovementSpeed * deltaTime;
	}

	if (keyStates[GLFW_KEY_A]) {
		movementX += -xMovementSpeed * deltaTime;
	}

	if (keyStates[GLFW_KEY_D]) {
		movementX += xMovementSpeed * deltaTime;
	}

	modelX += movementX;
	modelY += movementY;

	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(modelX, modelY, 0.0f)); //dunno
	seRenderer->UpdateModelPosition(seGame->GameMeshes.size() - 1, modelMatrix); //update in bulk
}
#pragma endregion

/**void fixedUpdate()
{
	// Your update logic here
	std::cout << "Fixed Update" << std::endl;
}**/

int main()
{
	seRenderer = new Renderer();
	seGame = new Game();

	// Setup the window
	InitWindow("Smoldering Engine", 800, 600);
	initializeKeyStates();
    glfwSetKeyCallback(seWindow, key_callback);

	// Setup the renderer
	if (seRenderer->InitRenderer(seWindow, seGame) == EXIT_FAILURE)
		return EXIT_FAILURE;

	//chrono stuff, update it 1/30 per second
	constexpr int updatesPerSecond = 30; //if you want this to not be initialized in compiler change constexpr to const
	constexpr std::chrono::duration<float> updateInterval(1.0 / updatesPerSecond); //if you want this to not be initialized in compiler change constexpr to const
	auto previousTime = std::chrono::high_resolution_clock::now(); //initialize
	//auto lag = std::chrono::duration<double>::zero();

	while (!glfwWindowShouldClose(seWindow))
	{
		

		//Need to account for lag
		/**
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsedTime = currentTime - previousTime;
		previousTime = currentTime;
		lag += elapsedTime;

		while (lag >= updateInterval)
		{
			fixedUpdate();
			lag -= updateInterval;
		}**/

		/** //Calculate time since last frame
		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;**/

		//process da inputs 30 times per second please 
		processInput(updateInterval.count());
		// Check for window inputs
		glfwPollEvents();
		// Draw all objects
		seRenderer->Draw();
		// Check for any collisions
		seCollision.CheckForCollisions(seGame);

		// Optionally sleep to prevent busy waiting
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

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
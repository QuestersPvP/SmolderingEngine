// Third Party
#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

//include these for keyStates
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

// Standard Library
#include <stdexcept>
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>

// Project Includes
#include "Engine/Public/Rendering/Renderer.h"
#include "Engine/Public/Collision/CollisionManager.h"
#include "Game/Public/Game.h"

Renderer* seRenderer;
Game* seGame;
GLFWwindow* seWindow;

CollisionManager seCollision;

const int JUMP_TIME = 30;
float deltaTime = 0.0f;
float lastTime = 0.0f;

float modelY = 0.0f;
float modelX = 0.0f;
float xMovementSpeed = 0.20f;
float yMovementSpeed = 0.20f;
bool playerJumping = false;
int jumpTime = JUMP_TIME;

// temp
float angle = 0.0f;
bool rotateLeft = true;

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

float ProcessJump(float inDeltaTime)
{
	return std::min(((yMovementSpeed + 0.2f) * inDeltaTime) / ((float)jumpTime / (float)JUMP_TIME), 0.1f);
}

void processInput(float inDeltaTime) {
	//technically stupid but it happens so fast you cannot see the stupidity
	float movementX = 0.0f;
	float movementY = 0.0f;

	//every delta time it takes all key inputs and then applies them at the same time XD
	if (keyStates[GLFW_KEY_W]) {
		// Currently, no action for W
	}

	if (keyStates[GLFW_KEY_SPACE] && !playerJumping && modelY == 0.0f) 
	{
		playerJumping = true;
		jumpTime = 1;

		movementY += ProcessJump(inDeltaTime);
		jumpTime++;
	}
	else if (playerJumping && jumpTime < JUMP_TIME)
	{
		movementY += ProcessJump(inDeltaTime);
		jumpTime++;

		if (jumpTime >= JUMP_TIME)
			playerJumping = false;
	}
	else
	{
		movementY += -yMovementSpeed * inDeltaTime;
	}

	if (keyStates[GLFW_KEY_A]) {
		movementX += -xMovementSpeed * inDeltaTime;
	}

	if (keyStates[GLFW_KEY_D]) {
		movementX += xMovementSpeed * inDeltaTime;
	}

	modelX += movementX;
	modelY += movementY;

	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(modelX, modelY, 0.0f)); // translate the player model

	// stop the player from going under the ground
	if (modelMatrix[3].y < 0.0f)
	{
		modelMatrix[3].y = 0.0f;
		modelY = 0.0f;
	}

	//seRenderer->UpdateModelPosition(seGame->gameObjects.size() - 1, modelMatrix , 0); //update in bulk
}
#pragma endregion

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

	// -------- imGui ------------
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(seWindow, true);
	// Initialize ImGui for Vulkan
	if (seRenderer->InitImGuiForVulkan() == EXIT_FAILURE)
		return EXIT_FAILURE;
	// -------- imGui ------------

	//chrono stuff, update it 1/30 per second
	constexpr int updatesPerSecond = 30; //if you want this to not be initialized in compiler change constexpr to const
	constexpr std::chrono::duration<float> updateInterval(1.0 / updatesPerSecond); //if you want this to not be initialized in compiler change constexpr to const
	auto previousTime = std::chrono::high_resolution_clock::now(); //initialize
	//auto lag = std::chrono::duration<double>::zero();

	while (!glfwWindowShouldClose(seWindow))
	{
		

		//TODO: work on this to account account for lag
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

		////Calculate time since last frame
		//float now = glfwGetTime();
		//deltaTime = now - lastTime;
		//lastTime = now;



		//process da inputs 30 times per second please 
		processInput(updateInterval.count());


		// temp
		float rotationSpeed = 2.5f;
		if (rotateLeft)
		{
			angle -= rotationSpeed * updateInterval.count();
			float rotateAmount = -rotationSpeed * updateInterval.count();
			seGame->gameObjects[0]->ApplyLocalYRotation(rotateAmount);

			if (angle <= -90.0f)
				rotateLeft = false;
		}
		//else if (!rotateLeft)
		//{
		//	angle += rotationSpeed * updateInterval.count();
		//	float rotateAmount = rotationSpeed * updateInterval.count();
		//	seGame->gameObjects[0]->ApplyLocalYRotation(rotateAmount);

		//	if (angle >= 0.0f)
		//		rotateLeft = true;
		//}

		// Check for window inputs
		glfwPollEvents();
		// Draw all objects
		seRenderer->Draw();

		// Check for any collisions
		seCollision.CheckForCollisions();
		
		//if (reset)
		//{
		//	modelX = 0.0f;
		//	modelY = 0.0f;
		//}

		// Sleep to prevent busy waiting
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
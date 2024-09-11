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
#include "Engine/Public/Input/InputManager.h"
#include "Engine/Public/Camera/Camera.h"
#include "Engine/Public/EngineLevel/EngineLevelManager.h"

#include "Game/Public/Game.h"

Renderer* seRenderer;
Game* seGame;
CollisionManager* seCollision;
Camera* seCamera;
InputManager* seInput;
EngineLevelManager* seEngineLevel;

GLFWwindow* seWindow;

int main()
{
	seInput = new InputManager();
	seCamera = new Camera();
	seRenderer = new Renderer();
	seGame = new Game(); // TODO: this should be broken into 2 classes at least - 1 for game management (engine side), 1 more focused directly gameplay
	seCollision = new CollisionManager();

	// Setup the window
	seInput->InitWindow("Smoldering Engine", 1280, 720);
	seInput->initializeKeyStates();
    //glfwSetKeyCallback(seWindow, key_callback);

	// Setup the renderer
	if (seRenderer->InitRenderer(seInput->window, seGame, seCamera) == EXIT_FAILURE)
		return EXIT_FAILURE;

	seEngineLevel = new EngineLevelManager(seRenderer->GetPhysicalDevice(), seRenderer->GetLogicalDevice(),
		seRenderer->GetGraphicsQueue(), seRenderer->GetGraphicsCommandPool(), seGame, seRenderer);
	seRenderer->SetEngineLevelManager(seEngineLevel); // will remove this soon
	seEngineLevel->LoadLevel(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/newLevel.selevel");

	//// --- TEMP ROTATION ETC. UNTIL WE ADD MORE FUNCTIONALITY TO FILE LOAD/SAVER ---
	// model was rotated strange - this is just to fix that for now.
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	seGame->gameObjects[0]->SetModel(modelMatrix);


	//ObjectData temp = seGame->gameObjects[0]->GetObjectData();
	//temp.objectMatrix = modelMatrix;
	//// --- TEMP ROTATION ETC. UNTIL WE ADD MORE FUNCTIONALITY TO FILE LOAD/SAVER ---

	// -------- imGui ------------
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(seInput->window, true);
	// Initialize ImGui for Vulkan
	if (seRenderer->InitImGuiForVulkan() == EXIT_FAILURE)
		return EXIT_FAILURE;
	// -------- imGui ------------

	//chrono stuff, update it 1/30 per second
	constexpr int updatesPerSecond = 30; //if you want this to not be initialized in compiler change constexpr to const
	constexpr std::chrono::duration<float> updateInterval(1.0 / updatesPerSecond); //if you want this to not be initialized in compiler change constexpr to const
	auto previousTime = std::chrono::high_resolution_clock::now(); //initialize
	//auto lag = std::chrono::duration<double>::zero();

	while (!glfwWindowShouldClose(seInput->window))
	{
		//process da inputs 30 times per second please 
		seInput->processInput(updateInterval.count(), seCamera);

		// Check for window inputs
		glfwPollEvents();
		// Draw all objects
		seRenderer->Draw();
		
		// Sleep to prevent busy waiting
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Destroys all Renderer resources, also destroys all game objects currently
	seRenderer->DestroyRenderer();

	delete(seEngineLevel);

	// Destroy GLFW window / GLFW
	glfwDestroyWindow(seInput->window);
	glfwTerminate();

	delete(seCollision);
	delete(seGame);
	delete(seRenderer);
	delete(seCamera);
	delete(seInput);

	return EXIT_SUCCESS;
}
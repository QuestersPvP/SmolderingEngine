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
#include "Engine/Public/Rendering/SkyboxRenderer.h"

#include "Engine/Public/Collision/CollisionManager.h"
#include "Engine/Public/Input/InputManager.h"
#include "Engine/Public/Camera/Camera.h"
#include "Engine/Public/EngineLevel/EngineLevelManager.h"

#include "Game/Public/Game.h"

Renderer* seRenderer;
SkyboxRenderer* seSkyboxRenderer;
Game* seGame;
CollisionManager* seCollision;
Camera* seCamera;
InputManager* seInput;
EngineLevelManager* seEngineLevel;

//GLFWwindow* seWindow;

int main()
{
	seInput = new InputManager();
	seCamera = new Camera();
	seRenderer = new Renderer();
	seSkyboxRenderer = new SkyboxRenderer();
	seGame = new Game(); // TODO: this should be broken into 2 classes at least - 1 for game management (engine side), 1 more focused directly gameplay
	seCollision = new CollisionManager();

	// Setup the window
	seInput->InitWindow("Smoldering Engine", 1280, 720);

	// Setup the renderer
	if (seRenderer->InitRenderer(seInput->window, seGame, seCamera) == EXIT_FAILURE)
		return EXIT_FAILURE;

	// --- LEVEL MANAGER ---
	seEngineLevel = new EngineLevelManager(seRenderer->GetPhysicalDevice(), seRenderer->GetLogicalDevice(),
		seRenderer->GetGraphicsQueue(), seRenderer->GetGraphicsCommandPool(), seGame, seRenderer);
	seRenderer->SetEngineLevelManager(seEngineLevel); // will remove this soon
	seEngineLevel->LoadLevel(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/defaultLevel.selevel");
	// --- LEVEL MANAGER ---

	// --- IMGUI ---
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(seInput->window, true);
	// Initialize ImGui for Vulkan
	if (seRenderer->InitImGuiForVulkan() == EXIT_FAILURE)
		return EXIT_FAILURE;
	// --- IMGUI ---

	// --- LOAD SKYBOX ---
	std::vector<std::string> imageNames =
								// TODO: is this correct?
		{"posx.jpg", "negx.jpg", "negy.jpg", "posy.jpg", "posz.jpg", "negz.jpg" };
		//{"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
	std::string fileLocation = 
		std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Skybox/mountain-skyboxes/Maskonaive2/";
	
	if (seSkyboxRenderer->InitSkyboxRenderer(fileLocation, imageNames, seRenderer) == EXIT_FAILURE)
		return EXIT_FAILURE;
	// --- LOAD SKYBOX ---

	//chrono stuff, update it 1/30 per second
	constexpr int updatesPerSecond = 30; //if you want this to not be initialized in compiler change constexpr to const
	constexpr std::chrono::duration<float> updateInterval(1.0 / updatesPerSecond); //if you want this to not be initialized in compiler change constexpr to const
	auto previousTime = std::chrono::high_resolution_clock::now(); //initialize
	//auto lag = std::chrono::duration<double>::zero();

	while (!glfwWindowShouldClose(seInput->window))
	{
		// Check for window inputs
		glfwPollEvents();

		// Ensure the window is not minimized
		if (glfwGetWindowAttrib(seInput->window, GLFW_ICONIFIED) == GLFW_FALSE)
		{
			// Handle window resizing
			int width, height;
			glfwGetFramebufferSize(seInput->window, &width, &height);
			if (seInput->windowWidth != width || seInput->windowHeight != height)
			{
				seRenderer->ResizeRenderer(width, height);
				seInput->windowWidth = width;
				seInput->windowHeight = height;
			}

			//process da inputs 30 times per second please 
			seInput->processInput(updateInterval.count(), seCamera);

			// TODO: Finish reworking the renderer classes. It is a work in progress currently.
			// There should be a skyboxrenderer (complete), a levelrenderer (not done), and finally
			// a general renderer that handles creating logical devices etc. and figuring out what to draw. 
			// Draw all objects
			seRenderer->Draw(seSkyboxRenderer);
		}
		
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
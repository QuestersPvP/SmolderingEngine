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
#include "Engine/Source/EngineManager.h"


#include "Engine/Source/Public/Rendering/Renderer.h"
//#include "Engine/Public/Rendering/SkyboxRenderer.h"

#include "Engine/Source/Public/Collision/CollisionManager.h"
#include "Engine/Source/Public/Input/InputManager.h"
#include "Engine/Source/Public/Camera/Camera.h"
#include "Engine/Source/Public/EngineLevel/EngineLevelManager.h"

#include "Game/Source/Public/Game.h"

EngineManager* seEngineManager;

//Renderer* seRenderer;
//SkyboxRenderer* seSkyboxRenderer;
Game* seGame;
CollisionManager* seCollision;
//Camera* seCamera;
//InputManager* seInput;
//EngineLevelManager* seEngineLevel;

//GLFWwindow* seWindow;

int main()
{
	seEngineManager = EngineManager::GetEngineManager();
	seEngineManager->GetEngineLevelManager()->LoadLevel(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/defaultLevel.selevel");

	// Engine manager should handle these
	//seInput = new InputManager();
	//seCamera = new Camera();
	seCollision = new CollisionManager(); // TODO: MAKE COLLISION MANAGER WORK AGAIN

	// Game manager
	seGame = new Game(); // TODO: this should be broken into 2 classes at least - 1 for game management (engine side), 1 more focused directly gameplay

	// Setup the window
	//seInput->InitWindow("Smoldering Engine", 1280, 720);

	// Setup the renderer
	//seRenderer = new Renderer(seInput->window, seCamera);

	// --- LEVEL MANAGER ---
	//seEngineLevel = new EngineLevelManager(seRenderer->GetPhysicalDevice(), seRenderer->GetLogicalDevice(),
	//	seRenderer->GetGraphicsQueue(), seRenderer->GetGraphicsCommandPool(), seRenderer);
	//seRenderer->SetEngineLevelManager(seEngineLevel); // will remove this soon
	//seEngineLevel->LoadLevel(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/defaultLevel.selevel");
	// --- LEVEL MANAGER ---

	//chrono stuff, update it 1/30 per second
	constexpr int updatesPerSecond = 30; //if you want this to not be initialized in compiler change constexpr to const
	constexpr std::chrono::duration<float> updateInterval(1.0 / updatesPerSecond); //if you want this to not be initialized in compiler change constexpr to const
	auto previousTime = std::chrono::high_resolution_clock::now(); //initialize
	//auto lag = std::chrono::duration<double>::zero();

	while (!glfwWindowShouldClose(seEngineManager->GetInputManager()->window))
	{
		// Check for window inputs
		glfwPollEvents();

		// Ensure the window is not minimized
		if (glfwGetWindowAttrib(seEngineManager->GetInputManager()->window, GLFW_ICONIFIED) == GLFW_FALSE)
		{
			// Handle window resizing
			int width, height;
			glfwGetFramebufferSize(seEngineManager->GetInputManager()->window, &width, &height);
			if (seEngineManager->GetInputManager()->windowWidth != width || seEngineManager->GetInputManager()->windowHeight != height)
			{
				seEngineManager->GetRenderer()->ResizeRenderer(width, height);
				seEngineManager->GetInputManager()->windowWidth = width;
				seEngineManager->GetInputManager()->windowHeight = height;
			}

			//process da inputs 30 times per second please 
			seEngineManager->GetInputManager()->processInput(updateInterval.count(), seEngineManager->GetCamera());

			// TODO: Finish reworking the renderer classes. It is a work in progress currently.
			// There should be a skyboxrenderer (complete), a levelrenderer (not done), and finally
			// a general renderer that handles creating logical devices etc. and figuring out what to draw. 
			// Draw all objects
			seEngineManager->GetRenderer()->Draw();
		}
		
		// Sleep to prevent busy waiting
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Destroys all Renderer resources, also destroys all game objects currently
	//seRenderer->DestroyRenderer();

	//delete(seEngineLevel);

	// Destroy GLFW window / GLFW
	//glfwDestroyWindow(seEngineManager->GetInputManager()->window);
	//glfwTerminate();

	delete(seCollision);
	delete(seGame);
	//delete(seCamera);
	//delete(seInput);

	return EXIT_SUCCESS;
}
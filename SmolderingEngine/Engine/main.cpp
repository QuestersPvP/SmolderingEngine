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

Game* seGame;
CollisionManager* seCollision;


int main()
{
	seEngineManager = EngineManager::GetEngineManager();

	seCollision = new CollisionManager(); // TODO: MAKE COLLISION MANAGER WORK AGAIN

	// Game manager
	seGame = new Game(); // TODO: this should be broken into 2 classes at least - 1 for game management (engine side), 1 more focused directly gameplay

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

			// TODO: This helps reduce loading hitches but is not perfect. Make it better
			seEngineManager->GetEngineLevelManager()->ProcessLevelModelTasks();

			//process da inputs 30 times per second please 
			seEngineManager->GetInputManager()->processInput(updateInterval.count(), seEngineManager->GetCamera());

			// Notifies all renderers to draw
			seEngineManager->GetRenderer()->Draw();
		}
		
		// Sleep to prevent busy waiting
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	delete(seCollision);
	delete(seGame);

	return EXIT_SUCCESS;
}
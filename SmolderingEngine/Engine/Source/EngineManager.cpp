#include "EngineManager.h"

// Project Includes
#include "Engine/Source/Public/Input/InputManager.h"
#include "Engine/Source/Public/Camera/Camera.h"
#include "Engine/Source/Public/Rendering/Renderer.h"
#include "Engine/Source/Public/EngineLevel/EngineLevelManager.h"

EngineManager* EngineManager::seEngineInstance = nullptr;

EngineManager* EngineManager::GetEngineManager()
{
	{
		if (seEngineInstance == nullptr)
			seEngineInstance = new EngineManager();

		return seEngineInstance;
	};
}

void EngineManager::DeleteEngineManager()
{
	// TODO: CLEANUP ENGINE MANAGER
}

EngineManager::EngineManager()
{
	seInputManager = new InputManager("Smoldering Engine", 1280, 720);
	seCamera = new Camera(45.f, 1280.f, 720.f, 0.1f, 1000.f);
	seRenderer = new Renderer(seInputManager->window, seCamera);

	seEngineLevel = new EngineLevelManager(seRenderer);
	// Load the level
	seEngineLevel->LoadLevel(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/defaultLevel.selevel");
}

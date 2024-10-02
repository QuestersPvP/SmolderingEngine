#pragma once

class EngineManager
{
	/* Variables */
private:
	// Reference to the instance of the EngineManager
	static EngineManager* seEngineInstance;

	/*
	* Holds all functionality to handle user input
	*/
	class InputManager* seInputManager = nullptr;
	
	/*
	* Holds functionality to setup where the scene is being "viewed from"
	*/
	class Camera* seCamera = nullptr;

	/*
	* Holds all renderers inside of a base renderer
	*/
	class Renderer* seRenderer = nullptr;

	/*
	* EngineLevelManager holds the functionality to Load / Save levels. It also holds the ObjectManager
	* These two classes hold all the objects within the scene we are in.
	*/
	class EngineLevelManager* seEngineLevel = nullptr;

	/* Functions */
public:

	// Get the instance of engine manager or create it if there is none
	static EngineManager* GetEngineManager();
	// Delete the engine manager
	void DeleteEngineManager();

	/* Getters */
	class InputManager* GetInputManager() { return seInputManager; };
	class Camera* GetCamera() { return seCamera; };
	class Renderer* GetRenderer() { return seRenderer; };
	class EngineLevelManager* GetEngineLevelManager() { return seEngineLevel; };

private:
	EngineManager();

	// Remove the copy and assignment operators
	EngineManager(const EngineManager&) = delete;
	EngineManager& operator=(const EngineManager&) = delete;
};
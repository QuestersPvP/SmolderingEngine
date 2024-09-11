#pragma once

// Standard Library
#include <string>
#include <vector>
#include <iostream>

#include <fstream>
#include <sstream>
#include <iomanip>  // Required for std::setprecision
#include <limits>   // Required for std::numeric_limits

// Third Party
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class EngineLevelManager
{
	/* Variables */
private:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue transferQueue;
	VkCommandPool transferCommandPool;

	// TODO: probs should make a Scene class that holds all the objects etc. instead of holding them in Game.h/cpp
	class Game* game;
	class Renderer* renderer;

	/* Functions*/
public:
	EngineLevelManager();
	EngineLevelManager(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue,
		VkCommandPool inTransferCommandPool, class Game* inGame, class Renderer* inRenderer);

	/*
	* Loads a level from the specified file path and level name.
	* Requires a full path e.g. C:/YourFolders/YourFolders/SmolderingEngine/SmolderingEngine/Game/Levels/newLevel.selevel
	*/
	void LoadLevel(std::string inLevelFilePath);

	/*
	* Saves a level to wherever you specify
	*/
	void SaveLevel(std::string inFileName);

	// TODO: make a class for this
	std::string MakeRelativePath(const std::string& inPath);
	std::string OpenFileExplorer();
	std::string SaveFileExplorer();

	// TODO: make private?
	void DestroyGameMeshes();
	void LoadNewScene();

private:
	/*
	* Loads a MeshModel (e.g. holds multiple meshes to form one model)
	* From the file path provided!
	*/
	void LoadMeshModel(struct ObjectData inObject);
};

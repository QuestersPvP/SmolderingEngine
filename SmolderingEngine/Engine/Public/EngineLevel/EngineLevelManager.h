#pragma once

// Standard Library
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

// Third Party
#include <vulkan/vulkan.h>

struct ObjectData
{
	int objectID = -1;
	int parentID = -1;
	std::string objectPath;
	std::string texturePath;
};

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


private:
	/*
	* Loads a MeshModel (e.g. holds multiple meshes to form one model)
	* From the file path provided!
	*/
	void LoadMeshModel(ObjectData inObject);
};

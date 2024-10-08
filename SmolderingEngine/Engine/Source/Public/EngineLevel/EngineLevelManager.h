#pragma once

// Standard Library
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>  // Required for std::setprecision
#include <limits>   // Required for std::numeric_limits

#include <queue>
#include <mutex>
#include <functional>

// Third Party
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct VulkanTask
{
	std::function<void()> function;
};

class EngineLevelManager
{
	/* Variables */
private:
	class ObjectManager* seObjectManager;
	class Renderer* seRenderer;
	class EngineManager* seEngineManager;

	// Basic vulkan variables needed for loading models
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue transferQueue;
	VkCommandPool transferCommandPool;

	// Multi-thread/synchronize loading objects
	std::queue<VulkanTask> vulkanTaskQueue;
	std::mutex taskQueueMutex;

	/* Functions*/
public:
	EngineLevelManager() {};
	EngineLevelManager(class Renderer* _renderer);

	/*
	* Loads a level from the specified file path and level name.
	* Requires a full path e.g. C:/YourFolders/YourFolders/SmolderingEngine/SmolderingEngine/Game/Levels/newLevel.selevel
	*/
	void LoadLevel(std::string inLevelFilePath);

	/*
	* Loads level models asynchronously when they are done loading
	*/
	void ProcessLevelModelTasks();

	/*
	* Saves a level to wherever you specify
	*/
	void SaveLevel(std::string inFileName);

	// TODO: make a class for this
	std::string MakeRelativePath(const std::string& inPath);
	std::string OpenFileExplorer();
	std::string SaveFileExplorer();

	void LoadNewScene();

	/* Getters + Setters */
	class ObjectManager* GetObjectManager() { return seObjectManager; };

private:
	/*
	* Loads a MeshModel (e.g. holds multiple meshes to form one model)
	* From the file path provided!
	*/
	void LoadMeshModel(struct ObjectData inObject);
};

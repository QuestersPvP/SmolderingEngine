#include "Engine/Public/EngineLevel/EngineLevelManager.h"

// Engine includes
#include "Engine/Public/Rendering/Renderer.h"
#include "Engine/Public/Rendering/Mesh.h"

#include "Engine/Public/Object/GameObject.h"

#include "Game/Public/Game.h"

EngineLevelManager::EngineLevelManager()
{
}

EngineLevelManager::EngineLevelManager(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
	Game* inGame, class Renderer* inRenderer)
{
	physicalDevice = inPhysicalDevice;
	logicalDevice = inLogicalDevice;
	transferQueue = inTransferQueue;
	transferCommandPool = inTransferCommandPool;
	game = inGame;
	renderer = inRenderer;
}

void EngineLevelManager::LoadLevel(std::string inLevelFilePath)
{
	std::ifstream file(inLevelFilePath);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << inLevelFilePath << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		ObjectData currentObject;

		if (line[0] == '@') // Determine what is being loaded
		{
			std::string keyword = line.substr(1);
			if (keyword == "objectID")
			{
				while (std::getline(file, line))
				{
					if (line.empty())
						continue;
					else if (line[0] == '@')
					{
						keyword = line.substr(1);
						break;
					}
					else if (line[0] == '~')
					{
						line = line.substr(1); // Remove '~'
						currentObject.objectID = std::stoi(line); // Parse the object ID
					}
				}
			}
			if (keyword == "parentID")
			{
				while (std::getline(file, line))
				{
					if (line.empty())
						continue;
					else if (line[0] == '@')
					{
						keyword = line.substr(1);
						break;
					}
					else if (line[0] == '~')
					{
						line = line.substr(1); // Remove '~'
						currentObject.parentID = std::stoi(line); // Parse the parent ID
					}
				}
			}
			if (keyword == "objectPath")
			{
				while (std::getline(file, line))
				{
					if (line.empty()) 
						continue;
					else if (line[0] == '@')
					{
						keyword = line.substr(1);
						break;
					}
					else if (line[0] == '~')
					{
						line = line.substr(1); // Remove '~'
						currentObject.objectPath = std::string(PROJECT_SOURCE_DIR) + line;
					}
				}
			}
			if (keyword == "texturePath")
			{
				// Load texture filename
				while (std::getline(file, line))
				{
					if (line.empty())
						continue;
					else if (line[0] == '-')
					{
						// Create the mesh
						LoadMeshModel(currentObject);
						break;
					}
					else if (line[0] == '@')
					{
						keyword = line.substr(1);
						break;
					}
					else if (line[0] == '~')
					{
						line = line.substr(1); // Remove '~'
						currentObject.texturePath = std::string(PROJECT_SOURCE_DIR) + line;
					}
					else
						std::cout << "Strange line encountered while reading in file: " + line + "\n";
				}
			}
		}
	}

	file.close();
}

void EngineLevelManager::LoadMeshModel(ObjectData inObject)
{
	// Import model scene
	Assimp::Importer importer;

	// After model included, make sure all faces are triangulated
	// Also make sure UVs match our UV system, and finally try to remove any duplicate verticies
	const aiScene* scene = importer.ReadFile(inObject.objectPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw std::runtime_error("failed to load the model passed in: " + inObject.objectPath);

	std::vector<std::string> textureNames = MeshModel::LoadMaterials(scene);

	// convert the material list IDs to descriptor array IDs
	std::vector<int> materialToTexture(textureNames.size());

	for (size_t i = 0; i < textureNames.size(); i++)
	{
		if (textureNames[i].empty())
			materialToTexture[i] = 0;
		else
		{
			//std::string fileLoc = (std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Models/Castle/Textures/" + textureNames[i]);
			//std::string fileLoc = (std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Models/Aircraft/textures/" + textureNames[i]);
			std::string fileLoc = (inObject.texturePath + textureNames[i]);
			materialToTexture[i] = renderer->CreateTexture(fileLoc);
		}
	}

	// Load in all the meshes
	std::vector<Mesh> modelMeshes = MeshModel::LoadNode(physicalDevice, logicalDevice, transferQueue, transferCommandPool, scene->mRootNode, scene, materialToTexture);

	MeshModel meshModel = MeshModel(modelMeshes);
	GameObject* object = new GameObject();
	object->objectMeshModel = meshModel;
	object->SetUseTexture(1);
	object->SetObjectID(0);
	object->SetObjectParentID(-1);
	game->gameObjects.push_back(object);
}

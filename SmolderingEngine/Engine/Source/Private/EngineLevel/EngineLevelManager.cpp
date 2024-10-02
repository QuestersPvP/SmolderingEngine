#include "Engine/Source/Public/EngineLevel/EngineLevelManager.h"

// Engine includes
#include "Engine/Source/Public/Rendering/Renderer.h"
#include "Engine/Source/Public/Rendering/LevelRenderer.h"

#include "Engine/Source/Public/Object/ObjectManager.h"
#include "Engine/Source/Public/Object/GameObject.h"
#include "Engine/Source/Public/Rendering/Mesh.h"

//#include "Game/Source/Public/Game.h"


EngineLevelManager::EngineLevelManager(Renderer* _renderer)
	: seRenderer(_renderer)
{
	physicalDevice = seRenderer->GetPhysicalDevice();
	logicalDevice = seRenderer->GetLogicalDevice();
	transferQueue = seRenderer->GetGraphicsQueue();
	transferCommandPool = seRenderer->GetGraphicsCommandPool();

	seObjectManager = new ObjectManager();
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
		currentObject.objectMatrix = glm::mat4(1.0f);

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
						currentObject.texturePath = std::string(PROJECT_SOURCE_DIR) + line;
					}
				}
			}
			if (keyword == "objectMatrix")
			{
				int row = 0;
				while (std::getline(file, line))
				{
					if (line.empty())
						continue;
					else if (line[0] == '-')
					{
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

						std::stringstream ss(line);
						std::string valueStr;
						float values[4];

						for (int col = 0; col < 4; ++col) 
						{
							std::getline(ss, valueStr, ',');
							values[col] = std::stof(valueStr);
						}

						currentObject.objectMatrix[row] = glm::vec4(values[0], values[1], values[2], values[3]);
						row++;
					}
				}
			}

			if (line[0] == '-')
			{
				// Create the mesh
				LoadMeshModel(currentObject);
				break;
			}
			else
			{
				std::cout << "Strange line encountered while reading in file: " + line + "\n";
			}
		}
	}

	file.close();
}

void EngineLevelManager::SaveLevel(std::string inFileName)
{
	std::ofstream outFile(inFileName);

	if (!outFile.is_open()) 
	{
		std::cout << "Error: Could not open the file for writing." << std::endl;
		return;
	}

	for (GameObject* object : seObjectManager->GetGameObjects())
	{
		ObjectData data = object->GetObjectData();

		outFile << "-" << std::endl;
		outFile << "@objectID" << std::endl;
		outFile << "~" << data.objectID << std::endl;
		outFile << "@parentID" << std::endl;
		outFile << "~" << data.parentID << std::endl;
		// For file paths we convert it to a relative file path before saving.
		outFile << "@objectPath" << std::endl;
		std::string objectPath = MakeRelativePath(data.objectPath);
		outFile << "~" << objectPath << std::endl;
		outFile << "@texturePath" << std::endl;
		std::string texturePath = MakeRelativePath(data.texturePath);
		outFile << "~" << texturePath << std::endl;
		
		// Write the model matrix
		outFile << "@objectMatrix" << std::endl;
		outFile << std::fixed << std::setprecision(8);

		for (int row = 0; row < 4; ++row) 
		{
			outFile << "~"; 
			for (int col = 0; col < 4; ++col) 
			{
				outFile << object->GetModel().modelMatrix[row][col];
				if (col < 3)
					outFile << ",";  // Add a comma between elements (but not after the last one)
			}
			outFile << std::endl;
		}

		outFile << "-" << std::endl;
	}

	outFile.close();
}

std::string EngineLevelManager::MakeRelativePath(const std::string& inPath)
{
	// TODO: The name of the folder could change, we should account for that at some point.
	// Define the base folder that marks the start of your relative path
	const std::string baseFolder = "SmolderingEngine";

	// Find the position of "SmolderingEngine" in the full path
	size_t position = inPath.find(baseFolder);
	position += baseFolder.size();

	// If "SmolderingEngine" is found, strip everything before it
	if (position != std::string::npos) 
	{
		return inPath.substr(position);
	}

	return inPath;
}

std::string EngineLevelManager::OpenFileExplorer()
{
	OPENFILENAME ofn;       // Common dialog box structure
	char szFile[260];       // Buffer for file name
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr; // Handle to owner window (nullptr for no specific window)
	ofn.lpstrFile = szFile;
	// Set initial filename to empty
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	// File type filter (show only .obj files)
	ofn.lpstrFilter = "SmolderingEngine Level Files\0*.selevel\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box
	if (GetOpenFileName(&ofn) == TRUE)
	{
		return std::string(ofn.lpstrFile); // Return the selected file path
	}

	return ""; // Return an empty string if no file is selected
}

std::string EngineLevelManager::SaveFileExplorer()
{
	OPENFILENAME ofn;       // Common dialog box structure
	char szFile[260];       // Buffer for file name
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr; // Handle to owner window (nullptr for no specific window)
	ofn.lpstrFile = szFile;
	// Set initial filename to empty
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	// File type filter (show only .selevel files)
	ofn.lpstrFilter = "SmolderingEngine Level Files\0*.selevel\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT; // Overwrite prompt flag for existing files

	// Display the Save dialog box
	if (GetSaveFileName(&ofn) == TRUE)
	{
		// Append the ".selevel" extension if not already provided
		std::string filePath(ofn.lpstrFile);
		if (filePath.find(".selevel") == std::string::npos)
		{
			filePath += ".selevel";  // Add the file extension if not present
		}
		return filePath;  // Return the selected file path
	}

	return ""; // Return an empty string if no file is selected
}

void EngineLevelManager::DestroyGameMeshes()
{
	// TODO: MOVE THIS

	// Wait until queues and all operations are done before cleaning up
	vkDeviceWaitIdle(logicalDevice);

	for (int i = seObjectManager->GetGameObjects().size() - 1; i >= 0; i--)
	{
		MeshModel* tempModel = seObjectManager->GetGameObjects()[i]->objectMeshModel;
		tempModel->DestroyMeshModel();
		delete seObjectManager->GetGameObjects()[i];
	}

	//game->gameObjects.clear();
}

void EngineLevelManager::LoadNewScene()
{
	// Wait until queues and all operations are done before cleaning up
	vkDeviceWaitIdle(logicalDevice);

	// Destroy texture-related Vulkan objects for the current level
	//renderer->DestroyAllRendererTextures();

	// TODO: Eventually we should do this, issue is the DescriptorPool wont allow it unless we modify it some.
	// it adds an excess of ~3mb of memory but technically doesnt leak it because when the descriptor pool is cleaned up
	// so it that memory. But in general we should clean it up so if someone uses the program for extended periods it wont
	// run out of memory because a vector keeps expanding its size.
	//vkFreeDescriptorSets(Devices.LogicalDevice, samplerDescriptorPool, static_cast<uint32_t>(samplerDescriptorSets.size()), samplerDescriptorSets.data());

	// Destroy current objects
	DestroyGameMeshes();

	std::string filePath = OpenFileExplorer();
	// the file path is returned such as C:\\name\\bleh.selvel
	// all we are doing is replacing all those \\ with a normal /
	std::replace(filePath.begin(), filePath.end(), '\\', '/');

	LoadLevel(filePath);
}

void EngineLevelManager::LoadMeshModel(ObjectData _objectData)
{
	// Import model scene
	Assimp::Importer importer;

	// After model included, make sure all faces are triangulated
	// Also make sure UVs match our UV system, and finally try to remove any duplicate verticies
	const aiScene* scene = importer.ReadFile(_objectData.objectPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw std::runtime_error("failed to load the model passed in: " + _objectData.objectPath);

	std::vector<std::string> textureNames = MeshModel::LoadMaterials(scene);

	// convert the material list IDs to descriptor array IDs
	std::vector<int> materialToTexture(textureNames.size());

	for (size_t i = 0; i < textureNames.size(); i++)
	{
		if (textureNames[i].empty())
			materialToTexture[i] = 0;
		else
		{
			std::string fileLoc = (_objectData.texturePath + textureNames[i]);
			materialToTexture[i] = seRenderer->GetLevelRenderer()->CreateTexture(fileLoc);

		}
	}

	// Load in all the meshes
	std::vector<Mesh> modelMeshes = MeshModel::LoadNode(physicalDevice, logicalDevice, transferQueue, transferCommandPool, scene->mRootNode, scene, materialToTexture);
	
	MeshModel* meshModel = new MeshModel(modelMeshes);
	
	seObjectManager->CreateGameObject(_objectData, nullptr, meshModel);


	//GameObject* object = new GameObject();
	//object->objectMeshModel = meshModel;
	//object->SetObjectData(inObject);
	//object->SetUseTexture(1);
	//object->SetObjectID(0);
	//object->SetObjectParentID(-1);
	//object->SetModel(inObject.objectMatrix);
	//game->gameObjects.push_back(object);
}

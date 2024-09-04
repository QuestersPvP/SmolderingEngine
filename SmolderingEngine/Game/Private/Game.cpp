#include "Game/Public/Game.h"

// Engine includes
#include "Engine/Public/Rendering/Renderer.h"

void Game::LoadMeshes(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool)
{
	std::ifstream file(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/level.selevel");
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/level.selevel" << std::endl;
		return;
	}
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::string texture;
	int useTexture = 0;
	int objectID = -1;
	int parentID = -1;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		vertices.clear();
		indices.clear();
		texture.clear();

		useTexture = 0;
		objectID = -1;
		parentID = -1;


		if (line[0] == '@') // Determine what is being loaded
		{
			std::string keyword = line.substr(1);
			if (keyword == "objectid")
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
						objectID = std::stoi(line); // Parse the object ID
					}
				}
			}
			if (keyword == "parentid")
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
						parentID = std::stoi(line); // Parse the parent ID
					}
				}
			}
			if (keyword == "vertex")
			{
				// Load vertices
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
						std::replace(line.begin(), line.end(), '\t', ' '); // Replace tabs with spaces

						std::istringstream iss(line);
						std::string segment;
						Vertex vertex;

						// Parse position
						std::getline(iss, segment, '%');
						std::replace(segment.begin(), segment.end(), ',', ' '); // Replace commas with spaces
						std::istringstream posStream(segment);
						posStream >> vertex.position.x >> vertex.position.y >> vertex.position.z;

						// Parse color
						std::getline(iss, segment, '%');
						std::replace(segment.begin(), segment.end(), ',', ' '); // Replace commas with spaces
						std::istringstream colorStream(segment);
						colorStream >> vertex.color.r >> vertex.color.g >> vertex.color.b;

						// Parse texture coordinates
						std::getline(iss, segment, '%');
						std::replace(segment.begin(), segment.end(), ',', ' '); // Replace commas with spaces
						std::istringstream texStream(segment);
						texStream >> vertex.texture.x >> vertex.texture.y;

						vertices.push_back(vertex);
					}
					else
						std::cout << "Strange line encountered while reading in file" + line + "\n";
				}
			}
			if (keyword == "index")
			{
				// Load indices
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

						std::istringstream iss(line);
						uint32_t index;
						while (iss >> index)
						{
							indices.push_back(index);
							if (iss.peek() == ',') iss.ignore(); // Handle commas
						}
					}
					else
						std::cout << "Strange line encountered while reading in file" + line + "\n";
				}
			}
			if (keyword == "useTexture")
			{
				//TODO: FIX! TURNS INTO JUNK VALUE
				// Load useTexture flag
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
						//line = line.substr(1); // Remove '~'
						//useTexture = std::stoi(line);
					}
					else
						std::cout << "Strange line encountered while reading in file" + line + "\n";
				}
			}
			if (keyword == "texture")
			{
				// Load texture filename
				while (std::getline(file, line))
				{
					if (line.empty())
						continue;
					else if (line[0] == '-')
					{
						// Create the mesh
						GameObject* loadedGameObject = new GameObject();

						Mesh& tempMesh = Mesh(inPhysicalDevice, inLogicalDevice, inTransferQueue, inTransferCommandPool, &vertices, &indices);
						tempMesh.SetTextureFilePath(texture);

						loadedGameObject->objectMesh = tempMesh;
						loadedGameObject->SetUseTexture(useTexture);
						loadedGameObject->SetObjectID(objectID);
						loadedGameObject->SetObjectParentID(parentID);

						// TODO: This is unsafe, adjust this asap
						if (parentID >= 0)
							gameObjects[parentID]->AddChildObject(loadedGameObject);

						gameObjects.push_back(loadedGameObject);
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
						texture = line;
						texture.erase(texture.find_last_not_of(" \n\r\t") + 1); // Trim any trailing whitespace
					}
					else
						std::cout << "Strange line encountered while reading in file: " + line + "\n";
				}
			}
		}
	}

	file.close();
}

void Game::LoadMeshModel(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
	std::string inModelFile, Renderer* inRenderer)
{
	// Import model scene
	Assimp::Importer importer;

													// After model included, make sure all faces are triangulated
													// Also make sure UVs match our UV system, and finally try to remove any duplicate verticies
	const aiScene* scene = importer.ReadFile(inModelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (!scene)
		throw std::runtime_error("failed to load the model passed in: " + inModelFile);

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
			std::string fileLoc = (std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Models/House/" + textureNames[i]);
			materialToTexture[i] = inRenderer->CreateTexture(fileLoc);
		}
	}

	// Load in all the meshes
	std::vector<Mesh> modelMeshes = MeshModel::LoadNode(inPhysicalDevice, inLogicalDevice, inTransferQueue, inTransferCommandPool, scene->mRootNode, scene, materialToTexture);

	MeshModel meshModel = MeshModel(modelMeshes);
	GameObject* object = new GameObject();
	object->objectMeshModel = meshModel;
	object->SetUseTexture(1);
	object->SetObjectID(0);
	object->SetObjectParentID(-1);
	gameObjects.push_back(object);
}

void Game::DestroyMeshes()
{
	// TODO: FIX
	//for (int i = gameObjects.size()-1; i >= 0; i--)
	//{
	//	gameObjects[i]->objectMesh.DestroyMesh();
	//	delete gameObjects[i];
	//}
}

void Game::SubscribeObjectsToCollisionManager(CollisionManager* inManager, int inParent)
{
	for (GameObject* object : gameObjects)
	{
		if (object->GetParentObjectID() == inParent && object->GetObjectID() == 11)
			inManager->SubscribeObjectToCollisionManager(object, CollisionTypes::MovableCollision);
		else if (object->GetParentObjectID() == inParent)
			inManager->SubscribeObjectToCollisionManager(object, CollisionTypes::StaticCollision);
	}
}

void Game::UnsubscribeObjectsFromCollisionManager(CollisionManager* inManager)
{
	for (GameObject* object : gameObjects)
			inManager->UnsubscribeObjectFromCollisionManager(object);

}

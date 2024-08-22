#include "Game/Public/Game.h"

void Game::LoadMeshes(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, VkQueue InTransferQueue, VkCommandPool InTransferCommandPool)
{
	std::ifstream file(std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/level.selevel");
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << std::string(PROJECT_SOURCE_DIR) + "/SmolderingEngine/Game/Levels/level.selevel" << std::endl;
		return;
	}
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	//int useTexture;
	std::string texture;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		vertices.clear();
		indices.clear();
		//useTexture = 0;
		texture.clear();

		if (line[0] == '@') // Determine what is being loaded
		{
			std::string keyword = line.substr(1);
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
						Mesh& tempMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &vertices, &indices);
						tempMesh.SetTextureFilePath(texture);

						GameMeshes.push_back(tempMesh);
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

void Game::DestroyMeshes()
{
	for (size_t i = 0; i < GameMeshes.size(); i++)
		GameMeshes[i].DestroyMesh();
}
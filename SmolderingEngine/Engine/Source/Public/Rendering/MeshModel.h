#pragma once

// Standard Library
#include <vector>

// Third Party
#include <GLM/glm.hpp>

#include <assimp/scene.h>

// Engine
#include "Engine/Source/Public/Rendering/Mesh.h"

class MeshModel
{
	/* Variables */
public:
	std::vector<Mesh> meshList;
	//glm::mat4 model;

	/* Functions */
public:
	MeshModel();
	MeshModel(std::vector<Mesh> inMeshList);
	void DestroyMeshModel();

	static std::vector<std::string> LoadMaterials(const aiScene* inScene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
		aiNode* inNode, const aiScene* inScene, std::vector<int> inMaterialToTexture);
	static Mesh LoadMesh(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool,
		aiMesh* inMesh, const aiScene* inScene, std::vector<int> inMaterialToTexture);

	size_t GetMeshCount();
	Mesh* GetMesh(size_t inIndex);


};
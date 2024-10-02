#include "Engine/Source/Public/Rendering/MeshModel.h"

MeshModel::MeshModel()
{
}

MeshModel::MeshModel(std::vector<Mesh> inMeshList)
{
	meshList = inMeshList;
}

void MeshModel::DestroyMeshModel()
{
	for (auto& mesh : meshList)
		mesh.DestroyMesh();
}

std::vector<std::string> MeshModel::LoadMaterials(const aiScene* inScene)
{
	std::vector<std::string> textureList(inScene->mNumMaterials);

	// copy each materials texture file name
	for (size_t i = 0; i < inScene->mNumMaterials; i++)
	{
		aiMaterial* material = inScene->mMaterials[i];
		textureList[i] = "";

		// TODO: SUPPORT MORE MATERIAL TYPES (OPACITY, AMBIENT, HEIGHT, NORMAL, ETC.)
		// First check if this material has a diffuse texture
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			// Then get path of the diffuse texture
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				// try to cut off extra info. E.g. S:/users/temp/blah/texture.jpg -> texture.jpg
				int index = std::string(path.data).rfind("\\");
				std::string fileName = std::string(path.data).substr(index + 1);

				textureList[i] = fileName;
			}
		}
	}

	return textureList;
}

std::vector<Mesh> MeshModel::LoadNode(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool, aiNode* inNode, const aiScene* inScene, std::vector<int> inMaterialToTexture)
{
	std::vector<Mesh> meshList;

	for (size_t i = 0; i < inNode->mNumMeshes; i++)
	{
		meshList.push_back(LoadMesh(inPhysicalDevice, inLogicalDevice, inTransferQueue, inTransferCommandPool, inScene->mMeshes[inNode->mMeshes[i]], inScene, inMaterialToTexture));
	}

	// Go through every child node and add the meshes to the meshList
	for (size_t i = 0; i < inNode->mNumChildren; i++)
	{
		std::vector<Mesh> newList = LoadNode(inPhysicalDevice, inLogicalDevice, inTransferQueue, inTransferCommandPool, inNode->mChildren[i], inScene, inMaterialToTexture);
		meshList.insert(meshList.end(), newList.begin(), newList.end());
	}

	return meshList;
}

Mesh MeshModel::LoadMesh(VkPhysicalDevice inPhysicalDevice, VkDevice inLogicalDevice, VkQueue inTransferQueue, VkCommandPool inTransferCommandPool, aiMesh* inMesh, const aiScene* inScene, std::vector<int> inMaterialToTexture)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices.resize(inMesh->mNumVertices);
	for (size_t i = 0; i < inMesh->mNumVertices; i++)
	{
		vertices[i].position = { inMesh->mVertices[i].x,inMesh->mVertices[i].y, inMesh->mVertices[i].z };

		// Set default color to white
		// TODO: Maybe change this?
		vertices[i].color = { 1.0f, 1.0f, 1.0f };

		// get first set of texture coordinates if they exist
		if (inMesh->mTextureCoords[0])
			vertices[i].texture = { inMesh->mTextureCoords[0][i].x,inMesh->mTextureCoords[0][i].y};
		else
			vertices[i].texture = { 0.0f, 0.0f };
	}

	// This is faces (e.g. trangles)
	for (size_t i = 0; i < inMesh->mNumFaces; i++)
	{
		// Get the face and then get the indices out of it
		aiFace face = inMesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// TODO: edit constructor to fix this
	Mesh newMesh = Mesh(inPhysicalDevice, inLogicalDevice, inTransferQueue, inTransferCommandPool, &vertices, &indices);
	newMesh.SetTextureID(inMaterialToTexture[inMesh->mMaterialIndex]);

	return newMesh;
}

size_t MeshModel::GetMeshCount()
{
	return meshList.size();
}

Mesh* MeshModel::GetMesh(size_t inIndex)
{
	if (inIndex >= meshList.size())
		throw std::runtime_error("Attempted to acces invalid Mesh Index!");

	return &meshList[inIndex];
}

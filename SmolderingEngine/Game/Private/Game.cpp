#include "Game/Public/Game.h"

void Game::LoadMeshes(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice)
{
	std::vector<Vertex> MeshVerticies =
	{
		{{0.0, -0.4, 0.0 }, {1.0, 0.0, 0.0}},
		{{0.4, 0.4, 0.0}, {0.0, 1.0, 0.0}},
		{{-0.4, 0.4, 0.0}, {0.0, 0.0, 1.0}}
	};
	MeshOne = Mesh(InPhysicalDevice, InLogicalDevice, &MeshVerticies);
}

void Game::DestroyMeshes()
{
	MeshOne.DestroyMesh();
}

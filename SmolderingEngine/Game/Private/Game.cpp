#include "Game/Public/Game.h"

void Game::LoadMeshes(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, VkQueue InTransferQueue, VkCommandPool InTransferCommandPool)
{
	// Vertex data
	std::vector<Vertex> PlayerMeshVerticies =
	{
		{{-2.6f, -1.9f, 0.0f}, {1.0f, 0.0f, 0.0f}},			// vertex 0, color 0
		{{-2.4f, -1.9f, 0.0f}, {0.0f, 1.0f, 0.0f}},		// vertex 1, color 1
		{{-2.4f, -1.7f, 0.0f}, {0.0f, 0.0f, 1.0f}},		// etc...
		{{-2.6f, -1.7f, 0.0f}, {1.0f, 1.0f, 0.0f}}
	};	
	
	std::vector<Vertex> FloorMeshVerticies =
	{
		{{-3.0, -2.5, 0.0 },	{0.4f, 0.2f, 0.0f}},
		{{3.0, -2.5, 0.0},		{0.4f, 0.2f, 0.0f}},
		{{3.0, -1.9, 0.0},		{0.4f, 0.2f, 0.0f}},
		{{-3.0, -1.9, 0.0},		{0.4f, 0.2f, 0.0f}}
	};

	std::vector<Vertex> WinMeshVerticies =
	{
		{{2.6, -1.9, 0.0 }, {0, 1.0, 0.0}},
		{{3.0, -1.9, 0.0}, {0, 1.0, 0.0}},
		{{3.0, 3.0, 0.0}, {0, 1.0, 0.0}},
		{{2.6, 3.0, 0.0}, {0, 1.0, 0.0}}
	};

	std::vector<Vertex> TrapMeshOneVertices =
	{
		// Base square (half the width)
		{{0.9f, -1.9f, 0.0f}, {0.1f, 0.1f, 0.1f}},  // Bottom-left
		{{1.5f, -1.9f, 0.0f}, {0.1f, 0.1f, 0.1f}},   // Bottom-right
		{{1.5f, -1.8f, 0.0f}, {0.1f, 0.1f, 0.1f}},   // Top-right
		{{0.9f, -1.8f, 0.0f}, {0.1f, 0.1f, 0.1f}},  // Top-left

		// First spike
		{{0.9f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left of first spike
		{{1.1f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-right of first spike
		{{1.0f, -1.6f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Top of first spike

		// Second spike
		{{1.1f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left of second spike
		{{1.3f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Bottom-right of second spike
		{{1.2f, -1.6f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Top of second spike

		// Third spike
		{{1.3f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Bottom-left of third spike
		{{1.5f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Bottom-right of third spike
		{{1.4f, -1.6f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Top of third spike
	};

	std::vector<Vertex> TrapMeshTwoVertices =
	{
		// Base square (half the width)
		{{0.9f - 2.0f, -1.9f, 0.0f}, {0.1f, 0.1f, 0.1f}},  // Bottom-left
		{{1.5f - 2.0f, -1.9f, 0.0f}, {0.1f, 0.1f, 0.1f}},   // Bottom-right
		{{1.5f - 2.0f, -1.8f, 0.0f}, {0.1f, 0.1f, 0.1f}},   // Top-right
		{{0.9f - 2.0f, -1.8f, 0.0f}, {0.1f, 0.1f, 0.1f}},  // Top-left

		// First spike
		{{0.9f - 2.0f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left of first spike
		{{1.1f - 2.0f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-right of first spike
		{{1.0f - 2.0f, -1.6f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Top of first spike

		// Second spike
		{{1.1f - 2.0f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left of second spike
		{{1.3f - 2.0f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Bottom-right of second spike
		{{1.2f - 2.0f, -1.6f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Top of second spike

		// Third spike
		{{1.3f - 2.0f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Bottom-left of third spike
		{{1.5f - 2.0f, -1.8f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Bottom-right of third spike
		{{1.4f - 2.0f, -1.6f, 0.0f}, {1.0f, 0.0f, 0.0f}},   // Top of third spike
	};

	// Index Data
	std::vector<uint32_t> SquareMeshIndicies = // (Can work for any square or rectangle)
	{
		0,1,2,	// triangle 0
		2,3,0	// triangle 1
	};

	std::vector<uint32_t> TrapMeshIndices =
	{
		// Base rectangle
		0, 1, 2,
		2, 3, 0,

		// Left triangle
		4, 5, 6,

		// Middle triangle
		7, 8, 9,

		// Right triangle
		10, 11, 12
	};

	Mesh FloorMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &FloorMeshVerticies, &SquareMeshIndicies);
	GameMeshes.push_back(FloorMesh);
	
	Mesh TrapMeshOne = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &TrapMeshOneVertices, &TrapMeshIndices);
	Mesh TrapMeshTwo = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &TrapMeshTwoVertices, &TrapMeshIndices);
	GameMeshes.push_back(TrapMeshOne);
	GameMeshes.push_back(TrapMeshTwo);

	Mesh WinMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &WinMeshVerticies, &SquareMeshIndicies);
	GameMeshes.push_back(WinMesh);

	Mesh PlayerMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &PlayerMeshVerticies, &SquareMeshIndicies);
	GameMeshes.push_back(PlayerMesh);
}

void Game::DestroyMeshes()
{
	for (size_t i = 0; i < GameMeshes.size(); i++)
		GameMeshes[i].DestroyMesh();
}
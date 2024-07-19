#include "Game/Public/Game.h"

void Game::LoadMeshes(VkPhysicalDevice InPhysicalDevice, VkDevice InLogicalDevice, VkQueue InTransferQueue, VkCommandPool InTransferCommandPool)
{
	// Vertex data
	std::vector<Vertex> PlayerMeshVerticies =
	{
		{{-0.85, 0.8, 0.0 }, {1.0, 0.0, 0.0}},	// vertex 0
		{{-0.85, 0.9, 0.0}, {0.0, 1.0, 0.0}},		// vertex 1
		{{-0.95, 0.9, 0.0}, {0.0, 0.0, 1.0}},	// etc...
		{{-0.95, 0.8, 0.0}, {1.0, 1.0, 0.0}}
	};	
	
	std::vector<Vertex> FloorMeshVerticies =
	{
		{{1.0, 0.9, 0.0 }, {.60, 0.35, 0.2}},
		{{1.0, 1.0, 0.0}, {.60, 0.35, 0.2}},
		{{-1.0, 1.0, 0.0}, {.60, 0.35, 0.2}},
		{{-1.0, 0.9, 0.0}, {.60, 0.35, 0.2}}
	};

	std::vector<Vertex> WinMeshVerticies =
	{
		{{1.0, -1.0, 0.0 }, {0, 1.0, 0.0}},
		{{1.0, 0.9, 0.0}, {0, 1.0, 0.0}},
		{{0.95, 0.9, 0.0}, {0, 1.0, 0.0}},
		{{0.95, -1.0, 0.0}, {0, 1.0, 0.0}}
	};

	std::vector<Vertex> TrapMeshOneVertices =
	{
		// Base rectangle
		{{0.175f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 0: Top-left
		{{0.425f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 1: Top-right
		{{0.425f, 0.9f, 0.0f}, {1.0, 0.0, 0.0}},   // 2: Bottom-right
		{{0.175f, 0.9f, 0.0f}, {1.0, 0.0, 0.0}},   // 3: Bottom-left

		// Left triangle
		{{0.2167f, 0.78f, 0.0f}, {1.0, 0.0, 0.0}}, // 4: Top
		{{0.2583f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}}, // 5: Base right
		{{0.175f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 6: Base left

		// Middle triangle
		{{0.3f, 0.78f, 0.0f}, {1.0, 0.0, 0.0}},     // 7: Top
		{{0.3417f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 8: Base right
		{{0.2583f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 9: Base left

		// Right triangle
		{{0.3833f, 0.78f, 0.0f}, {1.0, 0.0, 0.0}},  // 10: Top
		{{0.425f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},   // 11: Base right
		{{0.3417f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}}   // 12: Base left
	};

	std::vector<Vertex> TrapMeshTwoVertices =
	{
		// Base rectangle
		{{-0.525f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 0: Top-left
		{{-0.275f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 1: Top-right
		{{-0.275f, 0.9f, 0.0f}, {1.0, 0.0, 0.0}},   // 2: Bottom-right
		{{-0.525f, 0.9f, 0.0f}, {1.0, 0.0, 0.0}},   // 3: Bottom-left

		// Left triangle
		{{-0.4833f, 0.78f, 0.0f}, {1.0, 0.0, 0.0}}, // 4: Top
		{{-0.4417f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}}, // 5: Base right
		{{-0.525f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 6: Base left

		// Middle triangle
		{{-0.4f, 0.78f, 0.0f}, {1.0, 0.0, 0.0}},     // 7: Top
		{{-0.3583f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 8: Base right
		{{-0.4417f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},  // 9: Base left

		// Right triangle
		{{-0.3167f, 0.78f, 0.0f}, {1.0, 0.0, 0.0}},  // 10: Top
		{{-0.275f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}},   // 11: Base right
		{{-0.3583f, 0.85f, 0.0f}, {1.0, 0.0, 0.0}}   // 12: Base left
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

	Mesh PlayerMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &PlayerMeshVerticies, &SquareMeshIndicies);
	Mesh FloorMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &FloorMeshVerticies, &SquareMeshIndicies);
	Mesh WinMesh = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &WinMeshVerticies, &SquareMeshIndicies);
	Mesh TrapMeshOne = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &TrapMeshOneVertices, &TrapMeshIndices);
	Mesh TrapMeshTwo = Mesh(InPhysicalDevice, InLogicalDevice, InTransferQueue, InTransferCommandPool, &TrapMeshTwoVertices, &TrapMeshIndices);

	GameMeshes.push_back(PlayerMesh);
	GameMeshes.push_back(FloorMesh);
	GameMeshes.push_back(WinMesh);
	GameMeshes.push_back(TrapMeshOne);
	GameMeshes.push_back(TrapMeshTwo);
}

void Game::DestroyMeshes()
{
	for (size_t i = 0; i < GameMeshes.size(); i++)
		GameMeshes[i].DestroyMesh();
}

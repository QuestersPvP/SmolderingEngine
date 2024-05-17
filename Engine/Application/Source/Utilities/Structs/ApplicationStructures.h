#pragma once

#include "../Includes/ApplicationIncludes.h"
#include "../../VulkanTexture.h"


/* Window */

struct WindowParameters
{
    HINSTANCE           HInstance;
    HWND                HWnd;
};

/* Rendering */

struct SynchronizationSemaphores
{
    // Swap chain image presentation
    VkSemaphore presentComplete;
    // Command buffer submission and execution
    VkSemaphore renderComplete;
};

struct QueueFamilyIndices
{
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
};

struct DepthStencil
{
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct SwapChainBuffer
{
    VkImage         image;
    VkImageView     view;
};

// Same uniform buffer layout as shader
struct UBOVS
{
    glm::mat4 projection;
    glm::mat4 modelView;
    glm::vec4 lightPos = glm::vec4(0.0f, 2.0f, 1.0f, 0.0f);
};

/* Loading GLTF Models */

struct ShaderData
{
	//vks::Buffer buffer;
	struct Values 
	{
		glm::mat4 projection;
		glm::mat4 model;
		glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, -5.0f, 1.0f);
		glm::vec4 viewPos;
	} values;
};

// The vertex layout of model
struct SEVertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 color;
};

struct SEDescriptorSetLayouts
{
	VkDescriptorSetLayout matrices;
	VkDescriptorSetLayout textures;
};

// Single vertex buffer for all primitives
struct SEVertices
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

// Single index buffer for all primitives
struct SEIndices
{
	int count;
	VkBuffer buffer;
	VkDeviceMemory memory;
};

// A primitive contains the data for a single draw call
struct SEPrimitive
{
	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t materialIndex;
};

// Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
struct SEMesh
{
	std::vector<SEPrimitive> primitives;
};

// A node represents an object in the glTF scene graph
struct SENode
{
	SENode* parent;
	std::vector<SENode*> children;
	SEMesh mesh;
	glm::mat4 matrix;
	~SENode() {
		for (auto& child : children) {
			delete child;
		}
	}
};

// A glTF material stores information in e.g. the texture that is attached to it and colors
struct SEMaterial
{
	glm::vec4 baseColorFactor = glm::vec4(1.0f);
	uint32_t baseColorTextureIndex;
};

// Contains the texture and a descriptor se used to access the this texture from the fragment shader for a single glTF image
// Images may be reused by texture objects and are as such separated
struct SEImage
{
	Texture2D texture;
	VkDescriptorSet descriptorSet;
};

// A glTF texture stores a reference to the image and a sampler
struct SETexture
{
	int32_t imageIndex;
};

/* Camera */

struct CameraMatrices
{
    glm::mat4 perspective;
    glm::mat4 view;
};
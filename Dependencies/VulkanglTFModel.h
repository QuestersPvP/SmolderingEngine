/*
* -----------------------------------------------------------------------
* Modified version of Sascha Willems class, please see information below!
* -----------------------------------------------------------------------
* 
* Vulkan glTF model and texture loading class based on tinyglTF (https://github.com/syoyo/tinygltf)
* Copyright (C) 2018-2023 by Sascha Willems - www.saschawillems.de
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include "vulkan/vulkan.h"
#include "Rendering/RenderPass/RenderPass.h"
#include "Utilities/Structs/ApplicationStructures.h"
//#include "VulkanDevice.h"

#include <ktx.h>
#include <ktxvulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif
#include "../Dependencies/tinygltf/tiny_gltf.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

namespace vkglTF
{
	VkResult createBuffer(VkDevice logicalDevice, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties& memoryProperties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);


	enum DescriptorBindingFlags {
		ImageBaseColor = 0x00000001,
		ImageNormalMap = 0x00000002
	};

	extern VkDescriptorSetLayout descriptorSetLayoutImage;
	extern VkDescriptorSetLayout descriptorSetLayoutUbo;
	extern VkMemoryPropertyFlags memoryPropertyFlags;
	extern uint32_t descriptorBindingFlags;

	struct Node;

	/*
		glTF texture loading class
	*/
	struct Texture {
		//vks::VulkanDevice* device = nullptr;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;
		uint32_t index;
		void updateDescriptor();
		void destroy();
		void fromglTfImage(tinygltf::Image& gltfimage, std::string path,/* vks::VulkanDevice* device,*/ VkQueue copyQueue);
	};

	/*
		glTF material class
	*/
	struct Material {
		//vks::VulkanDevice* device = nullptr;
		enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
		AlphaMode alphaMode = ALPHAMODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		vkglTF::Texture* baseColorTexture = nullptr;
		vkglTF::Texture* metallicRoughnessTexture = nullptr;
		vkglTF::Texture* normalTexture = nullptr;
		vkglTF::Texture* occlusionTexture = nullptr;
		vkglTF::Texture* emissiveTexture = nullptr;

		vkglTF::Texture* specularGlossinessTexture;
		vkglTF::Texture* diffuseTexture;

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		//Material(vks::VulkanDevice* device) : device(device) {};
		Material() {};
		void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t descriptorBindingFlags);
	};

	/*
		glTF primitive
	*/
	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t firstVertex;
		uint32_t vertexCount;
		Material& material;

		struct Dimensions {
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
			glm::vec3 size;
			glm::vec3 center;
			float radius;
		} dimensions;

		void setDimensions(glm::vec3 min, glm::vec3 max);
		Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material) : firstIndex(firstIndex), indexCount(indexCount), material(material) {};
	};

	/*
		glTF mesh
	*/
	struct Mesh {
		//vks::VulkanDevice* device;
		VkDevice* logicalDevice;


		std::vector<Primitive*> primitives;
		std::string name;

		struct UniformBuffer {
			VkBuffer buffer;
			VkDeviceMemory memory;
			VkDescriptorBufferInfo descriptor;
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			void* mapped;
		} uniformBuffer;

		struct UniformBlock {
			glm::mat4 matrix;
			glm::mat4 jointMatrix[64]{};
			float jointcount{ 0 };
		} uniformBlock;

		//Mesh(vks::VulkanDevice* device, glm::mat4 matrix);
		Mesh(VkDevice& logicalDevice, VkPhysicalDeviceMemoryProperties& memoryProperties, glm::mat4 matrix);
		~Mesh();
	};

	/*
		glTF skin
	*/
	struct Skin {
		std::string name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node*> joints;
	};

	/*
		glTF node
	*/
	struct Node {
		Node* parent;
		uint32_t index;
		std::vector<Node*> children;
		glm::mat4 matrix;
		std::string name;
		Mesh* mesh;
		Skin* skin;
		int32_t skinIndex = -1;
		glm::vec3 translation{};
		glm::vec3 scale{ 1.0f };
		glm::quat rotation{};
		glm::mat4 localMatrix();
		glm::mat4 getMatrix();
		void update();
		~Node();
	};

	/*
		glTF animation channel
	*/
	struct AnimationChannel {
		enum PathType { TRANSLATION, ROTATION, SCALE };
		PathType path;
		Node* node;
		uint32_t samplerIndex;
	};

	/*
		glTF animation sampler
	*/
	struct AnimationSampler {
		enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	/*
		glTF animation
	*/
	struct Animation {
		std::string name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	/*
		glTF default vertex layout with easy Vulkan mapping functions
	*/
	enum class VertexComponent { Position, Normal, UV, Color, Tangent, Joint0, Weight0 };

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 color;
		//glm::vec4 joint0;
		//glm::vec4 weight0;
		//glm::vec4 tangent;
		//static VkVertexInputBindingDescription vertexInputBindingDescription;
		//static std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		//static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
		//static VkVertexInputBindingDescription inputBindingDescription(uint32_t binding);
		//static VkVertexInputAttributeDescription inputAttributeDescription(uint32_t binding, uint32_t location, VertexComponent component);
		//static std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions(uint32_t binding, const std::vector<VertexComponent> components);
		///** @brief Returns the default pipeline vertex input state create info structure for the requested vertex components */
		//static VkPipelineVertexInputStateCreateInfo* getPipelineVertexInputState(const std::vector<VertexComponent> components);
	};

	enum FileLoadingFlags {
		None = 0x00000000,
		PreTransformVertices = 0x00000001,
		PreMultiplyVertexColors = 0x00000002,
		FlipY = 0x00000004,
		DontLoadImages = 0x00000008
	};

	enum RenderFlags {
		BindImages = 0x00000001,
		RenderOpaqueNodes = 0x00000002,
		RenderAlphaMaskedNodes = 0x00000004,
		RenderAlphaBlendedNodes = 0x00000008
	};

	/*
		glTF model loading and rendering class
	*/
	class Model {
	private:
		vkglTF::Texture* getTexture(uint32_t index);
		vkglTF::Texture emptyTexture;
		void createEmptyTexture(VkQueue transferQueue);
	public:
		//vks::VulkanDevice* device;
		VkDescriptorPool descriptorPool;

		struct Vertices {
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertices;
		struct Indices {
			int count;
			VkBuffer buffer;
			VkDeviceMemory memory;
		} indices;

		std::vector<SENode*> nodes;
		//std::vector<Node*> nodes;
		std::vector<Node*> linearNodes;

		std::vector<Skin*> skins;

		std::vector<Texture> textures;
		std::vector<Material> materials;
		std::vector<Animation> animations;

		struct Dimensions {
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
			glm::vec3 size;
			glm::vec3 center;
			float radius;
		} dimensions;

		bool metallicRoughnessWorkflow = true;
		bool buffersBound = false;
		std::string path;

		Model() {};
		//~Model();
		//void loadNode(VkDevice& logicalDevice, VkPhysicalDeviceMemoryProperties& memoryProperties, vkglTF::Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale);
		void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, SENode* parent, std::vector<uint32_t>& indexBuffer, std::vector<SEVertex>& vertexBuffer);
		//void loadSkins(tinygltf::Model& gltfModel);
		void LoadImages(tinygltf::Model& gltfModel, std::vector<SEImage>& InImages, VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice,
			VkCommandPool& commandPool, VkPhysicalDeviceMemoryProperties& memoryProperties, VkQueue& copyQueue);
		void LoadTextures(tinygltf::Model& input, std::vector<SETexture>& textures);
		void LoadMaterials(tinygltf::Model& gltfModel, std::vector<SEMaterial>& materials);
		//void loadAnimations(tinygltf::Model& gltfModel);
		void loadFromFile(std::string filename, VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice, VkCommandPool& commandPool, VkQueue& transferQueue,
		VkQueue& copyQueue, VkPhysicalDeviceMemoryProperties& memoryProperties, std::vector<SEImage>& images, std::vector<SETexture>& textures, std::vector<SEMaterial>& materials,
		SEVertices& vertices, SEIndices& indices, uint32_t fileLoadingFlags = vkglTF::FileLoadingFlags::None, float scale = 1.0f);
		void bindBuffers(VkCommandBuffer& commandBuffer);
		//void drawNode(Node* node, VkCommandBuffer& commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		//void draw(VkCommandBuffer& commandBuffer, uint32_t renderFlags = 0, VkPipelineLayout pipelineLayout = VK_NULL_HANDLE, uint32_t bindImageSet = 1);
		void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, SENode* node, std::vector<SETexture> textures,
			std::vector<SEMaterial> materials, std::vector<SEImage>& images);
		void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, SEVertices& verts, SEIndices& indies, std::vector<SETexture>& textures,
			std::vector<SEMaterial>& materials, std::vector<SEImage>& images);
		void getNodeDimensions(Node* node, glm::vec3& min, glm::vec3& max);
		void getSceneDimensions();
		//void updateAnimation(uint32_t index, float time);
		//Node* findNode(Node* parent, uint32_t index);
		//Node* nodeFromIndex(uint32_t index);
		void prepareNodeDescriptor(vkglTF::Node* node, VkDescriptorSetLayout descriptorSetLayout, VkDevice logicalDevice);
	};
}
#include "Engine/Source/Public/Rendering/EngineGUIRenderer.h"

// Project Includes
#include "Engine/Source/EngineManager.h"
#include "Engine/Source/Public/EngineLevel/EngineLevelManager.h"
#include "Engine/Source/Public/Object/ObjectManager.h"

#include "Engine/Source/Public/Rendering/Renderer.h"

EngineGUIRenderer::EngineGUIRenderer(const VulkanResources* _resources)
	: vulkanResources(_resources)
{
	if (InitImGUI() == EXIT_FAILURE)
		std::cout << "EngineGUIRenderer::InitImGUI() -> Failed to initialize ImGUI!" << std::endl;
}

void EngineGUIRenderer::DestroyEngineGUIRenderer()
{
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(vulkanResources->logicalDevice, imguiDescriptorPool, nullptr);
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	delete(this);
}

void EngineGUIRenderer::RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex)
{
	if (seEngineManager == nullptr)
		seEngineManager = EngineManager::GetEngineManager();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)vulkanResources->swapchainExtent.width, (float)vulkanResources->swapchainExtent.height);

	// Insert your ImGui code here

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save Level"))
			{
				shouldSaveLevel = true;
			}
			if (ImGui::MenuItem("Load Level"))
			{
				shouldLoadLevel = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


	ImGui::Begin("Edit Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// Input field for the object ID
	ImGui::Text("Object ID");
	ImGui::InputInt("ID", &currentSelectedModelID);

	if (currentSelectedModelID < seEngineManager->GetEngineLevelManager()->GetObjectManager()->GetGameObjects().size())
	{
		glm::mat4 modelMat = seEngineManager->GetEngineLevelManager()->GetObjectManager()->GetGameObjects()[currentSelectedModelID]->GetModel().modelMatrix;

		float position[3] = { modelMat[3].x, modelMat[3].y, modelMat[3].z };
		// Input fields for the position
		ImGui::Text("Object Position");
		ImGui::InputFloat3("##Position", position);
	
		if (modelMat[3].x != position[0] || modelMat[3].y != position[1] || modelMat[3].z != position[2])
		{
			modelMat[3].x = position[0];
			modelMat[3].y = position[1];
			modelMat[3].z = position[2];
	
			seEngineManager->GetEngineLevelManager()->GetObjectManager()->GetGameObjects()[currentSelectedModelID]->SetModel(modelMat);
		}
	}

	ImGui::End();

	// Render ImGui's draw data into the command buffer
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffer);
}

void EngineGUIRenderer::ProcessEngineGUIInputs()
{
	if (seEngineManager == nullptr)
		seEngineManager = EngineManager::GetEngineManager();

	if (shouldLoadLevel)
	{
		seEngineManager->GetEngineLevelManager()->LoadNewScene();
		shouldLoadLevel = false;
	}
	if (shouldSaveLevel)
	{
		// get where they want to save and replace all the \\ with / so it works in file explorer
		std::string fileName = seEngineManager->GetEngineLevelManager()->SaveFileExplorer();
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
	
		seEngineManager->GetEngineLevelManager()->SaveLevel(fileName);
		shouldSaveLevel = false;
	}
}

bool EngineGUIRenderer::InitImGUI()
{
	// Create a descriptor pool for ImGui
	VkDescriptorPoolSize poolSizes[] = 
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // Allows freeing individual descriptor sets
	poolInfo.maxSets = 1000 * static_cast<uint32_t>(std::size(poolSizes)); // Number of descriptor sets
	poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
	poolInfo.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(vulkanResources->logicalDevice, &poolInfo, nullptr, &imguiDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create ImGui descriptor pool!");

	// Initialize ImGui for Vulkan
	ImGui_ImplVulkan_InitInfo imGuiCreateInfo = {};
	imGuiCreateInfo.Instance = vulkanResources->vulkanInstance;
	imGuiCreateInfo.PhysicalDevice = vulkanResources->physicalDevice;
	imGuiCreateInfo.Device = vulkanResources->logicalDevice;

	//QueueFamilyIndicies indicies = GetQueueFamilies(Devices.PhysicalDevice);
	//imGuiCreateInfo.QueueFamily = indicies.graphicsFamily;

	imGuiCreateInfo.Queue = vulkanResources->graphicsQueue;
	imGuiCreateInfo.PipelineCache = nullptr;
	imGuiCreateInfo.DescriptorPool = imguiDescriptorPool;
	imGuiCreateInfo.Allocator = nullptr;
	imGuiCreateInfo.MinImageCount = static_cast<uint32_t>(vulkanResources->swapchainImages.size());
	imGuiCreateInfo.ImageCount = static_cast<uint32_t>(vulkanResources->swapchainImages.size());
	imGuiCreateInfo.CheckVkResultFn = nullptr;				// TODO: Setup debug callback function for ImGui
	imGuiCreateInfo.RenderPass = vulkanResources->renderPass;

	// check ! because if it succeeds ImGui_ImplVulkan_Init returns 1 and that = EXIT_FAILURE.
	return !ImGui_ImplVulkan_Init(&imGuiCreateInfo);
}

void EngineGUIRenderer::ResultCheck(VkResult _error)
{
	if (_error != VK_SUCCESS)
		std::cout << "imGui error: " << _error << std::endl;
}

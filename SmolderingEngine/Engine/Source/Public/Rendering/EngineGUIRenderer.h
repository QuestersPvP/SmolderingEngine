#pragma once

// Standard Library

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

class EngineGUIRenderer
{
	/* Variables */

private:
	/* Struct that holds general vulkan resources (already created by Renderer) */
	const struct VulkanResources* vulkanResources;

	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;

	// Handle GUI input
	/* What model ID is currently selected */
	int currentSelectedModelID = 0;

private:
	class EngineManager* seEngineManager = nullptr;

	/* Engine GUI bools */
	bool shouldSaveLevel = false;
	bool shouldLoadLevel = false;


	/* Functions */
public:
	EngineGUIRenderer() {};
	EngineGUIRenderer(const VulkanResources* _resources);
	void DestroyEngineGUIRenderer();
	
	// Draw the GUI
	void RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex);

	// Process any GUI commands that may need to be done prior to rendering
	void ProcessEngineGUIInputs();

private:
	bool InitImGUI();

	// Helper Functions
	void ResultCheck(VkResult _error);
};
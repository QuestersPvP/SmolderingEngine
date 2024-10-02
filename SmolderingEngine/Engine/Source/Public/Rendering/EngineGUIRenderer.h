#pragma once

// Standard Library

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// Project includes
#include "Engine/Source/Public/Rendering/Renderer.h"

class EngineGUIRenderer
{
	/* Variables */

private:
	/* Struct that holds general vulkan resources (already created by Renderer) */
	const VulkanResources* vulkanResources;

	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;

	// Handle GUI input
	/* What model ID is currently selected */
	int currentSelectedModelID = 0;

	/* Functions */
public:
	EngineGUIRenderer() {};
	EngineGUIRenderer(const VulkanResources* _resources);
	void DestroyEngineGUIRenderer();

	void RecordToCommandBuffer(VkCommandBuffer _commandBuffer, uint32_t _imageIndex);


private:
	bool InitImGUI();

	// Helper Functions
	void ResultCheck(VkResult _error);
};
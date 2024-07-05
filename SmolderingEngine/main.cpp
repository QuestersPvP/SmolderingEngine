#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	printf("Exension count: %i\n", extensionCount);

	glm::mat4 testMatrix(1.0f);
	glm::vec4 testVector(1.0f);

	auto testResult = testMatrix * testVector;

	while (!glfwWindowShouldClose(window))
	{
		// check for input (e.g. window close etc.)
		glfwPollEvents();
	}

	// destroy glfw windows
	glfwDestroyWindow(window);
	// destroy glfw
	glfwTerminate();

	return 0;
}
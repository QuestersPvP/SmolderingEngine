#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> 

#include "Rendering/Instances/public/VulkanRendering.h"

#define WIDTH 1280
#define HEIGHT 720

int main() 
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "HELLO VULKAN ", nullptr, nullptr); 
 
   VulkanRendering::GetInstance()->InitVulkan(window); 

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

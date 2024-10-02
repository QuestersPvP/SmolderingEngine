#pragma once

// Standard Library
#include <unordered_map>
#include <iostream>

// Third Party
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

class InputManager 
{
    /* Variables */
public:
    GLFWwindow* window = nullptr;
    std::unordered_map<int, bool> keyStates;
    
    float keyboardMovementSpeed = 1.0f;
    float mouseMovementSpeed = 1.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float currentMouseX = 0.0f;
    float currentMouseY = 0.0f;    
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    
    float modelY = 0.0f;
    float modelX = 0.0f;
    float modelZ = 0.0f;

    // TODO: Sort this stuff out
    float deltaTime = 0.0f;
    float lastTime = 0.0f;
    bool playerJumping = false;
    float floorHeight = -2.4f;

    bool mouseModeChanged = false;

    int windowWidth;
    int windowHeight;

    /* Functions */
public:
    InputManager();
    InputManager(const std::string& _windowName = "Smoldering Engine", int _width = 1280, int _height = 720);

    //void InitWindow(const std::string& inWindowName = "Smoldering Engine", int inWidth = 1280, int inHeight = 720);
    void InitKeyStates();
    //void InitRendererCallback(class Renderer* inRenderer);

    void processInput(float inDeltaTime, class Camera* inCamera);

private:
    static void KeyCallback(GLFWwindow* _window, int _key, int _scanCode, int _action, int _mods);
    static void MouseCallback(GLFWwindow* _window, double _xPos, double _yPos);
    
    // Handle resizing of the window
    //static void WindowResizeCallback(GLFWwindow* inWindow, int inWidth, int inHeight);
};
#pragma once

// Third Party
//#include <GLFW/glfw3.h>

// Standard Library
#include <unordered_map>
#include <iostream>

// Engine includes
#include "Engine/Public/Camera/Camera.h"

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

class InputManager 
{
    /* Variables */
public:
    GLFWwindow* window;
    std::unordered_map<int, bool> keyStates;
    
    float keyboardMovementSpeed = 1.00f;
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

    /* Functions */
public:
    InputManager();

    void InitWindow(const std::string& inWindowName = "Smoldering Engine", int inWidth = 1280, int inHeight = 720);
    void initializeKeyStates();
    void processInput(float inDeltaTime, class Camera* inCamera);

private:
    static void KeyCallback(GLFWwindow* inWindow, int inKey, int inScancode, int inAction, int inMods);
    static void MouseCallback(GLFWwindow* inWindow, double inXPos, double inYPos);
};
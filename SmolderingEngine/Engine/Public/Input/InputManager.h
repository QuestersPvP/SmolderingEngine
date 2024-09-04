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

    // TODO: Sort this stuff out
    const int JUMP_TIME = 30;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    float modelY = 0.0f;
    float modelX = 0.0f;
    float xMovementSpeed = 0.20f;
    float yMovementSpeed = 0.20f;
    bool playerJumping = false;
    int jumpTime = JUMP_TIME;
    float floorHeight = -2.4f;

    // temp
    float angle = 0.0f;
    bool rotateLeft = true;

    /* Functions */
public:
    InputManager();
    ~InputManager();

    void InitWindow(const std::string& InWindowName = "Smoldering Engine", int InWidth = 1280, int InHeight = 720);
    void initializeKeyStates();
    void processInput(float inDeltaTime, class Camera* inCamera);
    float ProcessJump(float inDeltaTime);

private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
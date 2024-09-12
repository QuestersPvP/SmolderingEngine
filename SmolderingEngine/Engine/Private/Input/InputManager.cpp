#include "Engine/Public/Input/InputManager.h"

#include "Engine/Public/Rendering/Renderer.h"

InputManager::InputManager() : window(nullptr) 
{
    InitKeyStates();
}

void InputManager::InitWindow(const std::string& inWindowName, int inWidth, int inHeight)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(inWidth, inHeight, inWindowName.c_str(), nullptr, nullptr);
    windowWidth = inWidth;
    windowHeight = inHeight;

    // Initialize the mouse position to center of screen
    lastMouseX = inWidth / 2.f;
    lastMouseY = inHeight / 2.f;

    if (window) 
    {
        glfwSetWindowUserPointer(window, this);        
        
        // Input Handling
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, MouseCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Capture the mouse cursor
    }
}

void InputManager::InitKeyStates() 
{
    // --- ENGINE KEYBINDS ---
    keyStates[GLFW_KEY_ESCAPE] = false;

    // --- CAMERA MOVEMENT ---
    keyStates[GLFW_KEY_W] = false;
    keyStates[GLFW_KEY_SPACE] = false;
    keyStates[GLFW_KEY_A] = false;
    keyStates[GLFW_KEY_D] = false;
}

//void InputManager::InitRendererCallback(Renderer* inRenderer)
//{
//    // Set the user pointer to the Renderer instance
//    glfwSetWindowUserPointer(window, inRenderer);
//    // Called when window is resized
//    glfwSetFramebufferSizeCallback(window, WindowResizeCallback);
//}

void InputManager::KeyCallback(GLFWwindow* inWindow, int inKey, int inScancode, int inAction, int inMods)
{
    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(inWindow));
   
    if (inputManager) 
    {
        if (inAction == GLFW_PRESS)
        {
            inputManager->keyStates[inKey] = true;
        }
        else if (inAction == GLFW_RELEASE)
        {
            inputManager->keyStates[inKey] = false;

            if (GLFW_KEY_ESCAPE == inKey)
                inputManager->mouseModeChanged = false;
        }
    }
}

void InputManager::MouseCallback(GLFWwindow* inWindow, double inXPos, double inYPos)
{
    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(inWindow));
    if (inputManager)
    {
        inputManager->currentMouseX = inXPos;
        inputManager->currentMouseY = inYPos;
    }
}

//void InputManager::WindowResizeCallback(GLFWwindow* inWindow, int inWidth, int inHeight)
//{
//    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(inWindow));
//    Renderer* seRenderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(inWindow));
//
//    if (seRenderer && inputManager)
//    {
//        inputManager->resizingWindow = true;
//        seRenderer->ResizeRenderer(inWidth, inHeight);
//        inputManager->resizingWindow = false;
//    }
//}

void InputManager::processInput(float inDeltaTime, class Camera* inCamera)
{
    // -- MOUSE MOVEMENT -- 
    float offsetX = currentMouseX - lastMouseX;
    float offsetY = lastMouseY - currentMouseY; // Inverted Y-axis for pitch

    // Save the current mouse position for the next frame
    lastMouseX = currentMouseX;
    lastMouseY = currentMouseY;

    // Apply sensitivity to the mouse movement and make it frame-rate independent using deltaTime
    offsetX *= mouseMovementSpeed * inDeltaTime;
    offsetY *= mouseMovementSpeed * inDeltaTime;

    // Update yaw and pitch based on mouse movement
    yaw += offsetX;
    pitch += offsetY;

    // Constrain the pitch to avoid flipping the camera
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    // -- MOUSE MOVEMENT -- 

    // Calculate the front direction from where camera is looking
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    front = glm::normalize(front);

    // Calculate the right direction based on the front direction and world up direction
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

    // WSAD for movement
    glm::vec3 movement(0.0f);

    if (keyStates[GLFW_KEY_W]) // Move forward
        movement += front * keyboardMovementSpeed * inDeltaTime;
    if (keyStates[GLFW_KEY_S]) // Move backward
        movement -= front * keyboardMovementSpeed * inDeltaTime;
    if (keyStates[GLFW_KEY_A]) // Move left
        movement -= right * keyboardMovementSpeed * inDeltaTime;
    if (keyStates[GLFW_KEY_D]) // Move right
        movement += right * keyboardMovementSpeed * inDeltaTime;

    // Update position based on movement vector
    modelX += movement.x;
    modelY += movement.y;
    modelZ += movement.z;

    // Camera position based on updated model position
    glm::vec3 cameraPosition = glm::vec3(modelX, modelY, modelZ);

    // Camera target based on the front direction
    glm::vec3 targetPosition = cameraPosition + front;

    // Define up direction (Y-axis)
    glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);

    // Update the view matrix
    inCamera->uboViewProjection.view = glm::lookAt(cameraPosition, targetPosition, upDirection);


    // --- ENGINE KEYBINDS --- TODO: Separate these functions eventually
    if (keyStates[GLFW_KEY_ESCAPE] && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED && !mouseModeChanged)
    {
        mouseModeChanged = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else if (keyStates[GLFW_KEY_ESCAPE] && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL && !mouseModeChanged)
    {
        mouseModeChanged = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}
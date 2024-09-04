#include "Engine/Public/Input/InputManager.h"

InputManager::InputManager() : window(nullptr) 
{
    initializeKeyStates();
}

InputManager::~InputManager() 
{
    if (window) 
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

void InputManager::InitWindow(const std::string& InWindowName, int InWidth, int InHeight) 
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(InWidth, InHeight, InWindowName.c_str(), nullptr, nullptr);

    if (window) 
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, key_callback);
    }
}

void InputManager::initializeKeyStates() 
{
    keyStates[GLFW_KEY_W] = false;
    keyStates[GLFW_KEY_SPACE] = false;
    keyStates[GLFW_KEY_A] = false;
    keyStates[GLFW_KEY_D] = false;
}

void InputManager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
   
    if (inputManager) 
    {
        if (action == GLFW_PRESS) 
        {
            inputManager->keyStates[key] = true;
        }
        else if (action == GLFW_RELEASE) 
        {
            inputManager->keyStates[key] = false;
        }
    }
}

float InputManager::ProcessJump(float inDeltaTime) 
{
    return std::min(((yMovementSpeed + 0.2f) * inDeltaTime) / ((float)jumpTime / (float)JUMP_TIME), 0.1f);
}

void InputManager::processInput(float inDeltaTime, class Camera* inCamera)
{
    float movementX = 0.0f;
    float movementY = 0.0f;

    if (keyStates[GLFW_KEY_A])
        movementX += -xMovementSpeed * inDeltaTime;
    if (keyStates[GLFW_KEY_D])
        movementX += xMovementSpeed * inDeltaTime;

    //if ((keyStates[GLFW_KEY_SPACE] && !playerJumping) && (modelY == (floorHeight + 2.4f) || (modelY == (floorHeight + 1.0f) && !rotateLeft))) 
    //{
    //    playerJumping = true;
    //    jumpTime = 1;
    //    movementY += ProcessJump(inDeltaTime);
    //    jumpTime++;
    //}
    //else if (playerJumping && jumpTime < JUMP_TIME) {
    //    movementY += ProcessJump(inDeltaTime);
    //    jumpTime++;
    //    if (jumpTime >= JUMP_TIME)
    //        playerJumping = false;
    //}
    //else {
    //    movementY += -yMovementSpeed * inDeltaTime;
    //}

    modelX += movementX;
    modelY += movementY;

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(modelX, modelY, 0.0f));

    inCamera->uboViewProjection.view = glm::lookAt(glm::vec3(modelMatrix[3].x, modelMatrix[3].y + 20.0f, modelMatrix[3].z-50.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}
#pragma once

/* Temporary */
#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* OS Specific */
#include <Windows.h>

// For time tracking
#include <chrono>

/* General */
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <thread>
#include <cmath>
#include <functional>
#include <memory>
#include <cassert>

/* Third Party Libraries */
#include "../../../Dependencies/ktx/include/ktx.h"

/* Application Level */
#include "../Defines/ApplicationDefines.h"
#include "../Structs/ApplicationStructures.h"

/* Third Party Libraries */
#include "../../../Dependencies/VulkanglTFModel.h"

/* Rendering */
#include "Rendering/RenderPass/RenderPass.h"
#include "Rendering/Camera/Camera.h"

#include "Objects/Object.h"
//#include "Objects/GameObjects/GameObject.h"

/* Math */
#pragma once

/* Temporary */
//TODO: REMOVE TEMP INCLUDES
#include "vulkan/vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* Application Level */
#include "../Defines/ApplicationDefines.h"
#include "../Structs/ApplicationStructures.h"

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

/* Dependencies */
#include "../../../Dependencies/VulkanglTFModel.h"

/* Rendering */
#include "Rendering/RenderPass/RenderPass.h"
#include "Rendering/Camera/Camera.h"

/* Math */
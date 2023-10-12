#pragma once

#include "../../Common/Common.h"

namespace SmolderingEngine
{
    bool CreateCommandPool(VkDevice _logicalDevice, VkCommandPoolCreateFlags _commandPoolFlags, uint32_t _queueFamily, VkCommandPool& _commandPool);


    bool AllocateCommandBuffers(VkDevice _logicalDevice, VkCommandPool _commandPool, VkCommandBufferLevel _level, uint32_t _count, std::vector<VkCommandBuffer>& _commandBuffers);
};


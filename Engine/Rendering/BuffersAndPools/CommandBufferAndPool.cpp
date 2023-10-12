#include "CommandBufferAndPool.h"


namespace SmolderingEngine
{
	bool CreateCommandPool(VkDevice _logicalDevice, VkCommandPoolCreateFlags _commandPoolFlags, uint32_t _queueFamily, VkCommandPool& _commandPool)
	{
        // Pools inform the driver about intended usage of command buffers allocated from them. Set flags accordingly.
        VkCommandPoolCreateInfo commandPoolCreateInfo = 
        {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,   // VkStructureType              sType
            nullptr,                                      // const void                 * pNext
            _commandPoolFlags,                            // VkCommandPoolCreateFlags     flags
            _queueFamily                                  // uint32_t                     queueFamilyIndex
        };

        if (vkCreateCommandPool(_logicalDevice, &commandPoolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS)
        {
            std::cout << "Could not create command pool." << std::endl;
            return false;
        }

        return true;
	}


    bool AllocateCommandBuffers(VkDevice _logicalDevice, VkCommandPool _commandPool, VkCommandBufferLevel _level, uint32_t _count, std::vector<VkCommandBuffer>& _commandBuffers)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = 
        {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,     // VkStructureType          sType
            nullptr,                                            // const void             * pNext
            _commandPool,                                       // VkCommandPool            commandPool
            _level,                                             // VkCommandBufferLevel     level
            _count                                              // uint32_t                 commandBufferCount
        };

        _commandBuffers.resize(_count);

        if (vkAllocateCommandBuffers(_logicalDevice, &commandBufferAllocateInfo, _commandBuffers.data()) != VK_SUCCESS)
        {
            std::cout << "Could not allocate command buffers." << std::endl;
            return false;
        }

        return true;
    }
};

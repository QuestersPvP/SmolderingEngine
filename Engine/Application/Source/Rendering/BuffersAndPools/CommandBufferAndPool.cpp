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

    bool ResetCommandPool(VkDevice _logicalDevice, VkCommandPool _commandPool, bool _releaseResources)
    {
        // This resets all command buffers at once.
        if (vkResetCommandPool(_logicalDevice, _commandPool, _releaseResources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0) != VK_SUCCESS)
        {
            std::cout << "Error occurred during command pool reset." << std::endl;
            return false;
        }

        return true;
    }

    void DestroyCommandPool(VkDevice _logicalDevice, VkCommandPool& _commandPool)
    {
        if (_commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(_logicalDevice, _commandPool, nullptr);
            _commandPool = VK_NULL_HANDLE;
        }
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

    bool SubmitCommandBuffersToQueue(VkQueue _queue, std::vector<WaitSemaphoreInfo> _waitSemaphoreInfo, std::vector<VkCommandBuffer> _commandBuffers, std::vector<VkSemaphore> _signalSemaphores, VkFence _fence)
    {
        // Split semaphores and pipeline stages into separate arrays and populate them.
        std::vector<VkSemaphore> waitSemaphoreHandles;
        std::vector<VkPipelineStageFlags> waitSemaphoreStages;
        for (auto& waitSemaphoreInfo : _waitSemaphoreInfo)
        {
            waitSemaphoreHandles.emplace_back(waitSemaphoreInfo.Semaphore);
            waitSemaphoreStages.emplace_back(waitSemaphoreInfo.WaitingStage);
        }

        VkSubmitInfo submitInfo = 
        {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,                        // VkStructureType                sType
            nullptr,                                              // const void                   * pNext
            static_cast<uint32_t>(_waitSemaphoreInfo.size()),     // uint32_t                       waitSemaphoreCount
            waitSemaphoreHandles.data(),                          // const VkSemaphore            * pWaitSemaphores
            waitSemaphoreStages.data(),                           // const VkPipelineStageFlags   * pWaitDstStageMask
            static_cast<uint32_t>(_commandBuffers.size()),        // uint32_t                       commandBufferCount
            _commandBuffers.data(),                               // const VkCommandBuffer        * pCommandBuffers
            static_cast<uint32_t>(_signalSemaphores.size()),      // uint32_t                       signalSemaphoreCount
            _signalSemaphores.data()                              // const VkSemaphore            * pSignalSemaphores
        };

        if (vkQueueSubmit(_queue, 1, &submitInfo, _fence) != VK_SUCCESS)
        {
            std::cout << "Error occurred during command buffer submission." << std::endl;
            return false;
        }

        return true;
    }

    bool SynchronizeTwoCommandBuffers(VkQueue _firstQueue, std::vector<WaitSemaphoreInfo> _firstWaitSemaphoreInfo, std::vector<VkCommandBuffer> _firstCommandBuffers, std::vector<WaitSemaphoreInfo> _synchronizingSemaphores,
        VkQueue _secondQueue, std::vector<VkCommandBuffer> _secondCommandBuffers, std::vector<VkSemaphore> _secondSignalSemaphores, VkFence _secondFence)
    {
        // Signal all the semaphores included in the list of _synchronizingSemaphores.
        std::vector<VkSemaphore> firstSignalSemaphores;
        for (auto& semaphoreInfo : _synchronizingSemaphores)
        {
            firstSignalSemaphores.emplace_back(semaphoreInfo.Semaphore);
        }

        // Submit the semaphores to the queues.
        if (!SubmitCommandBuffersToQueue(_firstQueue, _firstWaitSemaphoreInfo, _firstCommandBuffers, firstSignalSemaphores, VK_NULL_HANDLE))
            return false;

        if (!SubmitCommandBuffersToQueue(_secondQueue, _synchronizingSemaphores, _secondCommandBuffers, _secondSignalSemaphores, _secondFence))
            return false;

        return true;
    }

    bool CheckIfProcessingOfSubmittedCommandBufferHasFinished(VkDevice _logicalDevice, VkQueue _queue, std::vector<WaitSemaphoreInfo> _waitSemaphoreInfo, std::vector<VkCommandBuffer> _commandBuffers,
        std::vector<VkSemaphore> _signalSemaphores, VkFence _fence, uint64_t _timeout, VkResult& _waitStatus)
    {
        if (!SubmitCommandBuffersToQueue(_queue, _waitSemaphoreInfo, _commandBuffers, _signalSemaphores, _fence))
            return false;

        return WaitForFences(_logicalDevice, { _fence }, VK_FALSE, _timeout);
    }

    void DestroyCommandBuffers(VkDevice _logicalDevice, VkCommandPool _commandPool, std::vector<VkCommandBuffer>& _commandBuffers)
    {
        if (_commandBuffers.size() > 0)
        {
            vkFreeCommandBuffers(_logicalDevice, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
            _commandBuffers.clear();
        }
    }

    bool WaitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue _queue)
    {
        if (vkQueueWaitIdle(_queue) != VK_SUCCESS)
        {
            std::cout << "Waiting for all operations submitted to queue failed." << std::endl;
            return false;
        }
        return true;
    }

    bool WaitForAllSubmittedCommandsToBeFinished(VkDevice _logicalDevice)
    {
        if (vkDeviceWaitIdle(_logicalDevice) != VK_SUCCESS)
        {
            std::cout << "Waiting on a device failed." << std::endl;
            return false;
        }

        return true;
    }

    bool BeginCommandBufferRecordingOperation(VkCommandBuffer _commandBuffer, VkCommandBufferUsageFlags _usage, VkCommandBufferInheritanceInfo* _secondaryCommandBufferInfo)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = 
        {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                        sType
            nullptr,                                        // const void                           * pNext
            _usage,                                         // VkCommandBufferUsageFlags              flags
            _secondaryCommandBufferInfo                     // const VkCommandBufferInheritanceInfo * pInheritanceInfo
        };

        if (vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
        {
            std::cout << "Could not begin command buffer recording operation." << std::endl;
            return false;
        }

        return true;
    }

    bool EndCommandBufferRecordingOperation(VkCommandBuffer _commandBuffer)
    {
        if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
        {
            std::cout << "Error occurred during command buffer recording." << std::endl;
            return false;
        }

        return true;
    }

    bool ResetCommandBuffer(VkCommandBuffer _commandBuffer, bool _releaseResources)
    {
        // Reset each command buffer individually. 
        if (vkResetCommandBuffer(_commandBuffer, _releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0) != VK_SUCCESS)
        {
            std::cout << "Error occurred during command buffer reset." << std::endl;
            return false;
        }

        return true;
    }

    bool CreateFence(VkDevice _logicalDevice, bool _signaled, VkFence& _fence)
    {
        //VkFenceCreateInfo fence_create_info = 
        //{
        //    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,            // VkStructureType        sType
        //    nullptr,                                        // const void           * pNext
        //    _signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0    // VkFenceCreateFlags     flags
        //};
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(_logicalDevice, &fenceCreateInfo, nullptr, &_fence) != VK_SUCCESS)
        {
            std::cout << "Could not create a fence." << std::endl;
            return false;
        }

        return true;
    }

    bool WaitForFences(VkDevice _logicalDevice, std::vector<VkFence> const& _fences, VkBool32 _waitForAll, uint64_t _timeout)
    {
        if (_fences.size() > 0)
        {
            if (vkWaitForFences(_logicalDevice, static_cast<uint32_t>(_fences.size()), _fences.data(), _waitForAll, _timeout) != VK_SUCCESS)
            {
                std::cout << "Waiting on fence failed." << std::endl;
                return false;
            }
            return true;
        }

        return false;
    }

    bool ResetFences(VkDevice _logicalDevice, std::vector<VkFence> const& _fences)
    {
        if (_fences.size() > 0)
        {
            if (vkResetFences(_logicalDevice, static_cast<uint32_t>(_fences.size()), _fences.data()) != VK_SUCCESS)
            {
                std::cout << "Error occurred when tried to reset fences." << std::endl;
                return false;
            }
            return true;
        }

        return false;
    }

    void DestroyFence(VkDevice _logicalDevice, VkFence& _fence)
    {
        if (_fence != VK_NULL_HANDLE) 
        {
            vkDestroyFence(_logicalDevice, _fence, nullptr);
            _fence = VK_NULL_HANDLE;
        }
    }

    void DestroySemaphore(VkDevice _logicalDevice, VkSemaphore& _semaphore)
    {
        if (_semaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(_logicalDevice, _semaphore, nullptr);
            _semaphore = VK_NULL_HANDLE;
        }
    }
};

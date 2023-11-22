#pragma once

#include "../../Common/Common.h"

namespace SmolderingEngine
{
    bool CreateCommandPool(VkDevice _logicalDevice, VkCommandPoolCreateFlags _commandPoolFlags, uint32_t _queueFamily, VkCommandPool& _commandPool);
    bool ResetCommandPool(VkDevice _logicalDevice, VkCommandPool _commandPool, bool _releaseResources);
    void DestroyCommandPool(VkDevice _logicalDevice, VkCommandPool& _commandPool);

    bool AllocateCommandBuffers(VkDevice _logicalDevice, VkCommandPool _commandPool, VkCommandBufferLevel _level, uint32_t _count, std::vector<VkCommandBuffer>& _commandBuffers);
    bool SubmitCommandBuffersToQueue(VkQueue _queue, std::vector<WaitSemaphoreInfo>  _waitSemaphoreInfo, std::vector<VkCommandBuffer> _commandBuffers, std::vector<VkSemaphore> _signalSemaphores, VkFence _fence);
    bool SynchronizeTwoCommandBuffers(VkQueue _firstQueue, std::vector<WaitSemaphoreInfo> _firstWaitSemaphoreInfo, std::vector<VkCommandBuffer> _firstCommandBuffers, std::vector<WaitSemaphoreInfo> _synchronizingSemaphores,
        VkQueue _secondQueue, std::vector<VkCommandBuffer> _secondCommandBuffers, std::vector<VkSemaphore> _secondSignalSemaphores, VkFence _secondFence);
    bool CheckIfProcessingOfSubmittedCommandBufferHasFinished(VkDevice _logicalDevice, VkQueue _queue, std::vector<WaitSemaphoreInfo> _waitSemaphoreInfo, std::vector<VkCommandBuffer> _commandBuffers,
        std::vector<VkSemaphore> _signalSemaphores, VkFence _fence, uint64_t _timeout, VkResult& _waitStatus);
    void DestroyCommandBuffers(VkDevice _logicalDevice, VkCommandPool  _commandPool, std::vector<VkCommandBuffer>& _commandBuffers);

    bool WaitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue _queue);
    bool WaitForAllSubmittedCommandsToBeFinished(VkDevice _logicalDevice);

    bool BeginCommandBufferRecordingOperation(VkCommandBuffer _commandBuffer, VkCommandBufferUsageFlags _usage, VkCommandBufferInheritanceInfo* _secondaryCommandBufferInfo);
    bool EndCommandBufferRecordingOperation(VkCommandBuffer _commandBuffer);  
    bool ResetCommandBuffer(VkCommandBuffer _commandBuffer, bool _releaseResources);

    bool CreateFence(VkDevice _logicalDevice, bool _signaled, VkFence& _fence);
    bool WaitForFences(VkDevice _logicalDevice, std::vector<VkFence> const& _fences, VkBool32 _waitForAll, uint64_t _timeout);
    bool ResetFences(VkDevice _logicalDevice, std::vector<VkFence> const& _fences);
    void DestroyFence(VkDevice _logicalDevice, VkFence& _fence);
    void DestroySemaphore(VkDevice _logicalDevice, VkSemaphore& _semaphore);
};


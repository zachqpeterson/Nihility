#include "VulkanCommandBuffer.hpp"

#include "VulkanDevice.hpp"

void VulkanCommandBuffer::Allocate(RendererState* rendererState, VkCommandPool pool, bool isPrimary)
{
    VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocateInfo.commandPool = pool;
    allocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VkCheck(vkAllocateCommandBuffers(rendererState->device->logicalDevice, &allocateInfo, &handle));
    state = COMMAND_BUFFER_STATE_READY;
}

void VulkanCommandBuffer::Free(RendererState* rendererState, VkCommandPool pool)
{
    vkFreeCommandBuffers(rendererState->device->logicalDevice, pool, 1, &handle);

    handle = nullptr;
    state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void VulkanCommandBuffer::Begin(bool singleUse, bool renderpassContinue, bool simultaneousUse)
{
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = 0;
    if (singleUse)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (renderpassContinue)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (simultaneousUse)
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VkCheck(vkBeginCommandBuffer(handle, &beginInfo));
    state = COMMAND_BUFFER_STATE_RECORDING;
}

void VulkanCommandBuffer::End()
{
    VkCheck(vkEndCommandBuffer(handle));
    state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void VulkanCommandBuffer::UpdateSubmitted()
{
    state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void VulkanCommandBuffer::Reset()
{
    state = COMMAND_BUFFER_STATE_READY;
}

void VulkanCommandBuffer::AllocateAndBeginSingleUse(RendererState* rendererState, VkCommandPool pool)
{
    Allocate(rendererState, pool, true);
    Begin(true, false, false);
}

void VulkanCommandBuffer::EndSingleUse(RendererState* rendererState, VkCommandPool pool, VkQueue queue)
{
    End();

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &handle;
    VkCheck(vkQueueSubmit(queue, 1, &submitInfo, 0));

    VkCheck(vkQueueWaitIdle(queue));

    Free(rendererState, pool);
}
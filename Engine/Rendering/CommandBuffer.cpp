#include "CommandBuffer.hpp"

#include "Renderer.hpp"















/*------COMMAND BUFFER RING------*/

void CommandBufferRing::Create()
{
    for (U32 i = 0; i < maxPools; ++i)
    {
        VkCommandPoolCreateInfo cmdPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr };
        cmdPoolInfo.queueFamilyIndex = Renderer::queueFamilyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkValidate(vkCreateCommandPool(Renderer::device, &cmdPoolInfo, Renderer::allocationCallbacks, &commandPools[i]));
    }

    for (U32 i = 0; i < maxBuffers; ++i)
    {
        VkCommandBufferAllocateInfo cmd = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr };
        const U32 poolIndex = PoolFromIndex(i);
        cmd.commandPool = commandPools[poolIndex];
        cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd.commandBufferCount = 1;
        VkValidate(vkAllocateCommandBuffers(Renderer::device, &cmd, &commandBuffers[i].commandBuffer));

        commandBuffers[i].handle = i;
        commandBuffers[i].Reset();
    }
}

void CommandBufferRing::Destroy()
{
    for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES * maxThreads; i++)
    {
        vkDestroyCommandPool(Renderer::device, commandPools[i], Renderer::allocationCallbacks);
    }
}

void CommandBufferRing::ResetPools(U32 frameIndex)
{

    for (U32 i = 0; i < maxThreads; i++)
    {
        vkResetCommandPool(Renderer::device, commandPools[frameIndex * maxThreads + i], 0);
    }
}

CommandBuffer* CommandBufferRing::GetCommandBuffer(U32 frame, bool begin)
{
    // TODO: take in account threads
    CommandBuffer* cb = &commandBuffers[frame * bufferPerPool];

    if (begin)
    {
        cb->Reset();

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cb->commandBuffer, &beginInfo);
    }

    return cb;
}

CommandBuffer* CommandBufferRing::GetCommandBufferInstant(U32 frame, bool begin)
{
    CommandBuffer* cb = &commandBuffers[frame * bufferPerPool + 1];
    return cb;
}
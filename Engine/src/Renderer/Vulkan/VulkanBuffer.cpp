#include "VulkanBuffer.hpp"

#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"

#include "Memory/Memory.hpp"

bool VulkanBuffer::Create(RendererState* rendererState, U64 size, VkBufferUsageFlagBits usage,
    U32 memoryPropertyFlags, bool bindOnCreate, bool useFreelist)
{
    hasFreelist = useFreelist;
    totalSize = size;
    this->usage = usage;
    this->memoryPropertyFlags = memoryPropertyFlags;

    if (useFreelist)
    {
        freelist.Resize(size);
    }

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

    VkCheck(vkCreateBuffer(rendererState->device->logicalDevice, &bufferInfo, rendererState->allocator, &handle));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(rendererState->device->logicalDevice, handle, &requirements);
    memoryIndex = rendererState->FindMemoryIndex(requirements.memoryTypeBits, memoryPropertyFlags);
    if (memoryIndex == -1)
    {
        LOG_ERROR("Unable to create vulkan buffer because the required memory type index was not found.");

        freelist.Cleanup();
        return false;
    }

    VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = (U32)memoryIndex;

    VkResult result = vkAllocateMemory(
        rendererState->device->logicalDevice,
        &allocateInfo,
        rendererState->allocator,
        &memory);

    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);

        freelist.Cleanup();
        return false;
    }

    if (bindOnCreate)
    {
        Bind(rendererState, 0);
    }

    return true;
}

void VulkanBuffer::Destroy(RendererState* rendererState)
{
    if (hasFreelist) { freelist.Cleanup(); }

    if (memory)
    {
        vkFreeMemory(rendererState->device->logicalDevice, memory, rendererState->allocator);
        memory = nullptr;
    }

    if (handle)
    {
        vkDestroyBuffer(rendererState->device->logicalDevice, handle, rendererState->allocator);
        handle = nullptr;
    }

    totalSize = 0;
    usage = (VkBufferUsageFlagBits)0;
    locked = false;
}

bool VulkanBuffer::Resize(RendererState* rendererState, U64 newSize, VkQueue queue, VkCommandPool pool)
{
    if (newSize < totalSize)
    {
        LOG_ERROR("Resize requires that new size be larger than the old. Not doing this could lead to data loss.");
        return false;
    }

    if (hasFreelist)
    {
        if (!freelist.Resize(newSize))
        {
            LOG_ERROR("vulkan_buffer_resize failed to resize internal free list.");
            freelist.Cleanup();
            return false;
        }
    }

    totalSize = newSize;

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = newSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

    VkBuffer newBuffer;
    VkCheck(vkCreateBuffer(rendererState->device->logicalDevice, &bufferInfo, rendererState->allocator, &newBuffer));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(rendererState->device->logicalDevice, newBuffer, &requirements);

    VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = (U32)memoryIndex;

    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(rendererState->device->logicalDevice, &allocateInfo, rendererState->allocator, &newMemory);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    VkCheck(vkBindBufferMemory(rendererState->device->logicalDevice, newBuffer, newMemory, 0));

    CopyTo(rendererState, pool, 0, queue, handle, 0, newBuffer, 0, totalSize);

    vkDeviceWaitIdle(rendererState->device->logicalDevice);

    if (memory)
    {
        vkFreeMemory(rendererState->device->logicalDevice, memory, rendererState->allocator);
        memory = 0;
    }
    if (handle)
    {
        vkDestroyBuffer(rendererState->device->logicalDevice, handle, rendererState->allocator);
        handle = 0;
    }

    totalSize = newSize;
    memory = newMemory;
    handle = newBuffer;

    return true;
}

void VulkanBuffer::Bind(RendererState* rendererState, U64 offset)
{
    VkCheck(vkBindBufferMemory(rendererState->device->logicalDevice, handle, memory, offset));
}

void* VulkanBuffer::LockMemory(RendererState* rendererState, U64 offset, U64 size, U32 flags)
{
    void* data;
    VkCheck(vkMapMemory(rendererState->device->logicalDevice, memory, offset, size, flags, &data));
    return data;
}

void VulkanBuffer::UnlockMemory(RendererState* rendererState)
{
    vkUnmapMemory(rendererState->device->logicalDevice, memory);
}

bool VulkanBuffer::Allocate(U64 size, U64* outOffset)
{
    if (!size || !outOffset)
    {
        LOG_ERROR("vulkan_buffer_allocate requires valid buffer, a nonzero size and valid pointer to hold offset.");
        return false;
    }

    if (!hasFreelist)
    {
        LOG_WARN("vulkan_buffer_allocate called on a buffer not using freelists. Offset will not be valid. Call vulkan_buffer_load_data instead.");
        *outOffset = 0;
        return true;
    }
    
    return freelist.AllocateBlock(size, outOffset);
}

bool VulkanBuffer::Free(U64 size, U64 offset)
{
    if (!size)
    {
        LOG_ERROR("vulkan_buffer_free requires valid buffer and a nonzero size.");
        return false;
    }

    if (!hasFreelist)
    {
        LOG_WARN("vulkan_buffer_allocate called on a buffer not using freelists. Nothing was done.");
        return true;
    }

    return freelist.FreeBlock(size, offset);
}

void VulkanBuffer::LoadData(RendererState* rendererState, U64 offset, U64 size, U32 flags, const void* data)
{
    void* dataPtr;
    VkCheck(vkMapMemory(rendererState->device->logicalDevice, memory, offset, size, flags, &dataPtr));
    Memory::Copy(dataPtr, data, size);
    vkUnmapMemory(rendererState->device->logicalDevice, memory);
}

void VulkanBuffer::CopyTo(RendererState* rendererState, VkCommandPool pool, VkFence fence, VkQueue queue,
    VkBuffer source, U64 sourceOffset, VkBuffer dest, U64 destOffset, U64 size)
{
    vkQueueWaitIdle(queue);
    
    VulkanCommandBuffer tempCommandBuffer;
    tempCommandBuffer.AllocateAndBeginSingleUse(rendererState, pool);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.dstOffset = destOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(tempCommandBuffer.handle, source, dest, 1, &copyRegion);

    tempCommandBuffer.EndSingleUse(rendererState, pool, queue);
}
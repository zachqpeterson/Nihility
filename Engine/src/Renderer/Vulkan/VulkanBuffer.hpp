#pragma once

#include "VulkanDefines.hpp"

#include "Containers/Freelist.hpp"

class VulkanBuffer
{
public:
    bool Create(RendererState* rendererState, U32 size, VkBufferUsageFlagBits usage,
        U32 memoryPropertyFlags, bool bindOnCreate, bool useFreelist);
    void Destroy(RendererState* rendererState);
    bool Resize(RendererState* rendererState, U32 newSize, VkQueue queue, VkCommandPool pool);
    void Bind(RendererState* rendererState, U32 offset);
    void* LockMemory(RendererState* rendererState, U32 offset, U32 size, U32 flags);
    void UnlockMemory(RendererState* rendererState);
    U32 Allocate(U32 size);
    bool Free(U32 size, U32 offset);
    void LoadData(RendererState* rendererState, U32 offset, U32 size, U32 flags, const void* data);
    void CopyTo(RendererState* rendererState, VkCommandPool pool, VkFence fence, VkQueue queue,
        VkBuffer source, U32 sourceOffset, VkBuffer dest, U32 destOffset, U32 size);

    VkBuffer handle;
    
private:
    U32 totalSize;
    VkBufferUsageFlagBits usage;
    bool locked;
    VkDeviceMemory memory;
    I32 memoryIndex;
    U32 memoryPropertyFlags;
    Freelist freelist;
    bool hasFreelist;
};
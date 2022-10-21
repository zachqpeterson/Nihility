#pragma once

#include "VulkanDefines.hpp"

#include "Containers/Freelist.hpp"

class VulkanBuffer
{
public:
	bool Create(RendererState* rendererState, U64 size, VkBufferUsageFlagBits usage,
		U32 memoryPropertyFlags, bool bindOnCreate, bool useFreelist);
	void Destroy(RendererState* rendererState);
	bool Resize(RendererState* rendererState, U64 newSize, VkQueue queue, VkCommandPool pool);
	void Bind(RendererState* rendererState, U64 offset);
	void* LockMemory(RendererState* rendererState, U64 offset, U64 size, U32 flags);
	void UnlockMemory(RendererState* rendererState);
	U64 Allocate(U64 size);
	bool Free(U64 size, U64 offset);
	void LoadData(RendererState* rendererState, U64 offset, U64 size, U32 flags, const void* data);
	void CopyTo(RendererState* rendererState, VkCommandPool pool, VkFence fence, VkQueue queue,
		VkBuffer source, U64 sourceOffset, VkBuffer dest, U64 destOffset, U64 size);
	void BatchCopyTo(RendererState* rendererState, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, VkBuffer dest, Vector<VkBufferCopy>& copies);

	VkBuffer handle{ nullptr };

private:
	U64 totalSize{ 0 };
	I32 memoryIndex{ 0 };
	U32 memoryPropertyFlags{ 0 };
	VkBufferUsageFlagBits usage{ VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM };
	VkDeviceMemory memory{ nullptr };
	Freelist freelist;
	bool locked{ false };
	bool hasFreelist{ false };
};
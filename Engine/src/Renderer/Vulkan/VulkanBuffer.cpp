#include "VulkanBuffer.hpp"

#include "Device.hpp"
#include "VulkanCommandBuffer.hpp"

#include "Memory/Memory.hpp"
#include "Core/Logger.hpp"
#include "Core/Time.hpp"

bool VulkanBuffer::Create(RendererState* rendererState, U64 size, VkBufferUsageFlagBits usage,
	U32 memoryPropertyFlags, bool bindOnCreate, bool useFreelist)
{
	hasFreelist = useFreelist;
	totalSize = size;
	this->usage = usage;
	this->memoryPropertyFlags = memoryPropertyFlags;

	if (useFreelist)
	{
		freelist.Create(size);
	}

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

	VkCheck_ERROR(vkCreateBuffer(Device::logicalDevice, &bufferInfo, rendererState->allocator, &handle));

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(Device::logicalDevice, handle, &requirements);
	memoryIndex = rendererState->FindMemoryIndex(requirements.memoryTypeBits, memoryPropertyFlags);
	if (memoryIndex == -1)
	{
		Logger::Error("Unable to create vulkan buffer because the required memory type index was not found.");
		return false;
	}

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = (U32)memoryIndex;

	VkCheck_ERROR(vkAllocateMemory(
		Device::logicalDevice,
		&allocateInfo,
		rendererState->allocator,
		&memory));

	if (bindOnCreate)
	{
		Bind(rendererState, 0);
	}

	return true;
}

void VulkanBuffer::Destroy(RendererState* rendererState)
{
	if (hasFreelist)
	{
		freelist.Destroy();
	}

	if (memory)
	{
		vkFreeMemory(Device::logicalDevice, memory, rendererState->allocator);
		memory = nullptr;
	}

	if (handle)
	{
		vkDestroyBuffer(Device::logicalDevice, handle, rendererState->allocator);
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
		Logger::Error("Resize requires that new size be larger than the old. Not doing this could lead to data loss.");
		return false;
	}

	if (hasFreelist)
	{
		if (!freelist.Resize(newSize))
		{
			Logger::Error("Resize failed to resize internal free list.");
			return false;
		}
	}

	totalSize = newSize;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = newSize;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

	VkBuffer newBuffer;
	VkCheck(vkCreateBuffer(Device::logicalDevice, &bufferInfo, rendererState->allocator, &newBuffer));

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(Device::logicalDevice, newBuffer, &requirements);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = (U32)memoryIndex;

	VkDeviceMemory newMemory;
	VkCheck_ERROR(vkAllocateMemory(Device::logicalDevice, &allocateInfo, rendererState->allocator, &newMemory));

	VkCheck(vkBindBufferMemory(Device::logicalDevice, newBuffer, newMemory, 0));

	CopyTo(rendererState, pool, 0, queue, handle, 0, newBuffer, 0, totalSize);

	vkDeviceWaitIdle(Device::logicalDevice);

	if (memory)
	{
		vkFreeMemory(Device::logicalDevice, memory, rendererState->allocator);
		memory = nullptr;
	}
	if (handle)
	{
		vkDestroyBuffer(Device::logicalDevice, handle, rendererState->allocator);
		handle = nullptr;
	}

	totalSize = newSize;
	memory = newMemory;
	handle = newBuffer;

	return true;
}

void VulkanBuffer::Bind(RendererState* rendererState, U64 offset)
{
	VkCheck(vkBindBufferMemory(Device::logicalDevice, handle, memory, offset));
}

void* VulkanBuffer::LockMemory(RendererState* rendererState, U64 offset, U64 size, U32 flags)
{
	void* data;
	VkCheck(vkMapMemory(Device::logicalDevice, memory, offset, size == U32_MAX ? VK_WHOLE_SIZE : size, flags, &data));
	return data;
}

void VulkanBuffer::UnlockMemory(RendererState* rendererState)
{
	vkUnmapMemory(Device::logicalDevice, memory);
}

U64 VulkanBuffer::Allocate(U64 size)
{
	if (!size)
	{
		Logger::Error("VulkanBuffer::Allocate requires a nonzero size!");
		return false;
	}

	if (!hasFreelist)
	{
		Logger::Error("VulkanBuffer::Allocate called on a buffer not using freelists. Offset will not be valid. Call LoadData instead.");
		return U32_MAX;
	}

	return freelist.AllocateBlock(size);
}

bool VulkanBuffer::Free(U64 size, U64 offset)
{
	if (!size)
	{
		Logger::Error("Free requires valid buffer and a nonzero size.");
		return false;
	}

	if (!hasFreelist)
	{
		Logger::Error("Allocate called on a buffer not using freelists. Nothing was done.");
		return true;
	}

	return freelist.FreeBlock(size, offset);
}

void VulkanBuffer::LoadData(RendererState* rendererState, U64 offset, U64 size, U32 flags, const void* data)
{
	void* dataPtr;
	VkCheck(vkMapMemory(Device::logicalDevice, memory, offset, size, flags, &dataPtr));
	Memory::Copy(dataPtr, data, size);
	vkUnmapMemory(Device::logicalDevice, memory);
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

void VulkanBuffer::BatchCopyTo(RendererState* rendererState, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer source, VkBuffer dest, Vector<VkBufferCopy>& copies)
{
	vkQueueWaitIdle(queue);

	VulkanCommandBuffer tempCommandBuffer;
	tempCommandBuffer.AllocateAndBeginSingleUse(rendererState, pool);
	
	vkCmdCopyBuffer(tempCommandBuffer.handle, source, dest, (U32)copies.Size(), copies.Data());

	tempCommandBuffer.EndSingleUse(rendererState, pool, queue);
}
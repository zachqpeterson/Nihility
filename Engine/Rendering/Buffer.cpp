#include "Buffer.hpp"

#include "Renderer.hpp"

#include "CommandBufferRing.hpp"
#include "Platform/Memory.hpp"

#include "vma/vk_mem_alloc.h"

bool Buffer::Create(BufferType type, U64 size)
{
	this->type = type;

	VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = size;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	VmaAllocationCreateInfo allocationCreateInfo{};
	allocationCreateInfo.flags = 0;
	allocationCreateInfo.requiredFlags = 0;
	allocationCreateInfo.preferredFlags = 0;
	allocationCreateInfo.memoryTypeBits = 0;
	allocationCreateInfo.pool = nullptr;
	allocationCreateInfo.pUserData = nullptr;
	allocationCreateInfo.priority = 0.0f;

	switch (type)
	{
	case BufferType::Vertex: {
		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBuffer, &bufferAllocation, nullptr));

		bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBufferStaging, &stagingBufferAllocation, nullptr));

		bufferSize = size;
	} return true;
	case BufferType::Index: {
		bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBuffer, &bufferAllocation, nullptr));

		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBufferStaging, &stagingBufferAllocation, nullptr));

		bufferSize = size;
	} return true;
	case BufferType::Shader: {
		bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBuffer, &bufferAllocation, nullptr));

		bufferSize = size;
	} return true;
	case BufferType::Uniform: {
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBuffer, &bufferAllocation, nullptr));

		bufferSize = size;
	} return true;
	case BufferType::Staging: {
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &bufferCreateInfo, &allocationCreateInfo, &vkBuffer, &bufferAllocation, nullptr));
	} return true;
	}

	return false;
}

void Buffer::Destroy()
{
	if (vkBuffer) { vmaDestroyBuffer(Renderer::vmaAllocator, vkBuffer, bufferAllocation); }
	if (vkBufferStaging) { vmaDestroyBuffer(Renderer::vmaAllocator, vkBufferStaging, stagingBufferAllocation); }

	bufferSize = 0;
	vkBuffer = VK_NULL_HANDLE;
	bufferAllocation = nullptr;
	vkBufferStaging = VK_NULL_HANDLE;
	stagingBufferAllocation = nullptr;
}

bool Buffer::UploadVertexData(const void* vertexData, U64 size, U64 offset, VkSemaphore waitSemaphore)
{
	if (bufferSize < size)
	{
		Destroy();

		if (!Create(type, size)) { return false; }

		bufferSize = size;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAllocation, &data));

	memcpy((U8*)data + offset, vertexData, size);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAllocation, 0, size);

	VkBufferMemoryBarrier2 vertexBufferBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
	vertexBufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	vertexBufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	vertexBufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
	vertexBufferBarrier.dstAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
	vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vertexBufferBarrier.buffer = vkBufferStaging;
	vertexBufferBarrier.offset = 0;
	vertexBufferBarrier.size = bufferSize;

	VkBufferCopy stagingBufferCopy{};
	stagingBufferCopy.srcOffset = 0;
	stagingBufferCopy.dstOffset = 0;
	stagingBufferCopy.size = bufferSize;

	CommandBuffer& commandBuffer = CommandBufferRing::GetWriteCommandBuffer(Renderer::frameIndex);

	commandBuffer.Begin();
	commandBuffer.BufferToBuffer(vkBufferStaging, vkBuffer, 1, &stagingBufferCopy);
	commandBuffer.PipelineBarrier(0, 1, &vertexBufferBarrier, 0, nullptr);
	commandBuffer.End();
	Renderer::commandBuffers[Renderer::frameIndex].Push(commandBuffer);

	return true;
}

bool Buffer::UploadIndexData(const void* indexData, U64 size, U64 offset)
{
	if (bufferSize < size)
	{
		Destroy();
		if (!Create(type, size)) { return false; }

		bufferSize = size;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAllocation, &data));

	memcpy((U8*)data + offset, indexData, size);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAllocation, 0, size);

	VkBufferMemoryBarrier2 indexBufferBarrier = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
	indexBufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	indexBufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	indexBufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
	indexBufferBarrier.dstAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
	indexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	indexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	indexBufferBarrier.buffer = vkBufferStaging;
	indexBufferBarrier.offset = 0;
	indexBufferBarrier.size = bufferSize;

	VkBufferCopy stagingBufferCopy{};
	stagingBufferCopy.srcOffset = 0;
	stagingBufferCopy.dstOffset = 0;
	stagingBufferCopy.size = bufferSize;

	CommandBuffer& commandBuffer = CommandBufferRing::GetWriteCommandBuffer(Renderer::frameIndex);

	commandBuffer.Begin();
	commandBuffer.BufferToBuffer(vkBufferStaging, vkBuffer, 1, &stagingBufferCopy);
	commandBuffer.PipelineBarrier(0, 1, &indexBufferBarrier, 0, nullptr);
	commandBuffer.End();
	Renderer::commandBuffers[Renderer::frameIndex].Push(commandBuffer);

	return true;
}

bool Buffer::UploadShaderData(const void* shaderData, U64 size, U64 offset)
{
	bool bufferResized = false;
	if (size > bufferSize)
	{
		Destroy();
		Create(type, size);
		bufferResized = true;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, bufferAllocation, &data));

	memcpy((U8*)data + offset, shaderData, size);
	vmaUnmapMemory(Renderer::vmaAllocator, bufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, bufferAllocation, 0, bufferSize);

	return bufferResized;
}

bool Buffer::CheckForResize(U64 size)
{
	if (size > bufferSize)
	{
		Destroy();
		
		return Create(type, size);
	}

	return false;
}

bool Buffer::UploadUniformData(const void* uniformData, U64 size, U64 offset)
{
	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, bufferAllocation, &data));

	memcpy((U8*)data + offset, uniformData, size);
	vmaUnmapMemory(Renderer::vmaAllocator, bufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, bufferAllocation, 0, bufferSize);

	return true;
}

bool Buffer::UploadStagingData(const void* stagingData, U64 size, U64 offset)
{
	stagingPointer = Math::Max(offset + size, stagingPointer);

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, bufferAllocation, &data));

	memcpy((U8*)data + offset, stagingData, size);
	vmaUnmapMemory(Renderer::vmaAllocator, bufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, bufferAllocation, 0, bufferSize);

	return true;
}

U64 Buffer::StagingPointer() const
{
	return stagingPointer;
}

Buffer::operator VkBuffer() const
{
	return vkBuffer;
}
#include "Buffer.hpp"

#include "Renderer.hpp"

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

bool Buffer::UploadVertexData(const MeshData& vertexData)
{
	U32 vertexDataSize = (U32)(vertexData.vertices.Size() * sizeof(VertexData));

	if (bufferSize < vertexDataSize)
	{
		Destroy();

		if (!Create(type, vertexDataSize)) { return false; }

		bufferSize = vertexDataSize;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAllocation, &data));

	memcpy(data, vertexData.vertices.Data(), vertexDataSize);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAllocation, 0, vertexDataSize);

	VkBufferMemoryBarrier vertexBufferBarrier{};
	vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vertexBufferBarrier.buffer = vkBufferStaging;
	vertexBufferBarrier.offset = 0;
	vertexBufferBarrier.size = bufferSize;

	VkBufferCopy stagingBufferCopy{};
	stagingBufferCopy.srcOffset = 0;
	stagingBufferCopy.dstOffset = 0;
	stagingBufferCopy.size = bufferSize;

	CommandBuffer commandBuffer;
	commandBuffer.CreateSingleShotBuffer(Renderer::commandPool);

	vkCmdCopyBuffer(commandBuffer, vkBufferStaging,
		vkBuffer, 1, &stagingBufferCopy);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

	return commandBuffer.SubmitSingleShotBuffer(Renderer::graphicsQueue);
}

bool Buffer::UploadVertexData(const Vector<Vector3>& vertexData)
{
	U32 vertexDataSize = (U32)(vertexData.Size() * sizeof(Vector3));

	if (bufferSize < vertexDataSize)
	{
		Destroy();

		if (!Create(type, vertexDataSize)) { return false; }
		bufferSize = vertexDataSize;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAllocation, &data));

	memcpy(data, vertexData.Data(), vertexDataSize);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAllocation, 0, vertexDataSize);

	VkBufferMemoryBarrier vertexBufferBarrier{};
	vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vertexBufferBarrier.buffer = vkBufferStaging;
	vertexBufferBarrier.offset = 0;
	vertexBufferBarrier.size = bufferSize;

	VkBufferCopy stagingBufferCopy{};
	stagingBufferCopy.srcOffset = 0;
	stagingBufferCopy.dstOffset = 0;
	stagingBufferCopy.size = bufferSize;

	CommandBuffer commandBuffer;
	commandBuffer.CreateSingleShotBuffer(Renderer::commandPool);

	vkCmdCopyBuffer(commandBuffer, vkBufferStaging,
		vkBuffer, 1, &stagingBufferCopy);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

	return commandBuffer.SubmitSingleShotBuffer(Renderer::graphicsQueue);
}

bool Buffer::UploadIndexData(const MeshData& indexData)
{
	U32 indexDataSize = (U32)(indexData.indices.Size() * sizeof(U32));
	if (bufferSize < indexDataSize)
	{
		Destroy();
		if (!Create(type, indexDataSize)) { return false; }

		bufferSize = indexDataSize;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAllocation, &data));

	memcpy(data, indexData.indices.Data(), indexDataSize);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAllocation, 0, indexDataSize);

	VkBufferMemoryBarrier indexBufferBarrier{};
	indexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	indexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	indexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	indexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	indexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	indexBufferBarrier.buffer = vkBufferStaging;
	indexBufferBarrier.offset = 0;
	indexBufferBarrier.size = bufferSize;

	VkBufferCopy stagingBufferCopy{};
	stagingBufferCopy.srcOffset = 0;
	stagingBufferCopy.dstOffset = 0;
	stagingBufferCopy.size = bufferSize;

	CommandBuffer commandBuffer;
	commandBuffer.CreateSingleShotBuffer(Renderer::commandPool);

	vkCmdCopyBuffer(commandBuffer, vkBufferStaging,
		vkBuffer, 1, &stagingBufferCopy);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &indexBufferBarrier, 0, nullptr);

	return commandBuffer.SubmitSingleShotBuffer(Renderer::graphicsQueue);
}

bool Buffer::UploadShaderData(const Vector<Matrix4>& bufferData)
{
	if (bufferData.Empty()) { return false; }

	bool bufferResized = false;
	U64 size = bufferData.Size() * sizeof(Matrix4);
	if (size > bufferSize)
	{
		Destroy();
		Create(type, size);
		bufferResized = true;
	}

	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, bufferAllocation, &data));

	memcpy(data, bufferData.Data(), size);
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

bool Buffer::UploadUniformData(const MatrixData& matrixData)
{
	void* data;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, bufferAllocation, &data));

	memcpy(data, &matrixData, sizeof(MatrixData));
	vmaUnmapMemory(Renderer::vmaAllocator, bufferAllocation);
	vmaFlushAllocation(Renderer::vmaAllocator, bufferAllocation, 0, bufferSize);

	return true;
}

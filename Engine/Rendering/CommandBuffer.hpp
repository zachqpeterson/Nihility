#pragma once

#include "Resources\ResourceDefines.hpp"

struct CommandBuffer
{
	void Create(QueueType type, bool baked);
	void Destroy();
	 
	void BindPass(Renderpass* renderpass);
	void BindPipeline(Pipeline* pipeline);
	void BindVertexBuffer(Buffer* buffer, U32 binding);
	void BindIndexBuffer(Buffer* buffer);
	void BindDescriptorSet(DescriptorSet** sets, U32 numLists, U32* offsets, U32 numOffsets);

	void Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount);
	void DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance);
	void DrawIndirect(Buffer* buffer, U32 offset, U32 stride);
	void DrawIndexedIndirect(Buffer* buffer, U32 offset, U32 stride);

	void Dispatch(U32 groupX, U32 groupY, U32 groupZ);
	void DispatchIndirect(Buffer* buffer, U32 offset);

	void PipelineBarrier(VkDependencyFlags dependencyFlags, U32 bufferBarrierCount, const VkBufferMemoryBarrier2* bufferBarriers, U32 imageBarrierCount, const VkImageMemoryBarrier2* imageBarriers);

	void FillBuffer(Buffer* buffer, U32 offset, U32 size, U32 data);

	VkCommandBuffer				commandBuffer{ nullptr };
	U32							handle{ U32_MAX };
	QueueType					type{ QUEUE_TYPE_GRAPHICS };
	bool						baked{ false };
};

struct CommandBufferRing
{
	void							Create();
	void							Destroy();

	void							ResetPools(U32 frameIndex);

	CommandBuffer* GetCommandBuffer(U32 frame, bool begin);
	CommandBuffer* GetCommandBufferInstant(U32 frame);

	static U16						PoolFromIndex(U32 index) { return (U16)index / bufferPerPool; }

	static constexpr U16			maxThreads = 1;
	static constexpr U16			maxPools = MAX_SWAPCHAIN_IMAGES * maxThreads;
	static constexpr U16			bufferPerPool = 4;
	static constexpr U16			maxBuffers = bufferPerPool * maxPools;

	VkCommandPool					commandPools[maxPools];
	CommandBuffer					commandBuffers[maxBuffers];
	U8								nextFreePerThreadFrame[maxPools];
};
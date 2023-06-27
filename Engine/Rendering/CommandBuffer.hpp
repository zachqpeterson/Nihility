#pragma once

#include "Resources\ResourceDefines.hpp"

//TODO: temp
struct NH_API CommandBuffer
{
	void Create(QueueType type, U32 bufferSize, U32 submitSize, bool baked);
	void Destroy();

	void BindPass(Renderpass* renderpass);
	void BindPipeline(Pipeline* pipeline);
	void BindVertexBuffer(Buffer* buffer, U32 binding);
	void BindIndexBuffer(Buffer* buffer);
	void BindDescriptorSet(DescriptorSet** sets, U32 numLists, U32* offsets, U32 numOffsets);

	void Draw(TopologyType topology, U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount);
	void DrawIndexed(TopologyType topology, U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance);
	void DrawIndirect(Buffer* buffer, U32 offset, U32 stride);
	void DrawIndexedIndirect(Buffer* buffer, U32 offset, U32 stride);

	void Dispatch(U32 groupX, U32 groupY, U32 groupZ);
	void DispatchIndirect(Buffer* buffer, U32 offset);

	void Barrier(const ExecutionBarrier& barrier);

	void FillBuffer(Buffer* buffer, U32 offset, U32 size, U32 data);

	void PushMarker(const char* name);
	void PopMarker();

	void Reset();

	VkCommandBuffer				commandBuffer;

	VkDescriptorSet				vkDescriptorSets[16];

	Renderpass*					currentRenderPass;
	Pipeline*					currentPipeline;

	U32							handle;

	U32							currentCommand;
	void*						resourceHandle;
	QueueType					type = QUEUE_TYPE_GRAPHICS;
	U32							bufferSize = 0;

	bool						baked = false;	// If baked reset will affect only the read of the commands.
};

struct CommandBufferRing
{
	void							Create();
	void							Destroy();

	void							ResetPools(U32 frameIndex);

	CommandBuffer*					GetCommandBuffer(U32 frame, bool begin);
	CommandBuffer*					GetCommandBufferInstant(U32 frame, bool begin);

	static U16						PoolFromIndex(U32 index) { return (U16)index / bufferPerPool; }

	static constexpr U16			maxThreads = 1;
	static constexpr U16			maxPools = MAX_SWAPCHAIN_IMAGES * maxThreads;
	static constexpr U16			bufferPerPool = 4;
	static constexpr U16			maxBuffers = bufferPerPool * maxPools;

	VkCommandPool					commandPools[maxPools];
	CommandBuffer					commandBuffers[maxBuffers];
	U8								nextFreePerThreadFrame[maxPools];
};
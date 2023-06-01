#pragma once

#include "Resources\ResourceDefines.hpp"
#include "Memory\Pool.hpp"

//TODO: temp
struct NH_API CommandBuffer
{
	void Create(QueueType type, U32 bufferSize, U32 submitSize, bool baked);
	void Destroy();

	DescriptorSet* CreateDescriptorSet(DescriptorSetCreation& info);
	void BindPass(RenderPass* renderPass);
	void BindPipeline(Pipeline* pipeline);
	void BindVertexBuffer(Buffer* buffer, U32 binding);
	void BindIndexBuffer(Buffer* buffer);
	void BindDescriptorSet(DescriptorSet** sets, U32 numLists, U32* offsets, U32 numOffsets);

	void SetViewport(const Viewport* viewport);
	void SetScissor(const Rect2DInt* rect);

	void Clear(F32 red, F32 green, F32 blue, F32 alpha);
	void ClearDepthStencil(F32 depth, U8 stencil);

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

	VkDescriptorPool			descriptorPool;
	Pool<DescriptorSet, 256>	descriptorSets;

	VkDescriptorSet				vkDescriptorSets[16];

	RenderPass*					currentRenderPass;
	Pipeline*					currentPipeline;
	VkClearValue				clears[2]; //0 = Color, 1 = Depth Stencil
	bool						isRecording;

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
#pragma once

#include "RenderingDefines.hpp"

struct CommandBuffer {

	void Create(QueueType type, U32 bufferSize, U32 submitSize, bool baked);
	void Destroy();

	void BindPass(RenderPassHandle handle);
	void BindPipeline(PipelineHandle handle);
	void BindVertexBuffer(BufferHandle handle, U32 binding, U32 offset);
	void BindIndexBuffer(BufferHandle handle, U32 offset, VkIndexType index_type);
	void BindDescriptorSet(DescriptorSetHandle* handles, U32 num_lists, U32* offsets, U32 num_offsets);

	void SetViewport(const Viewport* viewport);
	void SetScissor(const Rect2DInt* rect);

	void Clear(F32 red, F32 green, F32 blue, F32 alpha);
	void ClearDepthStencil(F32 depth, U8 stencil);

	void Draw(TopologyType topology, U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount);
	void DrawIndexed(TopologyType topology, U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance);
	void DrawIndirect(BufferHandle handle, U32 offset, U32 stride);
	void DrawIndexedIndirect(BufferHandle handle, U32 offset, U32 stride);

	void Dispatch(U32 groupX, U32 groupY, U32 groupZ);
	void DispatchIndirect(BufferHandle handle, U32 offset);

	void Barrier(const ExecutionBarrier& barrier);

	void FillBuffer(BufferHandle buffer, U32 offset, U32 size, U32 data);

	void PushMarker(const char* name);
	void PopMarker();

	void Reset();

	VkCommandBuffer	commandBuffer;

	VkDescriptorSet	descriptorSets[16];

	RenderPass* currentRenderPass;
	Pipeline* currentPipeline;
	VkClearValue	clears[2];          // 0 = color, 1 = depth stencil
	bool			isRecording;

	U32				handle;

	U32				currentCommand;
	ResourceHandle	resourceHandle;
	QueueType		type = QUEUE_TYPE_GRAPHICS;
	U32				bufferSize = 0;

	bool			baked = false;        // If baked reset will affect only the read of the commands.

};
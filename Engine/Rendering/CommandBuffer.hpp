#pragma once

#include "RenderingDefines.hpp"
#include "Resources\ResourceDefines.hpp"

struct Shader;
struct Pipeline;

struct CommandBuffer
{
	void Create(bool baked);
	void Destroy();

	VkResult Begin();
	VkResult End();
	VkResult Submit(VkQueue queue);
	VkResult Submit(VkQueue queue, VkPipelineStageFlags* stageMasks, U32 waitCount, VkSemaphore* waits, U32 signalCount, VkSemaphore* signals);
	VkResult Reset();

	void ClearAttachments(U32 attachmentCount, VkClearAttachment* attachments, U32 rectCount, VkClearRect* rects);

	void BeginRenderpass(Renderpass* renderpass);
	void EndRenderpass();

	void SetViewport(const VkViewport& viewport, const VkRect2D& scissor);
	void BindPipeline(const Pipeline* pipeline);
	void BindIndexBuffer(Shader* shader, const Buffer& buffer);
	void BindVertexBuffer(Shader* shader, const Buffer& buffer);
	void BindInstanceBuffer(Shader* shader, const Buffer& buffer);
	void BindDescriptorSets(Shader* shader, U32 setOffset, U32 setCount, VkDescriptorSet* sets);

	void PushDescriptors();
	void PushConstants(Shader* shader, U32 offset, U32 size, const void* data);

	void Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount);
	void DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance);
	void DrawIndirect(Buffer* buffer, U32 offset, U32 stride);
	void DrawIndexedIndirect(const Buffer& buffer, U32 count, U32 offset = 0);

	void Dispatch(U32 groupX, U32 groupY, U32 groupZ);
	void DispatchIndirect(Buffer* buffer, U32 offset);

	void BufferToImage(const Buffer& buffer, Texture* texture, U32 regionCount, const VkBufferImageCopy* regions);
	void ImageToBuffer(Texture* texture, const Buffer& buffer, U32 regionCount, const VkBufferImageCopy* regions);
	void BufferToBuffer(const Buffer& src, const Buffer& dst, U32 regionCount, const VkBufferCopy* regions);
	void ImageToImage(Texture* src, Texture* dst, U32 regionCount, const VkImageCopy* regions);
	void Blit(Texture* src, Texture* dst, VkFilter filter, U32 blitCount, const VkImageBlit* blits);

	void PipelineBarrier(I32 dependencyFlags, U32 bufferBarrierCount, const VkBufferMemoryBarrier2* bufferBarriers, U32 imageBarrierCount, const VkImageMemoryBarrier2* imageBarriers);

	U32							handle{ U32_MAX };
	bool						baked{ false };

private:
	VkCommandBuffer				commandBuffer{ nullptr };

	friend struct CommandBufferRing;
};

struct CommandBufferRing
{
	void							Create();
	void							Destroy();

	void							ResetPools(U32 frameIndex);

	CommandBuffer*					GetCommandBuffer(U32 frame);
	CommandBuffer*					GetCommandBufferInstant(U32 frame);

	static U16						PoolFromIndex(U32 index) { return (U16)index / bufferPerPool; }

	static constexpr U16			maxThreads = 1;
	static constexpr U16			maxPools = MAX_SWAPCHAIN_IMAGES * maxThreads;
	static constexpr U16			bufferPerPool = 4;
	static constexpr U16			maxBuffers = bufferPerPool * maxPools;

	VkCommandPool					commandPools[maxPools];
	CommandBuffer					commandBuffers[maxBuffers];
	U8								nextFreePerThreadFrame[maxPools];
};
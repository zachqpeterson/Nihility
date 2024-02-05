#pragma once

#include "Resources\ResourceDefines.hpp"

struct Shader;
struct Pipeline;
struct VkQueue_T;
struct VkSemaphore_T;
struct VkDescriptorSet_T;
struct VkCommandBuffer_T;
struct VkCommandPool_T;
struct VkClearAttachment;
struct VkClearRect;
struct VkViewport;
struct VkRect2D;
struct VkBufferImageCopy;
struct VkBufferCopy;
struct VkImageCopy;
struct VkImageBlit;
struct VkBufferMemoryBarrier2;
struct VkImageMemoryBarrier2;
enum VkFilter;
enum VkResult;

struct CommandBuffer
{
	VkResult Begin();
	VkResult End();

	void ClearAttachments(U32 attachmentCount, VkClearAttachment* attachments, U32 rectCount, VkClearRect* rects);

	void BeginRenderpass(Renderpass* renderpass);
	void NextSubpass();
	void EndRenderpass();

	void BindPipeline(const Pipeline* pipeline);
	void BindIndexBuffer(const ResourceRef<Shader>& shader, VkBuffer_T* buffer, U64 offset);
	void BindVertexBuffers(const ResourceRef<Shader>& shader, U32 bufferCount, VkBuffer_T* const* buffers, U64* offsets);
	void BindDescriptorSets(const ResourceRef<Shader>& shader, U32 setOffset, U32 setCount, VkDescriptorSet_T** sets);

	void PushDescriptors();
	void PushConstants(const ResourceRef<Shader>& shader, U32 offset, U32 size, const void* data);

	void Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount);
	void DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance);
	void DrawIndirect(VkBuffer_T* buffer, U32 offset, U32 stride);
	void DrawIndexedIndirect(VkBuffer_T* buffer, U32 count, U32 offset = 0);
	void DrawIndexedIndirectCount(VkBuffer_T* drawBuffer, VkBuffer_T* countBuffer, U32 drawOffset = 0, U32 countOffset = 0);
	void DrawIndirectCount(VkBuffer_T* drawBuffer, VkBuffer_T* countBuffer, U32 drawOffset = 0, U32 countOffset = 0);

	void Dispatch(U32 groupX, U32 groupY, U32 groupZ);
	void DispatchIndirect(const Buffer& buffer, U32 offset);

	void BufferToImage(const Buffer& buffer, const ResourceRef<Texture>& texture, U32 regionCount, const VkBufferImageCopy* regions);
	void ImageToBuffer(const ResourceRef<Texture>& texture, const Buffer& buffer, U32 regionCount, const VkBufferImageCopy* regions);
	void BufferToBuffer(const Buffer& src, const Buffer& dst, U32 regionCount, const VkBufferCopy* regions);
	void ImageToImage(const ResourceRef<Texture>& src, const ResourceRef<Texture>& dst, U32 regionCount, const VkImageCopy* regions);
	void Blit(const ResourceRef<Texture>& src, const ResourceRef<Texture>& dst, VkFilter filter, U32 blitCount, const VkImageBlit* blits);

	void PipelineBarrier(I32 dependencyFlags, U32 bufferBarrierCount, const VkBufferMemoryBarrier2* bufferBarriers, U32 imageBarrierCount, const VkImageMemoryBarrier2* imageBarriers);

private:
	VkCommandBuffer_T* vkCommandBuffer{ nullptr };
	bool recorded{ false };

	friend class Renderer;
	friend struct CommandBufferRing;
};
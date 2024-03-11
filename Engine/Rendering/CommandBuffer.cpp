#include "CommandBuffer.hpp"

#include "RenderingDefines.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Pipeline.hpp"

VkResult CommandBuffer::Begin()
{
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	recorded = true;

	return vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
}

VkResult CommandBuffer::End()
{
	return vkEndCommandBuffer(vkCommandBuffer);
}

void CommandBuffer::ClearAttachments(U32 attachmentCount, VkClearAttachment* attachments, U32 rectCount, VkClearRect* rects)
{
	vkCmdClearAttachments(vkCommandBuffer, attachmentCount, attachments, rectCount, rects);
}

void CommandBuffer::BeginRenderpass(Renderpass* renderpass)
{
	VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBegin.pNext = nullptr;
	renderPassBegin.renderPass = renderpass->renderpass;
	renderPassBegin.framebuffer = renderpass->frameBuffer;
	renderPassBegin.renderArea = TypePun<VkRect2D>(renderpass->renderArea);
	renderPassBegin.clearValueCount = renderpass->clearCount;
	renderPassBegin.pClearValues = (VkClearValue*)renderpass->clearValues;

	vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

	if (renderpass->resize)
	{
		VkViewport viewport{};
		viewport.x = Renderer::renderArea.x;
		viewport.y = Renderer::renderArea.y;
		viewport.width = Renderer::renderArea.z;
		viewport.height = Renderer::renderArea.w;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset.x = (I32)Renderer::renderArea.x;
		scissor.offset.y = (I32)Renderer::renderArea.y;
		scissor.extent.width = (U32)Renderer::renderArea.z;
		scissor.extent.height = (U32)Renderer::renderArea.w;

		vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
	}
	else
	{
		VkViewport viewport{};
		viewport.x = (F32)renderpass->renderArea.offset.x;
		viewport.y = (F32)renderpass->renderArea.offset.y;
		viewport.width = (F32)renderpass->renderArea.extent.x;
		viewport.height = (F32)renderpass->renderArea.extent.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset.x = (I32)renderpass->renderArea.offset.x;
		scissor.offset.y = (I32)renderpass->renderArea.offset.y;
		scissor.extent.width = (U32)renderpass->renderArea.extent.x;
		scissor.extent.height = (U32)renderpass->renderArea.extent.y;

		vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
	}
}

void CommandBuffer::NextSubpass()
{
	vkCmdNextSubpass(vkCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderpass()
{
	vkCmdEndRenderPass(vkCommandBuffer);
}

void CommandBuffer::BindPipeline(const Pipeline* pipeline)
{
	vkCmdBindPipeline(vkCommandBuffer, (VkPipelineBindPoint)pipeline->bindPoint, pipeline->pipeline);
}

void CommandBuffer::BindIndexBuffer(VkBuffer_T* buffer, U64 offset)
{
	vkCmdBindIndexBuffer(vkCommandBuffer, buffer, offset, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindVertexBuffers(U32 bufferCount, VkBuffer_T* const* buffers, U64* offsets)
{
	vkCmdBindVertexBuffers(vkCommandBuffer, 0, bufferCount, buffers, offsets);
}

void CommandBuffer::BindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, U32 setOffset, U32 setCount, VkDescriptorSet_T** sets)
{
	if (setCount)
	{
		vkCmdBindDescriptorSets(vkCommandBuffer, bindPoint, pipelineLayout, setOffset, setCount, sets, 0, nullptr);
	}
}

void CommandBuffer::PushConstants(VkPipelineLayout pipelineLayout, U32 stages, U32 offset, U32 size, const void* data)
{
	vkCmdPushConstants(vkCommandBuffer, pipelineLayout, stages, offset, size, data);
}

void CommandBuffer::Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount)
{
	vkCmdDraw(vkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance)
{
	vkCmdDrawIndexed(vkCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndexedIndirect(VkBuffer_T* buffer, U32 count, U32 offset)
{
	//TODO: Take into account physicalDeviceProperties.limits.maxDrawIndirectCount and physicalDeviceFeatures.drawIndirectFirstInstance;

	if (Renderer::GetDeviceFeatures().multiDrawIndirect)
	{
		vkCmdDrawIndexedIndirect(vkCommandBuffer, buffer, offset, count, sizeof(VkDrawIndexedIndirectCommand));
	}
	else
	{
		for (U32 i = 0; i < count; ++i)
		{
			vkCmdDrawIndexedIndirect(vkCommandBuffer, buffer, sizeof(VkDrawIndexedIndirectCommand) * i + offset, 1, sizeof(VkDrawIndexedIndirectCommand));
		}
	}
}

void CommandBuffer::DrawIndexedIndirectCount(VkBuffer_T* drawBuffer, VkBuffer_T* countBuffer, U32 drawOffset, U32 countOffset)
{
	vkCmdDrawIndexedIndirectCount(vkCommandBuffer, drawBuffer, drawOffset, countBuffer, countOffset, 4096u, sizeof(VkDrawIndexedIndirectCommand));
}

void CommandBuffer::DrawIndirectCount(VkBuffer_T* drawBuffer, VkBuffer_T* countBuffer, U32 drawOffset, U32 countOffset)
{
	vkCmdDrawIndirectCount(vkCommandBuffer, drawBuffer, drawOffset, countBuffer, countOffset, 4096u, sizeof(VkDrawIndexedIndirectCommand));
}

void CommandBuffer::Dispatch(U32 groupX, U32 groupY, U32 groupZ)
{
	vkCmdDispatch(vkCommandBuffer, groupX, groupY, groupZ);
}

void CommandBuffer::BufferToImage(const Buffer& buffer, const ResourceRef<Texture>& texture, U32 regionCount, const VkBufferImageCopy* regions)
{
	vkCmdCopyBufferToImage(vkCommandBuffer, buffer.vkBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, regions);
}

void CommandBuffer::ImageToBuffer(const ResourceRef<Texture>& texture, const Buffer& buffer, U32 regionCount, const VkBufferImageCopy* regions)
{
	vkCmdCopyImageToBuffer(vkCommandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.vkBuffer, regionCount, regions);
}

void CommandBuffer::BufferToBuffer(const Buffer& src, const Buffer& dst, U32 regionCount, const VkBufferCopy* regions)
{
	vkCmdCopyBuffer(vkCommandBuffer, src.vkBuffer, dst.vkBuffer, regionCount, regions);
}

void CommandBuffer::ImageToImage(const ResourceRef<Texture>& src, const ResourceRef<Texture>& dst, U32 regionCount, const VkImageCopy* regions)
{
	vkCmdCopyImage(vkCommandBuffer, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, regions);
}

void CommandBuffer::Blit(const ResourceRef<Texture>& src, const ResourceRef<Texture>& dst, VkFilter filter, U32 blitCount, const VkImageBlit* blits)
{
	vkCmdBlitImage(vkCommandBuffer, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, blitCount, blits, filter);
}

void CommandBuffer::PipelineBarrier(I32 dependencyFlags, U32 bufferBarrierCount, const VkBufferMemoryBarrier2* bufferBarriers, U32 imageBarrierCount, const VkImageMemoryBarrier2* imageBarriers)
{
	VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	dependencyInfo.dependencyFlags = dependencyFlags;
	dependencyInfo.bufferMemoryBarrierCount = bufferBarrierCount;
	dependencyInfo.pBufferMemoryBarriers = bufferBarriers;
	dependencyInfo.imageMemoryBarrierCount = imageBarrierCount;
	dependencyInfo.pImageMemoryBarriers = imageBarriers;

	vkCmdPipelineBarrier2(vkCommandBuffer, &dependencyInfo);
}
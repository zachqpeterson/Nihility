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

void CommandBuffer::SetViewport(const VkViewport& viewport, const VkRect2D& scissor)
{
	vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
}

void CommandBuffer::BeginRenderpass(Renderpass* renderpass)
{
	VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBegin.framebuffer = renderpass->frameBuffer;
	renderPassBegin.renderPass = renderpass->renderpass;

	renderPassBegin.renderArea.offset = { 0, 0 };
	renderPassBegin.renderArea.extent = { Settings::WindowWidth(), Settings::WindowHeight() };

	renderPassBegin.clearValueCount = renderpass->clearCount;
	renderPassBegin.pClearValues = (VkClearValue*)renderpass->clearValues;

	vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
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
	vkCmdBindPipeline(vkCommandBuffer, (VkPipelineBindPoint)pipeline->shader->bindPoint, pipeline->pipeline);
}

void CommandBuffer::BindIndexBuffer(Shader* shader, VkBuffer_T* buffer, U64 offset)
{
	vkCmdBindIndexBuffer(vkCommandBuffer, buffer, offset, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindVertexBuffers(Shader* shader, U32 bufferCount, VkBuffer_T* const* buffers)
{
	U64 offsets[8]{};

	vkCmdBindVertexBuffers(vkCommandBuffer, 0, bufferCount, buffers, offsets);
}

void CommandBuffer::BindDescriptorSets(Shader* shader, U32 setOffset, U32 setCount, VkDescriptorSet_T** sets)
{
	if (setCount)
	{
		vkCmdBindDescriptorSets(vkCommandBuffer, (VkPipelineBindPoint)shader->bindPoint, shader->pipelineLayout, setOffset, setCount, sets, 0, nullptr);
	}
}

void CommandBuffer::PushConstants(Shader* shader, U32 offset, U32 size, const void* data)
{
	vkCmdPushConstants(vkCommandBuffer, shader->pipelineLayout, shader->pushConstantStages, offset, size, data);
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

void CommandBuffer::BufferToImage(const Buffer& buffer, Texture* texture, U32 regionCount, const VkBufferImageCopy* regions)
{
	vkCmdCopyBufferToImage(vkCommandBuffer, buffer.vkBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, regions);
}

void CommandBuffer::ImageToBuffer(Texture* texture, const Buffer& buffer, U32 regionCount, const VkBufferImageCopy* regions)
{
	vkCmdCopyImageToBuffer(vkCommandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.vkBuffer, regionCount, regions);
}

void CommandBuffer::BufferToBuffer(const Buffer& src, const Buffer& dst, U32 regionCount, const VkBufferCopy* regions)
{
	vkCmdCopyBuffer(vkCommandBuffer, src.vkBuffer, dst.vkBuffer, regionCount, regions);
}

void CommandBuffer::ImageToImage(Texture* src, Texture* dst, U32 regionCount, const VkImageCopy* regions)
{
	vkCmdCopyImage(vkCommandBuffer, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, regions);
}

void CommandBuffer::Blit(Texture* src, Texture* dst, VkFilter filter, U32 blitCount, const VkImageBlit* blits)
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
#include "CommandBuffer.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Settings.hpp"
#include "Pipeline.hpp"

void CommandBuffer::Create(VkQueueFlagBits type, bool baked)
{
	this->type = type;
	this->baked = baked;
}

void CommandBuffer::Destroy()
{

}

VkResult CommandBuffer::Begin()
{
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	return vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

VkResult CommandBuffer::End()
{
	return vkEndCommandBuffer(commandBuffer);
}

VkResult CommandBuffer::Submit(VkQueue queue)
{
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	return vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
}

VkResult CommandBuffer::Submit(VkQueue queue, const VkPipelineStageFlags* stageMasks, U32 waitCount, const VkSemaphore* waits, U32 signalCount, const VkSemaphore* signals)
{
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = waitCount;
	submitInfo.pWaitSemaphores = waits;
	submitInfo.pWaitDstStageMask = stageMasks;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = signalCount;
	submitInfo.pSignalSemaphores = signals;

	return vkQueueSubmit(queue, 1, &submitInfo, nullptr);
}

VkResult CommandBuffer::Reset()
{
	return vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

void CommandBuffer::ClearAttachments(U32 attachmentCount, VkClearAttachment* attachments, U32 rectCount, VkClearRect* rects)
{
	vkCmdClearAttachments(commandBuffer, attachmentCount, attachments, rectCount, rects);
}

void CommandBuffer::BeginRenderpass(Renderpass* renderpass)
{
	VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBegin.framebuffer = renderpass->frameBuffer;
	renderPassBegin.renderPass = renderpass->renderpass;

	renderPassBegin.renderArea.offset = { 0, 0 };
	renderPassBegin.renderArea.extent = { Settings::WindowWidth(), Settings::WindowHeight() };

	renderPassBegin.clearValueCount = renderpass->clearCount;
	renderPassBegin.pClearValues = renderpass->clears;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, renderpass->viewport.viewportCount, renderpass->viewport.viewports);
	vkCmdSetScissor(commandBuffer, 0, renderpass->viewport.scissorCount, renderpass->viewport.scissors);
}

void CommandBuffer::EndRenderpass()
{
	vkCmdEndRenderPass(commandBuffer);
}

void CommandBuffer::BindPipeline(const Pipeline* pipeline)
{
	vkCmdBindPipeline(commandBuffer, pipeline->shader->bindPoint, pipeline->pipeline);
}

void CommandBuffer::BindIndexBuffer(Shader* shader, const Buffer& buffer)
{
	vkCmdBindIndexBuffer(commandBuffer, buffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindVertexBuffer(Shader* shader, const Buffer& buffer)
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer.vkBuffer, &offset);
}

void CommandBuffer::BindInstanceBuffer(Shader* shader, const Buffer& buffer)
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &buffer.vkBuffer, &offset);
}

void CommandBuffer::BindDescriptorSets(Shader* shader, U32 setOffset, U32 setCount, const VkDescriptorSet* sets)
{
	vkCmdBindDescriptorSets(commandBuffer, shader->bindPoint, shader->pipelineLayout, setOffset, setCount, sets, 0, nullptr);
}

void CommandBuffer::PushConstants(Shader* shader, U32 offset, U32 size, const void* data)
{
	vkCmdPushConstants(commandBuffer, shader->pipelineLayout, shader->pushConstantStages, offset, size, data);
}

void CommandBuffer::Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance)
{
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndexedIndirect(const Buffer& buffer, U32 count, U32 offset)
{
	//TODO: Take into account physicalDeviceProperties.limits.maxDrawIndirectCount and physicalDeviceFeatures.drawIndirectFirstInstance;

	if (Renderer::physicalDeviceFeatures.multiDrawIndirect)
	{
		vkCmdDrawIndexedIndirect(commandBuffer, buffer.vkBuffer, offset, count, sizeof(VkDrawIndexedIndirectCommand));
	}
	else
	{
		for (U32 i = 0; i < count; ++i)
		{
			vkCmdDrawIndexedIndirect(commandBuffer, buffer.vkBuffer, sizeof(VkDrawIndexedIndirectCommand) * i + offset, 1, sizeof(VkDrawIndexedIndirectCommand));
		}
	}
}

void CommandBuffer::Dispatch(U32 groupX, U32 groupY, U32 groupZ)
{
	vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
}

void CommandBuffer::BufferToImage(const Buffer& buffer, Texture* texture, U32 regionCount, const VkBufferImageCopy* regions)
{
	vkCmdCopyBufferToImage(commandBuffer, buffer.vkBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, regions);
}

void CommandBuffer::ImageToBuffer(Texture* texture, const Buffer& buffer, U32 regionCount, const VkBufferImageCopy* regions)
{
	vkCmdCopyImageToBuffer(commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.vkBuffer, regionCount, regions);
}

void CommandBuffer::BufferToBuffer(const Buffer& src, const Buffer& dst, U32 regionCount, const VkBufferCopy* regions)
{
	vkCmdCopyBuffer(commandBuffer, src.vkBuffer, dst.vkBuffer, regionCount, regions);
}

void CommandBuffer::ImageToImage(Texture* src, Texture* dst, U32 regionCount, const VkImageCopy* regions)
{
	vkCmdCopyImage(commandBuffer, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, regions);
}

void CommandBuffer::Blit(Texture* src, Texture* dst, VkFilter filter, U32 blitCount, const VkImageBlit* blits)
{
	vkCmdBlitImage(commandBuffer, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, blitCount, blits, filter);
}

void CommandBuffer::PipelineBarrier(VkDependencyFlags dependencyFlags, U32 bufferBarrierCount, const VkBufferMemoryBarrier2* bufferBarriers, U32 imageBarrierCount, const VkImageMemoryBarrier2* imageBarriers)
{
	VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
	dependencyInfo.dependencyFlags = dependencyFlags;
	dependencyInfo.bufferMemoryBarrierCount = bufferBarrierCount;
	dependencyInfo.pBufferMemoryBarriers = bufferBarriers;
	dependencyInfo.imageMemoryBarrierCount = imageBarrierCount;
	dependencyInfo.pImageMemoryBarriers = imageBarriers;

	vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

/*------COMMAND BUFFER RING------*/

void CommandBufferRing::Create()
{
	for (U32 i = 0; i < maxPools; ++i)
	{
		VkCommandPoolCreateInfo cmdPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr };
		cmdPoolInfo.queueFamilyIndex = Renderer::queueFamilyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkValidate(vkCreateCommandPool(Renderer::device, &cmdPoolInfo, Renderer::allocationCallbacks, &commandPools[i]));
	}

	for (U32 i = 0; i < maxBuffers; ++i)
	{
		VkCommandBufferAllocateInfo cmd{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr };
		const U32 poolIndex = PoolFromIndex(i);
		cmd.commandPool = commandPools[poolIndex];
		cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd.commandBufferCount = 1;
		VkValidate(vkAllocateCommandBuffers(Renderer::device, &cmd, &commandBuffers[i].commandBuffer));

		//TODO: Have a ring per queue per thread
		commandBuffers[i].handle = i;
		commandBuffers[i].Create(VK_QUEUE_GRAPHICS_BIT, false);
	}
}

void CommandBufferRing::Destroy()
{
	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES * maxThreads; i++)
	{
		vkDestroyCommandPool(Renderer::device, commandPools[i], Renderer::allocationCallbacks);
	}

	for (U32 i = 0; i < maxBuffers; ++i)
	{
		commandBuffers[i].Destroy();
	}
}

void CommandBufferRing::ResetPools(U32 frameIndex)
{
	for (U32 i = 0; i < maxThreads; i++)
	{
		vkResetCommandPool(Renderer::device, commandPools[frameIndex * maxThreads + i], 0);
	}
}

CommandBuffer* CommandBufferRing::GetCommandBuffer(U32 frame)
{
	// TODO: take in account threads
	return &commandBuffers[frame * bufferPerPool];
}

CommandBuffer* CommandBufferRing::GetCommandBufferInstant(U32 frame)
{
	return &commandBuffers[frame * bufferPerPool + 1];
}
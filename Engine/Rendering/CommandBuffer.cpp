#include "CommandBuffer.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Pipeline.hpp"

void CommandBuffer::Create(VkQueueFlagBits type, bool baked)
{
	this->type = type;
	this->baked = baked;
}

void CommandBuffer::Destroy()
{

}

void CommandBuffer::UnbindRenderpass()
{
	vkCmdEndRenderPass(commandBuffer);
}

void CommandBuffer::BindRenderpass(Renderpass* renderpass)
{
	VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBegin.framebuffer = renderpass->frameBuffers[renderpass->tiedToFrame ? Renderer::frameIndex : 0];
	renderPassBegin.renderPass = renderpass->renderpass;

	renderPassBegin.renderArea.offset = { 0, 0 };
	renderPassBegin.renderArea.extent = { renderpass->width, renderpass->height };

	renderPassBegin.clearValueCount = renderpass->clearCount;
	renderPassBegin.pClearValues = renderpass->clears;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, renderpass->viewport.viewportCount, renderpass->viewport.viewports);
	vkCmdSetScissor(commandBuffer, 0, renderpass->viewport.scissorCount, renderpass->viewport.scissors);
}

void CommandBuffer::BindPipeline(Pipeline* pipeline)
{
	vkCmdBindPipeline(commandBuffer, pipeline->shader->bindPoint, pipeline->pipeline);
}

void CommandBuffer::BindIndexBuffer(Buffer& buffer)
{
	vkCmdBindIndexBuffer(commandBuffer, buffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance)
{
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::Dispatch(U32 groupX, U32 groupY, U32 groupZ)
{
	vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
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

CommandBuffer* CommandBufferRing::GetCommandBuffer(U32 frame, bool begin)
{
	// TODO: take in account threads
	CommandBuffer* cb = &commandBuffers[frame * bufferPerPool];

	if (begin)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cb->commandBuffer, &beginInfo);
	}

	return cb;
}

CommandBuffer* CommandBufferRing::GetCommandBufferInstant(U32 frame)
{
	return &commandBuffers[frame * bufferPerPool + 1];
}
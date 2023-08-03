#include "CommandBuffer.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"
#include "Pipeline.hpp"

void CommandBuffer::Create(QueueType type, bool baked)
{
	this->type = type;
	this->baked = baked;

	Reset();
}

void CommandBuffer::Destroy()
{
	Reset();
}

void CommandBuffer::BindPass(Renderpass* renderpass)
{
	if (renderpass != currentRenderPass)
	{
		if (currentRenderPass && currentRenderPass->type != RENDERPASS_TYPE_COMPUTE)
		{
			vkCmdEndRenderPass(commandBuffer);
		}

		if (renderpass->type != RENDERPASS_TYPE_COMPUTE)
		{
			VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			renderPassBegin.framebuffer = renderpass->type == RENDERPASS_TYPE_SWAPCHAIN ? renderpass->frameBuffers[Renderer::imageIndex] : renderpass->frameBuffers[0];
			renderPassBegin.renderPass = renderpass->renderpass;

			renderPassBegin.renderArea.offset = { 0, 0 };
			renderPassBegin.renderArea.extent = { renderpass->width, renderpass->height };

			renderPassBegin.clearValueCount = renderpass->clearCount;
			renderPassBegin.pClearValues = renderpass->clears;

			vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

			currentRenderPass = renderpass;

			vkCmdSetViewport(commandBuffer, 0, renderpass->viewport.viewportCount, renderpass->viewport.viewports);
			vkCmdSetScissor(commandBuffer, 0, renderpass->viewport.scissorCount, renderpass->viewport.scissors);
		}
	}
}

void CommandBuffer::BindPipeline(Pipeline* pipeline)
{
	vkCmdBindPipeline(commandBuffer, pipeline->bindPoint, pipeline->pipeline);

	// Cache pipeline
	currentPipeline = pipeline;
}

void CommandBuffer::BindVertexBuffer(Buffer* buffer, U32 binding)
{
	VkDeviceSize offsets[]{ buffer->globalOffset };
	VkBuffer vkBuffer = buffer->buffer;
	// TODO: add global vertex buffer ?
	if (buffer->parentBuffer)
	{
		vkBuffer = buffer->parentBuffer->buffer;
	}

	vkCmdBindVertexBuffers(commandBuffer, binding, 1, &vkBuffer, offsets);
}

void CommandBuffer::BindIndexBuffer(Buffer* buffer)
{
	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = buffer->globalOffset;

	if (buffer->parentBuffer)
	{
		vkBuffer = buffer->parentBuffer->buffer;
	}

	vkCmdBindIndexBuffer(commandBuffer, vkBuffer, vkOffset, VK_INDEX_TYPE_UINT16);
}

Texture* textures[MAX_DESCRIPTORS_PER_SET]{};
Buffer* buffers[MAX_DESCRIPTORS_PER_SET]{};

DescriptorBinding	bindings[MAX_DESCRIPTORS_PER_SET]{};

void CommandBuffer::BindDescriptorSet(DescriptorSet** sets, U32 numLists, U32* offsets, U32 numOffsets)
{
	U32 offsetsCache[64];
	numOffsets = 0;
	
	for (U32 l = 0; l < numLists; ++l)
	{
		DescriptorSet* descriptorSet = sets[l];
		vkDescriptorSets[l] = descriptorSet->descriptorSet;
	
		for (U32 i = 0, b = 0; i < descriptorSet->bindingCount; ++i)
		{
			const DescriptorBinding& rb = descriptorSet->bindings[i];
	
			if (rb.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				offsetsCache[numOffsets++] = 0;//(U32)descriptorSet->offsetsCache[b++];
			}
		}
	}

	const U32 firstSet = 0;
	vkCmdBindDescriptorSets(commandBuffer, currentPipeline->bindPoint, currentPipeline->layout, firstSet,
		numLists, vkDescriptorSets, numOffsets, offsetsCache);

	if (currentPipeline->useBindless && Renderer::bindlessSupported)
	{
		vkCmdBindDescriptorSets(commandBuffer, currentPipeline->bindPoint, currentPipeline->layout, 1,
			1, &Resources::bindlessDescriptorSet, 0, nullptr);
	}
}

void CommandBuffer::Draw(U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance)
{
	vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndirect(Buffer* buffer, U32 offset, U32 stride)
{
	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = offset;

	vkCmdDrawIndirect(commandBuffer, vkBuffer, vkOffset, 1, sizeof(VkDrawIndirectCommand));
}

void CommandBuffer::DrawIndexedIndirect(Buffer* buffer, U32 offset, U32 stride)
{
	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = offset;

	vkCmdDrawIndexedIndirect(commandBuffer, vkBuffer, vkOffset, 1, sizeof(VkDrawIndirectCommand));
}

void CommandBuffer::Dispatch(U32 groupX, U32 groupY, U32 groupZ)
{
	vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
}

void CommandBuffer::DispatchIndirect(Buffer* buffer, U32 offset)
{
	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = offset;

	vkCmdDispatchIndirect(commandBuffer, vkBuffer, vkOffset);
}

static ResourceType ToResourceState(PipelineStage stage)
{
	static constexpr ResourceType states[]{
		RESOURCE_TYPE_INDIRECT_ARGUMENT,
		RESOURCE_TYPE_VERTEX_AND_CONSTANT_BUFFER,
		RESOURCE_TYPE_NON_PIXEL_SHADER_RESOURCE,
		RESOURCE_TYPE_PIXEL_SHADER_RESOURCE,
		RESOURCE_TYPE_RENDER_TARGET,
		RESOURCE_TYPE_UNORDERED_ACCESS,
		RESOURCE_TYPE_COPY_DEST
	};
	return states[stage];
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

void CommandBuffer::FillBuffer(Buffer* buffer, U32 offset, U32 size, U32 data)
{
	vkCmdFillBuffer(commandBuffer, buffer->buffer, VkDeviceSize(offset), size ? VkDeviceSize(size) : VkDeviceSize(buffer->size), data);
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
		commandBuffers[i].Create(QUEUE_TYPE_GRAPHICS, 0, 0, false);
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
		cb->Reset();

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
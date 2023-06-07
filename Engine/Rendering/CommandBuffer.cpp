#include "CommandBuffer.hpp"

#include "Renderer.hpp"
#include "Resources\Resources.hpp"


void CommandBuffer::Create(QueueType type, U32 bufferSize, U32 submitSize, bool baked)
{
	this->type = type;
	this->bufferSize = bufferSize;
	this->baked = baked;

	//////// Create Descriptor Pools
	static const U32 globalPoolElements = 128;
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, globalPoolElements}
	};
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = globalPoolElements * CountOf32(poolSizes);
	poolInfo.poolSizeCount = CountOf32(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	VkValidateF(vkCreateDescriptorPool(Renderer::device, &poolInfo, Renderer::allocationCallbacks, &descriptorPool));

	descriptorSets.Create();

	Reset();
}

void CommandBuffer::Destroy()
{
	Reset();

	descriptorSets.Destroy();

	vkDestroyDescriptorPool(Renderer::device, descriptorPool, Renderer::allocationCallbacks);
}

DescriptorSet* CommandBuffer::CreateDescriptorSet(DescriptorSetCreation& info)
{
	U64 handle = descriptorSets.ObtainResource();

	DescriptorSet* set = descriptorSets.GetResource(handle);

	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &info.layout->descriptorSetLayout;

	VkValidateF(vkAllocateDescriptorSets(Renderer::device, &allocInfo, &set->descriptorSet));

	// Cache data
	U8* memory;
	Memory::AllocateSize(&memory, (sizeof(void*) + sizeof(Sampler*) + sizeof(U16)) * info.resourceCount);
	set->resources = (void**)memory;
	set->samplers = (Sampler**)(memory + sizeof(void*) * info.resourceCount);
	set->bindings = (U16*)(memory + (sizeof(void*) + sizeof(Sampler*)) * info.resourceCount);
	set->resourceCount = info.resourceCount;
	set->layout = info.layout;

	// Update descriptor set
	VkWriteDescriptorSet descriptorWrite[8];
	VkDescriptorBufferInfo bufferInfo[8];
	VkDescriptorImageInfo imageInfo[8];

	U32 resourceCount = info.resourceCount;
	Renderer::FillWriteDescriptorSets(info.layout, set->descriptorSet, descriptorWrite, bufferInfo, imageInfo, Resources::AccessDefaultSampler()->sampler,
		resourceCount, info.resources, info.samplers, info.bindings);

	// Cache resources
	for (U32 r = 0; r < resourceCount; r++)
	{
		set->resources[r] = info.resources[r];
		set->samplers[r] = info.samplers[r];
		set->bindings[r] = info.bindings[r];
	}

	vkUpdateDescriptorSets(Renderer::device, resourceCount, descriptorWrite, 0, nullptr);

	return set;
}

void CommandBuffer::BindPass(RenderPass* renderPass)
{
	// Begin/End render pass are valid only for graphics render passes.
	if (currentRenderPass && (currentRenderPass->type != RENDER_PASS_TYPE_COMPUTE) && (renderPass != currentRenderPass))
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	if (renderPass != currentRenderPass && (renderPass->type != RENDER_PASS_TYPE_COMPUTE))
	{
		VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBegin.framebuffer = renderPass->type == RENDER_PASS_TYPE_SWAPCHAIN ? renderPass->frameBuffers[Renderer::imageIndex] : renderPass->frameBuffers[0];
		renderPassBegin.renderPass = renderPass->renderPass;

		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.renderArea.extent = { renderPass->width, renderPass->height };

		renderPassBegin.clearValueCount = 2;
		renderPassBegin.pClearValues = clears; //TODO: Move to RenderPass

		vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Cache render pass
	currentRenderPass = renderPass;
}

void CommandBuffer::BindPipeline(Pipeline* pipeline)
{
	vkCmdBindPipeline(commandBuffer, pipeline->bindPoint, pipeline->pipeline);

	// Cache pipeline
	currentPipeline = pipeline;
}

void CommandBuffer::BindVertexBuffer(Buffer* buffer, U32 binding)
{
	VkDeviceSize offsets[] = { buffer->globalOffset };
	VkBuffer vkBuffer = buffer->buffer;
	// TODO: add global vertex buffer ?
	if (buffer->parentBuffer != nullptr)
	{
		vkBuffer = buffer->parentBuffer->buffer;
	}

	vkCmdBindVertexBuffers(commandBuffer, binding, 1, &vkBuffer, offsets);
}

void CommandBuffer::BindIndexBuffer(Buffer* buffer)
{
	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = buffer->globalOffset;

	if (buffer->parentBuffer != nullptr)
	{
		vkBuffer = buffer->parentBuffer->buffer;
	}

	vkCmdBindIndexBuffer(commandBuffer, vkBuffer, vkOffset, VK_INDEX_TYPE_UINT16);
}

void CommandBuffer::BindDescriptorSet(DescriptorSet** sets, U32 numLists, U32* offsets, U32 numOffsets)
{
	// TODO:
	U32 offsetsCache[8];
	numOffsets = 0;

	for (U32 l = 0; l < numLists; ++l)
	{
		DescriptorSet* descriptorSet = sets[l];
		vkDescriptorSets[l] = descriptorSet->descriptorSet;

		// Search for dynamic buffers
		const DescriptorSetLayout* descriptorSetLayout = descriptorSet->layout;
		for (U32 i = 0; i < descriptorSetLayout->bindingCount; ++i)
		{
			const DescriptorBinding& rb = descriptorSetLayout->bindings[i];

			if (rb.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				// Search for the actual buffer offset
				const U32 resourceIndex = descriptorSet->bindings[i];
				Buffer* buffer = (Buffer*)descriptorSet->resources[resourceIndex];

				offsetsCache[numOffsets++] = (U32)buffer->globalOffset;
			}
		}
	}

	const U32 firstSet = 0;
	vkCmdBindDescriptorSets(commandBuffer, currentPipeline->bindPoint, currentPipeline->pipelineLayout, firstSet,
		numLists, vkDescriptorSets, numOffsets, offsetsCache);

	if (Renderer::bindlessSupported)
	{
		vkCmdBindDescriptorSets(commandBuffer, currentPipeline->bindPoint, currentPipeline->pipelineLayout, 1,
			1, &Renderer::bindlessDescriptorSet, 0, nullptr);
	}
}

void CommandBuffer::SetViewport(const Viewport* viewport)
{
	VkViewport vkViewport;

	if (viewport)
	{
		vkViewport.x = viewport->rect.x * 1.0f;
		vkViewport.width = viewport->rect.width * 1.0f;
		// Invert Y with negative height and proper offset - Vulkan has unique Clipping Y.
		vkViewport.y = viewport->rect.height * 1.0f - viewport->rect.y;
		vkViewport.height = -viewport->rect.height * 1.0f;
		vkViewport.minDepth = viewport->minDepth;
		vkViewport.maxDepth = viewport->maxDepth;
	}
	else
	{
		vkViewport.x = 0.f;

		if (currentRenderPass)
		{
			vkViewport.width = currentRenderPass->width * 1.0f;
			// Invert Y with negative height and proper offset - Vulkan has unique Clipping Y.
			vkViewport.y = currentRenderPass->height * 1.0f;
			vkViewport.height = -currentRenderPass->height * 1.0f;
		}
		else
		{
			vkViewport.width = Renderer::swapchain.renderPass->width * 1.0f;
			// Invert Y with negative height and proper offset - Vulkan has unique Clipping Y.
			vkViewport.y = Renderer::swapchain.renderPass->height * 1.0f;
			vkViewport.height = -Renderer::swapchain.renderPass->height * 1.0f;
		}
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;
	}

	vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
}

void CommandBuffer::SetScissor(const Rect2DInt* rect)
{
	VkRect2D vkScissor;

	if (rect)
	{
		vkScissor.offset.x = rect->x;
		vkScissor.offset.y = rect->y;
		vkScissor.extent.width = rect->width;
		vkScissor.extent.height = rect->height;
	}
	else
	{
		vkScissor.offset.x = 0;
		vkScissor.offset.y = 0;
		vkScissor.extent.width = Renderer::swapchain.renderPass->width;
		vkScissor.extent.height = Renderer::swapchain.renderPass->height;
	}

	vkCmdSetScissor(commandBuffer, 0, 1, &vkScissor);
}

void CommandBuffer::Clear(F32 red, F32 green, F32 blue, F32 alpha)
{
	clears[0].color = { red, green, blue, alpha };
}

void CommandBuffer::ClearDepthStencil(F32 depth, U8 stencil)
{
	clears[1].depthStencil.depth = depth;
	clears[1].depthStencil.stencil = stencil;
}

void CommandBuffer::Draw(TopologyType topology, U32 firstVertex, U32 vertexCount, U32 firstInstance, U32 instanceCount)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(TopologyType topology, U32 indexCount, U32 instanceCount, U32 firstIndex, I32 vertexOffset, U32 firstInstance)
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

void CommandBuffer::Barrier(const ExecutionBarrier& barrier)
{
	if (currentRenderPass && (currentRenderPass->type != RENDER_PASS_TYPE_COMPUTE))
	{
		vkCmdEndRenderPass(commandBuffer);
		currentRenderPass = nullptr;
	}

	static VkImageMemoryBarrier imageBarriers[8];
	// TODO: subpass
	if (barrier.newBarrierExperimental != U32_MAX)
	{
		VkPipelineStageFlags sourceStageMask = 0;
		VkPipelineStageFlags destinationStageMask = 0;
		VkAccessFlags sourceAccessFlags = VK_ACCESS_NONE_KHR, destinationAccessFlags = VK_ACCESS_NONE_KHR;

		for (U32 i = 0; i < barrier.textureBarrierCount; ++i)
		{
			Texture* texture = barrier.textureBarriers[i].texture;

			VkImageMemoryBarrier& vkBarrier = imageBarriers[i];
			const bool isColor = !Renderer::HasDepthOrStencil(texture->format);

			{
				VkImageMemoryBarrier* pImageBarrier = &vkBarrier;
				pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				pImageBarrier->pNext = NULL;

				ResourceType currentState = barrier.sourcePipelineStage == PIPELINE_STAGE_RENDER_TARGET ? RESOURCE_TYPE_RENDER_TARGET : RESOURCE_TYPE_PIXEL_SHADER_RESOURCE;
				ResourceType nextState = barrier.destinationPipelineStage == PIPELINE_STAGE_RENDER_TARGET ? RESOURCE_TYPE_RENDER_TARGET : RESOURCE_TYPE_PIXEL_SHADER_RESOURCE;
				if (!isColor)
				{
					currentState = barrier.sourcePipelineStage == PIPELINE_STAGE_RENDER_TARGET ? RESOURCE_TYPE_DEPTH_WRITE : RESOURCE_TYPE_DEPTH_READ;
					nextState = barrier.destinationPipelineStage == PIPELINE_STAGE_RENDER_TARGET ? RESOURCE_TYPE_DEPTH_WRITE : RESOURCE_TYPE_DEPTH_READ;
				}

				pImageBarrier->srcAccessMask = ToVkAccessFlags(currentState);
				pImageBarrier->dstAccessMask = ToVkAccessFlags(nextState);
				pImageBarrier->oldLayout = ToVkImageLayout(currentState);
				pImageBarrier->newLayout = ToVkImageLayout(nextState);

				pImageBarrier->image = texture->image;
				pImageBarrier->subresourceRange.aspectMask = isColor ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				pImageBarrier->subresourceRange.baseMipLevel = 0;
				pImageBarrier->subresourceRange.levelCount = 1;
				pImageBarrier->subresourceRange.baseArrayLayer = 0;
				pImageBarrier->subresourceRange.layerCount = 1;

				{
					pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				}

				sourceAccessFlags |= pImageBarrier->srcAccessMask;
				destinationAccessFlags |= pImageBarrier->dstAccessMask;
			}

			vkBarrier.oldLayout = texture->imageLayout;
			texture->imageLayout = vkBarrier.newLayout;
		}

		static VkBufferMemoryBarrier bufferMemoryBarriers[8];
		for (U32 i = 0; i < barrier.bufferBarrierCount; ++i)
		{
			VkBufferMemoryBarrier& vkBarrier = bufferMemoryBarriers[i];
			vkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

			Buffer* buffer = barrier.bufferBarriers[i].buffer;

			vkBarrier.buffer = buffer->buffer;
			vkBarrier.offset = 0;
			vkBarrier.size = buffer->size;

			ResourceType currentState = ToResourceState(barrier.sourcePipelineStage);
			ResourceType nextState = ToResourceState(barrier.destinationPipelineStage);
			vkBarrier.srcAccessMask = ToVkAccessFlags(currentState);
			vkBarrier.dstAccessMask = ToVkAccessFlags(nextState);

			sourceAccessFlags |= vkBarrier.srcAccessMask;
			destinationAccessFlags |= vkBarrier.dstAccessMask;

			vkBarrier.srcQueueFamilyIndex = 0;
			vkBarrier.dstQueueFamilyIndex = 0;
		}

		sourceStageMask = DeterminePipelineStageFlags(sourceAccessFlags, barrier.sourcePipelineStage == PIPELINE_STAGE_COMPUTE_SHADER ? QUEUE_TYPE_COMPUTE : QUEUE_TYPE_GRAPHICS);
		destinationStageMask = DeterminePipelineStageFlags(destinationAccessFlags, barrier.destinationPipelineStage == PIPELINE_STAGE_COMPUTE_SHADER ? QUEUE_TYPE_COMPUTE : QUEUE_TYPE_GRAPHICS);

		vkCmdPipelineBarrier(commandBuffer, sourceStageMask, destinationStageMask, 0, 0, nullptr, barrier.bufferBarrierCount, bufferMemoryBarriers, barrier.textureBarrierCount, imageBarriers);
		return;
	}

	VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkImageLayout newDepthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	VkAccessFlags sourceAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags sourceBufferAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags sourceDepthAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	VkAccessFlags destinationAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags destinationBufferAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags destinationDepthAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	switch (barrier.destinationPipelineStage)
	{
	case PIPELINE_STAGE_FRAGMENT_SHADER: {
		//newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	} break;
	case PIPELINE_STAGE_COMPUTE_SHADER: {
		newLayout = VK_IMAGE_LAYOUT_GENERAL;
	} break;
	case PIPELINE_STAGE_RENDER_TARGET: {
		newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		newDepthLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		destinationAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		destinationDepthAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	} break;

	case PIPELINE_STAGE_DRAW_INDIRECT: {
		destinationBufferAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	} break;
	}

	switch (barrier.sourcePipelineStage)
	{
	case PIPELINE_STAGE_FRAGMENT_SHADER: {
		//sourceAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	} break;
	case PIPELINE_STAGE_COMPUTE_SHADER: { } break;
	case PIPELINE_STAGE_RENDER_TARGET: {
		sourceAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		sourceDepthAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	} break;
	case PIPELINE_STAGE_DRAW_INDIRECT: {
		sourceBufferAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	} break;
	}

	bool hasDepth = false;

	for (U32 i = 0; i < barrier.textureBarrierCount; ++i)
	{
		Texture* texture = barrier.textureBarriers[i].texture;

		VkImageMemoryBarrier& vkBarrier = imageBarriers[i];
		vkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		const bool isColor = !Renderer::HasDepthOrStencil(texture->format);
		hasDepth = hasDepth || !isColor;

		vkBarrier.image = texture->image;
		vkBarrier.subresourceRange.aspectMask = isColor ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		vkBarrier.subresourceRange.baseMipLevel = 0;
		vkBarrier.subresourceRange.levelCount = 1;
		vkBarrier.subresourceRange.baseArrayLayer = 0;
		vkBarrier.subresourceRange.layerCount = 1;

		vkBarrier.oldLayout = texture->imageLayout;

		// Transition to...
		vkBarrier.newLayout = isColor ? newLayout : newDepthLayout;

		vkBarrier.srcAccessMask = isColor ? sourceAccessMask : sourceDepthAccessMask;
		vkBarrier.dstAccessMask = isColor ? destinationAccessMask : destinationDepthAccessMask;

		texture->imageLayout = vkBarrier.newLayout;
	}

	VkPipelineStageFlags sourceStageMask = ToVkPipelineStage((PipelineStage)barrier.sourcePipelineStage);
	VkPipelineStageFlags destinationStageMask = ToVkPipelineStage((PipelineStage)barrier.destinationPipelineStage);

	if (hasDepth)
	{
		sourceStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		destinationStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}

	static VkBufferMemoryBarrier bufferMemoryBarriers[8];
	for (U32 i = 0; i < barrier.bufferBarrierCount; ++i)
	{
		VkBufferMemoryBarrier& vkBarrier = bufferMemoryBarriers[i];
		vkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

		Buffer* buffer = barrier.bufferBarriers[i].buffer;

		vkBarrier.buffer = buffer->buffer;
		vkBarrier.offset = 0;
		vkBarrier.size = buffer->size;
		vkBarrier.srcAccessMask = sourceBufferAccessMask;
		vkBarrier.dstAccessMask = destinationBufferAccessMask;

		vkBarrier.srcQueueFamilyIndex = 0;
		vkBarrier.dstQueueFamilyIndex = 0;
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStageMask, destinationStageMask, 0, 0, nullptr, barrier.bufferBarrierCount, bufferMemoryBarriers, barrier.textureBarrierCount, imageBarriers);
}

void CommandBuffer::FillBuffer(Buffer* buffer, U32 offset, U32 size, U32 data)
{
	vkCmdFillBuffer(commandBuffer, buffer->buffer, VkDeviceSize(offset), size ? VkDeviceSize(size) : VkDeviceSize(buffer->size), data);
}

void CommandBuffer::PushMarker(const char* name)
{
	Renderer::PushGpuTimestamp(this, name);

	if (!Renderer::debugUtilsExtensionPresent) { return; }

	Renderer::PushMarker(commandBuffer, name);
}

void CommandBuffer::PopMarker()
{
	Renderer::PopGpuTimestamp(this);

	if (!Renderer::debugUtilsExtensionPresent) { return; }

	Renderer::PopMarker(commandBuffer);
}

void CommandBuffer::Reset()
{
	currentRenderPass = nullptr;
	currentPipeline = nullptr;
	currentCommand = 0;

	vkResetDescriptorPool(Renderer::device, descriptorPool, 0);

	U64 resourceCount = descriptorSets.lastFree;
	for (U64 i = 0; i < resourceCount; ++i)
	{
		DescriptorSet* descriptorSet = (DescriptorSet*)descriptorSets.GetResource(i);

		if (descriptorSet) { Memory::FreeSize(&descriptorSet->resources); }

		descriptorSets.ReleaseResource(i);
	}
}

/*------COMMAND BUFFER RING------*/

void CommandBufferRing::Create()
{
	for (U32 i = 0; i < maxPools; ++i)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr };
		cmdPoolInfo.queueFamilyIndex = Renderer::queueFamilyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkValidate(vkCreateCommandPool(Renderer::device, &cmdPoolInfo, Renderer::allocationCallbacks, &commandPools[i]));
	}

	for (U32 i = 0; i < maxBuffers; ++i)
	{
		VkCommandBufferAllocateInfo cmd = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr };
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

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cb->commandBuffer, &beginInfo);
	}

	return cb;
}

CommandBuffer* CommandBufferRing::GetCommandBufferInstant(U32 frame, bool begin)
{
	CommandBuffer* cb = &commandBuffers[frame * bufferPerPool + 1];
	return cb;
}
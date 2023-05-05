#include "CommandBuffer.hpp"

#include "Renderer.hpp"

void CommandBuffer::Create(QueueType type, U32 bufferSize, U32 submitSize, bool baked)
{
	this->type = type;
	this->bufferSize = bufferSize;
	this->baked = baked;

	Reset();
}

void CommandBuffer::Destroy()
{
	isRecording = false;
}

void CommandBuffer::BindPass(RenderPassHandle handle)
{
	//if (!isRecording)
	{
		isRecording = true;

		RenderPass* renderPass = Renderer::AccessRenderPass(handle);

		// Begin/End render pass are valid only for graphics render passes.
		if (currentRenderPass && (currentRenderPass->type != RENDER_PASS_TYPE_COMPUTE) && (renderPass != currentRenderPass))
		{
			vkCmdEndRenderPass(commandBuffer);
		}

		if (renderPass != currentRenderPass && (renderPass->type != RENDER_PASS_TYPE_COMPUTE))
		{
			VkRenderPassBeginInfo render_pass_begin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			render_pass_begin.framebuffer = renderPass->type == RENDER_PASS_TYPE_SWAPCHAIN ? Renderer::swapchainFramebuffers[Renderer::imageIndex] : renderPass->frameBuffer;
			render_pass_begin.renderPass = renderPass->renderPass;

			render_pass_begin.renderArea.offset = { 0, 0 };
			render_pass_begin.renderArea.extent = { renderPass->width, renderPass->height };

			// TODO: this breaks.
			render_pass_begin.clearValueCount = 2;// renderPass->output.color_operation ? 2 : 0;
			render_pass_begin.pClearValues = clears;

			vkCmdBeginRenderPass(commandBuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Cache render pass
		currentRenderPass = renderPass;
	}
}

void CommandBuffer::BindPipeline(PipelineHandle handle)
{
	Pipeline* pipeline = Renderer::AccessPipeline(handle);
	vkCmdBindPipeline(commandBuffer, pipeline->bindPoint, pipeline->pipeline);

	// Cache pipeline
	currentPipeline = pipeline;
}

void CommandBuffer::BindVertexBuffer(BufferHandle handle, U32 binding, U32 offset)
{
	Buffer* buffer = Renderer::AccessBuffer(handle);
	VkDeviceSize offsets[] = { offset };

	VkBuffer vk_buffer = buffer->buffer;
	// TODO: add global vertex buffer ?
	if (buffer->parentBuffer.index != INVALID_INDEX)
	{
		Buffer* parent_buffer = Renderer::AccessBuffer(buffer->parentBuffer);
		vk_buffer = parent_buffer->buffer;
		offsets[0] = buffer->globalOffset;
	}

	vkCmdBindVertexBuffers(commandBuffer, binding, 1, &vk_buffer, offsets);
}

void CommandBuffer::BindIndexBuffer(BufferHandle handle, U32 offset, VkIndexType indexType)
{
	Buffer* buffer = Renderer::AccessBuffer(handle);

	VkBuffer vk_buffer = buffer->buffer;
	VkDeviceSize offset = offset;
	if (buffer->parentBuffer.index != INVALID_INDEX)
	{
		Buffer* parent_buffer = Renderer::AccessBuffer(buffer->parentBuffer);
		vk_buffer = parent_buffer->buffer;
		offset = buffer->globalOffset;
	}
	vkCmdBindIndexBuffer(commandBuffer, vk_buffer, offset, indexType);
}

void CommandBuffer::BindDescriptorSet(DescriptorSetHandle* handles, U32 numLists, U32* offsets, U32 numOffsets)
{
	// TODO:
	U32 offsetsCache[8];
	numOffsets = 0;

	for (U32 l = 0; l < numLists; ++l)
	{
		DesciptorSet* descriptorSet = Renderer::AccessDescriptorSet(handles[l]);
		descriptorSets[l] = descriptorSet->descriptorSet;

		// Search for dynamic buffers
		const DesciptorSetLayout* descriptorSetLayout = descriptorSet->layout;
		for (U32 i = 0; i < descriptorSetLayout->numBindings; ++i)
		{
			const DescriptorBinding& rb = descriptorSetLayout->bindings[i];

			if (rb.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				// Search for the actual buffer offset
				const U32 resourceIndex = descriptorSet->bindings[i];
				ResourceHandle bufferHandle = descriptorSet->resources[resourceIndex];
				Buffer* buffer = Renderer::AccessBuffer({ bufferHandle });

				offsetsCache[numOffsets++] = buffer->globalOffset;
			}
		}
	}

	const U32 firstSet = 0;
	vkCmdBindDescriptorSets(commandBuffer, currentPipeline->bindPoint, currentPipeline->pipelineLayout, firstSet,
		numLists, descriptorSets, numOffsets, offsetsCache);
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
		vkViewport.minDepth = viewport->min_depth;
		vkViewport.maxDepth = viewport->max_depth;
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
			vkViewport.width = Renderer::swapchainWidth * 1.0f;
			// Invert Y with negative height and proper offset - Vulkan has unique Clipping Y.
			vkViewport.y = Renderer::swapchainHeight * 1.0f;
			vkViewport.height = -Renderer::swapchainHeight * 1.0f;
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
		vkScissor.extent.width = Renderer::swapchainWidth;
		vkScissor.extent.height = Renderer::swapchainHeight;
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

void CommandBuffer::DrawIndirect(BufferHandle handle, U32 offset, U32 stride)
{
	Buffer* buffer = Renderer::AccessBuffer(handle);

	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = offset;

	vkCmdDrawIndirect(commandBuffer, vkBuffer, vkOffset, 1, sizeof(VkDrawIndirectCommand));
}

void CommandBuffer::DrawIndexedIndirect(BufferHandle handle, U32 offset, U32 stride)
{
	Buffer* buffer = Renderer::AccessBuffer(handle);

	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = offset;

	vkCmdDrawIndexedIndirect(commandBuffer, vkBuffer, vkOffset, 1, sizeof(VkDrawIndirectCommand));
}

void CommandBuffer::Dispatch(U32 groupX, U32 groupY, U32 groupZ)
{
	vkCmdDispatch(commandBuffer, groupX, groupY, groupZ);
}

void CommandBuffer::DispatchIndirect(BufferHandle handle, U32 offset)
{
	Buffer* buffer = Renderer::AccessBuffer(handle);

	VkBuffer vkBuffer = buffer->buffer;
	VkDeviceSize vkOffset = offset;

	vkCmdDispatchIndirect(commandBuffer, vkBuffer, vkOffset);
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

		for (U32 i = 0; i < barrier.numTextureBarriers; ++i)
		{

			Texture* texture_vulkan = Renderer::AccessTexture(barrier.textureBarriers[i].texture);

			VkImageMemoryBarrier& vk_barrier = imageBarriers[i];
			const bool is_color = !Renderer::HasDepthOrStencil(texture_vulkan->format);

			{
				VkImageMemoryBarrier* pImageBarrier = &vk_barrier;
				pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				pImageBarrier->pNext = NULL;

				ResourceState current_state = barrier.source_pipeline_stage == PipelineStage::RenderTarget ? RESOURCE_STATE_RENDER_TARGET : RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				ResourceState next_state = barrier.destination_pipeline_stage == PipelineStage::RenderTarget ? RESOURCE_STATE_RENDER_TARGET : RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				if (!is_color)
				{
					current_state = barrier.source_pipeline_stage == PipelineStage::RenderTarget ? RESOURCE_STATE_DEPTH_WRITE : RESOURCE_STATE_DEPTH_READ;
					next_state = barrier.destination_pipeline_stage == PipelineStage::RenderTarget ? RESOURCE_STATE_DEPTH_WRITE : RESOURCE_STATE_DEPTH_READ;
				}

				pImageBarrier->srcAccessMask = util_to_vk_access_flags(current_state);
				pImageBarrier->dstAccessMask = util_to_vk_access_flags(next_state);
				pImageBarrier->oldLayout = util_to_vk_image_layout(current_state);
				pImageBarrier->newLayout = util_to_vk_image_layout(next_state);

				pImageBarrier->image = texture_vulkan->image;
				pImageBarrier->subresourceRange.aspectMask = is_color ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				pImageBarrier->subresourceRange.baseMipLevel = 0;
				pImageBarrier->subresourceRange.levelCount = 1;
				pImageBarrier->subresourceRange.baseArrayLayer = 0;
				pImageBarrier->subresourceRange.layerCount = 1;

				{
					pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				}

				source_access_flags |= pImageBarrier->srcAccessMask;
				destination_access_flags |= pImageBarrier->dstAccessMask;
			}

			vk_barrier.oldLayout = texture_vulkan->imageLayout;
			texture_vulkan->imageLayout = vk_barrier.newLayout;
		}

		static VkBufferMemoryBarrier buffer_memory_barriers[8];
		for (U32 i = 0; i < barrier.numBufferBarriers; ++i)
		{
			VkBufferMemoryBarrier& vk_barrier = buffer_memory_barriers[i];
			vk_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

			Buffer* buffer = device->access_buffer(barrier.bufferBarriers[i].buffer);

			vk_barrier.buffer = buffer->buffer;
			vk_barrier.offset = 0;
			vk_barrier.size = buffer->size;

			ResourceState current_state = to_resource_state(barrier.source_pipeline_stage);
			ResourceState next_state = to_resource_state(barrier.destination_pipeline_stage);
			vk_barrier.srcAccessMask = util_to_vk_access_flags(current_state);
			vk_barrier.dstAccessMask = util_to_vk_access_flags(next_state);

			source_access_flags |= vk_barrier.srcAccessMask;
			destination_access_flags |= vk_barrier.dstAccessMask;

			vk_barrier.srcQueueFamilyIndex = 0;
			vk_barrier.dstQueueFamilyIndex = 0;
		}

		sourceStageMask = util_determine_pipeline_stage_flags(source_access_flags, barrier.source_pipeline_stage == PipelineStage::ComputeShader ? QueueType::Compute : QueueType::Graphics);
		destination_stage_mask = util_determine_pipeline_stage_flags(destination_access_flags, barrier.destination_pipeline_stage == PipelineStage::ComputeShader ? QueueType::Compute : QueueType::Graphics);

		vkCmdPipelineBarrier(commandBuffer, sourceStageMask, destination_stage_mask, 0, 0, nullptr, barrier.numBufferBarriers, buffer_memory_barriers, barrier.numTextureBarriers, image_barriers);
		return;
	}

	VkImageLayout new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkImageLayout new_depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	VkAccessFlags source_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags source_buffer_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags source_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	VkAccessFlags destination_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags destination_buffer_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	VkAccessFlags destination_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	switch (barrier.destination_pipeline_stage)
	{

	case PipelineStage::FragmentShader:
	{
		//new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		break;
	}

	case PipelineStage::ComputeShader:
	{
		new_layout = VK_IMAGE_LAYOUT_GENERAL;


		break;
	}

	case PipelineStage::RenderTarget:
	{
		new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		new_depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		destination_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		destination_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		break;
	}

	case PipelineStage::DrawIndirect:
	{
		destination_buffer_access_mask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		break;
	}
	}

	switch (barrier.source_pipeline_stage)
	{

	case PipelineStage::FragmentShader:
	{
		//source_access_mask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;
	}

	case PipelineStage::ComputeShader:
	{

		break;
	}

	case PipelineStage::RenderTarget:
	{
		source_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		source_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		break;
	}

	case PipelineStage::DrawIndirect:
	{
		source_buffer_access_mask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		break;
	}
	}

	bool has_depth = false;

	for (U32 i = 0; i < barrier.num_image_barriers; ++i)
	{

		Texture* texture_vulkan = device->access_texture(barrier.imageBarriers[i].texture);

		VkImageMemoryBarrier& vk_barrier = imageBarriers[i];
		vk_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		vk_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vk_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		const bool is_color = !TextureFormat::has_depth_or_stencil(texture_vulkan->format);
		has_depth = has_depth || !is_color;

		vk_barrier.image = texture_vulkan->image;
		vk_barrier.subresourceRange.aspectMask = is_color ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		vk_barrier.subresourceRange.baseMipLevel = 0;
		vk_barrier.subresourceRange.levelCount = 1;
		vk_barrier.subresourceRange.baseArrayLayer = 0;
		vk_barrier.subresourceRange.layerCount = 1;

		vk_barrier.oldLayout = texture_vulkan->imageLayout;

		// Transition to...
		vk_barrier.newLayout = is_color ? new_layout : new_depth_layout;

		vk_barrier.srcAccessMask = is_color ? source_access_mask : source_depth_access_mask;
		vk_barrier.dstAccessMask = is_color ? destination_access_mask : destination_depth_access_mask;

		texture_vulkan->imageLayout = vk_barrier.newLayout;
	}

	VkPipelineStageFlags source_stage_mask = to_vk_pipeline_stage((PipelineStage)barrier.source_pipeline_stage);
	VkPipelineStageFlags destination_stage_mask = to_vk_pipeline_stage((PipelineStage)barrier.destination_pipeline_stage);

	if (has_depth)
	{

		source_stage_mask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		destination_stage_mask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}

	static VkBufferMemoryBarrier buffer_memory_barriers[8];
	for (U32 i = 0; i < barrier.numBufferBarriers; ++i)
	{
		VkBufferMemoryBarrier& vk_barrier = buffer_memory_barriers[i];
		vk_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

		Buffer* buffer = device->access_buffer(barrier.bufferBarriers[i].buffer);

		vk_barrier.buffer = buffer->buffer;
		vk_barrier.offset = 0;
		vk_barrier.size = buffer->size;
		vk_barrier.srcAccessMask = source_buffer_access_mask;
		vk_barrier.dstAccessMask = destination_buffer_access_mask;

		vk_barrier.srcQueueFamilyIndex = 0;
		vk_barrier.dstQueueFamilyIndex = 0;
	}

	vkCmdPipelineBarrier(commandBuffer, source_stage_mask, destination_stage_mask, 0, 0, nullptr, barrier.numBufferBarriers, buffer_memory_barriers, barrier.numTextureBarriers, imageBarriers);
}

void CommandBuffer::FillBuffer(BufferHandle buffer, U32 offset, U32 size, U32 data)
{
	Buffer* vkBuffer = Renderer::AccessBuffer(buffer);

	vkCmdFillBuffer(commandBuffer, vkBuffer->buffer, VkDeviceSize(offset), size ? VkDeviceSize(size) : VkDeviceSize(vkBuffer->size), data);
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
	isRecording = false;
	currentRenderPass = nullptr;
	currentPipeline = nullptr;
	currentCommand = 0;
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

        commandBuffers[i].handle = i;
        commandBuffers[i].Reset();
    }
}

void CommandBufferRing::Destroy()
{
    for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES * maxThreads; i++)
    {
        vkDestroyCommandPool(Renderer::device, commandPools[i], Renderer::allocationCallbacks);
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
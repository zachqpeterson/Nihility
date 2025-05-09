#include "Renderer.hpp"

#include "Platform/Platform.hpp"
#include "Core/Time.hpp"
#include "Math/Math.hpp"
#include "Resources/Resources.hpp"

#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

VmaAllocator Renderer::vmaAllocator;
VkAllocationCallbacks* Renderer::allocationCallbacks = VK_NULL_HANDLE;
Instance Renderer::instance;
Device Renderer::device;
VkQueue Renderer::graphicsQueue;
VkQueue Renderer::presentQueue;
Swapchain Renderer::swapchain;
VkCommandPool Renderer::commandPool = VK_NULL_HANDLE;
CommandBuffer Renderer::renderCommandBuffers[MaxSwapchainImages];
VkDescriptorPool Renderer::vkDescriptorPool = VK_NULL_HANDLE;
VkDescriptorPool Renderer::vkBindlessDescriptorPool = VK_NULL_HANDLE;
DescriptorSet Renderer::descriptorSet;
Renderpass Renderer::renderpass;
FrameBuffer Renderer::frameBuffer;
VkSemaphore Renderer::presentSemaphore = VK_NULL_HANDLE;
VkSemaphore Renderer::renderSemaphore = VK_NULL_HANDLE;
GlobalPushConstant Renderer::globalPushConstant;
Scene* Renderer::scene;

U32 Renderer::frameIndex;
U32 Renderer::imageIndex;
U32 Renderer::previousFrame;
VkSemaphore Renderer::imageAvailable[MaxSwapchainImages];
VkSemaphore Renderer::vertexInputFinished[MaxSwapchainImages];
VkFence Renderer::inFlight[MaxSwapchainImages];

Texture Renderer::depthTextures[MaxSwapchainImages];

bool Renderer::Initialize()
{
	Logger::Trace("Initializing Renderer...");

	if (!instance.Create()) { Logger::Fatal("Failed To Create Vulkan Instance!"); return false; }
	if (!device.Create()) { Logger::Fatal("Failed To Create Vulkan Device!"); return false; }
	if (!InitializeVma()) { Logger::Fatal("Failed To Initialize Vma!"); return false; }
	if (!GetQueues()) { return false; }
	if (!swapchain.Create(false)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateDepthTextures()) { Logger::Fatal("Failed To Create Depth Buffers!"); return false; }
	if (!(commandPool = CreateCommandPool(QueueType::Graphics))) { Logger::Fatal("Failed To Create Command Pool!"); return false; }
	if (!renderCommandBuffers[0].Create(commandPool)) { Logger::Fatal("Failed To Create Command Buffer!"); return false; }
	if (!renderCommandBuffers[1].Create(commandPool)) { Logger::Fatal("Failed To Create Command Buffer!"); return false; }
	if (!renderCommandBuffers[2].Create(commandPool)) { Logger::Fatal("Failed To Create Command Buffer!"); return false; }
	if (!CreateDescriptorPool()) { Logger::Fatal("Failed To Create Descriptor Pool!"); return false; }
	if (!CreateRenderpasses()) { Logger::Fatal("Failed To Create Renderpasses!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }
	if (!CreateSynchronization()) { Logger::Fatal("Failed To Create Synchronization Objects!"); return false; }

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Renderer...");

	vkDeviceWaitIdle(device);

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkDestroySemaphore(device, imageAvailable[i], allocationCallbacks);
		vkDestroySemaphore(device, vertexInputFinished[i], allocationCallbacks);
		vkDestroyFence(device, inFlight[i], allocationCallbacks);
	}

	frameBuffer.Destroy();

	renderpass.Destroy();

	vkDestroyDescriptorPool(device, vkDescriptorPool, allocationCallbacks);
	vkDestroyDescriptorPool(device, vkBindlessDescriptorPool, allocationCallbacks);

	renderCommandBuffers[0].Destroy();
	renderCommandBuffers[1].Destroy();
	renderCommandBuffers[2].Destroy();

	DestroyCommandPool(commandPool);

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkDestroyImageView(device, depthTextures[i].imageView, allocationCallbacks);
		vmaDestroyImage(vmaAllocator, depthTextures[i].image, depthTextures[i].allocation);
	}

	swapchain.Destroy();

	vmaDestroyAllocator(vmaAllocator);

	device.Destroy();

	instance.Destroy();
}

void Renderer::Update()
{
	Synchronize();

	//TODO: Reset command buffer and descriptor pool

	Resources::Update();
	scene->Update();

	globalPushConstant.viewProjection = scene->camera.ViewProjection();

	renderCommandBuffers[imageIndex].Reset();
	renderCommandBuffers[imageIndex].BeginSingleShot();

	VkClearValue colorClearValue;
	colorClearValue.color = { { 0.25f, 0.25f, 0.25f, 1.0f } };

	VkClearValue depthValue;
	depthValue.depthStencil.depth = 1.0f;

	Vector<VkClearValue> clearValues = { colorClearValue, depthValue };

	VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderpass;
	renderPassBeginInfo.framebuffer = frameBuffer.vkFramebuffers[frameIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = swapchain.extent;
	renderPassBeginInfo.clearValueCount = (U32)clearValues.Size();
	renderPassBeginInfo.pClearValues = clearValues.Data();

	vkCmdBeginRenderPass(renderCommandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (F32)swapchain.extent.width;
	viewport.height = (F32)swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.extent;

	vkCmdSetViewport(renderCommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(renderCommandBuffers[imageIndex], 0, 1, &scissor);

	scene->Render(renderCommandBuffers[imageIndex]);

	//TODO: Draw UI

	vkCmdEndRenderPass(renderCommandBuffers[imageIndex]);

	renderCommandBuffers[imageIndex].End();

	Submit();
}

void Renderer::Synchronize()
{
	vkWaitForFences(device, 1, &inFlight[frameIndex], VK_TRUE, UINT64_MAX);

	VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable[frameIndex], VK_NULL_HANDLE, &imageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) { RecreateSwapchain(); }
	else { VkValidateFExit(res); }
}

void Renderer::Submit()
{
	vkResetFences(device, 1, &inFlight[frameIndex]);

	VkSemaphore waits[]{ imageAvailable[frameIndex] };
	VkSemaphore signals[]{ vertexInputFinished[frameIndex] };
	VkPipelineStageFlags submitStageMasks[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = CountOf32(waits);
	submitInfo.pWaitSemaphores = waits;
	submitInfo.pWaitDstStageMask = submitStageMasks;
	submitInfo.commandBufferCount = 1u;
	submitInfo.pCommandBuffers = &renderCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = CountOf32(signals);
	submitInfo.pSignalSemaphores = signals;

	VkValidateFExit(vkQueueSubmit(graphicsQueue, 1u, &submitInfo, inFlight[frameIndex]));

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = CountOf32(signals);
	presentInfo.pWaitSemaphores = signals;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &frameIndex;
	presentInfo.pResults = nullptr;

	VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) { RecreateSwapchain(); }

	previousFrame = frameIndex;

	++frameIndex %= swapchain.imageCount;
}

void Renderer::SetScene(Scene* _scene)
{
	scene = _scene;
}

bool Renderer::InitializeVma()
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = device.physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

	VkValidateFR(vmaCreateAllocator(&allocatorInfo, &vmaAllocator));

	return true;
}

bool Renderer::GetQueues()
{
	graphicsQueue = device.GetQueue(QueueType::Graphics);
	if (graphicsQueue == nullptr)
	{
		Logger::Fatal("Failed To Get Graphics Queue!");
		return false;
	}

	presentQueue = device.GetQueue(QueueType::Present);
	if (presentQueue == nullptr)
	{
		Logger::Fatal("Failed To Get Present Queue!");
		return false;
	}

	return true;
}

bool Renderer::CreateDepthTextures()
{
	VkExtent3D depthImageExtent = {
		swapchain.extent.width,
		swapchain.extent.height,
		1
	};

	VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_D32_SFLOAT;
	imageCreateInfo.extent = depthImageExtent;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.flags = 0;
	allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	allocationInfo.preferredFlags = 0;
	allocationInfo.memoryTypeBits = 0;
	allocationInfo.pool = nullptr;
	allocationInfo.pUserData = nullptr;
	allocationInfo.priority = 0;

	VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.components = {};
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		VkValidateFR(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocationInfo, &depthTextures[i].image, &depthTextures[i].allocation, nullptr));
		imageViewCreateInfo.image = depthTextures[i].image;
		VkValidateFR(vkCreateImageView(device, &imageViewCreateInfo, allocationCallbacks, &depthTextures[i].imageView));
	}

	return true;
}

bool Renderer::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
	};

	VkDescriptorPoolSize bindlessPoolSizes[]
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 6144;
	descriptorPoolCreateInfo.poolSizeCount = 6;
	descriptorPoolCreateInfo.pPoolSizes = poolSizes;

	VkValidateFR(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, allocationCallbacks, &vkDescriptorPool));

	VkDescriptorPoolCreateInfo bindlessDescriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	bindlessDescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
	bindlessDescriptorPoolCreateInfo.maxSets = 2048;
	bindlessDescriptorPoolCreateInfo.poolSizeCount = 2;
	bindlessDescriptorPoolCreateInfo.pPoolSizes = bindlessPoolSizes;

	VkValidateFR(vkCreateDescriptorPool(device, &bindlessDescriptorPoolCreateInfo, allocationCallbacks, &vkBindlessDescriptorPool));

	return true;
}

bool Renderer::CreateRenderpasses()
{
	return renderpass.Create();
}

bool Renderer::CreateSynchronization()
{
	VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;

	VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAvailable[i]);
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &vertexInputFinished[i]);
		vkCreateFence(device, &fenceInfo, allocationCallbacks, &inFlight[i]);
	}

	return true;
}

bool Renderer::RecreateSwapchain()
{
	vkDeviceWaitIdle(device);

	frameBuffer.Destroy();
	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkDestroyImageView(device, depthTextures[i].imageView, allocationCallbacks);
		vmaDestroyImage(vmaAllocator, depthTextures[i].image, depthTextures[i].allocation);
	}

	if (!swapchain.Create(true)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateDepthTextures()) { Logger::Fatal("Failed To Create Depth Buffer!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }

	return true;
}

VkCommandPool Renderer::CreateCommandPool(QueueType queueType)
{
	VkCommandPool pool;

	VkCommandPoolCreateInfo poolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolCreateInfo.queueFamilyIndex = device.GetQueueIndex(queueType);
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkValidateF(vkCreateCommandPool(device, &poolCreateInfo, allocationCallbacks, &pool));

	return pool;
}

void Renderer::DestroyCommandPool(VkCommandPool pool)
{
	vkDestroyCommandPool(device, pool, allocationCallbacks);
}

bool Renderer::UploadTexture(Resource<Texture>& texture, U8* data, const Sampler& sampler)
{
	VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stagingBufferInfo.pNext = nullptr;
	stagingBufferInfo.flags = 0;
	stagingBufferInfo.size = texture->size;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	stagingBufferInfo.queueFamilyIndexCount = 0;
	stagingBufferInfo.pQueueFamilyIndices = nullptr;

	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferAlloc;

	VmaAllocationCreateInfo stagingAllocInfo{};
	stagingAllocInfo.flags = 0;
	stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	stagingAllocInfo.requiredFlags = 0;
	stagingAllocInfo.preferredFlags = 0;
	stagingAllocInfo.memoryTypeBits = 0;
	stagingAllocInfo.pool = nullptr;
	stagingAllocInfo.pUserData = nullptr;
	stagingAllocInfo.priority = 0.0f;

	VkValidateR(vmaCreateBuffer(Renderer::vmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingBufferAlloc, nullptr));

	void* uploadData;
	VkValidateR(vmaMapMemory(Renderer::vmaAllocator, stagingBufferAlloc, &uploadData));

	memcpy(uploadData, data, texture->size);
	vmaUnmapMemory(Renderer::vmaAllocator, stagingBufferAlloc);
	vmaFlushAllocation(Renderer::vmaAllocator, stagingBufferAlloc, 0, texture->size);

	CommandBuffer uploadCommandBuffer;
	uploadCommandBuffer.CreateSingleShotBuffer(commandPool);

	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.pNext = nullptr;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.extent.width = texture->width;
	imageInfo.extent.height = texture->height;
	imageInfo.extent.depth = texture->depth;
	imageInfo.mipLevels = texture->mipmapLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo imageAllocInfo{};
	imageAllocInfo.flags = 0;
	imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imageAllocInfo.requiredFlags = 0;
	imageAllocInfo.preferredFlags = 0;
	imageAllocInfo.memoryTypeBits = 0;
	imageAllocInfo.pool = nullptr;
	imageAllocInfo.pUserData = nullptr;
	imageAllocInfo.priority = 0.0f;

	VkValidateR(vmaCreateImage(vmaAllocator, &imageInfo, &imageAllocInfo, &texture->image, &texture->allocation, nullptr));

	VkImageSubresourceRange stagingBufferRange{};
	stagingBufferRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingBufferRange.baseMipLevel = 0;
	stagingBufferRange.levelCount = texture->mipmapLevels;
	stagingBufferRange.baseArrayLayer = 0;
	stagingBufferRange.layerCount = 1;

	VkImageMemoryBarrier stagingBufferTransferBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	stagingBufferTransferBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	stagingBufferTransferBarrier.pNext = nullptr;
	stagingBufferTransferBarrier.srcAccessMask = 0;
	stagingBufferTransferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	stagingBufferTransferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	stagingBufferTransferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingBufferTransferBarrier.srcQueueFamilyIndex = 0;
	stagingBufferTransferBarrier.dstQueueFamilyIndex = 0;
	stagingBufferTransferBarrier.image = texture->image;
	stagingBufferTransferBarrier.subresourceRange = stagingBufferRange;

	VkOffset3D textureOffset{};
	textureOffset.x = 0;
	textureOffset.y = 0;
	textureOffset.z = 0;

	VkExtent3D textureExtent{};
	textureExtent.width = texture->width;
	textureExtent.height = texture->height;
	textureExtent.depth = texture->depth;

	VkBufferImageCopy stagingBufferCopy{};
	stagingBufferCopy.bufferOffset = 0;
	stagingBufferCopy.bufferRowLength = 0;
	stagingBufferCopy.bufferImageHeight = 0;
	stagingBufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingBufferCopy.imageSubresource.mipLevel = 0;
	stagingBufferCopy.imageSubresource.baseArrayLayer = 0;
	stagingBufferCopy.imageSubresource.layerCount = 1;
	stagingBufferCopy.imageOffset = textureOffset;
	stagingBufferCopy.imageExtent = textureExtent;

	VkImageMemoryBarrier stagingBufferShaderBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	stagingBufferShaderBarrier.pNext = nullptr;
	stagingBufferShaderBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	stagingBufferShaderBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	stagingBufferShaderBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingBufferShaderBarrier.newLayout = texture->mipmapLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	stagingBufferShaderBarrier.srcQueueFamilyIndex = 0;
	stagingBufferShaderBarrier.dstQueueFamilyIndex = 0;
	stagingBufferShaderBarrier.image = texture->image;
	stagingBufferShaderBarrier.subresourceRange = stagingBufferRange;

	vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingBufferTransferBarrier);
	vkCmdCopyBufferToImage(uploadCommandBuffer, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &stagingBufferCopy);
	vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingBufferShaderBarrier);

	if (texture->mipmapLevels > 1)
	{
		VkImageSubresourceRange blitRange{};
		blitRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRange.baseMipLevel = 0;
		blitRange.levelCount = 1;
		blitRange.baseArrayLayer = 0;
		blitRange.layerCount = 1;

		VkImageMemoryBarrier firstBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		firstBarrier.pNext = nullptr;
		firstBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		firstBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		firstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		firstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		firstBarrier.srcQueueFamilyIndex = 0;
		firstBarrier.dstQueueFamilyIndex = 0;
		firstBarrier.image = texture->image;
		firstBarrier.subresourceRange = blitRange;

		VkImageMemoryBarrier secondBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		secondBarrier.pNext = nullptr;
		secondBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		secondBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		secondBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		secondBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		secondBarrier.srcQueueFamilyIndex = 0;
		secondBarrier.dstQueueFamilyIndex = 0;
		secondBarrier.image = texture->image;
		secondBarrier.subresourceRange = blitRange;

		VkImageBlit mipBlit{};
		mipBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipBlit.srcSubresource.mipLevel = 0;
		mipBlit.srcSubresource.baseArrayLayer = 0;
		mipBlit.srcSubresource.layerCount = 1;
		mipBlit.srcOffsets[0] = { 0, 0, 0 };
		mipBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipBlit.dstSubresource.mipLevel = 0;
		mipBlit.dstSubresource.baseArrayLayer = 0;
		mipBlit.dstSubresource.layerCount = 1;
		mipBlit.dstOffsets[0] = { 0, 0, 0 };

		I32 mipWidth = texture->width;
		I32 mipHeight = texture->height;

		for (I32 i = 1; i < texture->mipmapLevels; ++i)
		{
			mipBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			mipBlit.srcSubresource.mipLevel = i - 1;

			mipBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			mipBlit.dstSubresource.mipLevel = i;

			firstBarrier.subresourceRange.baseMipLevel = i - 1;
			vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &firstBarrier);

			vkCmdBlitImage(uploadCommandBuffer,
				texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &mipBlit, VK_FILTER_LINEAR);

			secondBarrier.subresourceRange.baseMipLevel = i - 1;
			vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &secondBarrier);

			if (mipWidth > 1) { mipWidth /= 2; }
			if (mipHeight > 1) { mipHeight /= 2; }
		}

		VkImageMemoryBarrier lastBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		lastBarrier.pNext = nullptr;
		lastBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		lastBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		lastBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		lastBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		lastBarrier.srcQueueFamilyIndex = 0;
		lastBarrier.dstQueueFamilyIndex = 0;
		lastBarrier.image = texture->image;
		lastBarrier.subresourceRange = blitRange;
		lastBarrier.subresourceRange.baseMipLevel = texture->mipmapLevels - 1;

		vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &lastBarrier);
	}

	bool commandResult = uploadCommandBuffer.SubmitSingleShotBuffer(graphicsQueue);
	vmaDestroyBuffer(vmaAllocator, stagingBuffer, stagingBufferAlloc);

	if (!commandResult) { return false; }

	VkImageViewCreateInfo texViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	texViewInfo.pNext = nullptr;
	texViewInfo.flags = 0;
	texViewInfo.image = texture->image;
	texViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	texViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	texViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	texViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	texViewInfo.subresourceRange.baseMipLevel = 0;
	texViewInfo.subresourceRange.levelCount = texture->mipmapLevels;
	texViewInfo.subresourceRange.baseArrayLayer = 0;
	texViewInfo.subresourceRange.layerCount = 1;

	VkValidateR(vkCreateImageView(device, &texViewInfo, allocationCallbacks, &texture->imageView));

	const VkBool32 anisotropyAvailable = device.physicalDevice.features.samplerAnisotropy;
	const F32 maxAnisotropy = device.physicalDevice.properties.limits.maxSamplerAnisotropy;

	VkSamplerCreateInfo texSamplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	texSamplerInfo.pNext = nullptr;
	texSamplerInfo.flags = 0;
	texSamplerInfo.magFilter = (VkFilter)sampler.filterMode;
	texSamplerInfo.minFilter = (VkFilter)sampler.filterMode;
	texSamplerInfo.mipmapMode = (VkSamplerMipmapMode)sampler.mipMapSampleMode;
	texSamplerInfo.addressModeU = (VkSamplerAddressMode)sampler.edgeSampleMode;
	texSamplerInfo.addressModeV = (VkSamplerAddressMode)sampler.edgeSampleMode;
	texSamplerInfo.addressModeW = (VkSamplerAddressMode)sampler.edgeSampleMode;
	texSamplerInfo.mipLodBias = 0.0f;
	texSamplerInfo.anisotropyEnable = anisotropyAvailable;
	texSamplerInfo.maxAnisotropy = maxAnisotropy;
	texSamplerInfo.compareEnable = VK_FALSE;
	texSamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	texSamplerInfo.minLod = 0.0f;
	texSamplerInfo.maxLod = (F32)texture->mipmapLevels;
	texSamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	texSamplerInfo.unnormalizedCoordinates = VK_FALSE;

	VkValidateR(vkCreateSampler(device, &texSamplerInfo, allocationCallbacks, &texture->sampler));

	return true;
}

void Renderer::DestroyTexture(Resource<Texture>& texture)
{
	vkDestroySampler(device, texture->sampler, allocationCallbacks);
	vkDestroyImageView(device, texture->imageView, allocationCallbacks);
	vmaDestroyImage(vmaAllocator, texture->image, texture->allocation);
}
#include "Renderer.hpp"

#include "CommandBufferRing.hpp"

#include "Platform/Platform.hpp"
#include "Core/Time.hpp"
#include "Math/Math.hpp"
#include "Resources/Resources.hpp"

#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION

#ifdef NH_DEBUG
#define VMA_DEBUG_LOG_ENABLED 1
#define VMA_DEBUG_ALWAYS_DEDICATED_MEMORY 0
#define VMA_RECORDING_ENABLED 1
#endif

#include "vma/vk_mem_alloc.h"

VmaAllocator Renderer::vmaAllocator;
VkAllocationCallbacks* Renderer::allocationCallbacks = VK_NULL_HANDLE;
VkDescriptorPool Renderer::vkDescriptorPool = VK_NULL_HANDLE;
VkDescriptorPool Renderer::vkBindlessDescriptorPool = VK_NULL_HANDLE;
DescriptorSet Renderer::descriptorSet;
Texture Renderer::depthTextures[MaxSwapchainImages];
Buffer Renderer::stagingBuffers[MaxSwapchainImages];

Instance Renderer::instance;
Device Renderer::device;
VkQueue Renderer::graphicsQueue;
VkQueue Renderer::presentQueue;
Swapchain Renderer::swapchain;
Renderpass Renderer::renderpass;
FrameBuffer Renderer::frameBuffer;

Vector<VkCommandBuffer> Renderer::commandBuffers[MaxSwapchainImages];
GlobalPushConstant Renderer::globalPushConstant;
Scene* Renderer::scene;

U32 Renderer::frameIndex;
U32 Renderer::previousFrame;
U32 Renderer::absoluteFrame;
VkSemaphore Renderer::imageAcquired[MaxSwapchainImages];
VkSemaphore Renderer::transferFinished[MaxSwapchainImages];
VkSemaphore Renderer::renderFinished[MaxSwapchainImages];
VkSemaphore Renderer::presentReady[MaxSwapchainImages];
U64 Renderer::renderWaitValues[MaxSwapchainImages];
U64 Renderer::transferWaitValues[MaxSwapchainImages];

bool Renderer::Initialize()
{
	Logger::Trace("Initializing Renderer...");

	if (!instance.Create()) { Logger::Fatal("Failed To Create Vulkan Instance!"); return false; }
	if (!device.Create()) { Logger::Fatal("Failed To Create Vulkan Device!"); return false; }
	if (!InitializeVma()) { Logger::Fatal("Failed To Initialize Vma!"); return false; }
	if (!GetQueues()) { return false; }
	if (!swapchain.Create(false)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateDepthTextures()) { Logger::Fatal("Failed To Create Depth Buffers!"); return false; }
	if (!CommandBufferRing::Initialize()) { Logger::Fatal("Failed To Create Command Buffers!"); return false; }
	if (!CreateDescriptorPool()) { Logger::Fatal("Failed To Create Descriptor Pool!"); return false; }
	if (!CreateRenderpasses()) { Logger::Fatal("Failed To Create Renderpasses!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }
	if (!CreateSynchronization()) { Logger::Fatal("Failed To Create Synchronization Objects!"); return false; }
	if (!CreateStagingBuffers()) { Logger::Fatal("Failed To Create Staging Buffers!"); return false; }

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Renderer...");

	vkDeviceWaitIdle(device);

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		stagingBuffers[i].Destroy();
	}

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkDestroySemaphore(device, imageAcquired[i], allocationCallbacks);
		vkDestroySemaphore(device, transferFinished[i], allocationCallbacks);
		vkDestroySemaphore(device, renderFinished[i], allocationCallbacks);
		vkDestroySemaphore(device, presentReady[i], allocationCallbacks);
	}

	frameBuffer.Destroy();

	renderpass.Destroy();

	vkDestroyDescriptorPool(device, vkDescriptorPool, allocationCallbacks);
	vkDestroyDescriptorPool(device, vkBindlessDescriptorPool, allocationCallbacks);

	CommandBufferRing::Shutdown();

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkDestroyImageView(device, depthTextures[i].imageView, allocationCallbacks);
		vmaDestroyImage(vmaAllocator, depthTextures[i].image, depthTextures[i].allocation);
	}

	swapchain.Destroy();

#if defined(NH_DEBUG) && 0
	char* statsString = nullptr;
	vmaBuildStatsString(vmaAllocator, &statsString, VK_TRUE);
	printf("%s\n", statsString);
	vmaFreeStatsString(vmaAllocator, statsString);
#endif

	vmaDestroyAllocator(vmaAllocator);

	device.Destroy();

	instance.Destroy();
}

void Renderer::Update()
{
	Synchronize();

	Resources::Update();
	scene->Update();

	SubmitTransfer();

	globalPushConstant.viewProjection = scene->camera.ViewProjection();

	CommandBuffer& commandBuffer = CommandBufferRing::GetDrawCommandBuffer(frameIndex);

	commandBuffer.Begin();
	commandBuffer.BeginRenderpass(renderpass, frameBuffer, swapchain);

	scene->Render(commandBuffer);

	//TODO: Draw UI

	commandBuffer.EndRenderpass();

	commandBuffer.End();

	Submit();
}

void Renderer::Synchronize()
{
	U32 i = absoluteFrame % swapchain.imageCount;
	VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAcquired[i], VK_NULL_HANDLE, &frameIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) { RecreateSwapchain(); }
	else { VkValidateFExit(res); }

	VkSemaphore waits[]{ renderFinished[previousFrame], transferFinished[previousFrame] };
	U64 waitValues[]{ renderWaitValues[previousFrame], transferWaitValues[previousFrame] };

	VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
	waitInfo.pNext = nullptr;
	waitInfo.flags = 0;
	waitInfo.semaphoreCount = CountOf32(waits);
	waitInfo.pSemaphores = waits;
	waitInfo.pValues = waitValues;

	vkWaitSemaphores(device, &waitInfo, U64_MAX);

	CommandBufferRing::ResetDraw(frameIndex);
	CommandBufferRing::ResetPool(frameIndex);
}

void Renderer::SubmitTransfer()
{
	if (commandBuffers[frameIndex].Size())
	{
		++transferWaitValues[frameIndex];

		VkTimelineSemaphoreSubmitInfo timelineInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
		timelineInfo.pNext = nullptr;
		timelineInfo.waitSemaphoreValueCount = 0;
		timelineInfo.pWaitSemaphoreValues = nullptr;
		timelineInfo.signalSemaphoreValueCount = 1;
		timelineInfo.pSignalSemaphoreValues = &transferWaitValues[frameIndex];

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.pNext = &timelineInfo;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = (U32)commandBuffers[frameIndex].Size();
		submitInfo.pCommandBuffers = commandBuffers[frameIndex].Data();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &transferFinished[frameIndex];

		VkValidateF(vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr));
		commandBuffers[frameIndex].Clear();
		stagingBuffers[frameIndex].stagingPointer = 0;
	}
}

void Renderer::Submit()
{
	++renderWaitValues[frameIndex];

	VkSemaphoreSubmitInfo waitSemaphores[] = {
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = transferFinished[frameIndex],
		.value = transferWaitValues[frameIndex],
		.stageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
		.deviceIndex = 0,
	},
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = imageAcquired[frameIndex],
		.value = 0,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		.deviceIndex = 0,
	}
	};

	VkSemaphoreSubmitInfo signalSemaphores[] = {
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = renderFinished[frameIndex],
		.value = renderWaitValues[frameIndex],
		.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR,
		.deviceIndex = 0,
	},
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = nullptr,
		.semaphore = presentReady[frameIndex],
		.value = 0,
		.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR,
		.deviceIndex = 0,
	}
	};

	VkCommandBufferSubmitInfo commandBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandBuffer = CommandBufferRing::GetDrawCommandBuffer(frameIndex);
	commandBufferInfo.deviceMask = 0;

	VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
	submitInfo.pNext = nullptr;
	submitInfo.flags = 0;
	submitInfo.waitSemaphoreInfoCount = CountOf32(waitSemaphores);
	submitInfo.pWaitSemaphoreInfos = waitSemaphores;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &commandBufferInfo;
	submitInfo.signalSemaphoreInfoCount = CountOf32(signalSemaphores);
	submitInfo.pSignalSemaphoreInfos = signalSemaphores;

	VkValidateFExit(vkQueueSubmit2(graphicsQueue, 1u, &submitInfo, nullptr));

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentReady[frameIndex];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &frameIndex;
	presentInfo.pResults = nullptr;

	VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);
	commandBuffers[frameIndex].Clear();

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) { RecreateSwapchain(); }

	previousFrame = frameIndex;

	++absoluteFrame;
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

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquired[i]);
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &presentReady[i]);
	}

	VkSemaphoreTypeCreateInfo semaphoreType{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	semaphoreType.pNext = nullptr;
	semaphoreType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	semaphoreType.initialValue = 0;

	semaphoreInfo.pNext = &semaphoreType;

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &renderFinished[i]);
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &transferFinished[i]);
	}

	return true;
}

bool Renderer::CreateStagingBuffers()
{
	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		stagingBuffers[i].Create(BufferType::Staging, Gigabytes(1));
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

bool Renderer::UploadTexture(Resource<Texture>& texture, U8* data, const Sampler& sampler)
{
	U64 offset = stagingBuffers[frameIndex].StagingPointer();

	stagingBuffers[frameIndex].UploadStagingData(data, texture->size, offset);

	CommandBuffer& commandBuffer = CommandBufferRing::GetWriteCommandBuffer(frameIndex);
	commandBuffer.Begin();

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

	VkImageMemoryBarrier2 stagingBufferTransferBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
	stagingBufferTransferBarrier.pNext = nullptr;
	stagingBufferTransferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
	stagingBufferTransferBarrier.srcAccessMask = VK_ACCESS_2_NONE;
	stagingBufferTransferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	stagingBufferTransferBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
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
	stagingBufferCopy.bufferOffset = offset;
	stagingBufferCopy.bufferRowLength = 0;
	stagingBufferCopy.bufferImageHeight = 0;
	stagingBufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingBufferCopy.imageSubresource.mipLevel = 0;
	stagingBufferCopy.imageSubresource.baseArrayLayer = 0;
	stagingBufferCopy.imageSubresource.layerCount = 1;
	stagingBufferCopy.imageOffset = textureOffset;
	stagingBufferCopy.imageExtent = textureExtent;

	VkImageMemoryBarrier2 stagingBufferShaderBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
	stagingBufferShaderBarrier.pNext = nullptr;
	stagingBufferShaderBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	stagingBufferShaderBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
	stagingBufferShaderBarrier.dstStageMask = texture->mipmapLevels > 1 ? VK_PIPELINE_STAGE_2_COPY_BIT : VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	stagingBufferShaderBarrier.dstAccessMask = texture->mipmapLevels > 1 ? VK_ACCESS_2_TRANSFER_WRITE_BIT : VK_ACCESS_2_SHADER_READ_BIT;
	stagingBufferShaderBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingBufferShaderBarrier.newLayout = texture->mipmapLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	stagingBufferShaderBarrier.srcQueueFamilyIndex = 0;
	stagingBufferShaderBarrier.dstQueueFamilyIndex = 0;
	stagingBufferShaderBarrier.image = texture->image;
	stagingBufferShaderBarrier.subresourceRange = stagingBufferRange;

	commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &stagingBufferTransferBarrier);
	commandBuffer.BufferToImage(stagingBuffers[frameIndex], texture, 1, &stagingBufferCopy);
	commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &stagingBufferShaderBarrier);

	if (texture->mipmapLevels > 1)
	{
		VkImageSubresourceRange blitRange{};
		blitRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRange.baseMipLevel = 0;
		blitRange.levelCount = 1;
		blitRange.baseArrayLayer = 0;
		blitRange.layerCount = 1;

		VkImageMemoryBarrier2 firstBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		firstBarrier.pNext = nullptr;
		firstBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		firstBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		firstBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		firstBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		firstBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		firstBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		firstBarrier.srcQueueFamilyIndex = 0;
		firstBarrier.dstQueueFamilyIndex = 0;
		firstBarrier.image = texture->image;
		firstBarrier.subresourceRange = blitRange;

		VkImageMemoryBarrier2 secondBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		secondBarrier.pNext = nullptr;
		secondBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		secondBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
		secondBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		secondBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
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
			secondBarrier.subresourceRange.baseMipLevel = i - 1;

			commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &firstBarrier);
			commandBuffer.Blit(texture, texture, VK_FILTER_LINEAR, 1, &mipBlit);
			commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &secondBarrier);

			if (mipWidth > 1) { mipWidth /= 2; }
			if (mipHeight > 1) { mipHeight /= 2; }
		}

		VkImageMemoryBarrier2 lastBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
		lastBarrier.pNext = nullptr;
		lastBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		lastBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
		lastBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		lastBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
		lastBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		lastBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		lastBarrier.srcQueueFamilyIndex = 0;
		lastBarrier.dstQueueFamilyIndex = 0;
		lastBarrier.image = texture->image;
		lastBarrier.subresourceRange = blitRange;
		lastBarrier.subresourceRange.baseMipLevel = texture->mipmapLevels - 1;

		commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &lastBarrier);
	}

	VkValidateR(commandBuffer.End());

	commandBuffers[frameIndex].Push(commandBuffer);

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
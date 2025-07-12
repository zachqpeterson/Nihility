#include "Renderer.hpp"

#include "CommandBufferRing.hpp"
#include "UI.hpp"
#include "LineRenderer.hpp"

#include "Platform/Platform.hpp"
#include "Core/Time.hpp"
#include "Math/Math.hpp"
#include "Resources/Resources.hpp"

#include "tracy/Tracy.hpp"

#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION

#ifdef NH_DEBUG
#define VMA_DEBUG_LOG_ENABLED 1
#define VMA_DEBUG_ALWAYS_DEDICATED_MEMORY 0
#define VMA_RECORDING_ENABLED 1
#endif

#include "vma/vk_mem_alloc.h"

VmaAllocator Renderer::vmaAllocator;
VkAllocationCallbacks* Renderer::allocationCallbacks;
VkDescriptorPool Renderer::vkDescriptorPool = VK_NULL_HANDLE;
VkDescriptorPool Renderer::vkBindlessDescriptorPool = VK_NULL_HANDLE;
DescriptorSet Renderer::descriptorSet;
Texture Renderer::colorTextures[MaxSwapchainImages];
Texture Renderer::depthTextures[MaxSwapchainImages];
Buffer Renderer::stagingBuffers[MaxSwapchainImages];

Instance Renderer::instance;
Device Renderer::device;
Swapchain Renderer::swapchain;
Renderpass Renderer::renderpass;
FrameBuffer Renderer::frameBuffer;

Vector<VkCommandBuffer> Renderer::commandBuffers[MaxSwapchainImages];
GlobalPushConstant Renderer::globalPushConstant;

bool Renderer::resize = false;
U32 Renderer::frameIndex;
U32 Renderer::previousFrame;
U32 Renderer::absoluteFrame;
VkSemaphore Renderer::imageAcquired[MaxSwapchainImages];
VkSemaphore Renderer::transferFinished[MaxSwapchainImages];
VkSemaphore Renderer::renderFinished[MaxSwapchainImages];
VkSemaphore Renderer::presentReady[MaxSwapchainImages];
U64 Renderer::renderWaitValues[MaxSwapchainImages];
U64 Renderer::transferWaitValues[MaxSwapchainImages];

bool Renderer::Initialize(const StringView& name, U32 version)
{
	Logger::Trace("Initializing Renderer...");

	if (!instance.Create(name, version)) { Logger::Fatal("Failed To Create Vulkan Instance!"); return false; }
	if (!device.Create()) { Logger::Fatal("Failed To Create Vulkan Device!"); return false; }
	if (!InitializeVma()) { Logger::Fatal("Failed To Initialize Vma!"); return false; }
	if (!swapchain.Create(false)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateColorTextures()) { Logger::Fatal("Failed To Create Color Buffers!"); return false; }
	if (!CreateDepthTextures()) { Logger::Fatal("Failed To Create Depth Buffers!"); return false; }
	if (!CommandBufferRing::Initialize()) { Logger::Fatal("Failed To Create Command Buffers!"); return false; }
	if (!CreateDescriptorPool()) { Logger::Fatal("Failed To Create Descriptor Pool!"); return false; }
	if (!CreateRenderpasses()) { Logger::Fatal("Failed To Create Renderpasses!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }
	if (!CreateSynchronization()) { Logger::Fatal("Failed To Create Synchronization Objects!"); return false; }
	if (!CreateStagingBuffers()) { Logger::Fatal("Failed To Create Staging Buffers!"); return false; }

#ifdef NH_DEBUG
	if (!LineRenderer::Initialize()) { Logger::Fatal("Failed To Create Line Renderer!"); return false; }
#endif

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Renderer...");

	vkDeviceWaitIdle(device);

#ifdef NH_DEBUG
	LineRenderer::Shutdown();
#endif

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
		vkDestroyImageView(device, colorTextures[i].imageView, allocationCallbacks);
		vmaDestroyImage(vmaAllocator, colorTextures[i].image, colorTextures[i].allocation);
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
	ZoneScopedN("RenderMain");

	if (!Synchronize()) { return; }

	Resources::Update();
	World::Update();
#ifdef NH_DEBUG
	LineRenderer::Update();
#endif
	UI::Update();

	SubmitTransfer();

	globalPushConstant.viewProjection = World::camera.ViewProjection();
	
	CommandBuffer& commandBuffer = CommandBufferRing::GetDrawCommandBuffer(frameIndex);

	commandBuffer.Begin();
	commandBuffer.BeginRenderpass(renderpass, frameBuffer, swapchain);

	World::Render(commandBuffer);

#ifdef NH_DEBUG
	LineRenderer::Render(commandBuffer);
#endif

	UI::Render(commandBuffer);

	commandBuffer.EndRenderpass();

	commandBuffer.End();

	Submit();
}

bool Renderer::Synchronize()
{
	ZoneScopedN("RenderSynchronize");

	if (resize)
	{
		resize = false;
		RecreateSwapchain();
	}

	U32 i = absoluteFrame % swapchain.imageCount;
	VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAcquired[i], VK_NULL_HANDLE, &frameIndex);

	VkSemaphore waits[]{ renderFinished[previousFrame], transferFinished[previousFrame] };
	U64 waitValues[]{ renderWaitValues[previousFrame], transferWaitValues[previousFrame] };

	VkSemaphoreWaitInfo waitInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.pNext = nullptr,
		.flags = 0,
		.semaphoreCount = CountOf32(waits),
		.pSemaphores = waits,
		.pValues = waitValues
	};

	vkWaitSemaphores(device, &waitInfo, U64_MAX);

	CommandBufferRing::ResetDraw(frameIndex);
	CommandBufferRing::ResetPool(frameIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		previousFrame = frameIndex;
		++absoluteFrame;
		resize = true;
		return false;
	}
	else { VkValidateFR(res); }

	return true;
}

void Renderer::SubmitTransfer()
{
	ZoneScopedN("RenderTransfer");

	if (commandBuffers[frameIndex].Size())
	{
		++transferWaitValues[frameIndex];

		VkTimelineSemaphoreSubmitInfo timelineInfo{
			.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreValueCount = 0,
			.pWaitSemaphoreValues = nullptr,
			.signalSemaphoreValueCount = 1,
			.pSignalSemaphoreValues = &transferWaitValues[frameIndex]
		};

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = &timelineInfo,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = nullptr,
			.commandBufferCount = (U32)commandBuffers[frameIndex].Size(),
			.pCommandBuffers = commandBuffers[frameIndex].Data(),
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &transferFinished[frameIndex]
		};

		VkValidateF(vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, nullptr));
		commandBuffers[frameIndex].Clear();
		stagingBuffers[frameIndex].stagingPointer = 0;
	}
}

void Renderer::Submit()
{
	ZoneScopedN("RenderSubmit");

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

	VkCommandBufferSubmitInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = CommandBufferRing::GetDrawCommandBuffer(frameIndex),
		.deviceMask = 0
	};

	VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = nullptr,
		.flags = 0,
		.waitSemaphoreInfoCount = CountOf32(waitSemaphores),
		.pWaitSemaphoreInfos = waitSemaphores,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &commandBufferInfo,
		.signalSemaphoreInfoCount = CountOf32(signalSemaphores),
		.pSignalSemaphoreInfos = signalSemaphores
	};

	VkValidateFExit(vkQueueSubmit2(device.graphicsQueue, 1u, &submitInfo, nullptr));

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &presentReady[frameIndex],
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &frameIndex,
		.pResults = nullptr
	};

	VkResult res = vkQueuePresentKHR(device.presentQueue, &presentInfo);
	commandBuffers[frameIndex].Clear();

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) { resize = true; }

	previousFrame = frameIndex;

	++absoluteFrame;
}

U32 Renderer::FrameIndex()
{
	return frameIndex;
}

U32 Renderer::PreviousFrame()
{
	return previousFrame;
}

U32 Renderer::AbsoluteFrame()
{
	return absoluteFrame;
}

Vector4Int Renderer::RenderSize()
{
	return { 0, 0, (I32)swapchain.width, (I32)swapchain.height };
}

const GlobalPushConstant* Renderer::GetGlobalPushConstant()
{
	return &globalPushConstant;
}

VkSemaphore_T* Renderer::RenderFinished()
{
	return renderFinished[previousFrame];
}

const Device& Renderer::GetDevice()
{
	return device;
}

bool Renderer::InitializeVma()
{
	VmaAllocatorCreateInfo allocatorInfo{
		.flags = 0,
		.physicalDevice = device.physicalDevice,
		.device = device,
		.preferredLargeHeapBlockSize = 0,
		.pAllocationCallbacks = allocationCallbacks,
		.pDeviceMemoryCallbacks = nullptr,
		.pHeapSizeLimit = nullptr,
		.pVulkanFunctions = nullptr,
		.instance = instance,
		.vulkanApiVersion = VK_API_VERSION_1_3,
		.pTypeExternalMemoryHandleTypes = nullptr
	};

	VkValidateFR(vmaCreateAllocator(&allocatorInfo, &vmaAllocator));

	return true;
}

bool Renderer::CreateColorTextures()
{
	VkExtent3D colorImageExtent{
		.width = swapchain.width,
		.height = swapchain.height,
		.depth = 1
	};

	VkImageCreateInfo imageCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = (VkFormat)swapchain.format,
		.extent = colorImageExtent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = (VkSampleCountFlagBits)device.physicalDevice.maxSampleCount, //TODO: Setting
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo allocationInfo{
		.flags = 0,
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool = nullptr,
		.pUserData = nullptr,
		.priority = 0.0f
	};

	VkImageViewCreateInfo imageViewCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = nullptr,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = imageCreateInfo.format,
		.components = {},
		.subresourceRange{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		VkValidateFR(vmaCreateImage(vmaAllocator, &imageCreateInfo, &allocationInfo, &colorTextures[i].image, &colorTextures[i].allocation, nullptr));
		imageViewCreateInfo.image = colorTextures[i].image;
		VkValidateFR(vkCreateImageView(device, &imageViewCreateInfo, allocationCallbacks, &colorTextures[i].imageView));
	}

	return true;
}

bool Renderer::CreateDepthTextures()
{
	VkExtent3D depthImageExtent{
		.width = swapchain.width,
		.height = swapchain.height,
		.depth = 1
	};

	VkImageCreateInfo imageCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT,
		.extent = depthImageExtent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = (VkSampleCountFlagBits)device.physicalDevice.maxSampleCount, //TODO: Setting
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo allocationInfo{
		.flags = 0,
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool = nullptr,
		.pUserData = nullptr,
		.priority = 0.0f
	};

	VkImageViewCreateInfo imageViewCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = nullptr,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = imageCreateInfo.format,
		.components = {},
		.subresourceRange{
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

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

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 6144,
		.poolSizeCount = CountOf32(poolSizes),
		.pPoolSizes = poolSizes
	};

	VkValidateFR(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, allocationCallbacks, &vkDescriptorPool));

	VkDescriptorPoolCreateInfo bindlessDescriptorPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
		.maxSets = 2048,
		.poolSizeCount = CountOf32(bindlessPoolSizes),
		.pPoolSizes = bindlessPoolSizes
	};

	VkValidateFR(vkCreateDescriptorPool(device, &bindlessDescriptorPoolCreateInfo, allocationCallbacks, &vkBindlessDescriptorPool));

	return true;
}

bool Renderer::CreateRenderpasses()
{
	return renderpass.Create();
}

bool Renderer::CreateSynchronization()
{
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0
	};

	for (U32 i = 0; i < swapchain.imageCount; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquired[i]);
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &presentReady[i]);
	}

	VkSemaphoreTypeCreateInfo semaphoreType{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.pNext = nullptr,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = 0
	};

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
		vkDestroyImageView(device, colorTextures[i].imageView, allocationCallbacks);
		vmaDestroyImage(vmaAllocator, colorTextures[i].image, colorTextures[i].allocation);
	}

	if (!swapchain.Create(true)) { Logger::Fatal("Failed To Create Swapchain!"); return false; }
	if (!CreateColorTextures()) { Logger::Fatal("Failed To Create Depth Buffer!"); return false; }
	if (!CreateDepthTextures()) { Logger::Fatal("Failed To Create Depth Buffer!"); return false; }
	if (!frameBuffer.Create()) { Logger::Fatal("Failed To Create Frame Buffers!"); return false; }

	return true;
}

bool Renderer::UploadTexture(Resource<Texture>& texture, void* data, const Sampler& sampler)
{
	U64 offset = NextMultipleOf(stagingBuffers[frameIndex].StagingPointer(), 16);

	stagingBuffers[frameIndex].UploadStagingData(data, texture->size, offset);

	CommandBuffer& commandBuffer = CommandBufferRing::GetWriteCommandBuffer(frameIndex);
	commandBuffer.Begin();

	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = (VkFormat)texture->format,
		.extent{
			.width = texture->width,
			.height = texture->height,
			.depth = texture->depth
		},
		.mipLevels = texture->mipmapLevels,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo imageAllocInfo{
		.flags = 0,
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = 0,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool = nullptr,
		.pUserData = nullptr,
		.priority = 0.0f
	};

	VkValidateR(vmaCreateImage(vmaAllocator, &imageInfo, &imageAllocInfo, &texture->image, &texture->allocation, nullptr));

	VkImageSubresourceRange stagingBufferRange{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = texture->mipmapLevels,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkImageMemoryBarrier2 stagingBufferTransferBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
		.srcAccessMask = VK_ACCESS_2_NONE,
		.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = 0,
		.dstQueueFamilyIndex = 0,
		.image = texture->image,
		.subresourceRange = stagingBufferRange
	};

	VkOffset3D textureOffset{
		.x = 0,
		.y = 0,
		.z = 0
	};

	VkExtent3D textureExtent{
		.width = texture->width,
		.height = texture->height,
		.depth = texture->depth
	};

	VkBufferImageCopy stagingBufferCopy{
		.bufferOffset = offset,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = textureOffset,
		.imageExtent = textureExtent
	};

	VkImageMemoryBarrier2 stagingBufferShaderBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
		.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		.dstStageMask = texture->mipmapLevels > 1 ? VK_PIPELINE_STAGE_2_COPY_BIT : VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
		.dstAccessMask = texture->mipmapLevels > 1 ? VK_ACCESS_2_TRANSFER_WRITE_BIT : VK_ACCESS_2_SHADER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = texture->mipmapLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.srcQueueFamilyIndex = 0,
		.dstQueueFamilyIndex = 0,
		.image = texture->image,
		.subresourceRange = stagingBufferRange
	};

	commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &stagingBufferTransferBarrier);
	commandBuffer.BufferToImage(stagingBuffers[frameIndex], texture, 1, &stagingBufferCopy);
	commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &stagingBufferShaderBarrier);

	if (texture->mipmapLevels > 1)
	{
		VkImageSubresourceRange blitRange{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		VkImageMemoryBarrier2 firstBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = texture->image,
			.subresourceRange = blitRange
		};

		VkImageMemoryBarrier2 secondBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = texture->image,
			.subresourceRange = blitRange
		};

		VkImageBlit mipBlit{
			.srcSubresource{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets{},
			.dstSubresource{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets{}
		};

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

		blitRange.baseMipLevel = texture->mipmapLevels - 1;

		VkImageMemoryBarrier2 lastBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
			.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = texture->image,
			.subresourceRange = blitRange
		};

		commandBuffer.PipelineBarrier(0, 0, nullptr, 1, &lastBarrier);
	}

	VkValidateR(commandBuffer.End());

	commandBuffers[frameIndex].Push(commandBuffer);

	VkImageViewCreateInfo texViewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = texture->image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = (VkFormat)texture->format,
		.components{
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = texture->mipmapLevels,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
	};

	VkValidateR(vkCreateImageView(device, &texViewInfo, allocationCallbacks, &texture->imageView));

	const VkBool32 anisotropyAvailable = device.physicalDevice.features.samplerAnisotropy;
	const F32 maxAnisotropy = device.physicalDevice.features.maxSamplerAnisotropy;

	VkSamplerCreateInfo texSamplerInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.magFilter = (VkFilter)sampler.filterMode,
		.minFilter = (VkFilter)sampler.filterMode,
		.mipmapMode = (VkSamplerMipmapMode)sampler.mipMapSampleMode,
		.addressModeU = (VkSamplerAddressMode)sampler.edgeSampleMode,
		.addressModeV = (VkSamplerAddressMode)sampler.edgeSampleMode,
		.addressModeW = (VkSamplerAddressMode)sampler.edgeSampleMode,
		.mipLodBias = 0.0f,
		.anisotropyEnable = anisotropyAvailable,
		.maxAnisotropy = maxAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = (F32)texture->mipmapLevels,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkValidateR(vkCreateSampler(device, &texSamplerInfo, allocationCallbacks, &texture->sampler));

	return true;
}

void Renderer::DestroyTexture(Resource<Texture>& texture)
{
	vkDestroySampler(device, texture->sampler, allocationCallbacks);
	vkDestroyImageView(device, texture->imageView, allocationCallbacks);
	vmaDestroyImage(vmaAllocator, texture->image, texture->allocation);
}
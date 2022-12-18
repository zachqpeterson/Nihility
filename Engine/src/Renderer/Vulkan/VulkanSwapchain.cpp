#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"
#include "VulkanImage.hpp"

#include "Math/Math.hpp"
#include "Core/Logger.hpp"
#include "Core/Settings.hpp"
#include "Memory/Memory.hpp"
#include <Containers/Vector.hpp>
#include "Resources/Resources.hpp"

VkSwapchainKHR Swapchain::handle;
VkSwapchainKHR Swapchain::oldHandle;

VkSurfaceFormatKHR Swapchain::imageFormat;
U8 Swapchain::maxFramesInFlight;

U32 Swapchain::imageCount;
Vector<Texture*> Swapchain::renderTextures;
Texture* Swapchain::colorTexture;
Texture* Swapchain::depthTexture;

RenderTarget Swapchain::renderTargets[3];

bool Swapchain::Initialize(RendererState* rendererState, U32 width, U32 height)
{
	Logger::Trace("Creating vulkan swapchain...");

	VkExtent2D swapchainExtent = { width, height };

	bool found = false;
	for (U32 i = 0; i < Device::swapchainSupport.formats.Size(); ++i)
	{
		VkSurfaceFormatKHR format = Device::swapchainSupport.formats[i];

		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			imageFormat = format;
			found = true;
			break;
		}
	}

	if (!found)
	{
		imageFormat = Device::swapchainSupport.formats[0];
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (U32 i = 0; i < Device::swapchainSupport.presentModes.Size(); ++i)
	{
		VkPresentModeKHR mode = Device::swapchainSupport.presentModes[i];
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = mode;
			break;
		}
	}

	Device::QuerySwapchainSupport(Device::physicalDevice, rendererState->surface, &Device::swapchainSupport);

	if (Device::swapchainSupport.capabilities.currentExtent.width != UINT32_MAX)
	{
		swapchainExtent = Device::swapchainSupport.capabilities.currentExtent;
	}

	VkExtent2D min = Device::swapchainSupport.capabilities.minImageExtent;
	VkExtent2D max = Device::swapchainSupport.capabilities.maxImageExtent;
	swapchainExtent.width = Math::Clamp(swapchainExtent.width, min.width, max.width);
	swapchainExtent.height = Math::Clamp(swapchainExtent.height, min.height, max.height);

	imageCount = Device::swapchainSupport.capabilities.minImageCount + 1;
	if (Device::swapchainSupport.capabilities.maxImageCount > 0 && imageCount > Device::swapchainSupport.capabilities.maxImageCount)
	{
		imageCount = Device::swapchainSupport.capabilities.maxImageCount;
	}

	maxFramesInFlight = imageCount - 1;

	VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainInfo.surface = rendererState->surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = imageFormat.format;
	swapchainInfo.imageColorSpace = imageFormat.colorSpace;
	swapchainInfo.imageExtent = swapchainExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (Device::graphicsQueueIndex != Device::presentQueueIndex)
	{
		U32 queueFamilyIndices[2] = {
			(U32)Device::graphicsQueueIndex,
			(U32)Device::presentQueueIndex };
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = 0;
	}

	swapchainInfo.preTransform = Device::swapchainSupport.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = oldHandle;

	VkCheck_FATAL(vkCreateSwapchainKHR(Device::logicalDevice, &swapchainInfo, rendererState->allocator, &handle));

	rendererState->currentFrame = 0;

	imageCount = 0;
	VkCheck_FATAL(vkGetSwapchainImagesKHR(Device::logicalDevice, handle, &imageCount, nullptr));
	if (!renderTextures.Size())
	{
		renderTextures.Resize(imageCount);

		for (U32 i = 0; i < imageCount; ++i)
		{
			VulkanImage* internalData = (VulkanImage*)Memory::Allocate(sizeof(VulkanImage), MEMORY_TAG_RENDERER);

			String texName("vulkanSwapchainImage");

			renderTextures[i] = Resources::CreateTextureFromInternal(
				texName,
				swapchainExtent.width,
				swapchainExtent.height,
				4, false, true, false,
				internalData);

			if (!renderTextures[i])
			{
				Logger::Fatal("Failed to generate new swapchain image texture!");
				return false;
			}
		}
	}
	else
	{
		for (U32 i = 0; i < imageCount; ++i)
		{
			Resources::ResizeTexture(renderTextures[i], swapchainExtent.width, swapchainExtent.height, false);
		}
	}

	VkImage swapchainImages[32];
	VkCheck_FATAL(vkGetSwapchainImagesKHR(Device::logicalDevice, handle, &imageCount, swapchainImages));

	for (U32 i = 0; i < imageCount; ++i)
	{
		VulkanImage* image = (VulkanImage*)renderTextures[i]->internalData;
		image->handle = swapchainImages[i];
		image->width = swapchainExtent.width;
		image->height = swapchainExtent.height;

		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = image->handle;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = imageFormat.format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkCheck_FATAL(vkCreateImageView(Device::logicalDevice, &viewInfo, rendererState->allocator, &image->view));
	}

	if (!Device::DetectDepthFormat())
	{
		Device::depthFormat = VK_FORMAT_UNDEFINED;
		Logger::Fatal("Failed to find a supported format!");
		return false;
	}

	VulkanImage* colorImage = (VulkanImage*)Memory::Allocate(sizeof(VulkanImage), MEMORY_TAG_RENDERER);
	colorImage->Create(
		rendererState,
		VK_IMAGE_TYPE_2D,
		swapchainExtent.width,
		swapchainExtent.height,
		imageFormat.format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkSampleCountFlagBits)Settings::MSAACount,
		true,
		VK_IMAGE_ASPECT_COLOR_BIT);

	colorTexture = Resources::CreateTextureFromInternal(
		"SwapchainColorTexture",
		swapchainExtent.width,
		swapchainExtent.height,
		4,
		false,
		true,
		false,
		colorImage);

	if (!colorTexture)
	{
		Logger::Fatal("Failed to generate new swapchain color texture!");
		return false;
	}

	VulkanImage* depthImage = (VulkanImage*)Memory::Allocate(sizeof(VulkanImage), MEMORY_TAG_RENDERER);
	depthImage->Create(
		rendererState,
		VK_IMAGE_TYPE_2D,
		swapchainExtent.width,
		swapchainExtent.height,
		Device::depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkSampleCountFlagBits)Settings::MSAACount,
		true,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	depthTexture = Resources::CreateTextureFromInternal(
		"SwapchainDepthTexture",
		swapchainExtent.width,
		swapchainExtent.height,
		Device::depthChannelCount,
		false,
		true,
		false,
		depthImage);

	if (!depthTexture)
	{
		Logger::Fatal("Failed to generate new swapchain depth texture!");
		return false;
	}

	return true;
}

void Swapchain::Shutdown(RendererState* rendererState, bool end)
{
	Logger::Trace("Destroying vulkan swapchain...");

	vkDeviceWaitIdle(Device::logicalDevice);
	((VulkanImage*)colorTexture->internalData)->Destroy(rendererState);
	Memory::Free(colorTexture->internalData, sizeof(VulkanImage), MEMORY_TAG_RENDERER);
	colorTexture->internalData = nullptr;
	Memory::Free(colorTexture, sizeof(Texture), MEMORY_TAG_TEXTURE);
	((VulkanImage*)depthTexture->internalData)->Destroy(rendererState);
	Memory::Free(depthTexture->internalData, sizeof(VulkanImage), MEMORY_TAG_RENDERER);
	depthTexture->internalData = nullptr;
	Memory::Free(depthTexture, sizeof(Texture), MEMORY_TAG_TEXTURE);

	for (U32 i = 0; i < imageCount; ++i)
	{
		VulkanImage* image = (VulkanImage*)renderTextures[i]->internalData;
		vkDestroyImageView(Device::logicalDevice, image->view, rendererState->allocator);
		Memory::Free(renderTextures[i], sizeof(Texture), MEMORY_TAG_TEXTURE);
	}

	renderTextures.Clear();

	if (end) { vkDestroySwapchainKHR(Device::logicalDevice, handle, rendererState->allocator); }
	else { oldHandle = handle; }
}

void Swapchain::Recreate(RendererState* rendererState, U32 width, U32 height)
{
	Shutdown(rendererState, false);
	Initialize(rendererState, width, height);
}

bool Swapchain::AcquireNextImageIndex(
	RendererState* rendererState,
	U64 timeoutNs,
	VkSemaphore imageAvailableSemaphore,
	VkFence fence,
	U32* outImageIndex)
{
	VkResult result = vkAcquireNextImageKHR(
		Device::logicalDevice,
		handle,
		timeoutNs,
		imageAvailableSemaphore,
		fence,
		outImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Recreate(rendererState, rendererState->framebufferWidth, rendererState->framebufferHeight);
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		Logger::Fatal("VulkanSwapchain::AcquireNextImageIndex: Failed to acquire swapchain image!");
		return false;
	}

	return true;
}

void Swapchain::Present(
	RendererState* rendererState,
	VkQueue graphicsQueue,
	VkQueue presentQueue,
	VkSemaphore renderCompleteSemaphore,
	U32 presentImageIndex)
{
	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &handle;
	presentInfo.pImageIndices = &presentImageIndex;
	presentInfo.pResults = 0;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		Recreate(rendererState, rendererState->framebufferWidth, rendererState->framebufferHeight);
		Logger::Info("VulkanSwapchain::Present: Swapchain recreated because swapchain returned out of date or suboptimal.");
	}
	else if (result != VK_SUCCESS)
	{
		Logger::Fatal("VulkanSwapchain::Present: Failed to present swapchain image!");
	}

	++rendererState->currentFrame %= maxFramesInFlight;
}
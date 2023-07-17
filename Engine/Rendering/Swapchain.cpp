#include "Swapchain.hpp"

#include "Renderer.hpp"
#include "Platform\Platform.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"

bool Swapchain::CreateSurface()
{
#ifdef PLATFORM_WINDOWS
	const WindowData& wd = Platform::GetWindowData();
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.pNext = nullptr;
	surfaceInfo.flags = 0;
	surfaceInfo.hinstance = wd.instance;
	surfaceInfo.hwnd = wd.window;

	VkValidateFR(vkCreateWin32SurfaceKHR(Renderer::instance, &surfaceInfo, Renderer::allocationCallbacks, &surface));
#elif PLATFORM_LINUX
	//TODO:
#elif PLATFORM_APPLE
	//TODO:
#endif

	return true;
}

bool Swapchain::GetFormat()
{
	U32 surfaceFormatCount = 0;
	VkValidateFR(vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::physicalDevice, surface, &surfaceFormatCount, nullptr));
	Vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount, {});
	VkValidateFR(vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::physicalDevice, surface, &surfaceFormatCount, surfaceFormats.Data()));

	bool foundFormat = false;
	for (VkSurfaceFormatKHR& format : surfaceFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			surfaceFormat.format = format.format;
			surfaceFormat.colorSpace = format.colorSpace;
			foundFormat = true;
			break;
		}
	}

	if (!foundFormat)
	{
		surfaceFormat.format = surfaceFormats[0].format;
		surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	}

	return true;
}

bool Swapchain::CreateRenderPass()
{
	Texture* depthTexture = nullptr;

	if (renderpass)
	{
		renderpass->width = renderPassInfo.width;
		renderpass->height = renderPassInfo.height;
		for (U32 i = 0; i < imageCount; ++i) { renderpass->outputTextures[i] = renderPassInfo.outputTextures[i]; }

		vkDestroyRenderPass(Renderer::device, renderpass->renderpass, Renderer::allocationCallbacks);

		for (U32 i = 0; i < imageCount; ++i)
		{
			vkDestroyFramebuffer(Renderer::device, renderpass->frameBuffers[i], Renderer::allocationCallbacks);
		}

		Resources::RecreateTexture(renderpass->outputDepth, renderPassInfo.width, renderPassInfo.height, 1);

		Renderer::CreateRenderPass(renderpass);
	}
	else
	{
		renderPassInfo.SetType(RENDERPASS_TYPE_SWAPCHAIN).SetName("Swapchain");
		renderPassInfo.SetOperations(RENDER_PASS_OP_CLEAR, RENDER_PASS_OP_CLEAR, RENDER_PASS_OP_CLEAR);

		TextureCreation textureInfo{};
		textureInfo.name = "swapchain_depth";
		textureInfo.format = VK_FORMAT_D32_SFLOAT;
		textureInfo.width = renderPassInfo.width;
		textureInfo.height = renderPassInfo.height;
		textureInfo.depth = 1;
		textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET_MASK;
		textureInfo.type = TEXTURE_TYPE_2D;

		Texture* texture = Resources::CreateTexture(textureInfo);
		renderPassInfo.SetDepthStencilTexture(texture);

		renderpass = Resources::CreateRenderPass(renderPassInfo);

		if (renderpass == nullptr) { return false; }
	}

	return true;
}

bool Swapchain::Create()
{
	VkSwapchainKHR oldSwapchain = swapchain;

	VkValidateFR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Renderer::physicalDevice, surface, &surfaceCapabilities));

	U32 presentModeCount = 0;
	VkPresentModeKHR presentModes[6]{};
	VkValidateFR(vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer::physicalDevice, surface, &presentModeCount, nullptr));
	VkValidateFR(vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer::physicalDevice, surface, &presentModeCount, presentModes));

	VkExtent2D swapchainExtent = surfaceCapabilities.currentExtent;

	if (swapchainExtent.width == U32_MAX)
	{
		swapchainExtent.width = Math::Clamp(swapchainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		swapchainExtent.height = Math::Clamp(swapchainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
	}

	renderPassInfo.width = swapchainExtent.width;
	renderPassInfo.height = swapchainExtent.height;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	if (!Settings::VSync())
	{
		for (U64 i = 0; i < presentModeCount; ++i)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = VK_PRESENT_MODE_MAILBOX_KHR; break; }
			else if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) { presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; }
		}
	}

	U32 desiredImageCount = Math::Min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount);

	VkSurfaceTransformFlagsKHR preTransform;

	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) { preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; }
	else { preTransform = surfaceCapabilities.currentTransform; }

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	static constexpr VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[]{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
	};

	for (U32 i = 0; i < CountOf32(compositeAlphaFlags); ++i)
	{
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) { compositeAlpha = compositeAlphaFlags[i]; break; };
	}

	VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) { imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; }
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) { imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }

	VkSwapchainCreateInfoKHR swapchainInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainInfo.pNext = nullptr;
	swapchainInfo.flags = 0;
	swapchainInfo.surface = surface;
	swapchainInfo.minImageCount = desiredImageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = swapchainExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = imageUsage;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 0;
	swapchainInfo.pQueueFamilyIndices = nullptr;
	swapchainInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainInfo.compositeAlpha = compositeAlpha;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = oldSwapchain;

	VkValidateFR(vkCreateSwapchainKHR(Renderer::device, &swapchainInfo, Renderer::allocationCallbacks, &swapchain));

	VkImage images[MAX_SWAPCHAIN_IMAGES];
	VkValidateFR(vkGetSwapchainImagesKHR(Renderer::device, swapchain, &imageCount, nullptr));
	VkValidateFR(vkGetSwapchainImagesKHR(Renderer::device, swapchain, &imageCount, images));

	if (oldSwapchain)
	{
		vkDestroySwapchainKHR(Renderer::device, oldSwapchain, Renderer::allocationCallbacks);

		renderPassInfo.renderTargetCount = imageCount;
		for (U32 i = 0; i < imageCount; ++i)
		{
			Resources::RecreateSwapchainTexture(renderPassInfo.outputTextures[i], images[i]);
		}
	}
	else
	{
		renderPassInfo.renderTargetCount = imageCount;
		for (U32 i = 0; i < imageCount; ++i)
		{
			renderPassInfo.outputTextures[i] = Resources::CreateSwapchainTexture(images[i], surfaceFormat.format, i);
		}
	}

	CreateRenderPass();

	return true;
}

void Swapchain::Destroy()
{
	if (swapchain) { vkDestroySwapchainKHR(Renderer::device, swapchain, Renderer::allocationCallbacks); }
	if (surface) { vkDestroySurfaceKHR(Renderer::instance, surface, Renderer::allocationCallbacks); }

	for (U32 i = 0; i < imageCount; ++i) { Resources::DestroyTexture(renderPassInfo.outputTextures[i]); }

	Resources::DestroyRenderPass(renderpass);

	renderPassInfo.Destroy();
	surface = nullptr;
	swapchain = nullptr;
}

VkResult Swapchain::NextImage(U32& imageIndex, VkSemaphore semaphore, VkFence fence)
{
	return vkAcquireNextImageKHR(Renderer::device, swapchain, U64_MAX, semaphore, fence, &imageIndex);
}

VkResult Swapchain::Present(VkQueue queue, U32 imageIndex, VkSemaphore semaphore)
{
	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pNext = 0;
	presentInfo.waitSemaphoreCount = semaphore != nullptr;
	presentInfo.pWaitSemaphores = semaphore ? &semaphore : nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	return vkQueuePresentKHR(queue, &presentInfo);
}
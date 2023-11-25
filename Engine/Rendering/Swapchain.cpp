#include "Swapchain.hpp"

#include "RenderingDefines.hpp"

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

VkSurfaceFormatKHR surfaceFormat{};

bool Swapchain::GetFormat()
{
	U32 surfaceFormatCount = 0;
	VkValidateFR(vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::physicalDevice, surface, &surfaceFormatCount, nullptr));
	Vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount, {});
	VkValidateFR(vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::physicalDevice, surface, &surfaceFormatCount, surfaceFormats.Data()));

	bool foundFormat = false;
	for (VkSurfaceFormatKHR& format : surfaceFormats)
	{
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
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

bool Swapchain::Create()
{
	VkSwapchainKHR oldSwapchain = swapchain;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

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

	vsync = Settings::VSync();
	Logger::Debug(vsync);
	width = swapchainExtent.width;
	height = swapchainExtent.height;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	if (!vsync)
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

		for (U32 i = 0; i < imageCount; ++i)
		{
			Resources::RecreateSwapchainTexture(renderTargets[i], images[i]);
		}
	}
	else
	{
		for (U32 i = 0; i < imageCount; ++i)
		{
			renderTargets[i] = Resources::CreateSwapchainTexture(images[i], surfaceFormat.format, i);
		}
	}

	Renderer::SetRenderArea();

	return true;
}

void Swapchain::Destroy()
{
	if (swapchain) { vkDestroySwapchainKHR(Renderer::device, swapchain, Renderer::allocationCallbacks); }
	if (surface) { vkDestroySurfaceKHR(Renderer::instance, surface, Renderer::allocationCallbacks); }

	for (U32 i = 0; i < imageCount; ++i) { Renderer::DestroyTextureInstant(renderTargets[i]); }

	surface = nullptr;
	swapchain = nullptr;
}

VkResult Swapchain::Update()
{
	if (width != Settings::WindowWidth() || height != Settings::WindowHeight() || vsync != Settings::VSync()) { return VK_ERROR_OUT_OF_DATE_KHR; }
	if (width == 0 || height == 0) { return VK_NOT_READY; }

	return VK_SUCCESS;
}

VkResult Swapchain::NextImage(U32& frameIndex, VkSemaphore semaphore, VkFence fence)
{
	return vkAcquireNextImageKHR(Renderer::device, swapchain, U64_MAX, semaphore, fence, &frameIndex);
}

VkResult Swapchain::Present(VkQueue queue, U32 imageIndex, U32 waitCount, VkSemaphore* waits)
{
	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = waitCount;
	presentInfo.pWaitSemaphores = waits;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	return vkQueuePresentKHR(queue, &presentInfo);
}
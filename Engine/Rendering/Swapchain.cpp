#include "Swapchain.hpp"

#include "Renderer.hpp"

#include "Math/Math.hpp"

bool Swapchain::Create(bool recreate)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &capabilities);

	imageCount = Math::Min(capabilities.minImageCount + 1, capabilities.maxImageCount, MaxSwapchainImages);

	VkExtent2D extent = FindExtent(capabilities, 0, 0);
	width = extent.width;
	height = extent.height;

	U32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &formatCount, nullptr);
	Vector<VkSurfaceFormatKHR> formats(formatCount, {});
	vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &formatCount, formats.Data());

	Vector<VkSurfaceFormatKHR> desiredFormats = {
		{ VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
		{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
	};

	VkSurfaceFormatKHR surfaceFormat = FindBestSurfaceFormat(formats, desiredFormats);
	format = surfaceFormat.format;

	U32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &presentModeCount, nullptr);
	Vector<VkPresentModeKHR> presentModes(presentModeCount, {});
	vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &presentModeCount, presentModes.Data());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	if (imageCount >= 3)
	{
		for (const VkPresentModeKHR& mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = mode; break; }
		}
	}

	bool sameQueue = Renderer::device.physicalDevice.graphicsQueueIndex == Renderer::device.physicalDevice.presentQueueIndex;
	U32 queueFamilyIndices[]{ Renderer::device.physicalDevice.graphicsQueueIndex, Renderer::device.physicalDevice.presentQueueIndex };

	VkSwapchainCreateInfoKHR swapchainCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = Renderer::device.vkSurface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = sameQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = sameQueue ? 0 : CountOf32(queueFamilyIndices),
		.pQueueFamilyIndices = sameQueue ? nullptr : queueFamilyIndices,
		.preTransform = capabilities.currentTransform,
#if defined(NH_PLATFORM_ANDROID)
		.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
#else
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
#endif
		.presentMode = presentMode,
		.clipped = true,
		.oldSwapchain = vkSwapchain
	};

	VkValidateFR(vkCreateSwapchainKHR(Renderer::device, &swapchainCreateInfo, Renderer::allocationCallbacks, &vkSwapchain));

	U32 imageCount;
	VkValidate(vkGetSwapchainImagesKHR(Renderer::device, vkSwapchain, &imageCount, nullptr));
	images.Resize(imageCount, nullptr);
	VkValidate(vkGetSwapchainImagesKHR(Renderer::device, vkSwapchain, &imageCount, images.Data()));

	imageViews.Resize(images.Size(), nullptr);

	VkImageViewUsageCreateInfo usage = { VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
	usage.pNext = nullptr;
	usage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	for (U64 i = 0; i < images.Size(); ++i)
	{
		VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = &usage,
			.flags = 0,
			.image = images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = surfaceFormat.format,
			.components {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		VkValidateFR(vkCreateImageView(Renderer::device, &createInfo, Renderer::allocationCallbacks, &imageViews[i]));
	}

	return true;
}

void Swapchain::Destroy()
{
	if (vkSwapchain) { vkDestroySwapchainKHR(Renderer::device, vkSwapchain, Renderer::allocationCallbacks); }

	vkSwapchain = nullptr;
}

VkSurfaceFormatKHR Swapchain::FindBestSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats, const Vector<VkSurfaceFormatKHR>& desiredFormats)
{
	for (const VkSurfaceFormatKHR& desiredFormat : desiredFormats)
	{
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (desiredFormat.format == availableFormat.format && desiredFormat.colorSpace == availableFormat.colorSpace)
			{
				return desiredFormat;
			}
		}
	}

	return availableFormats[0];
}

VkExtent2D Swapchain::FindExtent(const VkSurfaceCapabilitiesKHR& capabilities, U32 desiredWidth, U32 desiredHeight)
{
	if (capabilities.currentExtent.width != U32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { desiredWidth, desiredHeight };

		actualExtent.width = Math::Max(capabilities.minImageExtent.width, Math::Min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = Math::Max(capabilities.minImageExtent.height, Math::Min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

U32 Swapchain::ImageCount() const
{
	return imageCount;
}

Swapchain::operator VkSwapchainKHR_T* () const
{
	return vkSwapchain;
}

VkSwapchainKHR_T* const* Swapchain::operator&() const
{
	return &vkSwapchain;
}
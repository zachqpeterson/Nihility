#include "Swapchain.hpp"

#include "Renderer.hpp"

#include "Math/Math.hpp"

bool Swapchain::Create(bool recreate)
{
    if (!recreate)
    {
        surfaceSupport = QuerySurfaceSupportDetails();

        imageCount = Math::Min(surfaceSupport.capabilities.minImageCount + 1, surfaceSupport.capabilities.maxImageCount);

        surfaceFormat = FindBestSurfaceFormat(surfaceSupport.formats, surfaceSupport.formats);

        extent = FindExtent(surfaceSupport.capabilities, 0, 0);

        imageArrayLayers = 1;

        queueFamilyIndices[0] = Renderer::device.GetQueueIndex(QueueType::Graphics);
        queueFamilyIndices[1] = Renderer::device.GetQueueIndex(QueueType::Present);

        Vector<VkPresentModeKHR> desiredPresentModes = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };

        presentMode = FindPresentMode(surfaceSupport.presentModes, desiredPresentModes);

        preTransform = surfaceSupport.capabilities.currentTransform;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = Renderer::device.vkSurface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = imageArrayLayers;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode;
    swapchainCreateInfo.preTransform = preTransform;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.oldSwapchain = vkSwapchain;

    if (queueFamilyIndices[0] != queueFamilyIndices[1])
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

#if defined(NH_PLATFORM_ANDROID)
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif

    VkValidateFR(vkCreateSwapchainKHR(Renderer::device, &swapchainCreateInfo, Renderer::allocationCallbacks, &vkSwapchain));

    return true;
}

void Swapchain::Destroy()
{
    if (vkSwapchain) { vkDestroySwapchainKHR(Renderer::device, vkSwapchain, Renderer::allocationCallbacks); }

    vkSwapchain = nullptr;
}

Swapchain::SurfaceSupportDetails Swapchain::QuerySurfaceSupportDetails()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &capabilities);

    U32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &formatCount, nullptr);
    Vector<VkSurfaceFormatKHR> formats(formatCount, {});
    vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &formatCount, formats.Data());

    U32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &presentModeCount, nullptr);
    Vector<VkPresentModeKHR> presentModes(presentModeCount, {});
    vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer::device.physicalDevice, Renderer::device.vkSurface, &presentModeCount, presentModes.Data());

    return { capabilities, Move(formats), Move(presentModes) };
}

VkSurfaceFormatKHR Swapchain::FindDesiredSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats, const Vector<VkSurfaceFormatKHR>& desiredFormats)
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

    return {};
}

VkSurfaceFormatKHR Swapchain::FindBestSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats, const Vector<VkSurfaceFormatKHR>& desiredFormats)
{
    VkSurfaceFormatKHR surfaceFormat = FindDesiredSurfaceFormat(availableFormats, desiredFormats);
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED) return surfaceFormat;

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

VkPresentModeKHR Swapchain::FindPresentMode(const Vector<VkPresentModeKHR>& availableResentModes,
    const Vector<VkPresentModeKHR>& desiredPresentModes)
{
    for (const VkPresentModeKHR& desiredPm : desiredPresentModes)
    {
        for (const VkPresentModeKHR& availablePm : availableResentModes)
        {
            if (desiredPm == availablePm) { return desiredPm; }
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

Vector<VkImage> Swapchain::GetImages()
{
    U32 imageCount;
    VkValidate(vkGetSwapchainImagesKHR(Renderer::device, vkSwapchain, &imageCount, nullptr));
    Vector<VkImage> swapchainImages(imageCount, {});
    VkValidate(vkGetSwapchainImagesKHR(Renderer::device, vkSwapchain, &imageCount, swapchainImages.Data()));

    return swapchainImages;
}

Vector<VkImageView> Swapchain::GetImageViews()
{
    Vector<VkImage> images = GetImages();

    Vector<VkImageView> views(images.Size(), {});

    VkImageViewUsageCreateInfo usage = { VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
    usage.pNext = nullptr;
    usage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    for (U64 i = 0; i < images.Size(); ++i)
    {
        VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.pNext = &usage;
        createInfo.flags = 0;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = surfaceFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkValidateFR(vkCreateImageView(Renderer::device, &createInfo, Renderer::allocationCallbacks, &views[i]));
    }

    return views;
}

Swapchain::operator VkSwapchainKHR() const
{
	return vkSwapchain;
}

const VkSwapchainKHR* Swapchain::operator&() const
{
    return &vkSwapchain;
}
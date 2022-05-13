#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"

#include "Math/Math.hpp"
#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"

bool VulkanSwapchain::Create(RendererState* rendererState, U32 width, U32 height)
{
    LOG_INFO("Creating vulkan swapchain...");

    VkExtent2D swapchainExtent = { width, height };

    bool found = false;
    for (U32 i = 0; i < rendererState->device->swapchainSupport.formatCount; ++i)
    {
        VkSurfaceFormatKHR format = rendererState->device->swapchainSupport.formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            imageFormat = format;
            found = true;
            break;
        }
    }

    if (!found)
    {
        imageFormat = rendererState->device->swapchainSupport.formats[0];
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (U32 i = 0; i < rendererState->device->swapchainSupport.presentModeCount; ++i)
    {
        VkPresentModeKHR mode = rendererState->device->swapchainSupport.presentModes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            break;
        }
    }

    rendererState->device->QuerySwapchainSupport(rendererState->device->physicalDevice, rendererState->surface, &rendererState->device->swapchainSupport);

    if (rendererState->device->swapchainSupport.capabilities.currentExtent.width != UINT32_MAX)
    {
        swapchainExtent = rendererState->device->swapchainSupport.capabilities.currentExtent;
    }

    VkExtent2D min = rendererState->device->swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = rendererState->device->swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = Math::Clamp(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = Math::Clamp(swapchainExtent.height, min.height, max.height);

    U32 imageCount = rendererState->device->swapchainSupport.capabilities.minImageCount + 1;
    if (rendererState->device->swapchainSupport.capabilities.maxImageCount > 0 && imageCount > rendererState->device->swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = rendererState->device->swapchainSupport.capabilities.maxImageCount;
    }

    maxFramesInFlight = imageCount - 1;

    VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = NULL; //TODO: Look into mutable
    swapchainInfo.surface = rendererState->surface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = imageFormat.format;
    swapchainInfo.imageColorSpace = imageFormat.colorSpace;
    swapchainInfo.imageExtent = swapchainExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (rendererState->device->graphicsQueueIndex != rendererState->device->presentQueueIndex)
    {
        U32 queueFamilyIndices[] = { (U32)rendererState->device->graphicsQueueIndex, (U32)rendererState->device->presentQueueIndex };
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainInfo.preTransform = rendererState->device->swapchainSupport.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = nullptr; //TODO: Use this for recreation

    VkCheck(vkCreateSwapchainKHR(rendererState->device->logicalDevice, &swapchainInfo, rendererState->allocator, &swapchain));

    rendererState->currentFrame = 0;

    imageCount = 0;
    VkCheck(vkGetSwapchainImagesKHR(rendererState->device->logicalDevice, swapchain, &imageCount, 0));
    if (!images)
    {
        images = (VkImage*)Memory::Allocate(sizeof(VkImage) * imageCount, MEMORY_TAG_RENDERER);
    }
    if (!views)
    {
        views = (VkImageView*)Memory::Allocate(sizeof(VkImageView) * imageCount, MEMORY_TAG_RENDERER);
    }
    VkCheck(vkGetSwapchainImagesKHR(rendererState->device->logicalDevice, swapchain, &imageCount, images));

    for (U32 i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkCheck(vkCreateImageView(rendererState->device->logicalDevice, &viewInfo, rendererState->allocator, &views[i]));
    }

    if (!rendererState->device->DetectDepthFormat(rendererState))
    {
        rendererState->device->depthFormat = VK_FORMAT_UNDEFINED;
        LOG_FATAL("Failed to find a supported format!");
    }

    //vulkan_image_create(
    //    context,
    //    VK_IMAGE_TYPE_2D,
    //    swapchain_extent.width,
    //    swapchain_extent.height,
    //    context->device.depth_format,
    //    VK_IMAGE_TILING_OPTIMAL,
    //    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    //    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //    true,
    //    VK_IMAGE_ASPECT_DEPTH_BIT,
    //    &swapchain->depth_attachment);

    return true;
}

void VulkanSwapchain::Destroy(RendererState* rendererState)
{
    LOG_INFO("Destroying swapchain...");

    vkDeviceWaitIdle(rendererState->device->logicalDevice);
    //vulkan_image_destroy(context, &swapchain->depth_attachment);

    for (U32 i = 0; i < imageCount; ++i)
    {
        vkDestroyImageView(rendererState->device->logicalDevice, views[i], rendererState->allocator);
    }

    vkDestroySwapchainKHR(rendererState->device->logicalDevice, swapchain, rendererState->allocator);
}
#include "VulkanImage.hpp"

#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"

#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"

bool VulkanImage::Create(RendererState* rendererState,
    VkImageType imageType,
    U32 width,
    U32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    I32 createView,
    VkImageAspectFlags viewAspectFlags)
{
    this->width = width;
    this->height = height;

    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;  // TODO: Support configurable depth.
    imageInfo.mipLevels = 4;     // TODO: Support mip mapping
    imageInfo.arrayLayers = 1;   // TODO: Support number of layers in the image.
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;          // TODO: Configurable sample count.
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // TODO: Configurable sharing mode.

    VkCheck_ERROR(vkCreateImage(rendererState->device->logicalDevice, &imageInfo, rendererState->allocator, &handle));

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(rendererState->device->logicalDevice, handle, &memoryRequirements);

    U32 memoryType = rendererState->FindMemoryIndex(memoryRequirements.memoryTypeBits, memoryFlags);
    if (memoryType == -1)
    {
        Logger::Error("Required memory type not found. Image not valid.");
    }

    VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryType;
    VkCheck_ERROR(vkAllocateMemory(rendererState->device->logicalDevice, &memoryAllocateInfo, rendererState->allocator, &memory));

    VkCheck_ERROR(vkBindImageMemory(rendererState->device->logicalDevice, handle, memory, 0));  // TODO: configurable memory offset.

    if (createView)
    {
        view = nullptr;
        CreateImageView(rendererState, format, viewAspectFlags);
    }

    return true;
}

void VulkanImage::Destroy(RendererState* rendererState)
{
    if (view)
    {
        vkDestroyImageView(rendererState->device->logicalDevice, view, rendererState->allocator);
        view = nullptr;
    }
    if (memory)
    {
        vkFreeMemory(rendererState->device->logicalDevice, memory, rendererState->allocator);
        memory = nullptr;
    }
    if (handle)
    {
        vkDestroyImage(rendererState->device->logicalDevice, handle, rendererState->allocator);
        handle = nullptr;
    }
}

void VulkanImage::CreateImageView(RendererState* rendererState, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = handle;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // TODO: Make configurable.
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;

    // TODO: Make configurable
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkCheck(vkCreateImageView(rendererState->device->logicalDevice, &viewInfo, rendererState->allocator, &view));
}

void VulkanImage::TransitionLayout(RendererState* rendererState,
    VulkanCommandBuffer* commandBuffer,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = rendererState->device->graphicsQueueIndex;
    barrier.dstQueueFamilyIndex = rendererState->device->graphicsQueueIndex;
    barrier.image = handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        Logger::Fatal("unsupported layout transition!");
        return;
    }

    vkCmdPipelineBarrier(
        commandBuffer->handle,
        sourceStage, destStage,
        0,
        0, 0,
        0, 0,
        1, &barrier);
}

void VulkanImage::CopyFromBuffer(RendererState* rendererState,
    VkBuffer buffer,
    VulkanCommandBuffer* commandBuffer)
{
    VkBufferImageCopy region;
    Memory::Zero(&region, sizeof(VkBufferImageCopy));
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        commandBuffer->handle,
        buffer,
        handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}
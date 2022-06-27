#pragma once

#include "VulkanDefines.hpp"

class VulkanImage
{
public:
    bool Create(RendererState* rendererState,
        VkImageType imageType,
        U32 width,
        U32 height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memoryFlags,
        I32 createView,
        VkImageAspectFlags viewAspectFlags);
    void Destroy(RendererState* rendererState);

    void CreateImageView(RendererState* rendererState,
        VkFormat format,
        VkImageAspectFlags aspectFlags);
    void TransitionLayout(RendererState* rendererState,
        class VulkanCommandBuffer* commandBuffer,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);
    void CopyFromBuffer(RendererState* rendererState,
        VkBuffer buffer,
        class VulkanCommandBuffer* commandBuffer);

    VkImage handle;
    VkImageView view;
    VkDeviceMemory memory;

    U32 width;
    U32 height;
};
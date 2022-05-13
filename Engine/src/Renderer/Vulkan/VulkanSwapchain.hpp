#pragma once

#include "VulkanDefines.hpp"

class VulkanSwapchain
{
public:
    bool Create(RendererState* rendererState, U32 width, U32 height);
    void Recreate(RendererState* rendererState, U32 width, U32 height);
    void Destroy(RendererState* rendererState);

private:
    VkSwapchainKHR swapchain;

    VkSurfaceFormatKHR imageFormat;
    U8 maxFramesInFlight;

    VkSwapchainKHR handle;
    U32 imageCount;
    VkImage* images;
    VkImageView* views;

    class VulkanImage* depthAttachment;

    VkFramebuffer framebuffers[3];
};
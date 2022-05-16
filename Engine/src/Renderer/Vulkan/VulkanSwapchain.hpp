#pragma once

#include "VulkanDefines.hpp"

class VulkanSwapchain
{
public:
    bool Create(RendererState* rendererState, U32 width, U32 height);
    void Destroy(RendererState* rendererState);
    void Recreate(RendererState* rendererState, U32 width, U32 height);
    bool AcquireNextImageIndex(
        RendererState* rendererState,
        U64 timeoutNs,
        VkSemaphore imageAvailableSemaphore,
        VkFence fence,
        U32* outImageIndex);
    void Present(
        RendererState* rendererState,
        VkQueue graphicsQueue,
        VkQueue presentQueue,
        VkSemaphore renderCompleteSemaphore,
        U32 presentImageIndex);

public:
    VkSwapchainKHR handle;

    VkSurfaceFormatKHR imageFormat;
    U8 maxFramesInFlight;

    U32 imageCount;
    VkImage* images;
    VkImageView* views;

    class VulkanImage* depthAttachment;

    VkFramebuffer framebuffers[3];
};
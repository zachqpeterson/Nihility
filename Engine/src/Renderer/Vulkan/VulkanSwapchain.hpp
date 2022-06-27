#pragma once

#include "VulkanDefines.hpp"

template<typename> struct Vector;

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
    Vector<Texture*> renderTextures;
    Texture* depthTexture;

    RenderTarget renderTargets[3];
};
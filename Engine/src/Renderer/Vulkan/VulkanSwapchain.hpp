#pragma once

#include "VulkanDefines.hpp"

template<typename> struct Vector;

class Swapchain
{
public:
	static bool Initialize(RendererState* rendererState, U32 width, U32 height);
	static void Shutdown(RendererState* rendererState, bool end);
	static void Recreate(RendererState* rendererState, U32 width, U32 height);
	static bool AcquireNextImageIndex(
		RendererState* rendererState,
		U64 timeoutNs,
		VkSemaphore imageAvailableSemaphore,
		VkFence fence,
		U32* outImageIndex);
	static void Present(
		RendererState* rendererState,
		VkQueue graphicsQueue,
		VkQueue presentQueue,
		VkSemaphore renderCompleteSemaphore,
		U32 presentImageIndex);

public:
	static VkSwapchainKHR handle;
	static VkSwapchainKHR oldHandle;

	static VkSurfaceFormatKHR imageFormat;
	static U8 maxFramesInFlight;

	static U32 imageCount;
	static Vector<Texture*> renderTextures;
	static Texture* colorTexture;
	static Texture* depthTexture;

	static RenderTarget renderTargets[3];
};
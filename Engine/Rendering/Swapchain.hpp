#pragma once

#include "Resources\ResourceDefines.hpp"

struct Swapchain
{
public:
	bool CreateSurface();
	bool GetFormat();
	bool Create();
	void Destroy();

	VkResult Update();
	VkResult NextImage(U32& frameIndex, VkSemaphore semaphore = nullptr, VkFence fence = nullptr);
	VkResult Present(VkQueue queue, U32 imageIndex, U32 waitCount, const VkSemaphore* waits);

public:
	VkSwapchainKHR		swapchain{ nullptr };
	VkSurfaceKHR		surface{ nullptr };
	VkSurfaceFormatKHR	surfaceFormat{};
	U32					imageCount{ MAX_SWAPCHAIN_IMAGES };
	Texture*			renderTargets[MAX_SWAPCHAIN_IMAGES];

	U32 width;
	U32 height;

private:
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
};
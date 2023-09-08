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

public:
	VkSwapchainKHR		swapchain{ nullptr };
	VkSurfaceKHR		surface{ nullptr };
	VkSurfaceFormatKHR	surfaceFormat{};
	U32					imageCount{ 3 };
	Texture*			renderTargets[MAX_SWAPCHAIN_IMAGES]{ nullptr };

	U32					width;
	U32					height;

private:
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
};
#pragma once

#include "Resources\ResourceDefines.hpp"

struct Swapchain
{
public:
	bool CreateSurface();
	bool GetFormat();
	bool CreateRenderpass();
	bool Create();
	void Destroy();

	VkResult Update();
	VkResult NextImage(U32& frameIndex, VkSemaphore semaphore = nullptr, VkFence fence = nullptr);

public:
	VkSwapchainKHR		swapchain{ nullptr };
	VkSurfaceKHR		surface{ nullptr };
	VkSurfaceFormatKHR	surfaceFormat{};
	U32					imageCount{ 3 };
	Renderpass			renderpass{ };

private:
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	bool created{ false };
};
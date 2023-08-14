#pragma once

#include "Resources\ResourceDefines.hpp"

struct Swapchain
{
public:
	bool CreateSurface();
	bool GetFormat();
	bool CreateRenderPass();
	bool Create();
	void Destroy();

	VkResult Update();
	VkResult NextImage(U32& frameIndex, VkSemaphore semaphore = nullptr, VkFence fence = nullptr);

public:
	VkSwapchainKHR swapchain{ nullptr };
	VkSurfaceKHR surface{ nullptr };
	VkSurfaceFormatKHR surfaceFormat{};
	Renderpass* renderpass{ nullptr };
	U32 imageCount{ 3 };

private:
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	RenderpassInfo renderPassInfo{};
};
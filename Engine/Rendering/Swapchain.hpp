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

	VkResult NextImage(U32& imageIndex, VkSemaphore semaphore = nullptr, VkFence fence = nullptr);
	VkResult Present(VkQueue queue, U32 imageIndex, VkSemaphore semaphore = nullptr);

public:
	VkSwapchainKHR swapchain{ nullptr };
	VkSurfaceKHR surface{ nullptr };
	VkSurfaceFormatKHR surfaceFormat{};
	RenderPass* renderPass{ nullptr };
	U32 imageCount{ 3 };

private:
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	RenderPassCreation renderPassInfo{};
};
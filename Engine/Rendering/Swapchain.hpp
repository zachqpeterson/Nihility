#pragma once

#include "Resources\ResourceDefines.hpp"

struct VkQueue_T;
struct VkFence_T;
struct VkSemaphore_T;
struct VkSwapchainKHR_T;
struct VkSurfaceKHR_T;
enum VkResult;

struct Swapchain
{
public:
	bool CreateSurface();
	bool GetFormat();
	bool Create();
	void Destroy();

	VkResult Update();
	VkResult NextImage(U32& frameIndex, VkSemaphore_T* semaphore = nullptr, VkFence_T* fence = nullptr);
	VkResult Present(VkQueue_T* queue, U32 imageIndex, U32 waitCount, VkSemaphore_T** waits);

public:
	VkSwapchainKHR_T*			swapchain{ nullptr };
	VkSurfaceKHR_T*				surface{ nullptr };
	U32							imageCount{ MAX_SWAPCHAIN_IMAGES };
	ResourceRef<Texture>	renderTargets[MAX_SWAPCHAIN_IMAGES];

	U32							width;
	U32							height;
	bool						vsync;
};
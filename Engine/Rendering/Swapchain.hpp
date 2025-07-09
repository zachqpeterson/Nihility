#pragma once

#include "Defines.hpp"

#include "Containers/Vector.hpp"

struct VkSwapchainKHR_T;
struct VkImage_T;
struct VkImageView_T;
struct VkSurfaceFormatKHR;
struct VkExtent2D;
struct VkSurfaceCapabilitiesKHR;

struct Swapchain
{
	U32 ImageCount() const;

	operator VkSwapchainKHR_T* () const;
	VkSwapchainKHR_T* const* operator&() const;

private:
	bool Create(bool recreate);
	void Destroy();

	VkSurfaceFormatKHR FindBestSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats, const Vector<VkSurfaceFormatKHR>& desiredFormats);
	VkExtent2D FindExtent(const VkSurfaceCapabilitiesKHR& capabilities, U32 desiredWidth, U32 desiredHeight);

	U32 imageCount = 0;
	Vector<VkImage_T*> images;
	Vector<VkImageView_T*> imageViews;
	U32 format;
	U32 width;
	U32 height;

	VkSwapchainKHR_T* vkSwapchain = nullptr;

	friend class Renderer;
	friend class Resources;
	friend struct Renderpass;
	friend struct FrameBuffer;
	friend struct CommandBuffer;
};
#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Containers/Vector.hpp"

struct Swapchain
{
private:
	struct SurfaceSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		Vector<VkSurfaceFormatKHR> formats;
		Vector<VkPresentModeKHR> presentModes;
	};

	operator VkSwapchainKHR() const;
	const VkSwapchainKHR* operator&() const;

private:
	bool Create(bool recreate);
	void Destroy();

	SurfaceSupportDetails QuerySurfaceSupportDetails();
	VkSurfaceFormatKHR FindDesiredSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats, const Vector<VkSurfaceFormatKHR>& desiredFormats);
	VkSurfaceFormatKHR FindBestSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats, const Vector<VkSurfaceFormatKHR>& desiredFormats);
	VkExtent2D FindExtent(const VkSurfaceCapabilitiesKHR& capabilities, U32 desiredWidth, U32 desiredHeight);

	Vector<VkImage> GetImages();
	Vector<VkImageView> GetImageViews();

	VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
	SurfaceSupportDetails surfaceSupport;
	U32 imageCount = 0;
	VkSurfaceFormatKHR surfaceFormat;
	VkImageUsageFlags imageUsageFlags = 0;
	VkExtent2D extent = { 0, 0 };
	U32 imageArrayLayers = 0;
	U32 queueFamilyIndices[2];
	VkPresentModeKHR presentMode;
	VkSurfaceTransformFlagBitsKHR preTransform;

	friend class Renderer;
	friend class Resources;
	friend struct Renderpass;
	friend struct FrameBuffer;
};
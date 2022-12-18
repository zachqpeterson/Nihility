#pragma once

#include "VulkanDefines.hpp"

template<typename> struct Vector;

struct SwapchainSupportInfo
{
	VkSurfaceCapabilitiesKHR capabilities{};
	Vector<VkSurfaceFormatKHR> formats;
	Vector<VkPresentModeKHR> presentModes;
};

class Device
{
public:
	static bool Initialize(RendererState* rendererState);
	static void Shutdown(RendererState* rendererState);

	static void QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainSupportInfo* outSupportInfo);
	static bool DetectDepthFormat();

private:
	static bool SelectPhysicalDevice(RendererState* rendererState);
	static bool physicalDeviceMeetsRequirements(
		VkPhysicalDevice device,
		VkSurfaceKHR surface,
		const VkPhysicalDeviceProperties* properties,
		const VkPhysicalDeviceFeatures* features,
		const struct PhysicalDeviceRequirements* requirements,
		struct PhysicalDeviceQueueFamilyInfo* outQueueInfo,
		SwapchainSupportInfo* outSwapchainSupport);

public:
	static VkPhysicalDevice physicalDevice;
	static VkDevice logicalDevice;
	static SwapchainSupportInfo swapchainSupport;

	static I32 graphicsQueueIndex;
	static I32 presentQueueIndex;
	static I32 transferQueueIndex;
	static bool supportsDeviceLocalHostVisible;

	static VkQueue graphicsQueue;
	static VkQueue presentQueue;
	static VkQueue transferQueue;

	static VkCommandPool graphicsCommandPool;

	static VkPhysicalDeviceProperties properties;
	static VkPhysicalDeviceFeatures features;
	static VkPhysicalDeviceMemoryProperties memory;

	static VkSampleCountFlagBits maxSamples;
	static VkFormat depthFormat;
	static U8 depthChannelCount;
};
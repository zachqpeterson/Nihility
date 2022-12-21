#pragma once

#include "VulkanDefines.hpp"

struct RendererState;

struct PhysicalDeviceRequirements
{
	bool graphics;
	bool present;
	bool compute;
	bool transfer;
	bool samplerAnisotropy;
	bool discreteGpu;
	Vector<const char*> deviceExtensionNames;
};

struct PhysicalDeviceQueueFamilyInfo
{
	U32 graphicsFamilyIndex;
	U32 presentFamilyIndex;
	U32 computeFamilyIndex;
	U32 transferFamilyIndex;
};

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

	static void QuerySwapchainSupport(VkSurfaceKHR surface);
	static bool DetectDepthFormat();

private:
	static bool SelectPhysicalDevice(RendererState* rendererState);
	static bool PhysicalDeviceMeetsRequirements(VkSurfaceKHR surface, const PhysicalDeviceRequirements& requirements,
		PhysicalDeviceQueueFamilyInfo& outQueueInfo, SwapchainSupportInfo& outSwapchainSupport);

public:
	static VkPhysicalDevice physicalDevice;
	static VkDevice logicalDevice;
	static SwapchainSupportInfo swapchainSupport;

	static I32 graphicsQueueIndex;
	static I32 presentQueueIndex;
	static I32 transferQueueIndex;
	static I32 computeQueueIndex;
	static bool supportsDeviceLocalHostVisible;

	static VkQueue graphicsQueue;
	static VkQueue presentQueue;
	static VkQueue transferQueue;
	static VkQueue computeQueue;

	static VkCommandPool graphicsCommandPool;

	static VkPhysicalDeviceProperties properties;
	static VkPhysicalDeviceFeatures features;
	static VkPhysicalDeviceMemoryProperties memory;

	static I32 maxSamples;
	static VkFormat depthFormat;
	static U8 depthChannelCount;
};
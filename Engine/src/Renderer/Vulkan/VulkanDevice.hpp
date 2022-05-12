#pragma once

#include "VulkanDefines.hpp"

struct SwapchainSupportInfo
{
    VkSurfaceCapabilitiesKHR capabilities;
    U32 formatCount;
    VkSurfaceFormatKHR* formats;
    U32 presentModeCount;
    VkPresentModeKHR* presentModes;
};

class VulkanDevice
{
public:
    bool Create(RendererState* rendererState);
    void Destroy(RendererState* rendererState);

private:
    bool SelectPhysicalDevice(RendererState* rendererState);
    bool physicalDeviceMeetsRequirements(
        VkPhysicalDevice device,
        VkSurfaceKHR surface,
        const VkPhysicalDeviceProperties* properties,
        const VkPhysicalDeviceFeatures* features,
        const struct PhysicalDeviceRequirements* requirements,
        struct PhysicalDeviceQueueFamilyInfo* outQueueInfo,
        SwapchainSupportInfo* outSwapchainSupport);
    void QuerySwapchainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        SwapchainSupportInfo* outSupportInfo);
    bool DetectDepthFormat(RendererState* rendererState);

    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    SwapchainSupportInfo swapchainSupport;

    I32 graphicsQueueIndex;
    I32 presentQueueIndex;
    I32 transferQueueIndex;
    bool supportsDeviceLocalHostVisible;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkCommandPool graphicsCommandPool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depthFormat;
};
#pragma once

#include "VulkanDefines.hpp"

template<typename> struct Vector;

struct SwapchainSupportInfo
{
    VkSurfaceCapabilitiesKHR capabilities;
    Vector<VkSurfaceFormatKHR> formats;
    Vector<VkPresentModeKHR> presentModes;
};

class VulkanDevice
{
public:
    bool Create(RendererState* rendererState);
    void Destroy(RendererState* rendererState);

    void QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainSupportInfo* outSupportInfo);
    bool DetectDepthFormat();

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

public:
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

    VkSampleCountFlagBits maxSamples;
    VkFormat depthFormat;
    U8 depthChannelCount;
};
#pragma once

#include "Defines.hpp"

#if defined(PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(PLATFORM_LINUX)
#define VK_USE_PLATFORM_XCB_KHR
#elif defined(PLATFORM_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(PLATFORM_APPLE)
#define VK_USE_PLATFORM_METAL_EXT
#elif defined(PLATFORM_IOS)
#define VK_USE_PLATFORM_IOS_MVK
#endif

#include <vulkan/vulkan.hpp>

#define VkCheck(expr)                   \
{                                       \
    VkResult result = expr;             \
    ASSERT(result == VK_SUCCESS);       \
}

struct RendererState
{
    VkAllocationCallbacks* allocator;
    VkInstance instance;
#ifdef NH_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    VkSurfaceKHR surface;
    class VulkanDevice* device;
    class VulkanSwapchain* swapchain;

    U32 currentFrame;
};
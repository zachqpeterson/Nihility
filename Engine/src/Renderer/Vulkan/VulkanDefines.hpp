#pragma once

#include "Defines.hpp"

#include "Containers/Vector.hpp"

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
    //Function Pointers
    I32(*FindMemoryIndex)(U32, VkMemoryPropertyFlags);

    //Variables
    U32 framebufferWidth;
    U32 framebufferHeight;
    U64 framebufferSizeGeneration;
    U64 framebufferSizeLastGeneration;

    VkAllocationCallbacks* allocator;
    VkInstance instance;
#ifdef NH_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
    VkSurfaceKHR surface;
    class VulkanDevice* device;
    class VulkanSwapchain* swapchain;
    class VulkanRenderpass* mainRenderpass;
    class VulkanRenderpass* uiRenderpass;

    Vector<class VulkanCommandBuffer> graphicsCommandBuffers;
    Vector<VkSemaphore> imageAvailableSemaphores;
    Vector<VkSemaphore> queueCompleteSemaphores;

    U32 inFlightFenceCount;
    VkFence inFlightFences[2];
    VkFence imagesInFlight[3];

    VkFramebuffer worldFramebuffers[3];

    U32 currentFrame;
};
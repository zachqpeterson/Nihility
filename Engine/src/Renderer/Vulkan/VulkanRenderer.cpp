#include "VulkanRenderer.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanImage.hpp"
#include "VulkanCommandBuffer.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"
#include "Math/Math.hpp"

//TODO: tempary
#include <string>

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* user_data)
{
    switch (messageSeverity)
    {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERROR(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARN(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_INFO(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_TRACE(callbackData->pMessage); break;
    }

    return VK_FALSE;
}

static RendererState* rendererState;

bool VulkanRenderer::Initialize()
{
    rendererState = (RendererState*)Memory::Allocate(sizeof(RendererState), MEMORY_TAG_RENDERER);
    rendererState->FindMemoryIndex = FindMemoryIndex;
    rendererState->device = (VulkanDevice*)Memory::Allocate(sizeof(VulkanDevice), MEMORY_TAG_RENDERER);
    rendererState->swapchain = (VulkanSwapchain*)Memory::Allocate(sizeof(VulkanSwapchain), MEMORY_TAG_RENDERER);
    rendererState->mainRenderpass = (VulkanRenderpass*)Memory::Allocate(sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);
    rendererState->uiRenderpass = (VulkanRenderpass*)Memory::Allocate(sizeof(VulkanRenderpass), MEMORY_TAG_RENDERER);

    rendererState->framebufferWidth = 1280; //TODO: Get width from platform
    rendererState->framebufferHeight = 720;

    rendererState->allocator = nullptr;

    if (!CreateInstance() || !CreateDebugger() || !CreateSurface()) { return false; }

    rendererState->device->Create(rendererState);
    rendererState->swapchain->Create(rendererState, rendererState->framebufferWidth, rendererState->framebufferHeight);

    rendererState->mainRenderpass->Create(rendererState, { 0, 0, (F32)rendererState->framebufferWidth, (F32)rendererState->framebufferHeight }, { 0.0f, 0.0f, 0.2f, 1.0f },
        1.0f, 0, RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG | RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG | RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG, false, true);

    rendererState->uiRenderpass->Create(rendererState, { 0, 0, (F32)rendererState->framebufferWidth, (F32)rendererState->framebufferHeight }, { 0.0f, 0.0f, 0.0f, 0.0f },
        1.0f, 0, RENDERPASS_CLEAR_NONE_FLAG, true, false);

    RecreateFramebuffers();

    CreateCommandBuffers();

    CreateSyncObjects();

    return true;
}

void VulkanRenderer::Shutdown()
{
    LOG_INFO("Destroying vulkan sync objects...");

    for (U8 i = 0; i < rendererState->swapchain->maxFramesInFlight; ++i)
    {
        if (rendererState->imageAvailableSemaphores[i])
        {
            vkDestroySemaphore(rendererState->device->logicalDevice, rendererState->imageAvailableSemaphores[i], rendererState->allocator);
            rendererState->imageAvailableSemaphores[i] = nullptr;
        }
        if (rendererState->queueCompleteSemaphores[i])
        {
            vkDestroySemaphore(rendererState->device->logicalDevice, rendererState->queueCompleteSemaphores[i], rendererState->allocator);
            rendererState->queueCompleteSemaphores[i] = nullptr;
        }
        vkDestroyFence(rendererState->device->logicalDevice, rendererState->inFlightFences[i], rendererState->allocator);
    }

    LOG_INFO("Destroying vulkan command buffers...");
    for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        if (rendererState->graphicsCommandBuffers[i].handle)
        {
            rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
            rendererState->graphicsCommandBuffers[i].handle = nullptr;
        }
    }

    for (I32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        vkDestroyFramebuffer(rendererState->device->logicalDevice, rendererState->worldFramebuffers[i], rendererState->allocator);
        vkDestroyFramebuffer(rendererState->device->logicalDevice, rendererState->swapchain->framebuffers[i], rendererState->allocator);
    }

    rendererState->uiRenderpass->Destroy(rendererState);
    rendererState->mainRenderpass->Destroy(rendererState);

    rendererState->swapchain->Destroy(rendererState);

    rendererState->device->Destroy(rendererState);

    vkDestroySurfaceKHR(rendererState->instance, rendererState->surface, rendererState->allocator);
#ifdef NH_DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(rendererState->instance, "vkDestroyDebugUtilsMessengerEXT");
    func(rendererState->instance, rendererState->debugMessenger, rendererState->allocator);
#endif
    vkDestroyInstance(rendererState->instance, rendererState->allocator);

    Memory::Free(rendererState, sizeof(RendererState), MEMORY_TAG_RENDERER);
}

void* VulkanRenderer::operator new(U64 size)
{
    return Memory::Allocate(size, MEMORY_TAG_RENDERER);
}

void VulkanRenderer::operator delete(void* p)
{
    Memory::Free(p, sizeof(VulkanRenderer), MEMORY_TAG_RENDERER);
}

bool VulkanRenderer::CreateInstance()
{
    LOG_INFO("Creating vulkan instance...");

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_VERSION_1_3;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.pApplicationName = "TEST";
    appInfo.pEngineName = "Nihility";
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.pApplicationInfo = &appInfo;

    Vector<const char*> extentionNames;
    extentionNames.Push(VK_KHR_SURFACE_EXTENSION_NAME);
    GetPlatformExtentions(&extentionNames);

#ifdef NH_DEBUG
    extentionNames.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    Vector<const char*> layerNames;
    layerNames.Push("VK_LAYER_KHRONOS_validation");

    U32 availableLayerCount;
    VkCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
    Vector<VkLayerProperties> availableLayers(availableLayerCount, {});
    VkCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.Data()));

    for (U32 i = 0; i < layerNames.Size(); ++i)
    {
        bool found = false;
        for (U32 j = 0; j < availableLayerCount; ++j)
        {
            if (strcmp(layerNames[i], availableLayers[j].layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            LOG_FATAL("Required validation layer is missing: %s", (char*)layerNames[i]);
            return false;
        }
    }

    instanceInfo.enabledLayerCount = layerNames.Size();
    instanceInfo.ppEnabledLayerNames = layerNames.Data();
#else
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
#endif

    instanceInfo.enabledExtensionCount = extentionNames.Size();
    instanceInfo.ppEnabledExtensionNames = extentionNames.Data();

    instanceInfo.flags = NULL;
    instanceInfo.pNext = nullptr;

    return vkCreateInstance(&instanceInfo, rendererState->allocator, &rendererState->instance) == VK_SUCCESS;
}

bool VulkanRenderer::CreateDebugger()
{
#ifdef NH_DEBUG
    LOG_DEBUG("Creating vulkan debugger...");
    U32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
    //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    debugInfo.messageSeverity = logSeverity;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugInfo.pfnUserCallback = VkDebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(rendererState->instance, "vkCreateDebugUtilsMessengerEXT");
    ASSERT_MSG(func, "Failed to create debug messenger!");
    return func(rendererState->instance, &debugInfo, rendererState->allocator, &rendererState->debugMessenger) == VK_SUCCESS;
#else
    return true;
#endif
}

bool VulkanRenderer::CreateSurface()
{
    LOG_INFO("Creating vulkan surface...");

#ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = NULL;
    Platform::GetVulkanSurfaceInfo(&surfaceInfo.flags + sizeof(VkWin32SurfaceCreateFlagsKHR));
    return vkCreateWin32SurfaceKHR(rendererState->instance, &surfaceInfo, rendererState->allocator, &rendererState->surface) == VK_SUCCESS;
#elif PLATFORM_LINUX
    //TODO:
#elif PLATFORM_APPLE
    //TODO:
#endif
    return false;
}

void VulkanRenderer::CreateCommandBuffers()
{
    LOG_INFO("Creating vulkan command buffers...");

    if (!rendererState->graphicsCommandBuffers.Size())
    {
        rendererState->graphicsCommandBuffers.Resize(rendererState->swapchain->imageCount);
        for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
        {
            Memory::ZeroMemory(&rendererState->graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }

    for (I32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        if (rendererState->graphicsCommandBuffers[i].handle)
        {
            rendererState->graphicsCommandBuffers[i].Free(rendererState, rendererState->device->graphicsCommandPool);
        }

        Memory::ZeroMemory(&rendererState->graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        rendererState->graphicsCommandBuffers[i].Allocate(rendererState, rendererState->device->graphicsCommandPool, true);
    }
}

void VulkanRenderer::CreateSyncObjects()
{
    rendererState->imageAvailableSemaphores.Resize(rendererState->swapchain->maxFramesInFlight);
    rendererState->queueCompleteSemaphores.Resize(rendererState->swapchain->maxFramesInFlight);

    for (U8 i = 0; i < rendererState->swapchain->maxFramesInFlight; ++i)
    {
        VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore(rendererState->device->logicalDevice, &semaphoreInfo, rendererState->allocator, &rendererState->imageAvailableSemaphores[i]);
        vkCreateSemaphore(rendererState->device->logicalDevice, &semaphoreInfo, rendererState->allocator, &rendererState->queueCompleteSemaphores[i]);

        VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkCheck(vkCreateFence(rendererState->device->logicalDevice, &fenceInfo, rendererState->allocator, &rendererState->inFlightFences[i]));
    }

    for (U32 i = 0; i < rendererState->swapchain->imageCount; ++i)
    {
        rendererState->imagesInFlight[i] = 0;
    }
}

void VulkanRenderer::GetPlatformExtentions(Vector<const char*>* names)
{
#ifdef PLATFORM_WINDOWS
    names->Push("VK_KHR_win32_surface");
#elif PLATFORM_LINUX
    names->Push("VK_KHR_xcb_surface");
#elif PLATFORM_APPLE
    names->Push("VK_EXT_metal_surface");
#endif
}

I32 VulkanRenderer::FindMemoryIndex(U32 memoryTypeBits, VkMemoryPropertyFlags memoryFlags)
{
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(rendererState->device->physicalDevice, &properties);

    for (U32 i = 0; i < properties.memoryTypeCount; ++i)
    {
        if (memoryTypeBits & (1 << i) && (properties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags) { return i; }
    }

    LOG_ERROR("Unable to find suitable memory type!");
    return -1;
}

void VulkanRenderer::RecreateFramebuffers()
{
    LOG_INFO("Creating vulkan framebuffers...");

    U32 imageCount = rendererState->swapchain->imageCount;
    for (U32 i = 0; i < imageCount; ++i)
    {
        VkImageView worldAttachments[2] = { rendererState->swapchain->views[i], rendererState->swapchain->depthAttachment->view };
        VkFramebufferCreateInfo worldFramebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        worldFramebufferInfo.renderPass = rendererState->mainRenderpass->handle;
        worldFramebufferInfo.attachmentCount = 2;
        worldFramebufferInfo.pAttachments = worldAttachments;
        worldFramebufferInfo.width = rendererState->framebufferWidth;
        worldFramebufferInfo.height = rendererState->framebufferHeight;
        worldFramebufferInfo.layers = 1;

        VkCheck(vkCreateFramebuffer(rendererState->device->logicalDevice, &worldFramebufferInfo, rendererState->allocator, &rendererState->worldFramebuffers[i]));

        // Swapchain framebuffers (UI pass). Outputs to swapchain images
        VkImageView uiAttachments[1] = { rendererState->swapchain->views[i] };
        VkFramebufferCreateInfo uiFramebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        uiFramebufferInfo.renderPass = rendererState->uiRenderpass->handle;
        uiFramebufferInfo.attachmentCount = 1;
        uiFramebufferInfo.pAttachments = uiAttachments;
        uiFramebufferInfo.width = rendererState->framebufferWidth;
        uiFramebufferInfo.height = rendererState->framebufferHeight;
        uiFramebufferInfo.layers = 1;

        VkCheck(vkCreateFramebuffer(rendererState->device->logicalDevice, &uiFramebufferInfo, rendererState->allocator, &rendererState->swapchain->framebuffers[i]));
    }
}
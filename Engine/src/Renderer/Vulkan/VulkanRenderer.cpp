#include "VulkanRenderer.hpp"

#include "Memory/Memory.hpp"
#include "Platform/Platform.hpp"
#include "Containers/String.hpp"
#include "VulkanDevice.hpp"

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
    rendererState->device = (VulkanDevice*)Memory::Allocate(sizeof(VulkanDevice), MEMORY_TAG_RENDERER);

    rendererState->allocator = nullptr;

    return
        CreateInstance() &&
        CreateDebugger() &&
        CreateSurface() &&
        rendererState->device->Create(rendererState);
}

void VulkanRenderer::Shutdown()
{
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
    LOG_INFO("Creating Vulkan Instance...");

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
    LOG_DEBUG("Creating Vulkan Debugger...");
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
    LOG_INFO("Creating Vulkan Surface...");

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
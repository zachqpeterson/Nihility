#include "VulkanRenderer.hpp"

#include "Memory/Memory.hpp"
#include "Containers/String.hpp"

#include <vulkan/vulkan.hpp>

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* user_data)
{
    switch (messageSeverity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        ERROR(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        WARN(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        INFO(callbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        TRACE(callbackData->pMessage); break;
    }

    return VK_FALSE;
}

struct RendererState
{
#ifdef NH_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
};

static RendererState* rendererState;

bool VulkanRenderer::Initialize()
{
    rendererState = (RendererState*)Memory::Allocate(sizeof(RendererState), MEMORY_TAG_RENDERER);

    rendererState->allocator = nullptr;

    return 
        CreateInstance() &&
        CreateDebugger();
}

void VulkanRenderer::Shutdown()
{
    //TODO: Clean up vulkan

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
    INFO("Creating Vulkan Instance...");

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_VERSION_1_3;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.pApplicationName = "TEST";
    appInfo.pEngineName = "Nihility";
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.pApplicationInfo = &appInfo;

    Vector<String> extentionNames;
    extentionNames.Push(VK_KHR_SURFACE_EXTENSION_NAME);
    GetPlatformExtentions(&extentionNames);

#ifdef NH_DEBUG
    extentionNames.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    Vector<String> layerNames;
    layerNames.Push("VK_LAYER_KHRONOS_validation");

    U32 availableLayerCount;
    VkCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
    Vector<VkLayerProperties> availableLayers(availableLayerCount, {});
    VkCheck(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.Data()));

    for (U32 i = 0; i < layerNames.Size(); ++i)
    {
        bool found = false;
        for (U32 j = 0; j < availableLayerCount; ++j) {
            if (layerNames[i].Equals(availableLayers[j].layerName))
            {
                found = true;
                break;
            }
        }

        if (!found) {
            FATAL("Required validation layer is missing: %s", (char*)layerNames[i]);
            return false;
        }
    }

    instanceInfo.enabledLayerCount = layerNames.Size();
    instanceInfo.ppEnabledLayerNames = (const char**)layerNames.Data();
#else
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
#endif

    instanceInfo.enabledExtensionCount = extentionNames.Size();
    instanceInfo.ppEnabledExtensionNames = (const char**)extentionNames.Data();

    instanceInfo.flags = NULL;
    instanceInfo.pNext = nullptr;

    for(String& s : extentionNames)
    {
        DEBUG((char*)s);
    }

    VkCheck(vkCreateInstance(&instanceInfo, rendererState->allocator, &rendererState->instance));

    return true;
}

bool VulkanRenderer::CreateDebugger()
{
#ifdef NH_DEBUG
    DEBUG("Creating Vulkan debugger...");
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
    VkCheck(func(rendererState->instance, &debugInfo, rendererState->allocator, &rendererState->debugMessenger));
#endif

    return true;
}

void VulkanRenderer::GetPlatformExtentions(Vector<String>* names)
{
#ifdef PLATFORM_WINDOWS
    names->Push("VK_KHR_win32_surface");
#elif PLATFORM_LINUX
    names->Push("VK_KHR_xcb_surface");
#elif PLATFORM_APPLE
    names->Push("VK_EXT_metal_surface");
#endif
}
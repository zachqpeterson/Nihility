#include "Renderer.hpp"

#include "RenderingDefines.hpp"

#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Platform\Platform.hpp"

CSTR Renderer::appName;
U32 Renderer::appVersion;
VkInstance Renderer::instance;
VkSurfaceKHR_T* Renderer::surface;

VkAllocationCallbacks* Renderer::allocator;

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* user_data)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: { Logger::Error(callbackData->pMessage); } break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: { Logger::Warn(callbackData->pMessage); } break;
#if LOG_TRACE_ENABLED
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: { Logger::Info(callbackData->pMessage); } break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: { Logger::Trace(callbackData->pMessage); } break;
#endif
	}

	return VK_FALSE;
}

bool Renderer::Initialize(CSTR applicationName, U32 applicationVersion)
{
	Logger::Trace("Initializing Vulkan Renderer...");

	appName = applicationName;
	appVersion = applicationVersion;

	if (!CreateInstance()) { return false; }

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Vulkan Renderer...");

	vkDestroySurfaceKHR(instance, surface, allocator);

//#ifdef NH_DEBUG
//	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//	func(instance, debugMessenger, &allocator);
//#endif

	vkDestroyInstance(instance, allocator);
}

void Renderer::Update()
{

}

bool Renderer::CreateInstance()
{
	VkApplicationInfo applicationInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = appName;
	applicationInfo.applicationVersion = appVersion;
	applicationInfo.pEngineName = "Nihlity";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 3, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &applicationInfo;

	Vector<CSTR> extensions;
	extensions.Push(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.Push(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#if defined PLATFORM_WINDOWS
	extensions.Push("VK_KHR_win32_surface");
#elif defined PLATFORM_LINUX
	extensions.Push("VK_KHR_xcb_surface");
#elif defined PLATFORM_APPLE
	extensions.Push("VK_EXT_metal_surface");
#endif

#ifdef NH_DEBUG
	extensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	Vector<CSTR> layers;
	layers.Push("VK_LAYER_KHRONOS_validation");

	U32 count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	Vector<VkLayerProperties> availableLayers(count, {});
	VkResult r = vkEnumerateInstanceLayerProperties(&count, availableLayers.Data());

	bool foundAll = true;
	for (U32 i = 0; i < layers.Size(); ++i)
	{
		bool found = false;
		for (U32 j = 0; j < count; ++j)
		{
			if (Compare(layers[i], availableLayers[j].layerName)) { found = true; break; }
		}

		if (!found)
		{
			foundAll = false;
			Logger::Fatal("Required validation layer is missing: {}", layers[i]);
		}
	}

	if (!foundAll) { return false; }

	VkDebugUtilsMessengerCreateInfoEXT debugInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugInfo.pNext = nullptr;
	debugInfo.flags = 0;
	debugInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	debugInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugInfo.pfnUserCallback = VkDebugCallback;
	debugInfo.pUserData = nullptr;

	instanceInfo.pNext = &debugInfo;
	instanceInfo.enabledLayerCount = (U32)layers.Size();
	instanceInfo.ppEnabledLayerNames = layers.Data();
#else 
	instanceInfo.pNext = nullptr;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;
#endif

	instanceInfo.enabledExtensionCount = (U32)extensions.Size();
	instanceInfo.ppEnabledExtensionNames = extensions.Data();

	VkValidateFR(vkCreateInstance(&instanceInfo, allocator, &instance));

	return true;
}

bool Renderer::CreateAllocator()
{
#ifdef PLATFORM_WINDOWS
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.pNext = nullptr;
	surfaceInfo.flags = 0;
	const WindowData& wd = Platform::GetWindowData();
	surfaceInfo.hinstance = wd.instance;
	surfaceInfo.hwnd = wd.window;

	VkValidateFR(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, allocator, &surface));
#elif PLATFORM_LINUX
	//TODO:
#elif PLATFORM_APPLE
	//TODO:
#endif

	return true;
}
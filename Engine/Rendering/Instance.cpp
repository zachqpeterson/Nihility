#include "Instance.hpp"

#include "VulkanInclude.hpp"
#include "Renderer.hpp"

#include "Containers/Vector.hpp"
#include "Core/Logger.hpp"

PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;

VkBool32 __stdcall DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: { Logger::Debug(pCallbackData->pMessage); } break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: { Logger::Info(pCallbackData->pMessage); } break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: { Logger::Warn(pCallbackData->pMessage); } break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: { Logger::Error(pCallbackData->pMessage); } break;
	}
}

bool Instance::Create()
{
	VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Demo";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.pEngineName = "Nihility";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 4);
	applicationInfo.apiVersion = VK_VERSION_1_3;

	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };

	VkValidateFR(vkEnumerateInstanceVersion(&applicationInfo.apiVersion));

	Vector<const C8*> extensions;
	Vector<const C8*> layers;

	extensions.Push(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	extensions.Push(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef NH_PLATFORM_WINDOWS
	extensions.Push(VK_KHR_WIN32_SURFACE_EXTENSION_NAME); //TODO: Other platforms
#endif

#ifdef NH_DEBUG
	extensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	layers.Push("VK_LAYER_KHRONOS_validation");

	VkDebugUtilsMessengerCreateInfoEXT messengerInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerInfo.flags = 0;
	messengerInfo.messageSeverity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messengerInfo.pfnUserCallback = DebugCallback;
	messengerInfo.pUserData = nullptr;
	instanceInfo.pNext = &messengerInfo;

	Vector<VkValidationFeatureEnableEXT> validationFeatures;
	validationFeatures.Push(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
	validationFeatures.Push(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);

	VkValidationFeaturesEXT features = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
	features.pNext = nullptr;
	features.enabledValidationFeatureCount = (U32)validationFeatures.Size();
	features.pEnabledValidationFeatures = validationFeatures.Data();
	features.disabledValidationFeatureCount = 0;
	features.pDisabledValidationFeatures = nullptr;
	messengerInfo.pNext = &features;

	VkValidationFlagsEXT checks{};
	checks.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
	checks.pNext = nullptr;
	checks.disabledValidationCheckCount = 0;
	checks.pDisabledValidationChecks = nullptr;
	//features.pNext = &checks;
#endif

	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledLayerCount = (U32)layers.Size();
	instanceInfo.ppEnabledLayerNames = layers.Data();
	instanceInfo.enabledExtensionCount = (U32)extensions.Size();
	instanceInfo.ppEnabledExtensionNames = extensions.Data();

	VkValidateFR(vkCreateInstance(&instanceInfo, Renderer::allocationCallbacks, &instance));

#ifdef NH_DEBUG
	DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

	VkValidateFR(CreateDebugUtilsMessengerEXT(instance, &messengerInfo, Renderer::allocationCallbacks, &debugMessenger));
#endif

	return true;
}

void Instance::Destroy()
{
#ifdef NH_DEBUG
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, Renderer::allocationCallbacks);
#endif

	vkDestroyInstance(instance, Renderer::allocationCallbacks);
}

Instance::operator VkInstance_T*() const
{
	return instance;
}
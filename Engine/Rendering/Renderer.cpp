#include "Renderer.hpp"

#include "RenderingDefines.hpp"

#include "UI.hpp"
#include "CommandBuffer.hpp"
#include "Pipeline.hpp"
#include "Resources\Resources.hpp"
#include "Resources\Scene.hpp"
#include "Math\Math.hpp"

#define VMA_DEBUG_LOG
//#define VMA_DEBUG_LOG(...) printf(__VA_ARGS__); printf("\n")
#define VMA_VULKAN_VERSION 1003000
#define VMA_IMPLEMENTATION
#include "External\LunarG\vma\vk_mem_alloc.h"

import Core;
import Memory;
import Platform;

void CommandBufferRing::Create()
{
	for (U32 i = 0; i < maxPools; ++i) { freeCommandBuffers[i](buffersPerPool); }

	VkCommandPoolCreateInfo commandPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = Renderer::renderQueueIndex;

	VkValidate(vkCreateCommandPool(Renderer::device, &commandPoolInfo, Renderer::allocationCallbacks, &drawCommandPool));

	commandPoolInfo.queueFamilyIndex = Renderer::renderQueueIndex; //Renderer::transferQueueIndex;
	commandPoolInfo.flags = 0;

	for (U32 i = 0; i < maxPools; ++i)
	{
		VkValidate(vkCreateCommandPool(Renderer::device, &commandPoolInfo, Renderer::allocationCallbacks, &commandPools[i]));
	}

	VkCommandBufferAllocateInfo commandBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandPool = nullptr;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;

	for (U32 i = 0; i < maxPools; ++i)
	{
		commandBufferInfo.commandPool = drawCommandPool;
		VkValidate(vkAllocateCommandBuffers(Renderer::device, &commandBufferInfo, &drawCommandBuffers[i].vkCommandBuffer));

		commandBufferInfo.commandPool = commandPools[i];

		for (U32 j = 0; j < buffersPerPool; ++j)
		{
			VkValidate(vkAllocateCommandBuffers(Renderer::device, &commandBufferInfo, &commandBuffers[buffersPerPool * i + j].vkCommandBuffer));
		}
	}
}

void CommandBufferRing::Destroy()
{
	for (U32 i = 0; i < maxPools; ++i) { freeCommandBuffers[i].Destroy(); }

	vkDestroyCommandPool(Renderer::device, drawCommandPool, Renderer::allocationCallbacks);

	for (U32 i = 0; i < maxPools; ++i)
	{
		vkDestroyCommandPool(Renderer::device, commandPools[i], Renderer::allocationCallbacks);
	}
}

void CommandBufferRing::ResetDrawPool()
{
	vkResetCommandPool(Renderer::device, drawCommandPool, 0);
}

void CommandBufferRing::ResetDraw(U32 frameIndex)
{
	vkResetCommandBuffer(drawCommandBuffers[frameIndex].vkCommandBuffer, 0);
}

void CommandBufferRing::ResetPool(U32 frameIndex)
{
	freeCommandBuffers[frameIndex].Reset();

	vkResetCommandPool(Renderer::device, commandPools[frameIndex], 0);

	for (U32 i = 0; i < buffersPerPool; ++i)
	{
		commandBuffers[frameIndex * buffersPerPool + i].recorded = false;
	}
}

CommandBuffer* CommandBufferRing::GetDrawCommandBuffer(U32 frameIndex)
{
	return &drawCommandBuffers[frameIndex];
}

CommandBuffer* CommandBufferRing::GetWriteCommandBuffer(U32 frameIndex)
{
	I32 index = freeCommandBuffers[frameIndex].GetFree();

	if (index == U32_MAX) { BreakPoint; }

	return &commandBuffers[frameIndex * buffersPerPool + index];
}

static constexpr const C8* extensions[]{
	VK_KHR_SURFACE_EXTENSION_NAME,

#if defined NH_PLATFORM_WINDOWS
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined NH_PLATFORM_APPLE
	VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#elif defined NH_PLATFORM_LINUX
	VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined NH_PLATFORM_ANDROID
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_XLIB
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_WAYLAND
	VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_MIR || PLATFORM_DISPLAY
	VK_KHR_DISPLAY_EXTENSION_NAME,
#elif defined NH_PLATFORM_IOS
	VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif

#ifdef NH_DEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

#ifdef NH_DEBUG
static constexpr const C8* layers[]{
	"VK_LAYER_KHRONOS_validation",
	//"VK_LAYER_LUNARG_core_validation",
	//"VK_LAYER_LUNARG_image",
	//"VK_LAYER_LUNARG_parameter_validation",
	//"VK_LAYER_LUNARG_object_tracker",
};
#endif

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

// DEVICE
VkInstance							Renderer::instance;
VkPhysicalDevice					Renderer::physicalDevice;
VkDevice							Renderer::device;
Swapchain							Renderer::swapchain;
VkQueue								Renderer::renderQueue;
VkQueue								Renderer::transferQueue;
U32									Renderer::renderQueueIndex;
U32									Renderer::transferQueueIndex;

VkAllocationCallbacks* Renderer::allocationCallbacks;
VkDescriptorPool					Renderer::descriptorPools[MAX_SWAPCHAIN_IMAGES];
U64									Renderer::uboAlignment;
U64									Renderer::sboAlignemnt;

bool								Renderer::bindlessSupported = false;
bool								Renderer::pushDescriptorsSupported = false;
bool								Renderer::meshShadingSupported = false;

// WINDOW
Vector4								Renderer::renderArea;
U32									Renderer::frameIndex = 0;
U32									Renderer::previousFrame = 0;
U32									Renderer::absoluteFrame = 0;
bool								Renderer::resized = false;

// RESOURCES
Scene* Renderer::currentScene;
VmaAllocator_T* Renderer::allocator;
CommandBufferRing					Renderer::commandBufferRing;
Vector<VkCommandBuffer_T*>			Renderer::commandBuffers[MAX_SWAPCHAIN_IMAGES];
Buffer								Renderer::stagingBuffer;
Buffer								Renderer::materialBuffer;
Buffer								Renderer::globalsBuffer;
ShadowData							Renderer::shadowData;
GlobalData							Renderer::globalData;
SkyboxData							Renderer::skyboxData;
PostProcessData						Renderer::postProcessData;
ResourceRef<Texture>				Renderer::defaultRenderTarget;
ResourceRef<Texture>				Renderer::defaultDepthTarget;

// TIMING
VkSemaphore							Renderer::imageAcquired = nullptr;
VkSemaphore							Renderer::presentReady[MAX_SWAPCHAIN_IMAGES];
VkSemaphore							Renderer::renderCompleted[MAX_SWAPCHAIN_IMAGES];
VkSemaphore							Renderer::transferCompleted[MAX_SWAPCHAIN_IMAGES];
U64									Renderer::renderWaitValues[MAX_SWAPCHAIN_IMAGES];
U64									Renderer::transferWaitValues[MAX_SWAPCHAIN_IMAGES];

// DEBUG
VkDebugUtilsMessengerEXT			Renderer::debugMessenger;
bool								Renderer::debugUtilsExtensionPresent = false;

struct VulkanInfo
{
	// CAPABILITIES
	VkPhysicalDeviceFeatures				physicalDeviceFeatures;
	VkPhysicalDeviceProperties				physicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties		physicalDeviceMemoryProperties;
} vulkanInfo;

PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;

bool Renderer::Initialize(const StringView& applicationName, U32 applicationVersion)
{
	Logger::Trace("Initializing Renderer...");

	allocationCallbacks = nullptr;

	if (!CreateInstance(applicationName, applicationVersion)) { return false; }
	if (!swapchain.CreateSurface()) { return false; }
	if (!SelectGPU()) { return false; }
	if (!CreateDevice()) { return false; }
	if (!CreateResources()) { return false; }
	if (!swapchain.GetFormat()) { return false; }
	if (!swapchain.Create()) { return false; }

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Shutting Down Renderer...");

	if (currentScene)
	{
		Resources::SaveScene(currentScene);
	}

	vkDeviceWaitIdle(device);

	commandBufferRing.Destroy();

	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) { commandBuffers[i].Destroy(); }

	vkDestroySemaphore(device, imageAcquired, allocationCallbacks);
	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) { vkDestroySemaphore(device, presentReady[i], allocationCallbacks); }
	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) { vkDestroySemaphore(device, renderCompleted[i], allocationCallbacks); }
	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) { vkDestroySemaphore(device, transferCompleted[i], allocationCallbacks); }

	DestroyBuffer(stagingBuffer);
	DestroyBuffer(materialBuffer);
	DestroyBuffer(globalsBuffer);

	swapchain.Destroy();

	Shader::Shutdown();

	vmaDestroyAllocator(allocator);

#ifdef NH_DEBUG
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocationCallbacks);
#endif

	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i) { vkDestroyDescriptorPool(device, descriptorPools[i], allocationCallbacks); }

	vkDestroyDevice(device, allocationCallbacks);

	vkDestroyInstance(instance, allocationCallbacks);
}

bool Renderer::CreateInstance(const StringView& applicationName, U32 applicationVersion)
{
	VkApplicationInfo applicationInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = applicationName.Data();
	applicationInfo.applicationVersion = applicationVersion;
	applicationInfo.pEngineName = "Nihlity";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 3, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &applicationInfo;

#ifdef NH_DEBUG
	debugUtilsExtensionPresent = true;

	VkDebugUtilsMessengerCreateInfoEXT debugInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugInfo.pNext = nullptr;
	debugInfo.flags = 0;
	debugInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	debugInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugInfo.pfnUserCallback = VkDebugCallback;
	debugInfo.pUserData = nullptr;

#	ifdef VK_ADDITIONAL_VALIDATION
	const VkValidationFeatureEnableEXT featuresRequested[]{
		//VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,						//Addition diagnostic data
		VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,			//Resource access conflicts due to missing or incorrect synchronization operations
		VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,						//Warnings related to common misuse of the API
		//VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,						//Logging in shaders
		//VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,	//Validation layers reserve a descriptor set binding slot for their own use
	};

	VkValidationFeaturesEXT features{ VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
	features.pNext = &debugInfo;
	features.enabledValidationFeatureCount = CountOf32(featuresRequested);
	features.pEnabledValidationFeatures = featuresRequested;
	instanceInfo.pNext = &features;
#	else
	instanceInfo.pNext = &debugInfo;
#	endif

	instanceInfo.enabledLayerCount = CountOf32(layers);
	instanceInfo.ppEnabledLayerNames = layers;
#else 
	instanceInfo.pNext = nullptr;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;
#endif

	instanceInfo.enabledExtensionCount = CountOf32(extensions);
	instanceInfo.ppEnabledExtensionNames = extensions;

	VkValidateFR(vkCreateInstance(&instanceInfo, allocationCallbacks, &instance));

#ifdef NH_DEBUG
	DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	SetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

	VkValidateFR(CreateDebugUtilsMessengerEXT(instance, &debugInfo, allocationCallbacks, &debugMessenger));
#endif

	return true;
}

bool Renderer::SelectGPU()
{
	U32 physicalDeviceCount;
	VkValidateFR(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	Vector<VkPhysicalDevice> gpus(physicalDeviceCount, {});
	VkValidateFR(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, gpus.Data()));

	VkPhysicalDevice discreteGpu = VK_NULL_HANDLE;
	VkPhysicalDevice integratedGpu = VK_NULL_HANDLE;

	for (VkPhysicalDevice gpu : gpus)
	{
		vkGetPhysicalDeviceProperties(gpu, &vulkanInfo.physicalDeviceProperties);

		if (vulkanInfo.physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { if (GetFamilyQueue(gpu)) { discreteGpu = gpu; break; } }
		else if (vulkanInfo.physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) { if (GetFamilyQueue(gpu)) { integratedGpu = gpu; } }
	}

	if (discreteGpu != VK_NULL_HANDLE) { physicalDevice = discreteGpu; }
	else if (integratedGpu != VK_NULL_HANDLE) { physicalDevice = integratedGpu; }
	else { Logger::Fatal("No Suitable GPU Found!"); return false; }

	vkGetPhysicalDeviceProperties(physicalDevice, &vulkanInfo.physicalDeviceProperties);

	Logger::Trace("Best GPU Found: {}", vulkanInfo.physicalDeviceProperties.deviceName);

	uboAlignment = vulkanInfo.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	sboAlignemnt = vulkanInfo.physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

	vkGetPhysicalDeviceFeatures(physicalDevice, &vulkanInfo.physicalDeviceFeatures);

	return true;
}

bool Renderer::GetFamilyQueue(VkPhysicalDevice gpu)
{
	U32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
	Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount, {});
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilies.Data());

	VkBool32 surfaceSupported = VK_FALSE;
	for (U32 familyIndex = 0; familyIndex < queueFamilyCount; ++familyIndex)
	{
		VkQueueFamilyProperties queueFamily = queueFamilies[familyIndex];
		if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)) == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT))
		{
			VkValidateFR(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, familyIndex, swapchain.surface, &surfaceSupported));

			if (surfaceSupported) { renderQueueIndex = familyIndex; }
		}

		if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0 && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			transferQueueIndex = familyIndex;
		}
	}

	return surfaceSupported;
}

bool Renderer::CreateDevice()
{
	U32 queueCount = 1;
	const F32 queuePriorities[]{ 1.0f };
	VkDeviceQueueCreateInfo queueInfo[2];
	queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[0].pNext = nullptr;
	queueInfo[0].flags = 0;
	queueInfo[0].queueFamilyIndex = renderQueueIndex;
	queueInfo[0].queueCount = 1;
	queueInfo[0].pQueuePriorities = queuePriorities;

	if (transferQueueIndex != renderQueueIndex)
	{
		queueCount = 2;

		queueInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo[1].pNext = nullptr;
		queueInfo[1].flags = 0;
		queueInfo[1].queueFamilyIndex = transferQueueIndex;
		queueInfo[1].queueCount = 1;
		queueInfo[1].pQueuePriorities = queuePriorities;
	}

	U32 extensionCount = 0;
	VkValidateFR(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, 0));
	Vector<VkExtensionProperties> extensions(extensionCount, {});
	VkValidateFR(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, extensions.Data()));

	for (VkExtensionProperties& ext : extensions)
	{
		pushDescriptorsSupported |= Compare(ext.extensionName, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, 22);
		meshShadingSupported |= Compare(ext.extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME, 18);
	}

	//TODO: Remove
	pushDescriptorsSupported = false;
	meshShadingSupported = false;

	U32 deviceExtensionCount = 0;
	const C8* deviceExtensions[4];

	deviceExtensions[deviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	if (pushDescriptorsSupported) { deviceExtensions[deviceExtensionCount++] = VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME; }
	if (meshShadingSupported) { deviceExtensions[deviceExtensionCount++] = VK_EXT_MESH_SHADER_EXTENSION_NAME; }

	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
	VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures };

	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

	bindlessSupported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;

	VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	features.features.multiDrawIndirect = true;
	features.features.drawIndirectFirstInstance = true;
	features.features.pipelineStatisticsQuery = true;
	features.features.shaderFloat64 = true;
	features.features.shaderInt16 = true;
	features.features.shaderInt64 = true;

	VkPhysicalDeviceVulkan11Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	features11.storageBuffer16BitAccess = true;
	features11.shaderDrawParameters = true;

	VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.drawIndirectCount = true;
	features12.storageBuffer8BitAccess = true;
	features12.uniformAndStorageBuffer8BitAccess = true;
	features12.shaderFloat16 = true;
	features12.shaderInt8 = true;
	features12.samplerFilterMinmax = true;
	features12.scalarBlockLayout = true;
	features12.timelineSemaphore = true;

	if (bindlessSupported)
	{
		features12.descriptorBindingPartiallyBound = true;
		features12.runtimeDescriptorArray = true;
		features12.shaderSampledImageArrayNonUniformIndexing = true;
	}

	VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features13.dynamicRendering = true;
	features13.synchronization2 = true;
	features13.maintenance4 = true;

	VkPhysicalDeviceMeshShaderFeaturesEXT featuresMesh{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
	featuresMesh.taskShader = true;
	featuresMesh.meshShader = true;

	features.pNext = &features11;
	features11.pNext = &features12;
	features12.pNext = &features13;

	void** next = &features13.pNext;

	if (meshShadingSupported) { *next = &featuresMesh; next = &featuresMesh.pNext; }

	VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.queueCreateInfoCount = queueCount;
	deviceInfo.pQueueCreateInfos = queueInfo;
	deviceInfo.enabledExtensionCount = deviceExtensionCount;
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceInfo.pNext = &features;
	deviceInfo.pEnabledFeatures = nullptr;
	deviceInfo.flags = 0;

	VkValidateFR(vkCreateDevice(physicalDevice, &deviceInfo, allocationCallbacks, &device));

	vkGetDeviceQueue(device, renderQueueIndex, 0, &renderQueue);
	SetResourceName(VK_OBJECT_TYPE_QUEUE, (U64)renderQueue, "render_queue");

	if (transferQueueIndex != renderQueueIndex)
	{
		vkGetDeviceQueue(device, transferQueueIndex, 0, &transferQueue);
		SetResourceName(VK_OBJECT_TYPE_QUEUE, (U64)renderQueue, "transfer_queue");
	}
	else
	{
		transferQueue = renderQueue;
	}

	return true;
}

bool Renderer::CreateResources()
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	//allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	VkValidateFR(vmaCreateAllocator(&allocatorInfo, &allocator));

	static constexpr U32 globalPoolElements = 128;
	VkDescriptorPoolSize poolSizes[]
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, globalPoolElements },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, globalPoolElements}
	};

	VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = globalPoolElements * CountOf32(poolSizes);
	poolInfo.poolSizeCount = CountOf32(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
	{
		VkValidateFR(vkCreateDescriptorPool(device, &poolInfo, allocationCallbacks, &descriptorPools[i]));
	}

	if (!Shader::Initialize()) { return false; }

	VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;

	vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquired);
	SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (U64)imageAcquired, "image_aquired_semaphore");

	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &presentReady[i]);
		SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (U64)presentReady[i], "present_ready_semaphore");
	}

	VkSemaphoreTypeCreateInfo semaphoreType{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	semaphoreType.pNext = nullptr;
	semaphoreType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	semaphoreType.initialValue = 0;

	semaphoreInfo.pNext = &semaphoreType;

	for (U32 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &renderCompleted[i]);
		SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (U64)renderCompleted[i], "render_completed_semaphore");

		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &transferCompleted[i]);
		SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (U64)transferCompleted[i], "transfer_complete_semaphore");
	}

	commandBufferRing.Create();

	stagingBuffer = CreateBuffer(Megabytes(512), BUFFER_USAGE_TRANSFER_SRC, BUFFER_MEMORY_TYPE_CPU_VISIBLE | BUFFER_MEMORY_TYPE_CPU_COHERENT, "renderer_staging_buffer");
	materialBuffer = CreateBuffer(Megabytes(128), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL, "renderer_material_buffer");
	globalsBuffer = CreateBuffer(sizeof(GlobalData), BUFFER_USAGE_STORAGE_BUFFER | BUFFER_USAGE_TRANSFER_DST, BUFFER_MEMORY_TYPE_GPU_LOCAL, "renderer_globals_buffer");

	TextureInfo textureInfo{};
	textureInfo.name = "default_render_target";
	textureInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureInfo.width = Platform::windowWidth;
	textureInfo.height = Platform::windowHeight;
	textureInfo.depth = 1;
	textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
	textureInfo.type = VK_IMAGE_TYPE_2D;
	defaultRenderTarget = Resources::CreateTexture(textureInfo);

	textureInfo.name = "default_depth_target";
	textureInfo.format = VK_FORMAT_D32_SFLOAT;
	defaultDepthTarget = Resources::CreateTexture(textureInfo);

	//TODO: Default culling

	Vector<ResourceRef<Pipeline>> pipelines;

	pipelines.Push(Resources::LoadPipeline("pipelines/sprite.nhpln"));
	pipelines[0]->AddDescriptor({ Renderer::globalsBuffer.vkBuffer });
	pipelines[0]->AddDependancy({ DEPENDANCY_ENTITY_BUFFER });

	Resources::CreateMaterialEffect("spriteEffect", pipelines);

	PushConstant pushConstant{ 0, sizeof(ShadowData), Renderer::GetShadowData() };
	pipelines[0] = Resources::LoadPipeline("pipelines/shadowMap.nhpln", 1, &pushConstant);
	pipelines[0]->AddDependancy({ DEPENDANCY_ENTITY_BUFFER });

	pipelines.Push(Resources::LoadPipeline("pipelines/pbrOpaque.nhpln"));
	pipelines[1]->AddDescriptor({ Renderer::globalsBuffer.vkBuffer }); //TODO: specify binding
	pipelines[1]->AddDescriptor({ Renderer::materialBuffer.vkBuffer });
	pipelines[1]->AddDependancy({ DEPENDANCY_ENTITY_BUFFER });
	pipelines[1]->AddDependancy({ pipelines[0], DEPENDANCY_DEPTH_TARGET });

	Resources::CreateMaterialEffect("pbrOpaqueEffect", pipelines);

	pipelines[1] = Resources::LoadPipeline("pipelines/pbrTransparent.nhpln");
	pipelines[1]->AddDescriptor({ Renderer::globalsBuffer.vkBuffer });
	pipelines[1]->AddDescriptor({ Renderer::materialBuffer.vkBuffer });
	pipelines[1]->AddDependancy({ DEPENDANCY_ENTITY_BUFFER });
	pipelines[1]->AddDependancy({ pipelines[0], DEPENDANCY_DEPTH_TARGET });

	Resources::CreateMaterialEffect("pbrTransparentEffect", Move(pipelines));

	return true;
}

void Renderer::InitialSubmit()
{
	SubmitTransfer();
}

bool Renderer::BeginFrame()
{
	if (!currentScene) { return false; }

	VkResult result = swapchain.Update();
	if (result == VK_ERROR_OUT_OF_DATE_KHR) { Resize(); }
	else if (result == VK_NOT_READY) { return false; }

	result = swapchain.NextImage(frameIndex, imageAcquired);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Resize();
		return false;
	}

	VkSemaphore waits[]{ renderCompleted[previousFrame], transferCompleted[previousFrame] };
	U64 waitValues[]{ renderWaitValues[previousFrame], transferWaitValues[previousFrame] };

	VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
	waitInfo.pNext = nullptr;
	waitInfo.flags = 0;
	waitInfo.semaphoreCount = CountOf32(waits);
	waitInfo.pSemaphores = waits;
	waitInfo.pValues = waitValues;

	vkWaitSemaphores(device, &waitInfo, U64_MAX);

	commandBufferRing.ResetDraw(previousFrame);
	commandBufferRing.ResetPool(previousFrame);
	VkValidateF(vkResetDescriptorPool(device, descriptorPools[previousFrame], 0));

	stagingBuffer.allocationOffset = 0;

	return true;
}

void Renderer::EndFrame()
{
	currentScene->Update();
	Resources::Update();
	SubmitTransfer();
	VkCommandBuffer frameCB = Record();

	++renderWaitValues[frameIndex];

	VkSemaphore waits[]{ transferCompleted[frameIndex], imageAcquired };
	U64 waitValues[]{ transferWaitValues[frameIndex], 1 };

	VkSemaphore signals[]{ renderCompleted[frameIndex], presentReady[frameIndex] };
	U64 signalValues[]{ renderWaitValues[frameIndex], 1 };

	VkTimelineSemaphoreSubmitInfo timelineInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
	timelineInfo.pNext = nullptr;
	timelineInfo.waitSemaphoreValueCount = CountOf32(waitValues);
	timelineInfo.pWaitSemaphoreValues = waitValues;
	timelineInfo.signalSemaphoreValueCount = CountOf32(signalValues);
	timelineInfo.pSignalSemaphoreValues = signalValues;

	VkPipelineStageFlags submitStageMasks[]{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_NONE };
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pNext = &timelineInfo;
	submitInfo.waitSemaphoreCount = CountOf32(waits);
	submitInfo.pWaitSemaphores = waits;
	submitInfo.pWaitDstStageMask = submitStageMasks;
	submitInfo.commandBufferCount = 1u;
	submitInfo.pCommandBuffers = &frameCB;
	submitInfo.signalSemaphoreCount = CountOf32(signals);
	submitInfo.pSignalSemaphores = signals;

	VkValidate(vkQueueSubmit(renderQueue, 1u, &submitInfo, nullptr));
	commandBuffers[frameIndex].Clear();

	previousFrame = frameIndex;
	VkResult result = swapchain.Present(renderQueue, frameIndex, 1, &presentReady[frameIndex]);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) { Resize(); }

	++absoluteFrame;
}

void Renderer::SubmitTransfer()
{
	if (commandBuffers[frameIndex].Size())
	{
		++transferWaitValues[frameIndex];

		VkTimelineSemaphoreSubmitInfo timelineInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
		timelineInfo.pNext = nullptr;
		timelineInfo.waitSemaphoreValueCount = 0;
		timelineInfo.pWaitSemaphoreValues = nullptr;
		timelineInfo.signalSemaphoreValueCount = 1;
		timelineInfo.pSignalSemaphoreValues = &transferWaitValues[frameIndex];

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.pNext = &timelineInfo;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = (U32)commandBuffers[frameIndex].Size();
		submitInfo.pCommandBuffers = commandBuffers[frameIndex].Data();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &transferCompleted[frameIndex];

		VkValidateF(vkQueueSubmit(renderQueue, 1, &submitInfo, nullptr)); //TODO: use transfer queue
		commandBuffers[frameIndex].Clear();

		stagingBuffer.allocationOffset = 0;
	}
}

VkCommandBuffer Renderer::Record()
{
	CommandBuffer* commandBuffer = commandBufferRing.GetDrawCommandBuffer(frameIndex);
	commandBuffer->Begin();
	currentScene->Render(commandBuffer);

	VkImageMemoryBarrier2 copyBarriers[]{
		ImageBarrier(defaultRenderTarget->image,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
		ImageBarrier(swapchain.renderTargets[frameIndex]->image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	};

	commandBuffer->PipelineBarrier(VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, CountOf32(copyBarriers), copyBarriers);

	VkImageBlit blitRegion{};
	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.srcOffsets[1].x = swapchain.width;
	blitRegion.srcOffsets[1].y = swapchain.height;
	blitRegion.srcOffsets[1].z = 1;
	blitRegion.dstOffsets[1].x = swapchain.width;
	blitRegion.dstOffsets[1].y = swapchain.height;
	blitRegion.dstOffsets[1].z = 1;

	commandBuffer->Blit(defaultRenderTarget, swapchain.renderTargets[frameIndex], VK_FILTER_NEAREST, 1, &blitRegion);

	VkImageMemoryBarrier2 presentBarrier = ImageBarrier(swapchain.renderTargets[frameIndex]->image,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	commandBuffer->PipelineBarrier(VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &presentBarrier);
	commandBuffer->End();

	return commandBuffer->vkCommandBuffer;
}

void Renderer::Resize()
{
	vkDeviceWaitIdle(device);

	swapchain.Create();
	if (currentScene) { currentScene->Resize(); }

	vkDeviceWaitIdle(device);

	Platform::resized = false;
}

void Renderer::SetRenderArea()
{
	F32 aspectHeight = Platform::windowHeight * 1.77777777778f;
	F32 aspectWidth = Platform::windowWidth * 0.5625f;

	F32 offset;

	if (Platform::windowWidth > aspectHeight)
	{
		offset = (Platform::windowWidth - aspectHeight) * 0.5f;

		renderArea.x = offset;
		renderArea.y = 0.0f;
		renderArea.z = Platform::windowWidth - (offset * 2.0f);
		renderArea.w = (F32)Platform::windowHeight;
	}
	else
	{
		offset = (Platform::windowHeight - aspectWidth) * 0.5f;

		renderArea.x = 0.0f;
		renderArea.y = offset;
		renderArea.z = (F32)Platform::windowWidth;
		renderArea.w = Platform::windowHeight - (offset * 2.0f);
	}
}

void Renderer::LoadScene(Scene* scene)
{
	if (currentScene)
	{
		currentScene->Unload();
	}

	currentScene = scene;
	currentScene->Load();
}

ShadowData* Renderer::GetShadowData()
{
	return &shadowData;
}

GlobalData* Renderer::GetGlobalData()
{
	return &globalData;
}

SkyboxData* Renderer::GetSkyboxData()
{
	return &skyboxData;
}

PostProcessData* Renderer::GetPostProcessData()
{
	return &postProcessData;
}

void Renderer::SetResourceName(VkObjectType type, U64 handle, const String& name)
{
	if (!debugUtilsExtensionPresent) { return; }

	VkDebugUtilsObjectNameInfoEXT nameInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	nameInfo.objectType = type;
	nameInfo.objectHandle = handle;
	nameInfo.pObjectName = name.Data();
	SetDebugUtilsObjectNameEXT(device, &nameInfo);
}

const Vector4& Renderer::RenderArea()
{
	return renderArea;
}

U32 Renderer::FrameIndex()
{
	return frameIndex;
}

U32 Renderer::AbsoluteFrame()
{
	return absoluteFrame;
}

const VkPhysicalDeviceFeatures& Renderer::GetDeviceFeatures()
{
	return vulkanInfo.physicalDeviceFeatures;
}

const VkPhysicalDeviceProperties& Renderer::GetDeviceProperties()
{
	return vulkanInfo.physicalDeviceProperties;
}

const VkPhysicalDeviceMemoryProperties& Renderer::GetDeviceMemoryProperties()
{
	return vulkanInfo.physicalDeviceMemoryProperties;
}

VkImageMemoryBarrier2 Renderer::ImageBarrier(VkImage image, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
	VkImageLayout oldLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkImageLayout newLayout,
	VkImageAspectFlags aspectMask, U32 baseMipLevel, U32 levelCount, U32 layerCount)
{
	VkImageMemoryBarrier2 result{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };

	result.srcStageMask = srcStageMask;
	result.srcAccessMask = srcAccessMask;
	result.dstStageMask = dstStageMask;
	result.dstAccessMask = dstAccessMask;
	result.oldLayout = oldLayout;
	result.newLayout = newLayout;
	result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	result.image = image;
	result.subresourceRange.aspectMask = aspectMask;
	result.subresourceRange.baseMipLevel = baseMipLevel;
	result.subresourceRange.levelCount = levelCount;
	result.subresourceRange.layerCount = layerCount;

	return result;
}

VkBufferMemoryBarrier2 Renderer::BufferBarrier(VkBuffer buffer, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
	VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
{
	VkBufferMemoryBarrier2 result{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };

	result.srcStageMask = srcStageMask;
	result.srcAccessMask = srcAccessMask;
	result.dstStageMask = dstStageMask;
	result.dstAccessMask = dstAccessMask;
	result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	result.buffer = buffer;
	result.offset = 0;
	result.size = VK_WHOLE_SIZE;

	return result;
}

Buffer Renderer::CreateBuffer(U64 size, BufferUsageBits bufferUsage, BufferMemoryTypeBits memoryType, const String& name)
{
	Buffer buffer{};
	buffer.size = size;
	buffer.usage = bufferUsage;
	buffer.memoryProperties = memoryType;

	VkBufferCreateInfo info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	info.size = size;
	info.usage = bufferUsage;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	if (memoryType & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { memoryInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; }
	memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
	memoryInfo.requiredFlags = memoryType;

	VmaAllocationInfo allocationInfo{};
	allocationInfo.pName = "buffer";
	VkValidate(vmaCreateBuffer(allocator, &info, &memoryInfo, &buffer.vkBuffer, &buffer.allocation, &allocationInfo));

	SetResourceName(VK_OBJECT_TYPE_BUFFER, (U64)buffer.vkBuffer, name);

	buffer.deviceMemory = allocationInfo.deviceMemory;

	if (memoryType & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		buffer.data = allocationInfo.pMappedData;
		buffer.mapped = true;
	}

	return Move(buffer);
}

void Renderer::FillBuffer(Buffer& buffer, U64 size, const void* data, U32 regionCount, VkBufferCopy* regions)
{
	VkBufferMemoryBarrier2 memoryBarrier = BufferBarrier(buffer.vkBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_READ_BIT);

	if (stagingBuffer.allocationOffset + size > stagingBuffer.size)
	{
		Logger::Warn("Out Of Staging Memory!");
		Renderer::SubmitTransfer();

		VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
		waitInfo.pNext = nullptr;
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &transferCompleted[frameIndex];
		waitInfo.pValues = &transferWaitValues[previousFrame];

		vkWaitSemaphores(device, &waitInfo, U64_MAX);
	}

	Copy((U8*)stagingBuffer.data + stagingBuffer.allocationOffset, (U8*)data, size);
	for (U32 i = 0; i < regionCount; ++i)
	{
		if (regions[i].srcOffset + regions[i].size > size) { Logger::Error("Trying To Upload Data Outside Of Source Buffer Range!"); BreakPoint; }
		if (regions[i].dstOffset + regions[i].size > buffer.size) { Logger::Error("Trying To Upload Data Outside Of Destination Buffer Range!"); BreakPoint; }
		regions[i].srcOffset += stagingBuffer.allocationOffset;
	}

	CommandBuffer* commandBuffer = commandBufferRing.GetWriteCommandBuffer(frameIndex);

	commandBuffer->Begin();
	commandBuffer->BufferToBuffer(stagingBuffer, buffer, regionCount, regions);
	commandBuffer->PipelineBarrier(0, 1, &memoryBarrier, 0, nullptr);
	commandBuffer->End();

	commandBuffers[frameIndex].Push(commandBuffer->vkCommandBuffer);

	for (U32 i = 0; i < regionCount; ++i)
	{
		regions[i].srcOffset -= stagingBuffer.allocationOffset; //TODO: Better way to do this
	}

	stagingBuffer.allocationOffset += size;
}

void Renderer::FillBuffer(Buffer& buffer, const Buffer& stagingBuffer, U32 regionCount, VkBufferCopy* regions)
{
	VkBufferMemoryBarrier2 memoryBarrier = BufferBarrier(buffer.vkBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_READ_BIT);

	CommandBuffer* commandBuffer = commandBufferRing.GetWriteCommandBuffer(frameIndex);

	commandBuffer->Begin();
	commandBuffer->BufferToBuffer(stagingBuffer, buffer, regionCount, regions);
	commandBuffer->PipelineBarrier(0, 1, &memoryBarrier, 0, nullptr);
	commandBuffer->End();

	commandBuffers[frameIndex].Push(commandBuffer->vkCommandBuffer);
}

U64 Renderer::UploadToBuffer(Buffer& buffer, U64 size, const void* data)
{
	VkBufferCopy region{};
	region.dstOffset = buffer.allocationOffset;
	region.size = size;
	region.srcOffset = 0;

	FillBuffer(buffer, size, data, 1, &region);

	U64 offset = buffer.allocationOffset;
	buffer.allocationOffset += size;

	return offset;
}

void Renderer::MapBuffer(Buffer& buffer)
{
	if (buffer.memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		vmaMapMemory(allocator, buffer.allocation, &buffer.data);
		buffer.mapped = true;
	}
}

void Renderer::UnmapBuffer(Buffer& buffer)
{
	if (buffer.memoryProperties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		vmaUnmapMemory(allocator, buffer.allocation);
		buffer.data = nullptr;
		buffer.mapped = false;
	}
}

void Renderer::DestroyBuffer(Buffer& buffer)
{
	if (buffer.vkBuffer)
	{
		if (buffer.mapped)
		{
			if (!buffer.allocation->IsPersistentMap()) { vmaUnmapMemory(allocator, buffer.allocation); }
			buffer.data = nullptr;
			buffer.mapped = false;
		}

		vmaDestroyBuffer(allocator, buffer.vkBuffer, buffer.allocation);

		buffer.vkBuffer = nullptr;
	}
}

bool Renderer::CreateTexture(Texture* texture, void* data)
{
	//TODO: Check for blit feature

	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.format = (VkFormat)texture->format;
	imageInfo.flags = 0;
	imageInfo.imageType = (VkImageType)texture->type;
	imageInfo.extent.width = texture->width;
	imageInfo.extent.height = texture->height;
	imageInfo.extent.depth = texture->depth;
	imageInfo.mipLevels = texture->mipmapCount;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if (texture->flags & TEXTURE_FLAG_COMPUTE) { imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT; }
	if (texture->flags & TEXTURE_FLAG_RENDER_TARGET)
	{
		imageInfo.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		if (HasDepthOrStencil((VkFormat)texture->format)) { imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }
		else { imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
	}

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationInfo allocInfo{};
	allocInfo.pName = "texture";
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &texture->image, &texture->allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, texture->Name());

	VkImageViewCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = texture->image;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D; //TODO: Don't hardcode
	info.format = imageInfo.format;
	info.subresourceRange.levelCount = texture->mipmapCount;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.baseMipLevel = 0;

	if (HasDepthOrStencil((VkFormat)texture->format))
	{
		info.subresourceRange.aspectMask = HasDepth((VkFormat)texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		info.subresourceRange.aspectMask |= HasStencil((VkFormat)texture->format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
	}
	else
	{
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->imageView));
	texture->mipmaps[0] = texture->imageView;

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, texture->Name());

	if (data)
	{
		stagingBuffer.allocationOffset = NextMultipleOf(stagingBuffer.allocationOffset, 16);
		if (stagingBuffer.allocationOffset + texture->size > stagingBuffer.size)
		{
			Logger::Warn("Out Of Staging Memory!");
			Renderer::SubmitTransfer();

			VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
			waitInfo.pNext = nullptr;
			waitInfo.flags = 0;
			waitInfo.semaphoreCount = 1;
			waitInfo.pSemaphores = &transferCompleted[frameIndex];
			waitInfo.pValues = &transferWaitValues[previousFrame];

			vkWaitSemaphores(device, &waitInfo, U64_MAX);
		}

		Copy((U8*)stagingBuffer.data + stagingBuffer.allocationOffset, (U8*)data, texture->size);

		VkBufferImageCopy region{};
		region.bufferOffset = stagingBuffer.allocationOffset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { texture->width, texture->height, texture->depth };

		stagingBuffer.allocationOffset += texture->size;

		VkImageMemoryBarrier2 copyBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
		VkImageMemoryBarrier2 mipBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);

		CommandBuffer* commandBuffer = commandBufferRing.GetWriteCommandBuffer(frameIndex);
		commandBuffer->Begin();
		commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
		commandBuffer->BufferToImage(stagingBuffer, texture, 1, &region);
		commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &mipBarrier);

		info.subresourceRange.levelCount = 1;

		//TODO: Some textures could have mipmaps stored in them already
		for (U32 i = 1; i < texture->mipmapCount; ++i)
		{
			VkImageBlit blitRegion{};
			blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.srcSubresource.layerCount = 1;
			blitRegion.srcSubresource.mipLevel = i - 1;
			blitRegion.srcOffsets[1].x = texture->width >> (i - 1);
			blitRegion.srcOffsets[1].y = texture->height >> (i - 1);
			blitRegion.srcOffsets[1].z = 1;

			blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.dstSubresource.layerCount = 1;
			blitRegion.dstSubresource.mipLevel = i;
			blitRegion.dstOffsets[1].x = texture->width >> i;
			blitRegion.dstOffsets[1].y = texture->height >> i;
			blitRegion.dstOffsets[1].z = 1;

			copyBarrier.subresourceRange.baseMipLevel = i;
			mipBarrier.subresourceRange.baseMipLevel = i;
			info.subresourceRange.baseMipLevel = i;

			commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
			commandBuffer->Blit(texture, texture, VK_FILTER_LINEAR, 1, &blitRegion);
			commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &mipBarrier);

			VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->mipmaps[i]));

			SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, texture->Name() + "_mip1");
		}

		texture->mipmapsGenerated = texture->mipmapCount > 1;

		VkImageMemoryBarrier2 finalBarrier;

		if (texture->flags & TEXTURE_FLAG_RENDER_TARGET)
		{
			if (HasDepthOrStencil((VkFormat)texture->format))
			{
				finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 0, texture->mipmapCount);
			}
			else
			{
				finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, texture->mipmapCount);
			}
		}
		else
		{
			finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, texture->mipmapCount);
		}

		commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &finalBarrier);
		commandBuffer->End();

		commandBuffers[frameIndex].Push(commandBuffer->vkCommandBuffer);

		texture->imageLayout = finalBarrier.newLayout;
	}
	else if (texture->flags & TEXTURE_FLAG_FORCE_GENERATE_MIPMAPS)
	{
		info.subresourceRange.levelCount = 1;

		for (U32 i = 1; i < texture->mipmapCount; ++i)
		{
			info.subresourceRange.baseMipLevel = i;

			VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->mipmaps[i]));
		}
	}

	VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.magFilter = (VkFilter)texture->sampler.magFilter;
	createInfo.minFilter = (VkFilter)texture->sampler.minFilter;
	createInfo.mipmapMode = (VkSamplerMipmapMode)texture->sampler.mipFilter;
	createInfo.addressModeU = (VkSamplerAddressMode)texture->sampler.boundsModeU;
	createInfo.addressModeV = (VkSamplerAddressMode)texture->sampler.boundsModeV;
	createInfo.addressModeW = (VkSamplerAddressMode)texture->sampler.boundsModeW;
	//float                   mipLodBias;
	createInfo.anisotropyEnable = VK_FALSE;
	//float                   maxAnisotropy;
	createInfo.compareEnable = VK_FALSE;
	//VkCompareOp             compareOp;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = VK_LOD_CLAMP_NONE;
	createInfo.borderColor = (VkBorderColor)texture->sampler.border;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	vkCreateSampler(Renderer::device, &createInfo, Renderer::allocationCallbacks, &texture->sampler.vkSampler);

	SetResourceName(VK_OBJECT_TYPE_SAMPLER, (U64)texture->sampler.vkSampler, texture->Name());

	if (bindlessSupported && !(texture->flags & TEXTURE_FLAG_RENDER_TARGET || texture->flags & TEXTURE_FLAG_NO_BINDLESS))
	{
		Resources::bindlessTexturesToUpdate.Push({ texture->Handle(), frameIndex });
	}

	return true;
}

bool Renderer::CreateCubemap(Texture* texture, void* data, U32* layerSizes)
{
	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = (VkImageType)texture->type;
	imageInfo.format = (VkFormat)texture->format;
	imageInfo.mipLevels = texture->mipmapCount;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.extent = { texture->width, texture->height, texture->depth };
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.arrayLayers = 6;
	imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationInfo allocInfo{};
	allocInfo.pName = texture->Name();
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &texture->image, &texture->allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, texture->Name());

	stagingBuffer.allocationOffset = NextMultipleOf(stagingBuffer.allocationOffset, 16);
	if (stagingBuffer.allocationOffset + texture->size > stagingBuffer.size)
	{
		Logger::Warn("Out Of Staging Memory!");
		Renderer::SubmitTransfer();

		VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
		waitInfo.pNext = nullptr;
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &transferCompleted[frameIndex];
		waitInfo.pValues = &transferWaitValues[previousFrame];

		vkWaitSemaphores(device, &waitInfo, U64_MAX);
	}

	Copy((U8*)stagingBuffer.data + stagingBuffer.allocationOffset, (U8*)data, texture->size);

	VkBufferImageCopy bufferCopyRegions[6 * 14];
	U32 regionCount = 0;
	U64 offset = stagingBuffer.allocationOffset;
	stagingBuffer.allocationOffset += texture->size;

	for (U32 level = 0; level < texture->mipmapCount; ++level)
	{
		for (U32 face = 0; face < 6; ++face)
		{
			VkBufferImageCopy bufferCopyRegion{};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = texture->width >> level;
			bufferCopyRegion.imageExtent.height = texture->height >> level;
			bufferCopyRegion.imageExtent.depth = texture->depth;
			bufferCopyRegion.bufferOffset = offset;
			bufferCopyRegions[regionCount++] = bufferCopyRegion;
			offset += layerSizes[level];
		}
	}

	VkImageMemoryBarrier2 copyBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
	VkImageMemoryBarrier2 finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, texture->mipmapCount, 6);

	CommandBuffer* commandBuffer = commandBufferRing.GetWriteCommandBuffer(frameIndex);
	commandBuffer->Begin();
	commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
	commandBuffer->BufferToImage(stagingBuffer, texture, regionCount, bufferCopyRegions);
	commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &finalBarrier);
	commandBuffer->End();

	commandBuffers[frameIndex].Push(commandBuffer->vkCommandBuffer);

	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkImageViewCreateInfo view{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view.format = (VkFormat)texture->format;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.subresourceRange.layerCount = 6;
	view.subresourceRange.levelCount = texture->mipmapCount;
	view.image = texture->image;
	VkValidateR(vkCreateImageView(device, &view, allocationCallbacks, &texture->imageView));

	texture->mipmaps[0] = texture->imageView;

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, texture->Name());

	VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.magFilter = (VkFilter)texture->sampler.magFilter;
	createInfo.minFilter = (VkFilter)texture->sampler.minFilter;
	createInfo.mipmapMode = (VkSamplerMipmapMode)texture->sampler.mipFilter;
	createInfo.addressModeU = (VkSamplerAddressMode)texture->sampler.boundsModeU;
	createInfo.addressModeV = (VkSamplerAddressMode)texture->sampler.boundsModeV;
	createInfo.addressModeW = (VkSamplerAddressMode)texture->sampler.boundsModeW;
	//float                   mipLodBias;
	createInfo.anisotropyEnable = VK_FALSE;
	//float                   maxAnisotropy;
	createInfo.compareEnable = VK_FALSE;
	//VkCompareOp             compareOp;
	//float                   minLod;
	//float                   maxLod;
	createInfo.borderColor = (VkBorderColor)texture->sampler.border;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	vkCreateSampler(Renderer::device, &createInfo, Renderer::allocationCallbacks, &texture->sampler.vkSampler);

	SetResourceName(VK_OBJECT_TYPE_SAMPLER, (U64)texture->sampler.vkSampler, texture->Name());

	if (bindlessSupported && !(texture->flags & TEXTURE_FLAG_NO_BINDLESS)) { Resources::bindlessTexturesToUpdate.Push({ texture->Handle(), frameIndex }); }

	return true;
}

bool Renderer::CreateRenderpass(Renderpass* renderpass, const RenderpassInfo& info)
{
	renderpass->renderTargetCount = (U8)info.renderTargetCount;
	renderpass->depthStencilTarget = info.depthStencilTarget;
	renderpass->renderArea = info.renderArea;
	renderpass->subpassCount = info.subpassCount;
	renderpass->resize = info.resize;
	Copy(renderpass->subpasses, info.subpasses, info.subpassCount);
	Copy(renderpass->renderTargets, info.renderTargets, info.renderTargetCount);

	if (info.colorLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
	{
		for (U32 i = 0; i < info.renderTargetCount; ++i)
		{
			renderpass->clearValues[renderpass->clearCount++].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
	}

	if (info.depthLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR && renderpass->depthStencilTarget)
	{
		renderpass->clearValues[renderpass->clearCount++].depthStencil = { 1.0f, 0 };
	}

	VkAttachmentDescription2 attachments[MAX_IMAGE_OUTPUTS + 1]{};
	VkAttachmentReference2 colorAttachments[MAX_IMAGE_OUTPUTS]{};
	VkAttachmentReference2 depthAttachment{};

	VkAttachmentLoadOp colorLoadOp = (VkAttachmentLoadOp)info.colorLoadOp;
	VkAttachmentLoadOp depthLoadOp = (VkAttachmentLoadOp)info.depthLoadOp;
	VkAttachmentLoadOp stencilLoadOp = (VkAttachmentLoadOp)info.stencilLoadOp;

	U32 attachmentCount = 0;
	for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
	{
		attachments[attachmentCount].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		attachments[attachmentCount].pNext = nullptr;
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = (VkFormat)renderpass->renderTargets[i]->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = colorLoadOp;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = stencilLoadOp;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = colorLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		if (renderpass->renderTargets[i]->flags & TEXTURE_FLAG_RENDER_SAMPLED) { attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL; }
		else { attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; }

		colorAttachments[attachmentCount].sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		colorAttachments[attachmentCount].pNext = nullptr;
		colorAttachments[attachmentCount].attachment = attachmentCount;
		colorAttachments[attachmentCount].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachments[attachmentCount].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		++attachmentCount;
	}

	bool hasDepth = false;
	if (renderpass->depthStencilTarget)
	{
		attachments[attachmentCount].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		attachments[attachmentCount].pNext = nullptr;
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = (VkFormat)renderpass->depthStencilTarget->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = depthLoadOp;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = stencilLoadOp;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = depthLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		if (renderpass->depthStencilTarget->flags & TEXTURE_FLAG_RENDER_SAMPLED) { attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; }
		else { attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; }

		depthAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		depthAttachment.pNext = nullptr;
		depthAttachment.attachment = attachmentCount;
		depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		hasDepth = true;
	}

	Vector<VkSubpassDependency2> dependencies;
	Vector<VkSubpassDescription2> subpasses(info.subpassCount, {});
	Vector<VkAttachmentReference2> inputReferences;
	U32 inputOffset = 0;

	static VkMemoryBarrier2 depthBarrier{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR,
		.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};

	static VkMemoryBarrier2 renderBarrier{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
	};

	static VkMemoryBarrier2 inputBarrier{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
		.pNext = nullptr,
		.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
		.dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT
	};

	for (U32 i = 0; i < info.subpassCount; ++i)
	{
		for (U32 j = 0; j < info.subpasses[i].inputAttachmentCount; ++j)
		{
			VkAttachmentReference2 inputAttachment{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 };
			inputAttachment.pNext = nullptr;
			inputAttachment.attachment = info.subpasses[i].inputAttachments[j];
			inputAttachment.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			if (inputAttachment.attachment >= attachmentCount) { inputAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; }
			else { inputAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; }

			inputReferences.Push(inputAttachment);
		}

		VkSubpassDescription2& subpass = subpasses[i];
		subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.pNext = nullptr;
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.viewMask = 0;
		subpass.inputAttachmentCount = (U32)inputReferences.Size() - inputOffset;
		subpass.pInputAttachments = inputReferences.Data() + inputOffset;
		subpass.colorAttachmentCount = attachmentCount;
		subpass.pColorAttachments = colorAttachments;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = hasDepth ? &depthAttachment : nullptr;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		inputOffset = (U32)inputReferences.Size();

		if (i == 0) //First Subpass
		{
			if (hasDepth)
			{
				dependencies.Push({
					.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
					.pNext = &depthBarrier,
					.srcSubpass = VK_SUBPASS_EXTERNAL,
					.dstSubpass = i,
					.dependencyFlags = 0,
					.viewOffset = 0
					});
			}

			dependencies.Push({
				.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.pNext = &renderBarrier,
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = i,
				.dependencyFlags = 0,
				.viewOffset = 0
				});
		}
		else //Later Subpasses
		{
			if (hasDepth)
			{
				dependencies.Push({
					.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
					.pNext = &depthBarrier,
					.srcSubpass = i - 1,
					.dstSubpass = i,
					.dependencyFlags = 0,
					.viewOffset = 0
					});
			}

			dependencies.Push({
				.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.pNext = &renderBarrier,
				.srcSubpass = i - 1,
				.dstSubpass = i,
				.dependencyFlags = 0,
				.viewOffset = 0
				});

			dependencies.Push({
				.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
				.pNext = &inputBarrier,
				.srcSubpass = i - 1,
				.dstSubpass = i,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
				.viewOffset = 0
				});
		}
	}

	VkRenderPassCreateInfo2 renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = attachmentCount + hasDepth;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = (U32)subpasses.Size();
	renderPassInfo.pSubpasses = subpasses.Data();
	renderPassInfo.dependencyCount = (U32)dependencies.Size();
	renderPassInfo.pDependencies = dependencies.Data();
	renderPassInfo.correlatedViewMaskCount = 0;
	renderPassInfo.pCorrelatedViewMasks = nullptr;

	VkValidateFR(vkCreateRenderPass2(device, &renderPassInfo, allocationCallbacks, &renderpass->renderpass));

	VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = renderpass->renderpass;
	framebufferInfo.attachmentCount = attachmentCount + hasDepth;
	framebufferInfo.width = renderpass->renderArea.extent.x;
	framebufferInfo.height = renderpass->renderArea.extent.y;
	framebufferInfo.layers = 1;

	VkImageView framebufferAttachments[MAX_IMAGE_OUTPUTS + 1];

	attachmentCount = 0;
	for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
	{
		framebufferAttachments[attachmentCount++] = renderpass->renderTargets[i]->imageView;
	}

	if (renderpass->depthStencilTarget)
	{
		framebufferAttachments[attachmentCount++] = renderpass->depthStencilTarget->imageView;
	}

	framebufferInfo.pAttachments = framebufferAttachments;

	VkValidateFR(vkCreateFramebuffer(device, &framebufferInfo, allocationCallbacks, &renderpass->frameBuffer));

	renderpass->lastResize = absoluteFrame;

	return true;
}

bool Renderer::RecreateRenderpass(Renderpass* renderpass)
{
	if (renderpass->lastResize < absoluteFrame && renderpass->resize)
	{
		renderpass->renderArea.extent.x = Platform::windowWidth;
		renderpass->renderArea.extent.y = Platform::windowHeight;

		vkDestroyFramebuffer(device, renderpass->frameBuffer, allocationCallbacks);

		VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = renderpass->renderpass;
		framebufferInfo.attachmentCount = renderpass->renderTargetCount + (bool)renderpass->depthStencilTarget;
		framebufferInfo.width = Platform::windowWidth;
		framebufferInfo.height = Platform::windowHeight;
		framebufferInfo.layers = 1;

		VkImageView framebufferAttachments[MAX_IMAGE_OUTPUTS + 1];

		U32 attachmentCount = 0;
		for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
		{
			Resources::RecreateTexture(renderpass->renderTargets[i], Platform::windowWidth, Platform::windowHeight, 1);
			framebufferAttachments[attachmentCount++] = renderpass->renderTargets[i]->imageView;
		}

		if (renderpass->depthStencilTarget)
		{
			Resources::RecreateTexture(renderpass->depthStencilTarget, Platform::windowWidth, Platform::windowHeight, 1);
			framebufferAttachments[attachmentCount++] = renderpass->depthStencilTarget->imageView;
		}

		framebufferInfo.pAttachments = framebufferAttachments;

		VkValidateFR(vkCreateFramebuffer(device, &framebufferInfo, allocationCallbacks, &renderpass->frameBuffer));

		renderpass->lastResize = absoluteFrame;
		renderpass->renderArea = { { 0, 0 }, { framebufferInfo.width, framebufferInfo.height } };
	}

	return true;
}

void Renderer::DestroyTextureInstant(Texture* texture)
{
	if (texture)
	{
		for (U32 i = 0; i < texture->mipmapCount; ++i)
		{
			vkDestroyImageView(device, texture->mipmaps[i], allocationCallbacks);

			texture->mipmaps[i] = nullptr;
		}

		texture->imageView = nullptr;

		if (!texture->swapchainImage && texture->image) { vmaDestroyImage(allocator, texture->image, texture->allocation); texture->image = nullptr; }

		if (texture->sampler.vkSampler) { vkDestroySampler(device, texture->sampler.vkSampler, allocationCallbacks); texture->sampler.vkSampler = nullptr; }
	}
}

void Renderer::DestroyRenderPassInstant(Renderpass* renderpass)
{
	if (renderpass)
	{
		if (renderpass->frameBuffer) { vkDestroyFramebuffer(Renderer::device, renderpass->frameBuffer, Renderer::allocationCallbacks); }
		if (renderpass->renderpass) { vkDestroyRenderPass(device, renderpass->renderpass, allocationCallbacks); }
	}
}

bool Renderer::IsDepthStencil(VkFormat value)
{
	return value == VK_FORMAT_D16_UNORM_S8_UINT || value == VK_FORMAT_D24_UNORM_S8_UINT || value == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

bool Renderer::IsDepthOnly(VkFormat value)
{
	return value >= VK_FORMAT_D16_UNORM && value < VK_FORMAT_D32_SFLOAT;
}

bool Renderer::IsStencilOnly(VkFormat value)
{
	return value == VK_FORMAT_S8_UINT;
}

bool Renderer::HasDepth(VkFormat value)
{
	return (value >= VK_FORMAT_D16_UNORM && value < VK_FORMAT_S8_UINT) || (value >= VK_FORMAT_D16_UNORM_S8_UINT && value <= VK_FORMAT_D32_SFLOAT_S8_UINT);
}

bool Renderer::HasStencil(VkFormat value)
{
	return value >= VK_FORMAT_S8_UINT && value <= VK_FORMAT_D32_SFLOAT_S8_UINT;
}

bool Renderer::HasDepthOrStencil(VkFormat value)
{
	return value >= VK_FORMAT_D16_UNORM && value <= VK_FORMAT_D32_SFLOAT_S8_UINT;
}
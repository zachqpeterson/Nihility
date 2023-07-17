#include "Renderer.hpp"

#include "Profiler.hpp"
#include "CommandBuffer.hpp"
#include "Core\Logger.hpp"
#include "Core\File.hpp"
#include "Containers\Vector.hpp"
#include "Platform\Platform.hpp"
#include "Math\Math.hpp"
#include "Math\Hash.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"
#include "Memory\Memory.hpp"
#include "Pipeline.hpp"
#include "Resources\Scene.hpp"

#define VMA_VULKAN_VERSION 1003000
#define VMA_DEBUG_LOG
#define VMA_IMPLEMENTATION
#include "External\vma\vk_mem_alloc.h"

//#include "External\glslang\Include\glslang_c_interface.h"
#include "External\glslang\Public\ShaderLang.h"

static constexpr CSTR extensions[]{
	VK_KHR_SURFACE_EXTENSION_NAME,
	//VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,

#if defined PLATFORM_WINDOWS
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_APPLE
	VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_LINUX
	VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_ANDROID
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_XLIB
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_WAYLAND
	VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined PLATFORM_MIR || PLATFORM_DISPLAY
	VK_KHR_DISPLAY_EXTENSION_NAME,
#elif defined PLATFORM_IOS
	VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif

#ifdef NH_DEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

static constexpr CSTR layers[]{
#ifdef NH_DEBUG
	"VK_LAYER_KHRONOS_validation",
	//"VK_LAYER_LUNARG_core_validation",
	//"VK_LAYER_LUNARG_image",
	//"VK_LAYER_LUNARG_parameter_validation",
	//"VK_LAYER_LUNARG_object_tracker",
#else
	"",
#endif
};

static constexpr CSTR deviceExtensions[]{
	"VK_KHR_swapchain",
	"VK_KHR_dynamic_rendering",
};

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

// INFO
CSTR								Renderer::appName;
U32									Renderer::appVersion;

// CAPABILITIES
VkPhysicalDeviceProperties			Renderer::physicalDeviceProperties;

// DEVICE
VkInstance							Renderer::instance;
VkPhysicalDevice					Renderer::physicalDevice;
VkDevice							Renderer::device;
VkQueue								Renderer::deviceQueue;
Swapchain							Renderer::swapchain{};
U32									Renderer::queueFamilyIndex;

VkAllocationCallbacks* Renderer::allocationCallbacks;
VkDescriptorPool					Renderer::descriptorPool;
U64									Renderer::uboAlignment;
U64									Renderer::sboAlignemnt;

bool								Renderer::bindlessSupported{ false };

// RAY TRACING
VkPhysicalDeviceRayTracingPipelineFeaturesKHR		Renderer::rayTracingPipelineFeatures;
VkPhysicalDeviceRayTracingPipelinePropertiesKHR		Renderer::rayTracingPipelineProperties;
VkPhysicalDeviceAccelerationStructureFeaturesKHR	Renderer::accelerationStructureFeatures;
bool												Renderer::rayTracingPresent{ false };

// WINDOW
U32									Renderer::imageIndex{ 0 };
U32									Renderer::currentFrame{ 1 };
U32									Renderer::previousFrame{ 0 };
U32									Renderer::absoluteFrame{ 0 };
bool								Renderer::resized{ false };

// RESOURCES
Scene* Renderer::currentScene;
VmaAllocator_T* Renderer::allocator;
CommandBufferRing					Renderer::commandBufferRing;
CommandBuffer** Renderer::queuedCommandBuffers;
U32									Renderer::allocatedCommandBufferCount{ 0 };
U32									Renderer::queuedCommandBufferCount{ 0 };
U64									Renderer::dynamicMaxPerFrameSize;
Buffer* Renderer::dynamicBuffer;
U8* Renderer::dynamicMappedMemory;
U64									Renderer::dynamicAllocatedSize;
U64									Renderer::dynamicPerFrameSize;

// TIMING
F32									Renderer::timestampFrequency;
VkQueryPool							Renderer::timestampQueryPool;
VkSemaphore							Renderer::imageAcquired;
VkSemaphore							Renderer::renderCompleted[MAX_SWAPCHAIN_IMAGES];
VkFence								Renderer::commandBufferExecuted[MAX_SWAPCHAIN_IMAGES];
bool								Renderer::timestampsEnabled{ false };
bool								Renderer::timestampReset{ true };

// DEBUG
VkDebugUtilsMessengerEXT			Renderer::debugMessenger;
PFN_vkCreateDebugUtilsMessengerEXT	Renderer::vkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT	Renderer::vkDestroyDebugUtilsMessengerEXT;
PFN_vkSetDebugUtilsObjectNameEXT	Renderer::vkSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT	Renderer::vkCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT		Renderer::vkCmdEndDebugUtilsLabelEXT;
bool								Renderer::debugUtilsExtensionPresent{ false };

bool Renderer::Initialize(CSTR applicationName, U32 applicationVersion)
{
	Logger::Trace("Initializing Vulkan Renderer...");

	appName = applicationName;
	appVersion = applicationVersion;
	allocationCallbacks = nullptr;

	if (!CreateInstance()) { return false; }
	if (!swapchain.CreateSurface()) { return false; }
	if (!SelectGPU()) { return false; }
	if (!CreateDevice()) { return false; }
	if (!CreateResources()) { return false; }
	if (!swapchain.GetFormat()) { return false; }
	if (!swapchain.Create()) { return false; }
	if (!swapchain.CreateRenderPass()) { return false; }

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Vulkan Renderer...");

	if (currentScene)
	{
		Resources::SaveScene(currentScene);
	}

	vkDeviceWaitIdle(device);

	commandBufferRing.Destroy();

	for (U64 i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
	{
		vkDestroySemaphore(device, renderCompleted[i], allocationCallbacks);
		vkDestroyFence(device, commandBufferExecuted[i], allocationCallbacks);
	}

	vkDestroySemaphore(device, imageAcquired, allocationCallbacks);

	MapBufferParameters cbMap{ dynamicBuffer, 0, 0 };
	UnmapBuffer(cbMap);

	Profiler::Shutdown();

	swapchain.Destroy();

	Resources::Shutdown();

	vmaDestroyAllocator(allocator);

#ifdef NH_DEBUG
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocationCallbacks);
#endif

	vkDestroyDescriptorPool(device, descriptorPool, allocationCallbacks);
	vkDestroyQueryPool(device, timestampQueryPool, allocationCallbacks);

	vkDestroyDevice(device, allocationCallbacks);

	vkDestroyInstance(instance, allocationCallbacks);
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
		VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,							//Addition diagnostic data
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
	vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
	vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
	vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");

	VkValidateFR(vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, allocationCallbacks, &debugMessenger));
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
		vkGetPhysicalDeviceProperties(gpu, &physicalDeviceProperties);

		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { if (GetFamilyQueue(gpu)) { discreteGpu = gpu; break; } }
		else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) { if (GetFamilyQueue(gpu)) { integratedGpu = gpu; } }
	}

	if (discreteGpu != VK_NULL_HANDLE) { physicalDevice = discreteGpu; }
	else if (integratedGpu != VK_NULL_HANDLE) { physicalDevice = integratedGpu; }
	else { Logger::Fatal("No Suitable GPU Found!"); return false; }

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	timestampFrequency = physicalDeviceProperties.limits.timestampPeriod / (1000 * 1000);

	Logger::Trace("Best GPU Found: {}", physicalDeviceProperties.deviceName);

	uboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	sboAlignemnt = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

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
		VkQueueFamilyProperties queue_family = queueFamilies[familyIndex];
		if (queue_family.queueCount > 0 && queue_family.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
		{
			VkValidateFR(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, familyIndex, swapchain.surface, &surfaceSupported));

			if (surfaceSupported) { queueFamilyIndex = familyIndex; break; }
		}
	}

	return surfaceSupported;
}

bool Renderer::CreateDevice()
{
	const F32 queuePriorities[]{ 1.0f };
	VkDeviceQueueCreateInfo queueInfo[1]{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo[0].queueFamilyIndex = queueFamilyIndex;
	queueInfo[0].queueCount = 1;
	queueInfo[0].pQueuePriorities = queuePriorities;
	queueInfo[0].flags = 0;
	queueInfo[0].pNext = nullptr;

	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
	VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures };

	accelerationStructureFeatures = VkPhysicalDeviceAccelerationStructureFeaturesKHR{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	rayTracingPipelineFeatures = VkPhysicalDeviceRayTracingPipelineFeaturesKHR{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
	if (rayTracingPresent)
	{
		indexingFeatures.pNext = &accelerationStructureFeatures;
		accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
	}

	VkPhysicalDeviceFeatures2 physicalFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

	//TODO: Setup dynamic rendering
	//VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeaturesKHR{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
	//dynamicRenderingFeaturesKHR.dynamicRendering = VK_TRUE;
	//physicalFeatures2.pNext = &dynamicRenderingFeaturesKHR;

	bindlessSupported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;
	if (bindlessSupported) { physicalFeatures2.pNext = &indexingFeatures; }

	VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.queueCreateInfoCount = CountOf32(queueInfo);
	deviceInfo.pQueueCreateInfos = queueInfo;
	deviceInfo.enabledExtensionCount = CountOf32(deviceExtensions);
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceInfo.pNext = &physicalFeatures2;
	deviceInfo.pEnabledFeatures = nullptr;
	deviceInfo.flags = 0;

	VkValidateFR(vkCreateDevice(physicalDevice, &deviceInfo, allocationCallbacks, &device));

	vkGetDeviceQueue(device, queueFamilyIndex, 0, &deviceQueue);

	return true;
}

bool Renderer::CreateResources()
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

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

	VkValidateFR(vkCreateDescriptorPool(device, &poolInfo, allocationCallbacks, &descriptorPool));

	if (bindlessSupported) { Resources::CreateBindless(); }

	static constexpr U16 timeQueriesPerFrame = 32;

	VkQueryPoolCreateInfo queryPoolInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, };
	queryPoolInfo.pNext = nullptr;
	queryPoolInfo.flags = 0;
	queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolInfo.queryCount = timeQueriesPerFrame * 2u * MAX_SWAPCHAIN_IMAGES;
	queryPoolInfo.pipelineStatistics = 0;

	VkValidateFR(vkCreateQueryPool(device, &queryPoolInfo, allocationCallbacks, &timestampQueryPool));

	Profiler::Initialize(timeQueriesPerFrame, MAX_SWAPCHAIN_IMAGES);

	VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquired);

	for (U64 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &renderCompleted[i]);

		VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceInfo, allocationCallbacks, &commandBufferExecuted[i]);
	}

	commandBufferRing.Create();

	Memory::AllocateArray(&queuedCommandBuffers, 128UI8);

	// TODO: Dynamic buffer handling
	dynamicPerFrameSize = 1024 * 1024 * 10;
	BufferCreation bc;
	bc.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, dynamicPerFrameSize * MAX_SWAPCHAIN_IMAGES).
		SetName("Dynamic_Persistent_Buffer");
	dynamicBuffer = Resources::CreateBuffer(bc);

	MapBufferParameters cbMap{ dynamicBuffer, 0, 0 };
	dynamicMappedMemory = (U8*)MapBuffer(cbMap);

	Resources::CreateDefaults();

	//glslang_initialize_process();
	glslang::InitializeProcess();

	return true;
}

void Renderer::BeginFrame()
{
	// Fence wait and reset
	VkFence renderCompleteFence = commandBufferExecuted[currentFrame];

	if (vkGetFenceStatus(device, renderCompleteFence) != VK_SUCCESS)
	{
		vkWaitForFences(device, 1, &renderCompleteFence, VK_TRUE, UINT64_MAX);
	}

	vkResetFences(device, 1, &renderCompleteFence);
	// Command pool reset
	commandBufferRing.ResetPools(currentFrame);
	// Dynamic memory update
	const U64 usedSize = dynamicAllocatedSize - (dynamicPerFrameSize * previousFrame);
	dynamicMaxPerFrameSize = Math::Max(usedSize, dynamicMaxPerFrameSize);
	dynamicAllocatedSize = dynamicPerFrameSize * currentFrame;

	CommandBuffer* cb = GetCommandBuffer(QUEUE_TYPE_GRAPHICS, true);

	if (currentScene) //TODO: Default scene?
	{
		currentScene->Update();
	}
}

void Renderer::EndFrame()
{
	CommandBuffer* commands = GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	//TODO: Fog
	//TODO: Bloom
	//TODO: Exposure
	//TODO: White Balancing
	//TODO: Contrast
	//TODO: Brightness
	//TODO: Color Filtering
	//TODO: Saturation
	//TODO: Tonemapping
	//TODO: Gamma

	//TODO: UI

	Profiler::Update();

	Resources::Update();

	VkCommandBuffer enqueuedCommandBuffers[4];
	QueueCommandBuffer(enqueuedCommandBuffers, commands);

	VkResult result = swapchain.NextImage(imageIndex, imageAcquired);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Resize();
		FrameCountersAdvance();

		return;
	}

	VkFence renderCompleteFence = commandBufferExecuted[currentFrame];
	VkSemaphore renderCompleteSemaphore = renderCompleted[currentFrame];
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAcquired;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = queuedCommandBufferCount;
	submitInfo.pCommandBuffers = enqueuedCommandBuffers;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

	vkQueueSubmit(deviceQueue, 1, &submitInfo, renderCompleteFence);

	result = swapchain.Present(deviceQueue, imageIndex, renderCompleteSemaphore);

	queuedCommandBufferCount = 0;

	Profiler::Query();

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Settings::Resized())
	{
		Settings::resized = false;
		Resize();

		return;
	}

	FrameCountersAdvance();
}

void Renderer::Resize()
{
	vkDeviceWaitIdle(device);

	swapchain.Create();

	//TODO: Resize all programs

	vkDeviceWaitIdle(device);

	//TODO: Update camera here
}

U32 Renderer::GetFrameIndex()
{
	return imageIndex;
}

void Renderer::LoadScene(const String& name)
{
	if (currentScene)
	{
		Resources::SaveScene(currentScene);
		//TODO: Unload scene
	}

	currentScene = Resources::LoadScene(name);
}

void* Renderer::MapBuffer(const MapBufferParameters& parameters)
{
	if (parameters.buffer == nullptr) { return nullptr; }

	if (parameters.buffer->parentBuffer == dynamicBuffer)
	{
		parameters.buffer->globalOffset = dynamicAllocatedSize;

		return DynamicAllocate(parameters.size == 0 ? parameters.buffer->size : parameters.size);
	}

	void* data;
	vmaMapMemory(allocator, parameters.buffer->allocation, &data);

	return data;
}

void Renderer::UnmapBuffer(const MapBufferParameters& parameters)
{
	if (parameters.buffer == nullptr || parameters.buffer->parentBuffer == dynamicBuffer) { return; }

	vmaUnmapMemory(allocator, parameters.buffer->allocation);
}

void* Renderer::DynamicAllocate(U64 size)
{
	void* mappedMemory = dynamicMappedMemory + dynamicAllocatedSize;
	dynamicAllocatedSize += Memory::MemoryAlign(size, uboAlignment);
	return mappedMemory;
}

void Renderer::SetResourceName(VkObjectType type, U64 handle, CSTR name)
{
	if (!debugUtilsExtensionPresent) { return; }

	VkDebugUtilsObjectNameInfoEXT nameInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	nameInfo.objectType = type;
	nameInfo.objectHandle = handle;
	nameInfo.pObjectName = name;
	vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}

void Renderer::PushMarker(VkCommandBuffer commandBuffer, CSTR name)
{
	VkDebugUtilsLabelEXT label{ VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	label.pLabelName = name;
	label.color[0] = 1.0f;
	label.color[1] = 1.0f;
	label.color[2] = 1.0f;
	label.color[3] = 1.0f;
	vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);
}

void Renderer::PopMarker(VkCommandBuffer commandBuffer)
{
	vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

void Renderer::SetGpuTimestampsEnable(bool value)
{
	timestampsEnabled = value;
}

void Renderer::PushGpuTimestamp(CommandBuffer* commandBuffer, const String& name)
{
	if (!timestampsEnabled) { return; }

	U32 queryIndex = Profiler::Push(currentFrame, name);
	vkCmdWriteTimestamp(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, queryIndex);
}

void Renderer::PopGpuTimestamp(CommandBuffer* commandBuffer)
{
	if (!timestampsEnabled) { return; }

	U32 queryIndex = Profiler::Pop(currentFrame);
	vkCmdWriteTimestamp(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, queryIndex);
}

void Renderer::FrameCountersAdvance()
{
	previousFrame = currentFrame;
	currentFrame = (currentFrame + 1) % swapchain.imageCount;

	++absoluteFrame;
}

void Renderer::QueueCommandBuffer(VkCommandBuffer* enqueuedCommandBuffers, CommandBuffer* commandBuffer)
{
	queuedCommandBuffers[queuedCommandBufferCount++] = commandBuffer;

	// Copy all commands
	for (U32 c = 0; c < queuedCommandBufferCount; ++c)
	{
		CommandBuffer* commandBuffer = queuedCommandBuffers[c];

		enqueuedCommandBuffers[c] = commandBuffer->commandBuffer;

		//TODO: Check if commandBuffer recorded commands
		if (commandBuffer->currentRenderPass && (commandBuffer->currentRenderPass->type != RENDERPASS_TYPE_COMPUTE))
		{
			vkCmdEndRenderPass(commandBuffer->commandBuffer);
		}

		vkEndCommandBuffer(commandBuffer->commandBuffer);
	}
}

CommandBuffer* Renderer::GetCommandBuffer(QueueType type, bool begin)
{
	CommandBuffer* cb = commandBufferRing.GetCommandBuffer(currentFrame, begin);

	// The first commandbuffer issued in the frame is used to reset the timestamp queries used.
	if (timestampReset && begin)
	{
		// These are currently indices!
		vkCmdResetQueryPool(cb->commandBuffer, timestampQueryPool, currentFrame * Profiler::queriesPerFrame * 2, Profiler::queriesPerFrame);

		timestampReset = false;
	}

	return cb;
}

CommandBuffer* Renderer::GetInstantCommandBuffer()
{
	CommandBuffer* cb = commandBufferRing.GetCommandBufferInstant(currentFrame, false);
	return cb;
}

void Renderer::AddImageBarrier(VkCommandBuffer commandBuffer, VkImage image, ResourceType oldState, ResourceType newState, U32 baseMipLevel, U32 mipCount, bool isDepth)
{
	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = mipCount;

	barrier.subresourceRange.baseMipLevel = baseMipLevel;
	barrier.oldLayout = ToVkImageLayout(oldState);
	barrier.newLayout = ToVkImageLayout(newState);
	barrier.srcAccessMask = ToVkAccessFlags(oldState);
	barrier.dstAccessMask = ToVkAccessFlags(newState);

	const VkPipelineStageFlags sourceStageMask = DeterminePipelineStageFlags(barrier.srcAccessMask, QUEUE_TYPE_GRAPHICS);
	const VkPipelineStageFlags destinationStageMask = DeterminePipelineStageFlags(barrier.dstAccessMask, QUEUE_TYPE_GRAPHICS);

	vkCmdPipelineBarrier(commandBuffer, sourceStageMask, destinationStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Renderer::TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED: {imageMemoryBarrier.srcAccessMask = 0; } break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED: { imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: { imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: { imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: { imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT; } break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: { imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: { imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; } break;
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: { imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: { imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; } break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: { imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: { imageMemoryBarrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; } break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
		if (imageMemoryBarrier.srcAccessMask == 0) { imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT; }
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	} break;
	}

	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

bool Renderer::CreateSampler(Sampler* sampler)
{
	VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	createInfo.addressModeU = sampler->addressModeU;
	createInfo.addressModeV = sampler->addressModeV;
	createInfo.addressModeW = sampler->addressModeW;
	createInfo.minFilter = sampler->minFilter;
	createInfo.magFilter = sampler->magFilter;
	createInfo.mipmapMode = sampler->mipFilter;
	createInfo.anisotropyEnable = 0;
	createInfo.compareEnable = 0;
	createInfo.unnormalizedCoordinates = 0;
	createInfo.borderColor = sampler->border;
	// TODO:
	//float                   mipLodBias;
	//float                   maxAnisotropy;
	//float                   minLod;
	//float                   maxLod;
	//VkCompareOp             compareOp;

	vkCreateSampler(device, &createInfo, allocationCallbacks, &sampler->sampler);

	SetResourceName(VK_OBJECT_TYPE_SAMPLER, (U64)sampler->sampler, sampler->name);

	return true;
}

bool Renderer::CreateTexture(Texture* texture, void* data)
{
	//TODO: Check for blit feature

	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.format = texture->format;
	imageInfo.flags = texture->type == TEXTURE_TYPE_CUBE ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	imageInfo.imageType = ToVkImageType(texture->type);
	imageInfo.extent.width = texture->width;
	imageInfo.extent.height = texture->height;
	imageInfo.extent.depth = texture->depth;
	imageInfo.mipLevels = texture->mipmaps;
	imageInfo.arrayLayers = texture->type == TEXTURE_TYPE_CUBE ? 6 : 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	const bool isRenderTarget = (texture->flags & TEXTURE_FLAG_RENDER_TARGET_MASK) == TEXTURE_FLAG_RENDER_TARGET_MASK;
	const bool isComputeUsed = (texture->flags & TEXTURE_FLAG_COMPUTE_MASK) == TEXTURE_FLAG_COMPUTE_MASK;

	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.usage |= isComputeUsed ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

	if (HasDepthOrStencil(texture->format))
	{
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.usage |= isRenderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
	}

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationInfo allocInfo{};
	allocInfo.pName = "texture";
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &texture->image, &texture->allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, texture->name);

	VkImageViewCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = texture->image;
	info.viewType = ToVkImageViewType(texture->type);
	info.format = imageInfo.format;

	if (HasDepthOrStencil(texture->format))
	{
		info.subresourceRange.aspectMask = HasDepth(texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		//info.subresourceRange.aspectMask |= HasStencil(texture->format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
	}
	else
	{
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	info.subresourceRange.levelCount = 1;
	info.subresourceRange.layerCount = 1;
	VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->imageView));

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, texture->name);

	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (bindlessSupported) { Resources::bindlessTexturesToUpdate.Push({ RESOURCE_UPDATE_TYPE_TEXTURE, texture->handle, currentFrame }); }

	if (data)
	{
		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.size = texture->size;

		VmaAllocationCreateInfo memoryInfo{};
		memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocationInfo allocationInfo{};
		allocationInfo.pName = "staging";
		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VkValidate(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo,
			&stagingBuffer, &stagingAllocation, &allocationInfo));

		void* destinationData;
		vmaMapMemory(allocator, stagingAllocation, &destinationData);
		Memory::Copy(destinationData, data, texture->size);
		vmaUnmapMemory(allocator, stagingAllocation);

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CommandBuffer* commandBuffer = GetInstantCommandBuffer();
		vkBeginCommandBuffer(commandBuffer->commandBuffer, &beginInfo);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { texture->width, texture->height, texture->depth };

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		TransitionImage(commandBuffer->commandBuffer, texture->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		vkCmdCopyBufferToImage(commandBuffer->commandBuffer, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		TransitionImage(commandBuffer->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		vkEndCommandBuffer(commandBuffer->commandBuffer);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

		vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(deviceQueue);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

		vkResetCommandBuffer(commandBuffer->commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		CommandBuffer* blitCmd = GetInstantCommandBuffer();
		vkBeginCommandBuffer(blitCmd->commandBuffer, &beginInfo);

		//TODO: Some textures could have mipmaps stored in the already
		for (U32 i = 1; i < texture->mipmaps; ++i)
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

			VkImageSubresourceRange mipSubRange{};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			TransitionImage(blitCmd->commandBuffer, texture->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				mipSubRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			vkCmdBlitImage(blitCmd->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);
			TransitionImage(blitCmd->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				mipSubRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		}

		subresourceRange.levelCount = texture->mipmaps;
		TransitionImage(blitCmd->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT); //VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT

		vkEndCommandBuffer(blitCmd->commandBuffer);

		submitInfo.pCommandBuffers = &blitCmd->commandBuffer;

		vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(deviceQueue);

		vkResetCommandBuffer(blitCmd->commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	}

	return true;
}

bool Renderer::CreateCubeMap(Texture* texture, void* data, U32* layerSizes)
{
	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = texture->format;
	imageInfo.mipLevels = texture->mipmaps;
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
	allocInfo.pName = texture->name;
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &texture->image, &texture->allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, texture->name);

	VkBufferImageCopy bufferCopyRegions[6 * 14];
	U32 regionCount = 0;
	U32 offset = 0;

	for (U32 level = 0; level < texture->mipmaps; ++level)
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

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipmaps;
	subresourceRange.layerCount = 6;

	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	bufferInfo.size = texture->size;

	memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VmaAllocationInfo allocationInfo{};
	allocationInfo.pName = "staging";
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	VkValidate(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo,
		&stagingBuffer, &stagingAllocation, &allocationInfo));

	void* destinationData;
	vmaMapMemory(allocator, stagingAllocation, &destinationData);
	Memory::Copy(destinationData, data, texture->size);
	vmaUnmapMemory(allocator, stagingAllocation);

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	CommandBuffer* commandBuffer = GetInstantCommandBuffer();
	vkBeginCommandBuffer(commandBuffer->commandBuffer, &beginInfo);

	TransitionImage(commandBuffer->commandBuffer, texture->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	vkCmdCopyBufferToImage(commandBuffer->commandBuffer, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regionCount, bufferCopyRegions);
	TransitionImage(commandBuffer->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkEndCommandBuffer(commandBuffer->commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

	vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(deviceQueue);

	vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

	// TODO: free command buffer
	vkResetCommandBuffer(commandBuffer->commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	VkImageViewCreateInfo view{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view.format = texture->format;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.subresourceRange.layerCount = 6;
	view.subresourceRange.levelCount = texture->mipmaps;
	view.image = texture->image;
	VkValidateR(vkCreateImageView(device, &view, allocationCallbacks, &texture->imageView));

	if (bindlessSupported) { Resources::bindlessTexturesToUpdate.Push({ RESOURCE_UPDATE_TYPE_TEXTURE, texture->handle, currentFrame }); }

	return true;
}

bool Renderer::CreateBuffer(Buffer* buffer, void* data)
{
	static const VkBufferUsageFlags dynamicBufferMask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const bool useGlobalBuffer = (buffer->typeFlags & dynamicBufferMask) != 0;
	if (buffer->usage == RESOURCE_USAGE_DYNAMIC && useGlobalBuffer)
	{
		buffer->parentBuffer = dynamicBuffer;
		return true;
	}

	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer->typeFlags;
	bufferInfo.size = buffer->size > 0 ? buffer->size : 1;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VmaAllocationInfo allocationInfo{};
	allocationInfo.pName = "buffer";
	VkValidate(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo, &buffer->buffer, &buffer->allocation, &allocationInfo));

	SetResourceName(VK_OBJECT_TYPE_BUFFER, (U64)buffer->buffer, buffer->name);

	buffer->deviceMemory = allocationInfo.deviceMemory;

	if (data)
	{
		void* mappedData;
		vmaMapMemory(allocator, buffer->allocation, &mappedData);
		Memory::Copy(mappedData, data, (U64)buffer->size);
		vmaUnmapMemory(allocator, buffer->allocation);
	}

	// TODO
	//if ( persistent )
	//{
	//    mapped_data = static_cast<uint8_t *>(allocationInfo.pMappedData);
	//}

	return true;
}

bool Renderer::CreateDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout)
{
	U32 usedBindings = 0;
	for (U32 r = 0; r < descriptorSetLayout->bindingCount; ++r)
	{
		DescriptorBinding& binding = descriptorSetLayout->bindings[r];
		binding.binding = binding.binding == U16_MAX ? (U16)r : binding.binding;
		binding.count = 1;

		VkDescriptorSetLayoutBinding& vkBinding = descriptorSetLayout->vkBindings[usedBindings];
		++usedBindings;

		vkBinding.binding = binding.binding;
		vkBinding.descriptorType = binding.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : binding.type;
		vkBinding.descriptorCount = 1;

		// TODO: differentiate shader stages
		vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
		vkBinding.pImmutableSamplers = nullptr;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = usedBindings;
	layoutInfo.pBindings = descriptorSetLayout->vkBindings;

	vkCreateDescriptorSetLayout(device, &layoutInfo, allocationCallbacks, &descriptorSetLayout->descriptorSetLayout);

	return true;
}

bool Renderer::CreateRenderPass(Renderpass* renderpass)
{
	if (renderpass->type == RENDERPASS_TYPE_COMPUTE)
	{
		Logger::Error("Compute RenderPasses Are Not Yet Supported!");
		return false;
	}

	for (U32 i = 0; i < renderpass->renderTargetCount; ++i) { renderpass->output.Color(renderpass->outputTextures[i]->format); }
	if (renderpass->outputDepth) { renderpass->output.Depth(renderpass->outputDepth->format); }

	VkAttachmentLoadOp colorOp, depthOp, stencilOp;
	VkImageLayout colorInitial, depthInitial;

	//TODO: Store VkAttachmentLoadOp instead of RenderPassOperation
	//TODO: Check if this is first pass/last pass
	switch (renderpass->output.colorOperation)
	{
	case RENDER_PASS_OP_LOAD: { colorOp = VK_ATTACHMENT_LOAD_OP_LOAD; colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; } break;
	case RENDER_PASS_OP_CLEAR: { colorOp = VK_ATTACHMENT_LOAD_OP_CLEAR; colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; } break;
	case RENDER_PASS_OP_DONT_CARE:
	default: { colorOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; colorInitial = VK_IMAGE_LAYOUT_UNDEFINED; } break;
	}

	switch (renderpass->output.depthOperation)
	{
	case RENDER_PASS_OP_LOAD: { depthOp = VK_ATTACHMENT_LOAD_OP_LOAD; depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; } break;
	case RENDER_PASS_OP_CLEAR: { depthOp = VK_ATTACHMENT_LOAD_OP_CLEAR; depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; } break;
	case RENDER_PASS_OP_DONT_CARE:
	default: { depthOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; depthInitial = VK_IMAGE_LAYOUT_UNDEFINED; } break;
	}

	switch (renderpass->output.stencilOperation)
	{
	case RENDER_PASS_OP_LOAD: { stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD; } break;
	case RENDER_PASS_OP_CLEAR: { stencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR; } break;
	case RENDER_PASS_OP_DONT_CARE:
	default: { stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; } break;
	}

	VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1]{};
	VkAttachmentReference colorAttachments[MAX_IMAGE_OUTPUTS]{};
	VkAttachmentReference depthAttachment{};

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	U32 attachmentCount = 0;
	if (renderpass->type == RENDERPASS_TYPE_SWAPCHAIN)
	{
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = renderpass->outputTextures[0]->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = colorOp;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = stencilOp;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		colorAttachments[attachmentCount].attachment = attachmentCount;
		colorAttachments[attachmentCount].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		renderpass->clears[attachmentCount].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		++renderpass->clearCount;

		++attachmentCount;
	}
	else
	{
		for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
		{
			attachments[attachmentCount].flags = 0;
			attachments[attachmentCount].format = renderpass->outputTextures[i]->format;
			attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[attachmentCount].loadOp = colorOp;
			attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[attachmentCount].stencilLoadOp = stencilOp;
			attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			colorAttachments[attachmentCount].attachment = attachmentCount;
			colorAttachments[attachmentCount].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			++attachmentCount;
		}
	}

	subpass.colorAttachmentCount = attachmentCount;
	subpass.pColorAttachments = colorAttachments;

	if (renderpass->outputDepth)
	{
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = renderpass->outputDepth->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = depthOp;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = stencilOp;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthAttachment.attachment = attachmentCount;
		depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depthAttachment;

		if (renderpass->type == RENDERPASS_TYPE_SWAPCHAIN)
		{
			renderpass->clears[attachmentCount].depthStencil = { 1, 0 };
			++renderpass->clearCount;
		}

		++attachmentCount;
	}

	VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	//TODO: This is hardcoded
	if (renderpass->type == RENDERPASS_TYPE_SWAPCHAIN)
	{
		VkSubpassDependency dependencies[2]{};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].dstSubpass = 0;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].srcAccessMask = 0;
		dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies;
	}
	else
	{
		VkSubpassDependency dependencies[2]{};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies;
	}

	VkValidate(vkCreateRenderPass(device, &renderPassInfo, allocationCallbacks, &renderpass->renderpass));

	SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (U64)renderpass->renderpass, renderpass->name);

	renderpass->viewport.viewportCount = 0;
	renderpass->viewport.scissorCount = 0;

	//TODO: Pass in Viewport info
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.width = renderpass->width;
	viewport.y = renderpass->height;
	viewport.height = (F32)-renderpass->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	renderpass->viewport.viewports[0] = viewport;
	++renderpass->viewport.viewportCount;

	VkRect2D scissor{};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = renderpass->width;
	scissor.extent.height = renderpass->height;
	renderpass->viewport.scissors[0] = scissor;
	++renderpass->viewport.scissorCount;

	VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = renderpass->renderpass;
	framebufferInfo.attachmentCount = attachmentCount;
	framebufferInfo.width = renderpass->width;
	framebufferInfo.height = renderpass->height;
	framebufferInfo.layers = 1;

	VkImageView framebufferAttachments[MAX_IMAGE_OUTPUTS + 1];

	if (renderpass->type == RENDERPASS_TYPE_SWAPCHAIN)
	{
		for (U64 i = 0; i < swapchain.imageCount; i++)
		{
			framebufferAttachments[0] = renderpass->outputTextures[i]->imageView;
			framebufferAttachments[1] = renderpass->outputDepth->imageView;
			framebufferInfo.pAttachments = framebufferAttachments;

			vkCreateFramebuffer(device, &framebufferInfo, allocationCallbacks, &renderpass->frameBuffers[i]);
			SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)renderpass->frameBuffers[i], renderpass->name);
		}
	}
	else
	{
		attachmentCount = 0;
		for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
		{
			framebufferAttachments[attachmentCount++] = renderpass->outputTextures[i]->imageView;
		}

		if (renderpass->outputDepth)
		{
			framebufferAttachments[attachmentCount++] = renderpass->outputDepth->imageView;
		}

		framebufferInfo.pAttachments = framebufferAttachments;

		vkCreateFramebuffer(device, &framebufferInfo, allocationCallbacks, &renderpass->frameBuffers[0]);
		SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)renderpass->frameBuffers[0], renderpass->name);
	}

	if (renderpass->type == RENDERPASS_TYPE_SWAPCHAIN)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CommandBuffer* commandBuffer = GetInstantCommandBuffer();
		vkBeginCommandBuffer(commandBuffer->commandBuffer, &beginInfo);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { renderpass->width, renderpass->height, 1 };

		for (U64 i = 0; i < swapchain.imageCount; ++i)
		{
			AddImageBarrier(commandBuffer->commandBuffer, renderpass->outputTextures[i]->image, RESOURCE_TYPE_UNDEFINED, RESOURCE_TYPE_PRESENT, 0, 1, false);
		}

		vkEndCommandBuffer(commandBuffer->commandBuffer);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

		vkQueueSubmit(deviceQueue, 1, &submitInfo, nullptr);
		vkQueueWaitIdle(deviceQueue);
	}

	return true;
}

void Renderer::DestroySamplerInstant(Sampler* sampler)
{
	if (sampler)
	{
		vkDestroySampler(device, sampler->sampler, allocationCallbacks);
	}
}

void Renderer::DestroyTextureInstant(Texture* texture)
{
	if (texture)
	{
		vkDestroyImageView(device, texture->imageView, allocationCallbacks);
		if (!texture->swapchainImage) { vmaDestroyImage(allocator, texture->image, texture->allocation); }
	}
}

void Renderer::DestroyBufferInstant(Buffer* buffer)
{
	if (buffer)
	{
		static const VkBufferUsageFlags dynamicBufferMask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		const bool useGlobalBuffer = (buffer->typeFlags & dynamicBufferMask) != 0;

		if (!(buffer->usage == RESOURCE_USAGE_DYNAMIC && useGlobalBuffer))
		{
			vmaDestroyBuffer(allocator, buffer->buffer, buffer->allocation);
		}
	}
}

void Renderer::DestroyRenderPassInstant(Renderpass* renderpass)
{
	if (renderpass)
	{
		for (U32 i = 0; i < swapchain.imageCount; ++i)
		{
			vkDestroyFramebuffer(Renderer::device, renderpass->frameBuffers[i], Renderer::allocationCallbacks);
		}

		for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
		{
			Resources::DestroyTexture(renderpass->outputTextures[i]);
		}

		if (renderpass->outputDepth)
		{
			Resources::DestroyTexture(renderpass->outputDepth);
		}

		vkDestroyRenderPass(device, renderpass->renderpass, allocationCallbacks);
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
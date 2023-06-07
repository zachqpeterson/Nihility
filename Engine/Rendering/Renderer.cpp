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

#define VMA_VULKAN_VERSION 1003000
#define VMA_DEBUG_LOG
#define VMA_IMPLEMENTATION
#include "External\vk_mem_alloc.h"

static constexpr CSTR extensions[]{
	VK_KHR_SURFACE_EXTENSION_NAME,

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
//	"VK_KHR_dynamic_rendering",
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

VkAllocationCallbacks*				Renderer::allocationCallbacks;
VkDescriptorPool					Renderer::descriptorPool;
U64									Renderer::uboAlignment;
U64									Renderer::sboAlignemnt;

Queue<ResourceUpdate>				Renderer::bindlessTexturesToUpdate{};
VkDescriptorPool					Renderer::bindlessDescriptorPool;
VkDescriptorSet						Renderer::bindlessDescriptorSet;
VkDescriptorSetLayout				Renderer::bindlessDescriptorSetLayout;
bool								Renderer::bindlessSupported{ false };

// RAY TRACING
VkPhysicalDeviceRayTracingPipelineFeaturesKHR		Renderer::rayTracingPipelineFeatures;
VkPhysicalDeviceRayTracingPipelinePropertiesKHR		Renderer::rayTracingPipelineProperties;
VkPhysicalDeviceAccelerationStructureFeaturesKHR	Renderer::accelerationStructureFeatures;
bool												Renderer::rayTracingPresent{ false };

// WINDOW
RenderPassOutput					Renderer::swapchainOutput;
RenderPass*							Renderer::offscreenPass;
RenderPass*							Renderer::filterPass;
U32									Renderer::imageIndex{ 0 };
U32									Renderer::currentFrame{ 1 };
U32									Renderer::previousFrame{ 0 };
U32									Renderer::absoluteFrame{ 0 };
bool								Renderer::resized{ false };

// RESOURCES
VmaAllocator_T*						Renderer::allocator;
Queue<DescriptorSetUpdate>			Renderer::descriptorSetUpdates;
CommandBufferRing					Renderer::commandBufferRing;
CommandBuffer**						Renderer::queuedCommandBuffers;
U32									Renderer::allocatedCommandBufferCount{ 0 };
U32									Renderer::queuedCommandBufferCount{ 0 };
U64									Renderer::dynamicMaxPerFrameSize;
Buffer*								Renderer::dynamicBuffer;
U8*									Renderer::dynamicMappedMemory;
U64									Renderer::dynamicAllocatedSize;
U64									Renderer::dynamicPerFrameSize;
C8									Renderer::binariesPath[512];
Buffer*								Renderer::fullscreenVertexBuffer;
Buffer*								Renderer::dummyConstantBuffer; //TODO: Move to Resources

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

	vkDeviceWaitIdle(device);

	commandBufferRing.Destroy();

	for (U64 i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
	{
		vkDestroySemaphore(device, renderCompleted[i], allocationCallbacks);
		vkDestroyFence(device, commandBufferExecuted[i], allocationCallbacks);
	}

	vkDestroySemaphore(device, imageAcquired, allocationCallbacks);

	MapBufferParameters cbMap = { dynamicBuffer, 0, 0 };
	UnmapBuffer(cbMap);

	Resources::Shutdown();

	Profiler::Shutdown();

	swapchain.Destroy();

	vmaDestroyAllocator(allocator);

	descriptorSetUpdates.Destroy();
	bindlessTexturesToUpdate.Destroy();

#ifdef NH_DEBUG
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocationCallbacks);
#endif

	vkDestroyDescriptorSetLayout(device, bindlessDescriptorSetLayout, allocationCallbacks);
	vkDestroyDescriptorPool(device, bindlessDescriptorPool, allocationCallbacks);
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

	if (bindlessSupported)
	{
		VkDescriptorPoolSize bindlessPoolSizes[]
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxBindlessResources },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxBindlessResources },
		};

		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
		poolInfo.maxSets = maxBindlessResources * CountOf32(bindlessPoolSizes);
		poolInfo.poolSizeCount = CountOf32(bindlessPoolSizes);
		poolInfo.pPoolSizes = bindlessPoolSizes;

		VkValidateFR(vkCreateDescriptorPool(device, &poolInfo, allocationCallbacks, &bindlessDescriptorPool));

		const U32 poolCount = CountOf32(bindlessPoolSizes);
		VkDescriptorSetLayoutBinding vkBinding[4];

		// Actual descriptor set layout
		VkDescriptorSetLayoutBinding& imageSamplerBinding = vkBinding[0];
		imageSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageSamplerBinding.descriptorCount = maxBindlessResources;
		imageSamplerBinding.binding = bindlessTextureBinding;
		imageSamplerBinding.stageFlags = VK_SHADER_STAGE_ALL;
		imageSamplerBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding& storageImageBinding = vkBinding[1];
		storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		storageImageBinding.descriptorCount = maxBindlessResources;
		storageImageBinding.binding = bindlessTextureBinding + 1;
		storageImageBinding.stageFlags = VK_SHADER_STAGE_ALL;
		storageImageBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = poolCount;
		layoutInfo.pBindings = vkBinding;
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		// TODO: reenable variable descriptor count
		// Binding flags
		VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | /*VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |*/ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		VkDescriptorBindingFlags bindingFlags[4];

		bindingFlags[0] = bindlessFlags;
		bindingFlags[1] = bindlessFlags;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
		extendedInfo.bindingCount = poolCount;
		extendedInfo.pBindingFlags = bindingFlags;

		layoutInfo.pNext = &extendedInfo;

		VkValidateFR(vkCreateDescriptorSetLayout(device, &layoutInfo, allocationCallbacks, &bindlessDescriptorSetLayout));

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = bindlessDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &bindlessDescriptorSetLayout;

		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
		U32 maxBinding = maxBindlessResources - 1;
		countInfo.descriptorSetCount = 1;
		// This number is the max allocatable count
		countInfo.pDescriptorCounts = &maxBinding;
		//allocInfo.pNext = &countInfo;

		VkValidateFR(vkAllocateDescriptorSets(device, &allocInfo, &bindlessDescriptorSet));
	}

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

	descriptorSetUpdates.Reserve(16);

	BufferCreation fullscreenVbCreation{};
	fullscreenVbCreation.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, 0);
	fullscreenVbCreation.SetName("Fullscreen_vb");
	fullscreenVertexBuffer = Resources::CreateBuffer(fullscreenVbCreation);

	BufferCreation dummyConstantBufferCreation{};
	dummyConstantBufferCreation.Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, 16);
	dummyConstantBufferCreation.SetName("Dummy_cb");
	dummyConstantBuffer = Resources::CreateBuffer(dummyConstantBufferCreation);

#if defined(_MSC_VER)
	ExpandEnvironmentStringsA("%VULKAN_SDK%", binariesPath, 512);
	Memory::Copy(binariesPath + Length(binariesPath), "\\Bin\\", 7);
#else
	String vulkanEnv(getenv("VULKAN_SDK"));
	vulkanEnv.Append("/bin/");
	Memory::Copy(binariesPath + Length(binariesPath), vulkanEnv.Data(), 7);
#endif

	// TODO: Dynamic buffer handling
	dynamicPerFrameSize = 1024 * 1024 * 10;
	BufferCreation bc;
	bc.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, dynamicPerFrameSize * MAX_SWAPCHAIN_IMAGES).
		SetName("Dynamic_Persistent_Buffer");
	dynamicBuffer = Resources::CreateBuffer(bc);

	MapBufferParameters cbMap = { dynamicBuffer, 0, 0 };
	dynamicMappedMemory = (U8*)MapBuffer(cbMap);

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

	// Descriptor Set Updates
	while (descriptorSetUpdates.Size())
	{
		DescriptorSetUpdate update;
		descriptorSetUpdates.Pop(update);

		UpdateDescriptorSetInstant(update);
	}

	GetCommandBuffer(QUEUE_TYPE_GRAPHICS, true);
}

void Renderer::EndFrame()
{
	CommandBuffer* cb = GetCommandBuffer(QUEUE_TYPE_GRAPHICS, false);

	Profiler::Update();

	//TODO: temp
	QueueCommandBuffer(cb);

	VkResult result = swapchain.NextImage(imageIndex, imageAcquired);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Resize();
		FrameCountersAdvance();

		return;
	}

	VkFence renderCompleteFence = commandBufferExecuted[currentFrame];
	VkSemaphore renderCompleteSemaphore = renderCompleted[currentFrame];

	// Copy all commands
	VkCommandBuffer enqueuedCommandBuffers[4];
	for (U32 c = 0; c < queuedCommandBufferCount; ++c)
	{
		CommandBuffer* commandBuffer = queuedCommandBuffers[c];

		enqueuedCommandBuffers[c] = commandBuffer->commandBuffer;

		//TODO: Check if commandBuffer recorded commands
		if (commandBuffer->currentRenderPass && (commandBuffer->currentRenderPass->type != RENDER_PASS_TYPE_COMPUTE))
		{
			vkCmdEndRenderPass(commandBuffer->commandBuffer);
		}

		vkEndCommandBuffer(commandBuffer->commandBuffer);
	}

	if (bindlessTexturesToUpdate.Size())
	{
		VkWriteDescriptorSet bindlessDescriptorWrites[maxBindlessResources];
		VkDescriptorImageInfo bindlessImageInfo[maxBindlessResources];

		Texture* dummyTexture = Resources::AccessDummyTexture();

		U32 currentWriteIndex = 0;

		while (bindlessTexturesToUpdate.Size())
		{
			ResourceUpdate textureToUpdate;
			bindlessTexturesToUpdate.Pop(textureToUpdate);

			//TODO: Maybe check frame
			{
				Texture* texture = Resources::AccessTexture(textureToUpdate.handle);

				VkWriteDescriptorSet& descriptorWrite = bindlessDescriptorWrites[currentWriteIndex];
				descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.dstArrayElement = (U32)textureToUpdate.handle;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.dstSet = bindlessDescriptorSet;
				descriptorWrite.dstBinding = bindlessTextureBinding;

				Sampler* defaultSampler = Resources::AccessDefaultSampler();
				VkDescriptorImageInfo& descriptorImageInfo = bindlessImageInfo[currentWriteIndex];

				if (texture->sampler != nullptr) { descriptorImageInfo.sampler = texture->sampler->sampler; }
				else { descriptorImageInfo.sampler = defaultSampler->sampler; }

				descriptorImageInfo.imageView = texture->format != VK_FORMAT_UNDEFINED ? texture->imageView : dummyTexture->imageView;
				descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptorWrite.pImageInfo = &descriptorImageInfo;

				++currentWriteIndex;
			}
		}

		if (currentWriteIndex) { vkUpdateDescriptorSets(device, currentWriteIndex, bindlessDescriptorWrites, 0, nullptr); }
	}

	VkSemaphore waitSemaphores[]{ imageAcquired };
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
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
		FrameCountersAdvance();

		return;
	}

	FrameCountersAdvance();

	Resources::Update();
}

void Renderer::Resize()
{
	vkDeviceWaitIdle(device);

	swapchain.Create();

	vkDeviceWaitIdle(device);

	//TODO: Update camera here
}

void Renderer::UpdateDescriptorSet(DescriptorSet* descriptorSet)
{
	if (descriptorSet) { descriptorSetUpdates.Push({ descriptorSet, currentFrame }); }
	else { Logger::Error("Trying to update invalid DescriptorSet!"); }
}

void Renderer::UpdateDescriptorSetInstant(const DescriptorSetUpdate& update)
{
	DescriptorSet* descriptorSet = update.descriptorSet;
	DescriptorSetLayout* descriptorSetLayout = descriptorSet->layout;

	DescriptorSet deleteDescriptorSet{};
	deleteDescriptorSet.descriptorSet = descriptorSet->descriptorSet;

	DestroyDescriptorSetInstant(&deleteDescriptorSet);

	VkWriteDescriptorSet descriptorWrite[8];
	VkDescriptorBufferInfo bufferInfo[8];
	VkDescriptorImageInfo imageInfo[8];

	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSet->layout->descriptorSetLayout;
	vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet->descriptorSet);

	U32 resourceCount = descriptorSetLayout->bindingCount;
	FillWriteDescriptorSets(descriptorSetLayout, descriptorSet->descriptorSet, descriptorWrite, bufferInfo, imageInfo, Resources::AccessDefaultSampler()->sampler,
		resourceCount, descriptorSet->resources, descriptorSet->samplers, descriptorSet->bindings);

	vkUpdateDescriptorSets(device, resourceCount, descriptorWrite, 0, nullptr);
}

//void Renderer::ResizeOutputTextures(RenderPass* renderPass, U32 width, U32 height)

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

	VkDebugUtilsObjectNameInfoEXT nameInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	nameInfo.objectType = type;
	nameInfo.objectHandle = handle;
	nameInfo.pObjectName = name;
	vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}

void Renderer::PushMarker(VkCommandBuffer commandBuffer, CSTR name)
{
	VkDebugUtilsLabelEXT label = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
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

void Renderer::QueueCommandBuffer(CommandBuffer* commandBuffer)
{
	queuedCommandBuffers[queuedCommandBufferCount++] = commandBuffer;
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

void Renderer::FillWriteDescriptorSets(const DescriptorSetLayout* descriptorSetLayout, VkDescriptorSet vkDescriptorSet,
	VkWriteDescriptorSet* descriptorWrite, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo,
	VkSampler vkDefaultSampler, U32& resourceCount, void** resources, Sampler** samplers, U16* bindings)
{
	U32 usedResources = 0;
	for (U32 r = 0; r < resourceCount; ++r)
	{
		// Binding array contains the index into the resource layout binding to retrieve
		// the correct binding informations.
		U32 layoutBindingIndex = bindings[r];

		const DescriptorBinding& binding = descriptorSetLayout->bindings[layoutBindingIndex];

		U32 i = usedResources;
		++usedResources;

		descriptorWrite[i] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite[i].dstSet = vkDescriptorSet;
		// Use binding array to get final binding point.
		const U32 bindingPoint = binding.start;
		descriptorWrite[i].dstBinding = bindingPoint;
		descriptorWrite[i].dstArrayElement = 0;
		descriptorWrite[i].descriptorCount = 1;

		switch (binding.type)
		{
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		{
			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			Texture* textureData = (Texture*)resources[r];

			// TODO: improve. Remove the single texture interface ?
			imageInfo[i].sampler = vkDefaultSampler;
			if (textureData->sampler) { imageInfo[i].sampler = textureData->sampler->sampler; }
			else if (samplers[r] != nullptr) { imageInfo[i].sampler = samplers[r]->sampler; }

			imageInfo[i].imageLayout = HasDepthOrStencil(textureData->format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[i].imageView = textureData->imageView;

			descriptorWrite[i].pImageInfo = &imageInfo[i];

			break;
		}

		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		{
			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

			Texture* textureData = (Texture*)resources[r];

			imageInfo[i].sampler = nullptr;
			imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo[i].imageView = textureData->imageView;

			descriptorWrite[i].pImageInfo = &imageInfo[i];

			break;
		}

		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		{
			Buffer* buffer = (Buffer*)resources[r];

			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite[i].descriptorType = buffer->usage == RESOURCE_USAGE_DYNAMIC ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			if (buffer->parentBuffer != nullptr) { bufferInfo[i].buffer = buffer->parentBuffer->buffer; }
			else { bufferInfo[i].buffer = buffer->buffer; }

			bufferInfo[i].offset = 0;
			bufferInfo[i].range = buffer->size;

			descriptorWrite[i].pBufferInfo = &bufferInfo[i];

			break;
		}

		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			Buffer* buffer = (Buffer*)resources[r];

			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

			if (buffer->parentBuffer != nullptr) { bufferInfo[i].buffer = buffer->parentBuffer->buffer; }
			else { bufferInfo[i].buffer = buffer->buffer; }

			bufferInfo[i].offset = 0;
			bufferInfo[i].range = buffer->size;

			descriptorWrite[i].pBufferInfo = &bufferInfo[i];

			break;
		}

		default:
		{
			Logger::Fatal("Resource type {} not supported in descriptor set creation!", (U32)binding.type);
			break;
		}
		}
	}

	resourceCount = usedResources;
}

//TODO: Cache compiled shaders
VkShaderModuleCreateInfo Renderer::CompileShader(CSTR path, VkShaderStageFlagBits stage, CSTR name)
{
	VkShaderModuleCreateInfo shaderCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

	// TODO: detect if input is HLSL

	String stageDefine(ToStageDefines(stage), "_", name);
	stageDefine.ToUpper();

	String glslCompilerPath(binariesPath, "glslangValidator.exe");
	String finalSpirvFilename("shader_final.spv");

#if defined(_MSC_VER)
	// TODO: add optional debug information in shaders (option -g).
	String arguments("glslangValidator.exe {} -V --target-env vulkan1.3 -o {} -S {} --D {} --D {}", path, finalSpirvFilename, ToCompilerExtension(stage), stageDefine, ToStageDefines(stage));
#else
	String arguments("{} -V --target-env vulkan1.3 -o {} -S {} --D {} --D {}", path, finalSpirvFilename, ToCompilerExtension(stage), stageDefine, ToStageDefines(stage));
#endif

	Platform::ExecuteProcess(".", glslCompilerPath, arguments, "");

	bool optimizeShaders = false;

	if (optimizeShaders)
	{
		String spirvOptimizerPath(binariesPath, "spirv-opt.exe");
		String optimizedSpirvFilename("shader_opt.spv");
		String spirvOptArguments("spirv-opt.exe -O --preserve-bindings {} -o {}", finalSpirvFilename, optimizedSpirvFilename);

		Platform::ExecuteProcess(".", spirvOptimizerPath, spirvOptArguments, "");

		// Read back SPV file.
		File optimizedSpirvFile(optimizedSpirvFilename, FILE_OPEN_RESOURCE_READ);
		shaderCreateInfo.codeSize = optimizedSpirvFile.ReadAll(&shaderCreateInfo.pCode);

		optimizedSpirvFile.Close();
		File::Delete(optimizedSpirvFilename);
	}
	else
	{
		// Read back SPV file.
		File optimizedSpirvFile(finalSpirvFilename, FILE_OPEN_RESOURCE_READ);
		shaderCreateInfo.codeSize = optimizedSpirvFile.ReadAll(&shaderCreateInfo.pCode);

		optimizedSpirvFile.Close();
		File::Delete(finalSpirvFilename);
	}

	return shaderCreateInfo;
}

VkRenderPass Renderer::CreateVulkanRenderPass(const RenderPassOutput& output)
{
	VkAttachmentDescription colorAttachments[8] = {};
	VkAttachmentReference colorAttachmentsRef[8] = {};

	VkAttachmentLoadOp colorOp, depthOp, stencilOp;
	VkImageLayout colorInitial, depthInitial;

	switch (output.colorOperation)
	{
	case RENDER_PASS_OP_LOAD: { colorOp = VK_ATTACHMENT_LOAD_OP_LOAD; colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; } break;
	case RENDER_PASS_OP_CLEAR: { colorOp = VK_ATTACHMENT_LOAD_OP_CLEAR; colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; } break;
	default: { colorOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; colorInitial = VK_IMAGE_LAYOUT_UNDEFINED; } break;
	}

	switch (output.depthOperation)
	{
	case RENDER_PASS_OP_LOAD: { depthOp = VK_ATTACHMENT_LOAD_OP_LOAD; depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; } break;
	case RENDER_PASS_OP_CLEAR: { depthOp = VK_ATTACHMENT_LOAD_OP_CLEAR; depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; } break;
	default: { depthOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; depthInitial = VK_IMAGE_LAYOUT_UNDEFINED; } break;
	}

	switch (output.stencilOperation)
	{
	case RENDER_PASS_OP_LOAD: { stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD; } break;
	case RENDER_PASS_OP_CLEAR: { stencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR; } break;
	default: { stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; } break;
	}

	U32 attachment = 0;
	for (; attachment < output.colorFormatCount; ++attachment)
	{
		VkAttachmentDescription& colorAttachment = colorAttachments[attachment];
		colorAttachment.format = output.colorFormats[attachment];
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = colorOp;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = stencilOp;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = colorInitial;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference& colorAttachmentRef = colorAttachmentsRef[attachment];
		colorAttachmentRef.attachment = attachment;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentDescription depthAttachment{};
	VkAttachmentReference depthAttachmentRef{};

	if (output.depthStencilFormat != VK_FORMAT_UNDEFINED)
	{
		depthAttachment.format = output.depthStencilFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = depthOp;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = stencilOp;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = depthInitial;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthAttachmentRef.attachment = attachment;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Create subpass.
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// Calculate active attachments for the subpass
	VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1]{};
	U32 activeAttachments = 0;
	for (; activeAttachments < output.colorFormatCount; ++activeAttachments)
	{
		attachments[activeAttachments] = colorAttachments[activeAttachments];
		++activeAttachments;
	}
	subpass.colorAttachmentCount = activeAttachments ? activeAttachments - 1 : 0;
	subpass.pColorAttachments = colorAttachmentsRef;

	U32 depthStencilCount = 0;
	if (output.depthStencilFormat != VK_FORMAT_UNDEFINED)
	{
		attachments[subpass.colorAttachmentCount] = depthAttachment;

		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		depthStencilCount = 1;
	}

	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };

	renderPassInfo.attachmentCount = (activeAttachments ? activeAttachments - 1 : 0) + depthStencilCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	//TODO: Create external subpass dependencies
	//VkSubpassDependency externalDependencies[16];
	//U32 externalDependencyCount = 0;

	VkRenderPass renderPass;
	VkValidate(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	return renderPass;
}

void Renderer::CreateSwapchainPass(RenderPass* renderPass)
{
	//TODO: This is hardcoded, pass in attachment info

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = renderPass->outputTextures[0].format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = renderPass->outputDepth.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkValidate(vkCreateRenderPass(device, &renderPassInfo, allocationCallbacks, &renderPass->renderPass));

	SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (U64)renderPass->renderPass, renderPass->name);

	VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = renderPass->renderPass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.width = renderPass->width;
	framebufferInfo.height = renderPass->height;
	framebufferInfo.layers = 1;
	
	VkImageView framebufferAttachments[2];
	framebufferAttachments[1] = renderPass->outputDepth.imageView;
	
	for (U64 i = 0; i < swapchain.imageCount; i++)
	{
		framebufferAttachments[0] = renderPass->outputTextures[i].imageView;
		framebufferInfo.pAttachments = framebufferAttachments;
	
		vkCreateFramebuffer(device, &framebufferInfo, allocationCallbacks, &renderPass->frameBuffers[i]);
		SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)renderPass->frameBuffers[i], renderPass->name);
	}

	// Manually transition the texture
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
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
	region.imageExtent = { renderPass->width, renderPass->height, 1 };

	// Transition
	for (U64 i = 0; i < swapchain.imageCount; ++i)
	{
		AddImageBarrier(commandBuffer->commandBuffer, renderPass->outputTextures[i].image, RESOURCE_TYPE_UNDEFINED, RESOURCE_TYPE_PRESENT, 0, 1, false);
	}

	vkEndCommandBuffer(commandBuffer->commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

	vkQueueSubmit(deviceQueue, 1, &submitInfo, nullptr);
	vkQueueWaitIdle(deviceQueue);
}

RenderTarget Renderer::CreateRenderTarget(U16 width, U16 height, VkFormat format, bool depth)
{
	RenderTarget target{};
	target.depth = depth;
	target.format = format;

	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.format = format;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (depth)
	{
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationInfo allocInfo{};
	allocInfo.pName = "texture";
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &target.image, &target.allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)target.image, "RenderTarget");

	VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = target.image;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = format;

	if (depth) { info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; }
	else { info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; }

	info.subresourceRange.levelCount = 1;
	info.subresourceRange.layerCount = 1;
	VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &target.imageView));

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)target.imageView, "RenderTargetView");

	return target;
}

void Renderer::RecreateRenderTarget(RenderTarget& target, U16 width, U16 height)
{
	DestroyRenderTarget(target);

	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.format = target.format;
	imageInfo.flags = 0;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (target.depth)
	{
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationInfo allocInfo{};
	allocInfo.pName = "texture";
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &target.image, &target.allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)target.image, "RenderTarget");

	VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = target.image;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = target.format;

	if (target.depth) { info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; }
	else { info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; }

	info.subresourceRange.levelCount = 1;
	info.subresourceRange.layerCount = 1;
	VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &target.imageView));

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)target.imageView, "RenderTargetView");
}

void Renderer::CreateFramebuffer(RenderPass* renderPass)
{
	VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = renderPass->renderPass;
	framebufferInfo.width = renderPass->width;
	framebufferInfo.height = renderPass->height;
	framebufferInfo.layers = 1;

	VkImageView framebufferAttachments[MAX_IMAGE_OUTPUTS + 1]{};
	U32 activeAttachments = 0;
	for (; activeAttachments < renderPass->renderTargetCount; ++activeAttachments) { framebufferAttachments[activeAttachments] = renderPass->outputTextures[activeAttachments].imageView; }

	if (renderPass->outputDepth.image) { framebufferAttachments[activeAttachments++] = renderPass->outputDepth.imageView; }
	framebufferInfo.pAttachments = framebufferAttachments;
	framebufferInfo.attachmentCount = activeAttachments;

	vkCreateFramebuffer(device, &framebufferInfo, nullptr, &renderPass->frameBuffers[0]);
	SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)renderPass->frameBuffers[0], renderPass->name);
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
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	// TODO:
	/*float                   mipLodBias;
	float                   maxAnisotropy;
	VkCompareOp             compareOp;
	float                   minLod;
	float                   maxLod;
	VkBorderColor           borderColor;
	VkBool32                unnormalizedCoordinates;*/

	vkCreateSampler(device, &createInfo, allocationCallbacks, &sampler->sampler);

	SetResourceName(VK_OBJECT_TYPE_SAMPLER, (U64)sampler->sampler, sampler->name);

	return true;
}

bool Renderer::CreateTexture(Texture* texture, void* data)
{
	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.format = texture->format;
	imageInfo.flags = 0;
	imageInfo.imageType = ToVkImageType(texture->type);
	imageInfo.extent.width = texture->width;
	imageInfo.extent.height = texture->height;
	imageInfo.extent.depth = texture->depth;
	imageInfo.mipLevels = texture->mipmaps;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	const bool isRenderTarget = (texture->flags & TEXTURE_FLAG_RENDER_TARGET_MASK) == TEXTURE_FLAG_RENDER_TARGET_MASK;
	const bool isComputeUsed = (texture->flags & TEXTURE_FLAG_COMPUTE_MASK) == TEXTURE_FLAG_COMPUTE_MASK;

	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.usage |= isComputeUsed ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

	if (HasDepthOrStencil(texture->format))
	{
		// Depth/Stencil textures are normally textures you render into.
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO
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

	VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = texture->image;
	info.viewType = ToVkImageViewType(texture->type);
	info.format = imageInfo.format;

	if (HasDepthOrStencil(texture->format))
	{
		info.subresourceRange.aspectMask = HasDepth(texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		// TODO:gs
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

	texture->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Add deferred bindless update.
	if (bindlessSupported) { bindlessTexturesToUpdate.Push({ RESOURCE_UPDATE_TYPE_TEXTURE, texture->handle, currentFrame }); }

	if (data)
	{
		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		U32 imageSize = texture->width * texture->height * 4;
		bufferInfo.size = imageSize;

		VmaAllocationCreateInfo memoryInfo{};
		memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocationInfo allocationInfo{};
		allocationInfo.pName = "staging";
		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VkValidate(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo,
			&stagingBuffer, &stagingAllocation, &allocationInfo));

		// Copy buffer_data
		void* destinationData;
		vmaMapMemory(allocator, stagingAllocation, &destinationData);
		Memory::Copy(destinationData, data, (U64)imageSize);
		vmaUnmapMemory(allocator, stagingAllocation);

		// Execute command buffer
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

		// Copy from the staging buffer to the image
		AddImageBarrier(commandBuffer->commandBuffer, texture->image, RESOURCE_TYPE_UNDEFINED, RESOURCE_TYPE_COPY_DEST, 0, 1, false);
		// Copy
		vkCmdCopyBufferToImage(commandBuffer->commandBuffer, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		// Prepare first mip to create lower mipmaps
		if (texture->mipmaps > 1)
		{
			AddImageBarrier(commandBuffer->commandBuffer, texture->image, RESOURCE_TYPE_COPY_DEST, RESOURCE_TYPE_COPY_SOURCE, 0, 1, false);
		}

		I32 w = texture->width;
		I32 h = texture->height;

		for (I32 mipIndex = 1; mipIndex < texture->mipmaps; ++mipIndex)
		{
			AddImageBarrier(commandBuffer->commandBuffer, texture->image, RESOURCE_TYPE_UNDEFINED, RESOURCE_TYPE_COPY_DEST, mipIndex, 1, false);

			VkImageBlit blitRegion{ };
			blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.srcSubresource.mipLevel = mipIndex - 1;
			blitRegion.srcSubresource.baseArrayLayer = 0;
			blitRegion.srcSubresource.layerCount = 1;

			blitRegion.srcOffsets[0] = { 0, 0, 0 };
			blitRegion.srcOffsets[1] = { w, h, 1 };

			w /= 2;
			h /= 2;

			blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.dstSubresource.mipLevel = mipIndex;
			blitRegion.dstSubresource.baseArrayLayer = 0;
			blitRegion.dstSubresource.layerCount = 1;

			blitRegion.dstOffsets[0] = { 0, 0, 0 };
			blitRegion.dstOffsets[1] = { w, h, 1 };

			vkCmdBlitImage(commandBuffer->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);

			// Prepare current mip for next level
			AddImageBarrier(commandBuffer->commandBuffer, texture->image, RESOURCE_TYPE_COPY_DEST, RESOURCE_TYPE_COPY_SOURCE, mipIndex, 1, false);
		}

		// Transition
		AddImageBarrier(commandBuffer->commandBuffer, texture->image, (texture->mipmaps > 1) ? 
			RESOURCE_TYPE_COPY_SOURCE : RESOURCE_TYPE_COPY_DEST, RESOURCE_TYPE_SHADER_RESOURCE, 0, texture->mipmaps, false);

		vkEndCommandBuffer(commandBuffer->commandBuffer);

		// Submit command buffer
		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

		vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(deviceQueue);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

		// TODO: free command buffer
		vkResetCommandBuffer(commandBuffer->commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

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
		binding.start = binding.start == U16_MAX ? (U16)r : binding.start;
		binding.count = 1;

		VkDescriptorSetLayoutBinding& vkBinding = descriptorSetLayout->binding[usedBindings];
		++usedBindings;

		vkBinding.binding = binding.start;
		vkBinding.descriptorType = binding.type;
		vkBinding.descriptorType = vkBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : vkBinding.descriptorType;
		vkBinding.descriptorCount = 1;

		// TODO:
		vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
		vkBinding.pImmutableSamplers = nullptr;
	}

	// Create the descriptor set layout
	VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layout_info.bindingCount = usedBindings;
	layout_info.pBindings = descriptorSetLayout->binding;

	vkCreateDescriptorSetLayout(device, &layout_info, allocationCallbacks, &descriptorSetLayout->descriptorSetLayout);

	return true;
}

bool Renderer::CreateDescriptorSet(DescriptorSet* descriptorSet)
{
	// Allocate descriptor set
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSet->layout->descriptorSetLayout;

	VkValidate(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet->descriptorSet));

	// Update descriptor set
	VkWriteDescriptorSet descriptorWrite[8];
	VkDescriptorBufferInfo bufferInfo[8];
	VkDescriptorImageInfo imageInfo[8];

	U32 resourceCount = descriptorSet->resourceCount;
	FillWriteDescriptorSets(descriptorSet->layout, descriptorSet->descriptorSet, descriptorWrite, bufferInfo, imageInfo, Resources::AccessDefaultSampler()->sampler,
		resourceCount, descriptorSet->resources, descriptorSet->samplers, descriptorSet->bindings);

	vkUpdateDescriptorSets(device, resourceCount, descriptorWrite, 0, nullptr);

	return true;
}

bool Renderer::CreateRenderPass(RenderPass* renderPass)
{
	switch (renderPass->type)
	{
	case RENDER_PASS_TYPE_SWAPCHAIN: {
		CreateSwapchainPass(renderPass);
	} break;
	case RENDER_PASS_TYPE_COMPUTE: { Logger::Error("Compute RenderPasses Are Not Yet Supported!"); } return false;
	case RENDER_PASS_TYPE_GEOMETRY: {
		for (U32 i = 0; i < renderPass->renderTargetCount; ++i) { renderPass->output.Color(renderPass->outputTextures[i].format); }
		if (renderPass->outputDepth.image) { renderPass->output.Depth(renderPass->outputDepth.format); }

		renderPass->renderPass = CreateVulkanRenderPass(renderPass->output);
		SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (U64)renderPass, renderPass->name);

		CreateFramebuffer(renderPass);
	} break;
	}

	return true;
}

bool Renderer::CreateShaderState(ShaderState* shaderState, const ShaderStateCreation& info)
{
	shaderState->graphicsPipeline = true;
	shaderState->activeShaders = 0;

	U32 compiledShaders = 0;

	for (compiledShaders = 0; compiledShaders < info.stagesCount; ++compiledShaders)
	{
		const ShaderStage& stage = info.stages[compiledShaders];

		// Gives priority to compute: if any is present (and it should not be) then it is not a graphics pipeline.
		if (stage.type == VK_SHADER_STAGE_COMPUTE_BIT) { shaderState->graphicsPipeline = false; }

		VkShaderModuleCreateInfo shaderInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

		shaderInfo = CompileShader(stage.name, stage.type, info.name);

		Resources::ParseSPIRV(shaderInfo, shaderState);

		// Compile shader module
		VkPipelineShaderStageCreateInfo& shaderStageInfo = shaderState->shaderStageInfos[compiledShaders];
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.pNext = nullptr;
		shaderStageInfo.flags = 0;
		shaderStageInfo.stage = stage.type;
		shaderStageInfo.pName = shaderState->entry.Data();
		shaderStageInfo.pSpecializationInfo = nullptr;

		if (vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderStageInfo.module) != VK_SUCCESS) { break; }


		SetResourceName(VK_OBJECT_TYPE_SHADER_MODULE, (U64)shaderStageInfo.module, info.name);
	}

	bool creationFailed = compiledShaders != info.stagesCount;
	if (!creationFailed)
	{
		shaderState->activeShaders = compiledShaders;
		shaderState->name = info.name;
	}

	if (creationFailed)
	{
		Logger::Error("Error in creation of shader {}! Dumping all shader informations...", info.name);

		return false;
	}

	return true;
}

bool Renderer::CreatePipeline(Pipeline* pipeline, RenderPass* renderPass, const String& cachePath)
{
	VkPipelineCache pipelineCache = nullptr;
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

	bool cacheExists = false;
	File cache(cachePath, FILE_OPEN_RESOURCE_READ);
	if (cache.Opened())
	{
		U8* data = nullptr;
		U32 size = cache.ReadAll(&data);

		VkPipelineCacheHeaderVersionOne* cacheHeader = (VkPipelineCacheHeaderVersionOne*)data;

		if (cacheHeader->deviceID == physicalDeviceProperties.deviceID &&
			cacheHeader->vendorID == physicalDeviceProperties.vendorID &&
			memcmp(cacheHeader->pipelineCacheUUID, physicalDeviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0)
		{
			pipelineCacheCreateInfo.initialDataSize = size;
			pipelineCacheCreateInfo.pInitialData = data;
			cacheExists = true;
		}

		VkValidate(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, allocationCallbacks, &pipelineCache));

		cache.Close();
	}
	else
	{
		VkValidate(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, allocationCallbacks, &pipelineCache));
	}

	VkDescriptorSetLayout vkLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	U32 activeLayoutCount = pipeline->shaderState->setCount;

	for (U32 l = 0; l < pipeline->shaderState->setCount; ++l)
	{
		pipeline->descriptorSetLayouts[l] = Resources::CreateDescriptorSetLayout(pipeline->shaderState->sets[l]);
		vkLayouts[l] = pipeline->descriptorSetLayouts[l]->descriptorSetLayout;
	}

	pipeline->activeLayoutCount = activeLayoutCount;

	U32 bindlessActive = 0;
	if (bindlessSupported)
	{
		vkLayouts[pipeline->activeLayoutCount] = bindlessDescriptorSetLayout;
		bindlessActive = 1;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.pSetLayouts = vkLayouts;
	pipelineLayoutInfo.setLayoutCount = pipeline->activeLayoutCount + bindlessActive;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.pNext = nullptr;
	//TODO:
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout;
	VkValidate(vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocationCallbacks, &pipelineLayout));

	pipeline->pipelineLayout = pipelineLayout;

	if (pipeline->shaderState->graphicsPipeline)
	{
		VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		pipelineInfo.pStages = pipeline->shaderState->shaderStageInfos;
		pipelineInfo.stageCount = pipeline->shaderState->activeShaders;
		pipelineInfo.layout = pipelineLayout;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		VkVertexInputAttributeDescription vertexAttributes[8];
		if (pipeline->shaderState->vertexAttributeCount)
		{
			for (U32 i = 0; i < pipeline->shaderState->vertexAttributeCount; ++i)
			{
				const VertexAttribute& vertexAttribute = pipeline->shaderState->vertexAttributes[i];
				vertexAttributes[i] = { vertexAttribute.location, vertexAttribute.binding, ToVkVertexFormat(vertexAttribute), vertexAttribute.offset };
			}

			vertexInputInfo.vertexAttributeDescriptionCount = pipeline->shaderState->vertexAttributeCount;
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes;
		}
		else
		{
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		}

		VkVertexInputBindingDescription vertexBindings[8];
		if (pipeline->shaderState->vertexStreamCount)
		{
			vertexInputInfo.vertexBindingDescriptionCount = pipeline->shaderState->vertexStreamCount;

			for (U32 i = 0; i < pipeline->shaderState->vertexStreamCount; ++i)
			{
				const VertexStream& vertexStream = pipeline->shaderState->vertexStreams[i];
				VkVertexInputRate vertexRate = vertexStream.inputRate == VERTEX_INPUT_RATE_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
				vertexBindings[i] = { vertexStream.binding, vertexStream.stride, vertexRate };
			}
			vertexInputInfo.pVertexBindingDescriptions = vertexBindings;
		}
		else
		{
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr;
		}

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		pipelineInfo.pInputAssemblyState = &inputAssembly;

		VkPipelineColorBlendAttachmentState colorBlendAttachment[8]{};

		if (pipeline->blendState.activeStates)
		{
			for (U64 i = 0; i < pipeline->blendState.activeStates; ++i)
			{
				const BlendState& blendState = pipeline->blendState.blendStates[i];

				colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachment[i].blendEnable = blendState.blendEnabled ? VK_TRUE : VK_FALSE;
				colorBlendAttachment[i].srcColorBlendFactor = blendState.sourceColor;
				colorBlendAttachment[i].dstColorBlendFactor = blendState.destinationColor;
				colorBlendAttachment[i].colorBlendOp = blendState.colorOperation;

				if (blendState.separateBlend)
				{
					colorBlendAttachment[i].srcAlphaBlendFactor = blendState.sourceAlpha;
					colorBlendAttachment[i].dstAlphaBlendFactor = blendState.destinationAlpha;
					colorBlendAttachment[i].alphaBlendOp = blendState.alphaOperation;
				}
				else
				{
					colorBlendAttachment[i].srcAlphaBlendFactor = blendState.sourceColor;
					colorBlendAttachment[i].dstAlphaBlendFactor = blendState.destinationColor;
					colorBlendAttachment[i].alphaBlendOp = blendState.colorOperation;
				}
			}
		}
		else
		{
			colorBlendAttachment[0].blendEnable = VK_FALSE;
			colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = pipeline->blendState.activeStates ? pipeline->blendState.activeStates : 1;
		colorBlending.pAttachments = colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		pipelineInfo.pColorBlendState = &colorBlending;

		VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		depthStencil.depthWriteEnable = pipeline->depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencil.stencilTestEnable = pipeline->depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthTestEnable = pipeline->depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = pipeline->depthStencil.depthComparison;
		if (pipeline->depthStencil.stencilEnable)
		{
			// TODO: Support stencil buffers
			Logger::Error("Stencil Buffers Not Yet Supported!");
		}

		pipelineInfo.pDepthStencilState = &depthStencil;

		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		pipelineInfo.pMultisampleState = &multisampling;

		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = pipeline->rasterization.cullMode;
		rasterizer.frontFace = pipeline->rasterization.front;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		pipelineInfo.pRasterizationState = &rasterizer;

		pipelineInfo.pTessellationState;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (F32)renderPass->width;
		viewport.height = (F32)renderPass->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { renderPass->width, renderPass->height };

		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		pipelineInfo.pViewportState = &viewportState;

		pipelineInfo.renderPass = renderPass->renderPass;

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = CountOf32(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;

		pipelineInfo.pDynamicState = &dynamicState;

		vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, allocationCallbacks, &pipeline->pipeline);

		pipeline->bindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
	else
	{
		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

		pipelineInfo.stage = pipeline->shaderState->shaderStageInfos[0];
		pipelineInfo.layout = pipelineLayout;

		vkCreateComputePipelines(device, pipelineCache, 1, &pipelineInfo, allocationCallbacks, &pipeline->pipeline);

		pipeline->bindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
	}

	if (!cacheExists)
	{
		cache.Open(cachePath, FILE_OPEN_WRITE_SETTINGS);

		U64 cacheDataSize = 0;
		VkValidate(vkGetPipelineCacheData(device, pipelineCache, &cacheDataSize, nullptr));

		void* cacheData;
		Memory::AllocateSize(&cacheData, cacheDataSize);
		VkValidate(vkGetPipelineCacheData(device, pipelineCache, &cacheDataSize, cacheData));

		cache.Write(cacheData, (U32)cacheDataSize);
		Memory::FreeSize(&cacheData);

		cache.Close();
	}

	vkDestroyPipelineCache(device, pipelineCache, allocationCallbacks);

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
		vmaDestroyImage(allocator, texture->image, texture->allocation);
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

void Renderer::DestroyDescriptorSetLayoutInstant(DescriptorSetLayout* layout)
{
	if (layout)
	{
		vkDestroyDescriptorSetLayout(device, layout->descriptorSetLayout, allocationCallbacks);

		Memory::FreeSize(&layout->bindings);
	}
}

void Renderer::DestroyDescriptorSetInstant(DescriptorSet* set)
{
	if (set)
	{
		Memory::FreeSize(&set->resources);
	}
}

void Renderer::DestroyRenderPassInstant(RenderPass* renderPass)
{
	if (renderPass)
	{
		for (U32 i = 0; i < swapchain.imageCount; ++i)
		{
			vkDestroyFramebuffer(Renderer::device, renderPass->frameBuffers[i], Renderer::allocationCallbacks);
		}

		for (U32 i = 0; i < renderPass->renderTargetCount; ++i)
		{
			DestroyRenderTarget(renderPass->outputTextures[i]);
		}

		if (renderPass->outputDepth.image)
		{
			DestroyRenderTarget(renderPass->outputDepth);
		}

		vkDestroyRenderPass(device, renderPass->renderPass, allocationCallbacks);
	}
}

void Renderer::DestroyShaderStateInstant(ShaderState* shader)
{
	if (shader)
	{
		for (U64 i = 0; i < shader->activeShaders; i++)
		{
			vkDestroyShaderModule(device, shader->shaderStageInfos[i].module, allocationCallbacks);
		}
	}
}

void Renderer::DestroyPipelineInstant(Pipeline* pipeline)
{
	if (pipeline)
	{
		vkDestroyPipeline(device, pipeline->pipeline, allocationCallbacks);
		vkDestroyPipelineLayout(device, pipeline->pipelineLayout, allocationCallbacks);
	}
}

void Renderer::DestroyRenderTarget(RenderTarget& target)
{
	vkDestroyImageView(device, target.imageView, allocationCallbacks);
	if (!target.swapchainTarget) { vmaDestroyImage(allocator, target.image, target.allocation); }
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
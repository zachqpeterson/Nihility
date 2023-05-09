#include "Renderer.hpp"

#include "CommandBuffer.hpp"
#include "Core\Logger.hpp"
#include "Core\File.hpp"
#include "Containers\Vector.hpp"
#include "Platform\Platform.hpp"
#include "Math\Math.hpp"
#include "Math\Hash.hpp"
#include "Resources\Settings.hpp"

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

bool Renderer::Initialize(CSTR applicationName, U32 applicationVersion)
{
	Logger::Trace("Initializing Vulkan Renderer...");

	appName = applicationName;
	appVersion = applicationVersion;
	allocationCallbacks = nullptr;

	if (!CreateInstance()) { return false; }
	if (!CreateSurface()) { return false; }
	if (!SelectGPU()) { return false; }
	if (!CreateDevice()) { return false; }
	if (!SetFormats()) { return false; }
	if (!CreateSwapchain()) { return false; }
	if (!CreatePools()) { return false; }
	if (!CreatePrimitiveResources()) { return false; }

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Vulkan Renderer...");

	vkDeviceWaitIdle(device);

	commandBufferRing.Destroy();

	for (size_t i = 0; i < MAX_SWAPCHAIN_IMAGES; i++)
	{
		vkDestroySemaphore(device, renderCompleted[i], allocationCallbacks);
		vkDestroyFence(device, commandBufferExecuted[i], allocationCallbacks);
	}

	vkDestroySemaphore(device, imageAcquired, allocationCallbacks);

	timestampManager->Destroy();

	MapBufferParameters cbMap = { dynamicBuffer, 0, 0 };
	UnmapBuffer(cbMap);

	Memory::FreeSize(&timestampManager);

	DestroyTexture(depthTexture);
	DestroyBuffer(fullscreenVertexBuffer);
	DestroyBuffer(dynamicBuffer);
	DestroyRenderPass(swapchainPass);
	DestroyTexture(dummyTexture);
	DestroyBuffer(dummyConstantBuffer);
	DestroySampler(defaultSampler);

	while (resourceDeletionQueue.Size())
	{
		ResourceUpdate resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == -1) { continue; }

		switch (resourceDeletion.type)
		{
		case RESOURCE_DELETE_TYPE_BUFFER: { DestroyBufferInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_PIPELINE: { DestroyPipelineInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_RENDER_PASS: { DestroyRenderPassInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET: { DestroyDescriptorSetInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT: { DestroyDescriptorSetLayoutInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_SAMPLER: { DestroySamplerInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_SHADER_STATE: { DestroyShaderStateInstant(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_TEXTURE: { DestroyTextureInstant(resourceDeletion.handle); } break;
		}
	}

	auto it = renderPassCache.begin();
	auto end = renderPassCache.end();
	for (; it != end; ++it)
	{
		if (it.Valid()) { vkDestroyRenderPass(device, *it, allocationCallbacks); }
	}

	renderPassCache.Destroy();

	RenderPass* pass = AccessRenderPass(swapchainPass);
	vkDestroyRenderPass(device, pass->renderPass, allocationCallbacks);

	DestroySwapchain();
	vkDestroySurfaceKHR(instance, surface, allocationCallbacks);

	vmaDestroyAllocator(allocator);

	resourceDeletionQueue.Destroy();
	descriptorSetUpdates.Destroy();

	pipelines.Destroy();
	buffers.Destroy();
	shaders.Destroy();
	textures.Destroy();
	samplers.Destroy();
	descriptorSetLayouts.Destroy();
	descriptorSets.Destroy();
	renderPasses.Destroy();

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

	VkValidationFeaturesEXT features = {};
	features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
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

bool Renderer::CreateSurface()
{
#ifdef PLATFORM_WINDOWS
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.pNext = nullptr;
	surfaceInfo.flags = 0;
	const WindowData& wd = Platform::GetWindowData();
	surfaceInfo.hinstance = wd.instance;
	surfaceInfo.hwnd = wd.window;

	VkValidateFR(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, allocationCallbacks, &surface));
#elif PLATFORM_LINUX
	//TODO:
#elif PLATFORM_APPLE
	//TODO:
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
	ssboAlignemnt = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

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
			VkValidateFR(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, familyIndex, surface, &surfaceSupported));

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

	VkPhysicalDeviceFeatures2 physicalFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalFeatures2);

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

bool Renderer::SetFormats()
{
	const VkFormat imageFormats[]{
		VK_FORMAT_R16G16B16A16_SFLOAT, //TODO: Enable and disable HDR
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_R8G8B8_UNORM,
		VK_FORMAT_B8G8R8_UNORM
	};
	const VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

	U32 supportedCount;
	VkValidateFR(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &supportedCount, nullptr));
	Vector<VkSurfaceFormatKHR> supportedFormats(supportedCount, {});
	VkValidateFR(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &supportedCount, supportedFormats.Data()));

	swapchainOutput.Reset();

	bool formatFound = false;
	const U32 surfaceFormatCount = CountOf32(imageFormats);

	for (U32 i = 0; i < surfaceFormatCount; ++i)
	{
		for (U32 j = 0; j < supportedCount; ++j)
		{
			if (supportedFormats[j].format == imageFormats[i] && supportedFormats[j].colorSpace == colorSpace)
			{
				surfaceFormat = supportedFormats[j];
				swapchainOutput.Color(imageFormats[j]);
				formatFound = true;
				break;
			}
		}

		if (formatFound) { break; }
	}

	if (!formatFound) { Logger::Fatal("Failed to Find a Valid Surface Format!"); return false; }

	supportedCount = 0;
	VkPresentModeKHR supportedModes[8];
	VkValidateFR(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &supportedCount, nullptr));
	VkValidateFR(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &supportedCount, supportedModes));

	bool modeFound = false;
	VkPresentModeKHR requestedMode = VK_PRESENT_MODE_FIFO_KHR; //TODO: request VK_PRESENT_MODE_MAILBOX_KHR
	for (U32 i = 0; i < supportedCount; ++i)
	{
		if (requestedMode == supportedModes[i]) { modeFound = true; break; }
	}

	presentMode = modeFound ? requestedMode : VK_PRESENT_MODE_FIFO_KHR;
	swapchainImageCount = 3;

	return true;
}

bool Renderer::CreateSwapchain()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkValidateFR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

	VkExtent2D swapchainExtent = surfaceCapabilities.currentExtent;
	if (swapchainExtent.width == UINT32_MAX)
	{
		swapchainExtent.width = Math::Clamp(swapchainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		swapchainExtent.height = Math::Clamp(swapchainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
	}

	swapchainWidth = (U16)swapchainExtent.width;
	swapchainHeight = (U16)swapchainExtent.height;

	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = surface;
	swapchain_create_info.minImageCount = swapchainImageCount;
	swapchain_create_info.imageFormat = surfaceFormat.format;
	swapchain_create_info.imageExtent = swapchainExtent;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.preTransform = surfaceCapabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = presentMode;

	VkValidateFR(vkCreateSwapchainKHR(device, &swapchain_create_info, 0, &swapchain));

	VkValidateFR(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr));
	VkValidateFR(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages));

	for (size_t i = 0; i < swapchainImageCount; ++i)
	{
		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = surfaceFormat.format;
		viewInfo.image = swapchainImages[i];
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

		VkValidateFR(vkCreateImageView(device, &viewInfo, allocationCallbacks, swapchainImageViews + i));
	}

	return true;
}

bool Renderer::CreatePools()
{
	static constexpr U16 timeQueriesPerFrame = 32; //TODO: Don't hardcode this

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;

	VkValidateFR(vmaCreateAllocator(&allocatorInfo, &allocator));

	static constexpr U32 globalPoolElements = 128;
	VkDescriptorPoolSize poolSizes[] =
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

	VkQueryPoolCreateInfo queryPoolInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, };
	queryPoolInfo.pNext = nullptr;
	queryPoolInfo.flags = 0;
	queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolInfo.queryCount = timeQueriesPerFrame * 2u * MAX_SWAPCHAIN_IMAGES;
	queryPoolInfo.pipelineStatistics = 0;

	VkValidateFR(vkCreateQueryPool(device, &queryPoolInfo, allocationCallbacks, &timestampQueryPool));

	buffers.Create();
	textures.Create();
	renderPasses.Create();
	descriptorSetLayouts.Create();
	pipelines.Create();
	shaders.Create();
	descriptorSets.Create();
	samplers.Create();

	VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquired);

	for (U64 i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
	{
		vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &renderCompleted[i]);

		VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceInfo, allocationCallbacks, &commandBufferExecuted[i]);
	}

	U8* memory;
	Memory::AllocateSize(&memory, sizeof(GPUTimestampManager) + sizeof(CommandBuffer*) * 128);

	timestampManager = (GPUTimestampManager*)(memory);
	timestampManager->Create(timeQueriesPerFrame, MAX_SWAPCHAIN_IMAGES);

	commandBufferRing.Create();

	queuedCommandBuffers = (CommandBuffer**)(timestampManager + 1);
	CommandBuffer** correctlyAllocatedBuffer = (CommandBuffer**)(memory + sizeof(GPUTimestampManager));
	numAllocatedCommandBuffers = 0;
	numQueuedCommandBuffers = 0;

	if (queuedCommandBuffers != correctlyAllocatedBuffer)
	{
		Logger::Fatal("Wrong calculations for queued command buffers arrays. Should be {}, but it is {}!", correctlyAllocatedBuffer, queuedCommandBuffers);
		return false;
	}

	imageIndex = 0;
	currentFrame = 1;
	previousFrame = 0;
	absoluteFrame = 0;
	timestampsEnabled = false;

	resourceDeletionQueue.Reserve(16);
	descriptorSetUpdates.Reserve(16);

	return true;
}

bool Renderer::CreatePrimitiveResources()
{
	SamplerCreation sc{};
	sc.SetAddressModeUVW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
		.SetMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR).SetName("Sampler Default");
	defaultSampler = CreateSampler(sc);

	BufferCreation fullscreenVbCreation{ VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, 0, nullptr, "Fullscreen_vb" };
	fullscreenVertexBuffer = CreateBuffer(fullscreenVbCreation);

	TextureCreation depthTextureCreation{ nullptr, swapchainWidth, swapchainHeight, 1, 1, 0, VK_FORMAT_D32_SFLOAT, TEXTURE_TYPE_2D, "DepthImage_Texture" };
	depthTexture = CreateTexture(depthTextureCreation);

	swapchainOutput.Depth(VK_FORMAT_D32_SFLOAT);

	RenderPassCreation swapchainPassCreation{};
	swapchainPassCreation.SetType(RENDER_PASS_TYPE_SWAPCHAIN).SetName("Swapchain");
	swapchainPassCreation.SetOperations(RENDER_PASS_OP_CLEAR, RENDER_PASS_OP_CLEAR, RENDER_PASS_OP_CLEAR);
	swapchainPass = CreateRenderPass(swapchainPassCreation);

	TextureCreation dummyTextureCreation = { nullptr, 1, 1, 1, 1, 0, VK_FORMAT_R8_UINT, TEXTURE_TYPE_2D };
	dummyTexture = CreateTexture(dummyTextureCreation);

	BufferCreation dummyConstantBufferCreation = { VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, 16, nullptr, "Dummy_cb" };
	dummyConstantBuffer = CreateBuffer(dummyConstantBufferCreation);

#if defined(_MSC_VER)
	ExpandEnvironmentStringsA("%VULKAN_SDK%", binariesPath, 512);
	Copy(binariesPath + Length(binariesPath), "%s\\Bin\\", 7);
#else
	String vulkanEnv(getenv("VULKAN_SDK"));
	vulkanEnv.Append("%s/bin/");
	strcpy(binariesPath, vulkanEnv.Data());
#endif

	// TODO: Dynamic buffer handling
	dynamicPerFrameSize = 1024 * 1024 * 10;
	BufferCreation bc;
	bc.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, dynamicPerFrameSize * MAX_SWAPCHAIN_IMAGES).
		SetName("Dynamic_Persistent_Buffer");
	dynamicBuffer = CreateBuffer(bc);

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
	const U32 usedSize = dynamicAllocatedSize - (dynamicPerFrameSize * previousFrame);
	dynamicMaxPerFrameSize = Math::Max(usedSize, dynamicMaxPerFrameSize);
	dynamicAllocatedSize = dynamicPerFrameSize * currentFrame;

	// Descriptor Set Updates
	while (descriptorSetUpdates.Size())
	{
		DescriptorSetUpdate update;
		descriptorSetUpdates.Pop(update);

		UpdateDescriptorSetInstant(update);
	}
}

void Renderer::EndFrame()
{
	VkResult result = vkAcquireNextImageKHR(device, swapchain, U64_MAX, imageAcquired, nullptr, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ResizeSwapchain();
		FrameCountersAdvance();

		return;
	}

	VkFence renderCompleteFence = commandBufferExecuted[currentFrame];
	VkSemaphore renderCompleteSemaphore = renderCompleted[currentFrame];

	// Copy all commands
	VkCommandBuffer enqueuedCommandBuffers[4];
	for (U32 c = 0; c < numQueuedCommandBuffers; ++c)
	{
		CommandBuffer* commandBuffer = queuedCommandBuffers[c];

		enqueuedCommandBuffers[c] = commandBuffer->commandBuffer;

		if (commandBuffer->isRecording && commandBuffer->currentRenderPass && (commandBuffer->currentRenderPass->type != RENDER_PASS_TYPE_COMPUTE))
		{
			vkCmdEndRenderPass(commandBuffer->commandBuffer);
		}

		vkEndCommandBuffer(commandBuffer->commandBuffer);
	}

	// Submit command buffers
	VkSemaphore waitSemaphores[] = { imageAcquired };
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = numQueuedCommandBuffers;
	submitInfo.pCommandBuffers = enqueuedCommandBuffers;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = renderCompleted;

	vkQueueSubmit(deviceQueue, 1, &submitInfo, renderCompleteFence);

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = renderCompleted;

	VkSwapchainKHR swap_chains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swap_chains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	result = vkQueuePresentKHR(deviceQueue, &presentInfo);

	numQueuedCommandBuffers = 0;

	// GPU Timestamp resolve
	if (timestampsEnabled)
	{
		if (timestampManager->HasValidQueries())
		{
			// Query GPU for all timestamps.
			const U32 queryOffset = (currentFrame * timestampManager->queriesPerFrame) * 2;
			const U32 queryCount = timestampManager->currentQuery * 2;
			vkGetQueryPoolResults(device, timestampQueryPool, queryOffset, queryCount,
				sizeof(U64) * queryCount * 2, &timestampManager->timestampsData[queryOffset],
				sizeof(timestampManager->timestampsData[0]), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

			// Calculate and cache the elapsed time
			for (U32 i = 0; i < timestampManager->currentQuery; ++i)
			{
				U32 index = (currentFrame * timestampManager->queriesPerFrame) + i;

				GPUTimestamp& timestamp = timestampManager->timestamps[index];

				F64 start = (F64)timestampManager->timestampsData[(index * 2)];
				F64 end = (F64)timestampManager->timestampsData[(index * 2) + 1];
				F64 range = end - start;
				F64 elapsedTime = range * timestampFrequency;

				timestamp.elapsedMs = elapsedTime;
				timestamp.frameIndex = absoluteFrame;
			}
		}
		else if (timestampManager->currentQuery)
		{
			Logger::Error("Asymmetrical GPU queries, missing pop of some markers!\n");
		}

		timestampManager->Reset();
		timestampReset = true;
	}
	else
	{
		timestampReset = false;
	}


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized)
	{
		resized = false;
		ResizeSwapchain();
		FrameCountersAdvance();

		return;
	}

	FrameCountersAdvance();

	while (resourceDeletionQueue.Size())
	{
		ResourceUpdate resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == currentFrame)
		{
			switch (resourceDeletion.type)
			{
			case RESOURCE_DELETE_TYPE_BUFFER: { DestroyBufferInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_PIPELINE: { DestroyPipelineInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_RENDER_PASS: { DestroyRenderPassInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET: { DestroyDescriptorSetInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT: { DestroyDescriptorSetLayoutInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_SAMPLER: { DestroySamplerInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_SHADER_STATE: { DestroyShaderStateInstant(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_TEXTURE: { DestroyTextureInstant(resourceDeletion.handle); } break;
			}
		}
	}
}

void Renderer::Resize()
{
	swapchainWidth = Settings::WindowWidth();
	swapchainHeight = Settings::WindowHeight();

	resized = true;
}

void Renderer::ResizeSwapchain()
{
	vkDeviceWaitIdle(device);

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	VkExtent2D swapchainExtent = surfaceCapabilities.currentExtent;

	if (swapchainExtent.width == 0 || swapchainExtent.height == 0) { return; }

	// Internal destroy of swapchain pass to retain the same handle.
	RenderPass* vkSwapchainPass = AccessRenderPass(swapchainPass);
	vkDestroyRenderPass(device, vkSwapchainPass->renderPass, allocationCallbacks);

	// Destroy swapchain images and framebuffers
	DestroySwapchain();
	vkDestroySurfaceKHR(instance, surface, allocationCallbacks);

	// Recreate window surface
	if (!CreateSurface()) { Logger::Error("Failed to create window surface!"); }

	// Create swapchain
	CreateSwapchain();

	// Resize depth texture, maintaining handle, using a dummy texture to destroy.
	TextureHandle textureToDelete = { textures.ObtainResource() };
	Texture* vkTextureToDelete = AccessTexture(textureToDelete);
	vkTextureToDelete->handle = textureToDelete;
	Texture* vkDepthTexture = AccessTexture(depthTexture);
	ResizeTexture(vkDepthTexture, vkTextureToDelete, swapchainWidth, swapchainHeight, 1);

	DestroyTexture(textureToDelete);

	RenderPassCreation swapchainPassCreation{};
	swapchainPassCreation.SetType(RENDER_PASS_TYPE_SWAPCHAIN).SetName("Swapchain");
	CreateSwapchainPass(swapchainPassCreation, vkSwapchainPass);

	vkDeviceWaitIdle(device);
}

void Renderer::ResizeTexture(Texture* texture, Texture* textureToDelete, U16 width, U16 height, U16 depth)
{

	// Cache handles to be delayed destroyed
	textureToDelete->imageView = texture->imageView;
	textureToDelete->image = texture->image;
	textureToDelete->allocation = texture->allocation;

	// Re-create image in place.
	TextureCreation tc;
	tc.SetFlags(texture->mipmaps, texture->flags).SetFormatType(texture->format, texture->type).SetName(texture->name).SetSize(width, height, depth);
	CreateTexture(tc, texture->handle, texture);
}

void Renderer::DestroySwapchain()
{
	for (size_t i = 0; i < swapchainImageCount; ++i)
	{
		vkDestroyImageView(device, swapchainImageViews[i], allocationCallbacks);
		vkDestroyFramebuffer(device, swapchainFramebuffers[i], allocationCallbacks);
	}

	vkDestroySwapchainKHR(device, swapchain, allocationCallbacks);
}

void Renderer::UpdateDescriptorSet(DescriptorSetHandle descriptorSet)
{
	if (descriptorSet.index < descriptorSets.ResourceCount) { descriptorSetUpdates.Push({ descriptorSet, currentFrame }); }
	else { Logger::Error("Graphics error: trying to update invalid DescriptorSet {}", descriptorSet.index); }
}

void Renderer::UpdateDescriptorSetInstant(const DescriptorSetUpdate& update)
{
	// Use a dummy descriptor set to delete the vulkan descriptor set handle
	DescriptorSetHandle dummyDeleteDescriptorSetHandle = { descriptorSets.ObtainResource() };
	DesciptorSet* dummyDeleteDescriptorSet = AccessDescriptorSet(dummyDeleteDescriptorSetHandle);

	DesciptorSet* descriptorSet = AccessDescriptorSet(update.descriptorSet);
	const DesciptorSetLayout* descriptorSetLayout = descriptorSet->layout;

	dummyDeleteDescriptorSet->descriptorSet = descriptorSet->descriptorSet;
	dummyDeleteDescriptorSet->bindings = nullptr;
	dummyDeleteDescriptorSet->resources = nullptr;
	dummyDeleteDescriptorSet->samplers = nullptr;
	dummyDeleteDescriptorSet->numResources = 0;

	DestroyDescriptorSet(dummyDeleteDescriptorSetHandle);

	// Allocate the new descriptor set and update its content.
	VkWriteDescriptorSet descriptorWrite[8];
	VkDescriptorBufferInfo bufferInfo[8];
	VkDescriptorImageInfo imageInfo[8];

	Sampler* vkDefaultSampler = AccessSampler(defaultSampler);

	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSet->layout->descriptorSetLayout;
	vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet->descriptorSet);

	U32 numResources = descriptorSetLayout->numBindings;
	FillWriteDescriptorSets(descriptorSetLayout, descriptorSet->descriptorSet, descriptorWrite, bufferInfo, imageInfo, vkDefaultSampler->sampler,
		numResources, descriptorSet->resources, descriptorSet->samplers, descriptorSet->bindings);

	vkUpdateDescriptorSets(device, numResources, descriptorWrite, 0, nullptr);
}

//void Renderer::ResizeOutputTextures(RenderPassHandle renderPass, U32 width, U32 height)






void* Renderer::MapBuffer(const MapBufferParameters& parameters)
{
	if (parameters.buffer.index == INVALID_INDEX)
		return nullptr;

	Buffer* buffer = AccessBuffer(parameters.buffer);

	if (buffer->parentBuffer.index == dynamicBuffer.index)
	{

		buffer->globalOffset = dynamicAllocatedSize;

		return DynamicAllocate(parameters.size == 0 ? buffer->size : parameters.size);
	}

	void* data;
	vmaMapMemory(allocator, buffer->allocation, &data);

	return data;
}

void Renderer::UnmapBuffer(const MapBufferParameters& parameters)
{
	if (parameters.buffer.index == INVALID_INDEX)
		return;

	Buffer* buffer = AccessBuffer(parameters.buffer);
	if (buffer->parentBuffer.index == dynamicBuffer.index)
		return;

	vmaUnmapMemory(allocator, buffer->allocation);
}

void* Renderer::DynamicAllocate(U32 size)
{
	void* mappedMemory = dynamicMappedMemory + dynamicAllocatedSize;
	dynamicAllocatedSize += (U32)Memory::MemoryAlign(size, uboAlignment);
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

void Renderer::PushGpuTimestamp(CommandBuffer* commandBuffer, CSTR name)
{
	if (!timestampsEnabled) { return; }

	U32 queryIndex = timestampManager->Push(currentFrame, name);
	vkCmdWriteTimestamp(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, queryIndex);
}

void Renderer::PopGpuTimestamp(CommandBuffer* commandBuffer)
{
	if (!timestampsEnabled) { return; }

	U32 queryIndex = timestampManager->Pop(currentFrame);
	vkCmdWriteTimestamp(commandBuffer->commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, timestampQueryPool, queryIndex);
}

void Renderer::FrameCountersAdvance()
{
	previousFrame = currentFrame;
	currentFrame = (currentFrame + 1) % swapchainImageCount;

	++absoluteFrame;
}

void Renderer::QueueCommandBuffer(CommandBuffer* commandBuffer)
{
	queuedCommandBuffers[numQueuedCommandBuffers++] = commandBuffer;
}

CommandBuffer* Renderer::GetCommandBuffer(QueueType type, bool begin)
{
	CommandBuffer* cb = commandBufferRing.GetCommandBuffer(currentFrame, begin);

	// The first commandbuffer issued in the frame is used to reset the timestamp queries used.
	if (timestampReset && begin)
	{
		// These are currently indices!
		vkCmdResetQueryPool(cb->commandBuffer, timestampQueryPool, currentFrame * timestampManager->queriesPerFrame * 2, timestampManager->queriesPerFrame);

		timestampReset = false;
	}

	return cb;
}

CommandBuffer* Renderer::GetInstantCommandBuffer()
{
	CommandBuffer* cb = commandBufferRing.GetCommandBufferInstant(currentFrame, false);
	return cb;
}

void Renderer::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		//Logger::Error("Unsupported layout transition: {} -> {}!", (U32)oldLayout, (U32)newLayout);
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Renderer::FillWriteDescriptorSets(const DesciptorSetLayout* descriptorSetLayout, VkDescriptorSet vkDescriptorSet,
	VkWriteDescriptorSet* descriptorWrite, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo,
	VkSampler vkDefaultSampler, U32& numResources, const ResourceHandle* resources, const SamplerHandle* samplers, const U16* bindings)
{
	U32 usedResources = 0;
	for (U32 r = 0; r < numResources; ++r)
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
		const U32 binding_point = binding.start;
		descriptorWrite[i].dstBinding = binding_point;
		descriptorWrite[i].dstArrayElement = 0;
		descriptorWrite[i].descriptorCount = 1;

		switch (binding.type)
		{
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		{
			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			TextureHandle texture_handle = { resources[r] };
			Texture* textureData = AccessTexture(texture_handle);

			// Find proper sampler.
			// TODO: improve. Remove the single texture interface ?
			imageInfo[i].sampler = vkDefaultSampler;
			if (textureData->sampler)
			{
				imageInfo[i].sampler = textureData->sampler->sampler;
			}
			// TODO: else ?
			if (samplers[r].index != INVALID_INDEX)
			{
				Sampler* sampler = AccessSampler({ samplers[r] });
				imageInfo[i].sampler = sampler->sampler;
			}

			imageInfo[i].imageLayout = HasDepthOrStencil(textureData->format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[i].imageView = textureData->imageView;

			descriptorWrite[i].pImageInfo = &imageInfo[i];

			break;
		}

		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		{
			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

			TextureHandle texture_handle = { resources[r] };
			Texture* textureData = AccessTexture(texture_handle);

			imageInfo[i].sampler = nullptr;
			imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo[i].imageView = textureData->imageView;

			descriptorWrite[i].pImageInfo = &imageInfo[i];

			break;
		}

		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		{
			BufferHandle buffer_handle = { resources[r] };
			Buffer* buffer = AccessBuffer(buffer_handle);

			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite[i].descriptorType = buffer->usage == RESOURCE_USAGE_DYNAMIC ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

			// Bind parent buffer if present, used for dynamic resources.
			if (buffer->parentBuffer.index != INVALID_INDEX)
			{
				Buffer* parentBuffer = AccessBuffer(buffer->parentBuffer);

				bufferInfo[i].buffer = parentBuffer->buffer;
			}
			else
			{
				bufferInfo[i].buffer = buffer->buffer;
			}

			bufferInfo[i].offset = 0;
			bufferInfo[i].range = buffer->size;

			descriptorWrite[i].pBufferInfo = &bufferInfo[i];

			break;
		}

		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			BufferHandle buffer_handle = { resources[r] };
			Buffer* buffer = AccessBuffer(buffer_handle);

			descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			// Bind parent buffer if present, used for dynamic resources.
			if (buffer->parentBuffer.index != INVALID_INDEX)
			{
				Buffer* parentBuffer = AccessBuffer(buffer->parentBuffer);

				bufferInfo[i].buffer = parentBuffer->buffer;
			}
			else
			{
				bufferInfo[i].buffer = buffer->buffer;
			}

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

	numResources = usedResources;
}

//TODO: Cache compiled shaders
VkShaderModuleCreateInfo Renderer::CompileShader(CSTR code, U32 codeSize, VkShaderStageFlagBits stage, CSTR name)
{
	VkShaderModuleCreateInfo shaderCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

	// Compile from glsl to SpirV.
	// TODO: detect if input is HLSL.
	const char* tempFilename = "temp.shader";

	// Write current shader to file.
	File tempShaderFile(tempFilename, FILE_OPEN_TEMP_RESOURCE);
	tempShaderFile.Write(code, codeSize);

	// Add uppercase define as STAGE_NAME
	String stageDefine(ToStageDefines(stage), "_", name);
	stageDefine.ToUpper();

	// Compile to SPV
	String glslCompilerPath(binariesPath, "glslangValidator.exe");
	String finalSpirvFilename("shader_final.spv");

#if defined(_MSC_VER)
	// TODO: add optional debug information in shaders (option -g).
	String arguments("glslangValidator.exe {} -V --target-env vulkan1.3 -o {} -S {} --D {} --D {}", tempFilename, finalSpirvFilename, ToCompilerExtension(stage), stageDefine, ToStageDefines(stage));
#else
	String arguments("{} -V --target-env vulkan1.2 -o {} -S {} --D {} --D {}", tempFilename, finalSpirvFilename, ToCompilerExtension(stage), stageDefine, ToStageDefines(stage));
#endif

	Platform::ExecuteProcess(".", glslCompilerPath, arguments, "");

	bool optimizeShaders = false;

	if (optimizeShaders)
	{
		// TODO: add optional optimization stage
		//"spirv-opt -O input -o output
		String spirvOptimizerPath(binariesPath, "spirv-opt.exe");
		String optimizedSpirvFilename("shader_opt.spv");
		String spirvOptArguments("spirv-opt.exe -O --preserve-bindings {} -o {}", finalSpirvFilename, optimizedSpirvFilename);

		Platform::ExecuteProcess(".", spirvOptimizerPath, spirvOptArguments, "");

		// Read back SPV file.
		File optimizedSpirvFile(optimizedSpirvFilename, FILE_OPEN_RESOURCE);
		optimizedSpirvFile.ReadAll(&shaderCreateInfo.pCode);

		File::Delete(optimizedSpirvFilename);
	}
	else
	{
		// Read back SPV file.
		File optimizedSpirvFile(finalSpirvFilename, FILE_OPEN_RESOURCE);
		optimizedSpirvFile.ReadAll(&shaderCreateInfo.pCode);

		File::Delete(finalSpirvFilename);
	}

	// Handling compilation error
	if (shaderCreateInfo.pCode == nullptr)
	{
		//TODO: Cleanup code str
	}

	// Temporary files cleanup
	tempShaderFile.Close();

	return shaderCreateInfo;
}

VkRenderPass Renderer::CreateVulkanRenderPass(const RenderPassOutput& output, CSTR name)
{
	VkAttachmentDescription colorAttachments[8] = {};
	VkAttachmentReference colorAttachmentsRef[8] = {};

	VkAttachmentLoadOp colorOp, depthOp, stencilOp;
	VkImageLayout colorInitial, depthInitial;

	switch (output.colorOperation)
	{
	case RENDER_PASS_OP_LOAD: {
		colorOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} break;
	case RENDER_PASS_OP_CLEAR: {
		colorOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} break;
	default: {
		colorOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
	} break;
	}

	switch (output.depthOperation)
	{
	case RENDER_PASS_OP_LOAD: {
		depthOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} break;
	case RENDER_PASS_OP_CLEAR: {
		depthOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	} break;
	default: {
		depthOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
	} break;
	}

	switch (output.stencilOperation)
	{
	case RENDER_PASS_OP_LOAD: {
		stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	} break;
	case RENDER_PASS_OP_CLEAR: {
		stencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	} break;
	default: {
		stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	} break;
	}

	// Color attachments
	U32 c = 0;
	for (; c < output.numColorFormats; ++c)
	{
		VkAttachmentDescription& colorAttachment = colorAttachments[c];
		colorAttachment.format = output.colorFormats[c];
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = colorOp;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = stencilOp;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = colorInitial;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference& colorAttachmentRef = colorAttachmentsRef[c];
		colorAttachmentRef.attachment = c;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	// Depth attachment
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

		depthAttachmentRef.attachment = c;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Create subpass.
	// TODO: for now is just a simple subpass, evolve API.
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// Calculate active attachments for the subpass
	VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1]{};
	U32 activeAttachments = 0;
	for (; activeAttachments < output.numColorFormats; ++activeAttachments)
	{
		attachments[activeAttachments] = colorAttachments[activeAttachments];
		++activeAttachments;
	}
	subpass.colorAttachmentCount = activeAttachments ? activeAttachments - 1 : 0;
	subpass.pColorAttachments = colorAttachmentsRef;

	subpass.pDepthStencilAttachment = nullptr;

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

	// Create external subpass dependencies
	//VkSubpassDependency external_dependencies[ 16 ];
	//u32 num_external_dependencies = 0;

	VkRenderPass renderPass;
	VkValidate(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (U64)renderPass, name);

	return renderPass;
}

VkRenderPass Renderer::GetRenderPass(const RenderPassOutput& output, CSTR name)
{
	U64 hashedMemory = Hash(output);
	VkRenderPass renderPass;
	renderPassCache.Get(hashedMemory, renderPass);
	if (renderPass)
	{
		return renderPass;
	}
	renderPass = CreateVulkanRenderPass(output, name);
	renderPassCache.Insert(hashedMemory, renderPass);

	return renderPass;
}

void Renderer::CreateSwapchainPass(const RenderPassCreation& creation, RenderPass* renderPass)
{
	// Color attachment
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment
	VkAttachmentDescription depthAttachment{};
	Texture* depthTextureVk = AccessTexture(depthTexture);
	depthAttachment.format = depthTextureVk->format;
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

	VkValidate(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass->renderPass));

	SetResourceName(VK_OBJECT_TYPE_RENDER_PASS, (U64)renderPass->renderPass, creation.name);

	// Create framebuffer into the device.
	VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = renderPass->renderPass;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.width = swapchainWidth;
	framebufferInfo.height = swapchainHeight;
	framebufferInfo.layers = 1;

	VkImageView framebufferAttachments[2];
	framebufferAttachments[1] = depthTextureVk->imageView;

	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		framebufferAttachments[0] = swapchainImageViews[i];
		framebufferInfo.pAttachments = framebufferAttachments;

		vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]);
		SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)swapchainFramebuffers[i], creation.name);
	}

	renderPass->width = swapchainWidth;
	renderPass->height = swapchainHeight;

	// Manually transition the texture
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	CommandBuffer* commandBuffer = GetInstantCommandBuffer();
	vkBeginCommandBuffer(commandBuffer->commandBuffer, &beginInfo);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { swapchainWidth, swapchainHeight, 1 };

	// Transition
	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		TransitionImageLayout(commandBuffer->commandBuffer, swapchainImages[i], surfaceFormat.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, false);
	}

	vkEndCommandBuffer(commandBuffer->commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

	vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(deviceQueue);
}

void Renderer::CreateFramebuffer(RenderPass* renderPass, const TextureHandle* outputTextures, U32 numRenderTargets, TextureHandle depthStencilTexture)
{
	// Create framebuffer
	VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferInfo.renderPass = renderPass->renderPass;
	framebufferInfo.width = renderPass->width;
	framebufferInfo.height = renderPass->height;
	framebufferInfo.layers = 1;

	VkImageView framebufferAttachments[MAX_IMAGE_OUTPUTS + 1]{};
	U32 activeAttachments = 0;
	for (; activeAttachments < numRenderTargets; ++activeAttachments)
	{
		Texture* textureVk = AccessTexture(outputTextures[activeAttachments]);
		framebufferAttachments[activeAttachments] = textureVk->imageView;
	}

	if (depthStencilTexture.index != INVALID_INDEX)
	{
		Texture* depthTextureVk = AccessTexture(depthStencilTexture);
		framebufferAttachments[activeAttachments++] = depthTextureVk->imageView;
	}
	framebufferInfo.pAttachments = framebufferAttachments;
	framebufferInfo.attachmentCount = activeAttachments;

	vkCreateFramebuffer(device, &framebufferInfo, nullptr, &renderPass->frameBuffer);
	SetResourceName(VK_OBJECT_TYPE_FRAMEBUFFER, (U64)renderPass->frameBuffer, renderPass->name);
}

RenderPassOutput Renderer::FillRenderPassOutput(const RenderPassCreation& creation)
{
	RenderPassOutput output;
	output.Reset();

	for (U32 i = 0; i < creation.numRenderTargets; ++i)
	{
		Texture* texture_vk = AccessTexture(creation.outputTextures[i]);
		output.Color(texture_vk->format);
	}
	if (creation.depthStencilTexture.index != INVALID_INDEX)
	{
		Texture* texture_vk = AccessTexture(creation.depthStencilTexture);
		output.Depth(texture_vk->format);
	}

	output.colorOperation = creation.colorOperation;
	output.depthOperation = creation.depthOperation;
	output.stencilOperation = creation.stencilOperation;

	return output;
}

void Renderer::CreateTexture(const TextureCreation& creation, TextureHandle handle, Texture* texture)
{
	texture->width = creation.width;
	texture->height = creation.height;
	texture->depth = creation.depth;
	texture->mipmaps = creation.mipmaps;
	texture->type = creation.type;
	texture->name = creation.name;
	texture->format = creation.format;
	texture->sampler = nullptr;
	texture->flags = creation.flags;
	texture->handle = handle;

	//// Create the image
	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.format = texture->format;
	imageInfo.flags = 0;
	imageInfo.imageType = ToVkImageType(creation.type);
	imageInfo.extent.width = creation.width;
	imageInfo.extent.height = creation.height;
	imageInfo.extent.depth = creation.depth;
	imageInfo.mipLevels = creation.mipmaps;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	const bool is_render_target = (creation.flags & TEXTURE_FLAG_RENDER_TARGET_MASK) == TEXTURE_FLAG_RENDER_TARGET_MASK;
	const bool is_compute_used = (creation.flags & TEXTURE_FLAG_COMPUTE_MASK) == TEXTURE_FLAG_COMPUTE_MASK;

	// Default to always readable from shader.
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	imageInfo.usage |= is_compute_used ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

	if (HasDepthOrStencil(creation.format))
	{
		// Depth/Stencil textures are normally textures you render into.
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO
		imageInfo.usage |= is_render_target ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
	}

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo memory_info{};
	memory_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkValidate(vmaCreateImage(allocator, &imageInfo, &memory_info,
		&texture->image, &texture->allocation, nullptr));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, creation.name);

	//// Create the image view
	VkImageViewCreateInfo info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = texture->image;
	info.viewType = ToVkImageViewType(creation.type);
	info.format = imageInfo.format;

	if (HasDepthOrStencil(creation.format))
	{

		info.subresourceRange.aspectMask = HasDepth(creation.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		// TODO:gs
		//info.subresourceRange.aspectMask |= HasStencil( creation.format ) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
	}
	else
	{
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	info.subresourceRange.levelCount = 1;
	info.subresourceRange.layerCount = 1;
	VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->imageView));

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, creation.name);

	texture->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

BufferHandle Renderer::CreateBuffer(const BufferCreation& creation)
{
	U32 resourceIndex = buffers.ObtainResource();
	BufferHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	Buffer* buffer = AccessBuffer(handle);

	buffer->name = creation.name;
	buffer->size = creation.size;
	buffer->typeFlags = creation.typeFlags;
	buffer->usage = creation.usage;
	buffer->handle = handle;
	buffer->globalOffset = 0;
	buffer->parentBuffer = INVALID_BUFFER;

	static const VkBufferUsageFlags dynamicBufferMask = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	const bool useGlobalBuffer = (creation.typeFlags & dynamicBufferMask) != 0;
	if (creation.usage == RESOURCE_USAGE_DYNAMIC && useGlobalBuffer)
	{
		buffer->parentBuffer = dynamicBuffer;
		return handle;
	}

	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | creation.typeFlags;
	bufferInfo.size = creation.size > 0 ? creation.size : 1;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VmaAllocationInfo allocationInfo{};
	VkValidate(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo, &buffer->buffer, &buffer->allocation, &allocationInfo));

	SetResourceName(VK_OBJECT_TYPE_BUFFER, (U64)buffer->buffer, creation.name);

	buffer->deviceMemory = allocationInfo.deviceMemory;

	if (creation.initialData)
	{
		void* data;
		vmaMapMemory(allocator, buffer->allocation, &data);
		memcpy(data, creation.initialData, (U64)creation.size);
		vmaUnmapMemory(allocator, buffer->allocation);
	}

	// TODO
	//if ( persistent )
	//{
	//    mapped_data = static_cast<uint8_t *>(allocationInfo.pMappedData);
	//}

	return handle;
}

TextureHandle Renderer::CreateTexture(const TextureCreation& creation)
{
	U32 resourceIndex = textures.ObtainResource();
	TextureHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	Texture* texture = AccessTexture(handle);

	CreateTexture(creation, handle, texture);

	if (creation.initialData)
	{
		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		U32 imageSize = creation.width * creation.height * 4;
		bufferInfo.size = imageSize;

		VmaAllocationCreateInfo memoryInfo{};
		memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
		memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocationInfo allocationInfo{};
		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;
		VkValidate(vmaCreateBuffer(allocator, &bufferInfo, &memoryInfo,
			&stagingBuffer, &stagingAllocation, &allocationInfo));

		// Copy buffer_data
		void* destinationData;
		vmaMapMemory(allocator, stagingAllocation, &destinationData);
		memcpy(destinationData, creation.initialData, static_cast<size_t>(imageSize));
		vmaUnmapMemory(allocator, stagingAllocation);

		// Execute command buffer
		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CommandBuffer* commandBuffer = GetInstantCommandBuffer();
		vkBeginCommandBuffer(commandBuffer->commandBuffer, &beginInfo);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { creation.width, creation.height, creation.depth };

		// Transition
		TransitionImageLayout(commandBuffer->commandBuffer, texture->image, texture->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);
		// Copy
		vkCmdCopyBufferToImage(commandBuffer->commandBuffer, stagingBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		// Transition
		TransitionImageLayout(commandBuffer->commandBuffer, texture->image, texture->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);

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

	return handle;
}

PipelineHandle Renderer::CreatePipeline(const PipelineCreation& creation)
{
	U32 resourceIndex = pipelines.ObtainResource();
	PipelineHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	ShaderStateHandle shaderState = CreateShaderState(creation.shaders);
	if (shaderState.index == INVALID_INDEX)
	{
		// Shader did not compile.
		pipelines.ReleaseResource(handle.index);
		handle.index = INVALID_INDEX;

		return handle;
	}

	// Now that shaders have compiled we can create the pipeline.
	Pipeline* pipeline = AccessPipeline(handle);
	ShaderState* shaderStateData = AccessShaderState(shaderState);

	pipeline->shaderState = shaderState;

	VkDescriptorSetLayout vkLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];

	// Create VkPipelineLayout
	for (U32 l = 0; l < creation.numActiveLayouts; ++l)
	{
		pipeline->descriptorSetLayouts[l] = AccessDescriptorSetLayout(creation.descriptorSetLayouts[l]);
		pipeline->descriptorSetLayoutHandles[l] = creation.descriptorSetLayouts[l];

		vkLayouts[l] = pipeline->descriptorSetLayouts[l]->descriptorSetLayout;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.pSetLayouts = vkLayouts;
	pipelineLayoutInfo.setLayoutCount = creation.numActiveLayouts;

	VkPipelineLayout pipelineLayout;
	VkValidate(vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocationCallbacks, &pipelineLayout));
	// Cache pipeline layout
	pipeline->pipelineLayout = pipelineLayout;
	pipeline->numActiveLayouts = creation.numActiveLayouts;

	// Create full pipeline
	if (shaderStateData->graphicsPipeline)
	{
		VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

		//// Shader stage
		pipelineInfo.pStages = shaderStateData->shaderStageInfos;
		pipelineInfo.stageCount = shaderStateData->activeShaders;
		//// PipelineLayout
		pipelineInfo.layout = pipelineLayout;

		//// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		// Vertex attributes.
		VkVertexInputAttributeDescription vertexAttributes[8];
		if (creation.vertexInput.numVertexAttributes)
		{
			for (U32 i = 0; i < creation.vertexInput.numVertexAttributes; ++i)
			{
				const VertexAttribute& vertexAttribute = creation.vertexInput.vertexAttributes[i];
				vertexAttributes[i] = { vertexAttribute.location, vertexAttribute.binding, ToVkVertexFormat(vertexAttribute.format), vertexAttribute.offset };
			}

			vertexInputInfo.vertexAttributeDescriptionCount = creation.vertexInput.numVertexAttributes;
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes;
		}
		else
		{
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		}
		// Vertex bindings
		VkVertexInputBindingDescription vertexBindings[8];
		if (creation.vertexInput.numVertexStreams)
		{
			vertexInputInfo.vertexBindingDescriptionCount = creation.vertexInput.numVertexStreams;

			for (U32 i = 0; i < creation.vertexInput.numVertexStreams; ++i)
			{
				const VertexStream& vertexStream = creation.vertexInput.vertexStreams[i];
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

		//// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		pipelineInfo.pInputAssemblyState = &inputAssembly;

		//// Color Blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment[8];

		if (creation.blendState.activeStates)
		{
			for (size_t i = 0; i < creation.blendState.activeStates; i++)
			{
				const BlendState& blendState = creation.blendState.blendStates[i];

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
			// Default non blended state
			colorBlendAttachment[0] = {};
			colorBlendAttachment[0].blendEnable = VK_FALSE;
			colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = creation.blendState.activeStates ? creation.blendState.activeStates : 1; // Always have 1 blend defined.
		colorBlending.pAttachments = colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		pipelineInfo.pColorBlendState = &colorBlending;

		//// Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

		depthStencil.depthWriteEnable = creation.depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencil.stencilTestEnable = creation.depthStencil.stencilEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthTestEnable = creation.depthStencil.depthEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = creation.depthStencil.depthComparison;
		if (creation.depthStencil.stencilEnable)
		{
			// TODO: add stencil
			Logger::Error("");
		}

		pipelineInfo.pDepthStencilState = &depthStencil;

		//// Multisample
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		pipelineInfo.pMultisampleState = &multisampling;

		//// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = creation.rasterization.cullMode;
		rasterizer.frontFace = creation.rasterization.front;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		pipelineInfo.pRasterizationState = &rasterizer;

		//// Tessellation
		pipelineInfo.pTessellationState;

		//// Viewport state
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (F32)swapchainWidth;
		viewport.height = (F32)swapchainHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { swapchainWidth, swapchainHeight };

		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		pipelineInfo.pViewportState = &viewportState;

		//// Render Pass
		pipelineInfo.renderPass = GetRenderPass(creation.renderPass, creation.name);

		//// Dynamic states
		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = CountOf32(dynamicStates);
		dynamicState.pDynamicStates = dynamicStates;

		pipelineInfo.pDynamicState = &dynamicState;

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, allocationCallbacks, &pipeline->pipeline);

		pipeline->bindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
	else
	{
		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

		pipelineInfo.stage = shaderStateData->shaderStageInfos[0];
		pipelineInfo.layout = pipelineLayout;

		vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, allocationCallbacks, &pipeline->pipeline);

		pipeline->bindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
	}

	return handle;
}

SamplerHandle Renderer::CreateSampler(const SamplerCreation& creation)
{
	U32 resourceIndex = samplers.ObtainResource();
	SamplerHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	Sampler* sampler = AccessSampler(handle);

	sampler->addressModeU = creation.addressModeU;
	sampler->addressModeV = creation.addressModeV;
	sampler->addressModeW = creation.addressModeW;
	sampler->minFilter = creation.minFilter;
	sampler->magFilter = creation.magFilter;
	sampler->mipFilter = creation.mipFilter;
	sampler->name = creation.name;

	VkSamplerCreateInfo create_info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	create_info.addressModeU = creation.addressModeU;
	create_info.addressModeV = creation.addressModeV;
	create_info.addressModeW = creation.addressModeW;
	create_info.minFilter = creation.minFilter;
	create_info.magFilter = creation.magFilter;
	create_info.mipmapMode = creation.mipFilter;
	create_info.anisotropyEnable = 0;
	create_info.compareEnable = 0;
	create_info.unnormalizedCoordinates = 0;
	create_info.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	// TODO:
	/*float                   mipLodBias;
	float                   maxAnisotropy;
	VkCompareOp             compareOp;
	float                   minLod;
	float                   maxLod;
	VkBorderColor           borderColor;
	VkBool32                unnormalizedCoordinates;*/

	vkCreateSampler(device, &create_info, allocationCallbacks, &sampler->sampler);

	SetResourceName(VK_OBJECT_TYPE_SAMPLER, (U64)sampler->sampler, creation.name);

	return handle;
}

DescriptorSetLayoutHandle Renderer::CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& creation)
{
	U32 resourceIndex = descriptorSetLayouts.ObtainResource();
	DescriptorSetLayoutHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	DesciptorSetLayout* descriptorSetLayout = AccessDescriptorSetLayout(handle);

	// TODO: add support for multiple sets.
	// Create flattened binding list
	descriptorSetLayout->numBindings = (U16)creation.numBindings;
	U8* memory;
	Memory::AllocateSize(&memory, (sizeof(VkDescriptorSetLayoutBinding) + sizeof(DescriptorBinding)) * creation.numBindings);
	descriptorSetLayout->bindings = (DescriptorBinding*)memory;
	descriptorSetLayout->binding = (VkDescriptorSetLayoutBinding*)(memory + sizeof(DescriptorBinding) * creation.numBindings);
	descriptorSetLayout->handle = handle;
	descriptorSetLayout->setIndex = (U16)creation.setIndex;

	U32 used_bindings = 0;
	for (U32 r = 0; r < creation.numBindings; ++r)
	{
		DescriptorBinding& binding = descriptorSetLayout->bindings[r];
		const DescriptorSetLayoutCreation::Binding& inputBinding = creation.bindings[r];
		binding.start = inputBinding.start == U16_MAX ? (U16)r : inputBinding.start;
		binding.count = 1;
		binding.type = inputBinding.type;
		binding.name = inputBinding.name;

		VkDescriptorSetLayoutBinding& vkBinding = descriptorSetLayout->binding[used_bindings];
		++used_bindings;

		vkBinding.binding = binding.start;
		vkBinding.descriptorType = inputBinding.type;
		vkBinding.descriptorType = vkBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : vkBinding.descriptorType;
		vkBinding.descriptorCount = 1;

		// TODO:
		vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
		vkBinding.pImmutableSamplers = nullptr;
	}

	// Create the descriptor set layout
	VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layout_info.bindingCount = used_bindings;// creation.numBindings;
	layout_info.pBindings = descriptorSetLayout->binding;

	vkCreateDescriptorSetLayout(device, &layout_info, allocationCallbacks, &descriptorSetLayout->descriptorSetLayout);

	return handle;
}

DescriptorSetHandle Renderer::CreateDescriptorSet(const DescriptorSetCreation& creation)
{
	U32 resourceIndex = descriptorSets.ObtainResource();
	DescriptorSetHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	DesciptorSet* descriptorSet = AccessDescriptorSet(handle);
	const DesciptorSetLayout* descriptorSetLayout = AccessDescriptorSetLayout(creation.layout);

	// Allocate descriptor set
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout->descriptorSetLayout;

	VkValidate(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet->descriptorSet));

	// Cache data
	U8* memory;
	Memory::AllocateSize(&memory, (sizeof(ResourceHandle) + sizeof(SamplerHandle) + sizeof(U16)) * creation.numResources);
	descriptorSet->resources = (ResourceHandle*)memory;
	descriptorSet->samplers = (SamplerHandle*)(memory + sizeof(ResourceHandle) * creation.numResources);
	descriptorSet->bindings = (U16*)(memory + (sizeof(ResourceHandle) + sizeof(SamplerHandle)) * creation.numResources);
	descriptorSet->numResources = creation.numResources;
	descriptorSet->layout = descriptorSetLayout;

	// Update descriptor set
	VkWriteDescriptorSet descriptorWrite[8];
	VkDescriptorBufferInfo bufferInfo[8];
	VkDescriptorImageInfo imageInfo[8];

	Sampler* vkDefaultSampler = AccessSampler(defaultSampler);

	U32 numResources = creation.numResources;
	FillWriteDescriptorSets(descriptorSetLayout, descriptorSet->descriptorSet, descriptorWrite, bufferInfo, imageInfo, vkDefaultSampler->sampler,
		numResources, creation.resources, creation.samplers, creation.bindings);

	// Cache resources
	for (U32 r = 0; r < creation.numResources; r++)
	{
		descriptorSet->resources[r] = creation.resources[r];
		descriptorSet->samplers[r] = creation.samplers[r];
		descriptorSet->bindings[r] = creation.bindings[r];
	}

	vkUpdateDescriptorSets(device, numResources, descriptorWrite, 0, nullptr);

	return handle;
}

RenderPassHandle Renderer::CreateRenderPass(const RenderPassCreation& creation)
{
	U32 resourceIndex = renderPasses.ObtainResource();
	RenderPassHandle handle = { resourceIndex };
	if (resourceIndex == INVALID_INDEX) { return handle; }

	RenderPass* renderPass = AccessRenderPass(handle);
	renderPass->type = creation.type;
	// Init the rest of the struct.
	renderPass->numRenderTargets = (U8)creation.numRenderTargets;
	renderPass->dispatchX = 0;
	renderPass->dispatchY = 0;
	renderPass->dispatchZ = 0;
	renderPass->name = creation.name;
	renderPass->frameBuffer = nullptr;
	renderPass->renderPass = nullptr;
	renderPass->scaleX = creation.scaleX;
	renderPass->scaleY = creation.scaleY;
	renderPass->resize = creation.resize;

	// Cache texture handles
	U32 c = 0;
	for (; c < creation.numRenderTargets; ++c)
	{
		Texture* texture = AccessTexture(creation.outputTextures[c]);

		renderPass->width = texture->width;
		renderPass->height = texture->height;

		// Cache texture handles
		renderPass->outputTextures[c] = creation.outputTextures[c];
	}

	renderPass->outputDepth = creation.depthStencilTexture;

	switch (creation.type)
	{
	case RENDER_PASS_TYPE_SWAPCHAIN: {
		CreateSwapchainPass(creation, renderPass);
	} break;
	case RENDER_PASS_TYPE_COMPUTE: {} break;
	case RENDER_PASS_TYPE_GEOMETRY: {
		renderPass->output = FillRenderPassOutput(creation);
		renderPass->renderPass = GetRenderPass(renderPass->output, creation.name);

		CreateFramebuffer(renderPass, creation.outputTextures, creation.numRenderTargets, creation.depthStencilTexture);
	} break;
	}

	return handle;
}

ShaderStateHandle Renderer::CreateShaderState(const ShaderStateCreation& creation)
{
	ShaderStateHandle handle = { INVALID_INDEX };

	if (creation.stagesCount == 0 || creation.stages == nullptr)
	{
		Logger::Error("Shader {} does not contain shader stages!", creation.name);
		return handle;
	}

	handle.index = shaders.ObtainResource();
	if (handle.index == INVALID_INDEX) { return handle; }

	// For each shader stage, compile them individually.
	U32 compiledShaders = 0;

	ShaderState* shaderState = AccessShaderState(handle);
	shaderState->graphicsPipeline = true;
	shaderState->activeShaders = 0;

	for (compiledShaders = 0; compiledShaders < creation.stagesCount; ++compiledShaders)
	{
		const ShaderStage& stage = creation.stages[compiledShaders];

		// Gives priority to compute: if any is present (and it should not be) then it is not a graphics pipeline.
		if (stage.type == VK_SHADER_STAGE_COMPUTE_BIT)
		{
			shaderState->graphicsPipeline = false;
		}

		VkShaderModuleCreateInfo shaderInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

		if (creation.spvInput)
		{
			shaderInfo.codeSize = stage.codeSize;
			shaderInfo.pCode = reinterpret_cast<const U32*>(stage.code);
		}
		else
		{
			shaderInfo = CompileShader(stage.code, stage.codeSize, stage.type, creation.name);
		}

		// Compile shader module
		VkPipelineShaderStageCreateInfo& shader_stage_info = shaderState->shaderStageInfos[compiledShaders];
		memset(&shader_stage_info, 0, sizeof(VkPipelineShaderStageCreateInfo));
		shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage_info.pName = "main";
		shader_stage_info.stage = stage.type;

		if (vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderState->shaderStageInfos[compiledShaders].module) != VK_SUCCESS)
		{
			break;
		}

		SetResourceName(VK_OBJECT_TYPE_SHADER_MODULE, (U64)shaderState->shaderStageInfos[compiledShaders].module, creation.name);
	}

	bool creationFailed = compiledShaders != creation.stagesCount;
	if (!creationFailed)
	{
		shaderState->activeShaders = compiledShaders;
		shaderState->name = creation.name;
	}

	if (creationFailed)
	{
		DestroyShaderState(handle);
		handle.index = INVALID_INDEX;

		// Dump shader code
		Logger::Error("Error in creation of shader {}! Dumping all shader informations...", creation.name);
		for (compiledShaders = 0; compiledShaders < creation.stagesCount; ++compiledShaders)
		{
			const ShaderStage& stage = creation.stages[compiledShaders];
			Logger::Error("{}:\n{}", (U32)stage.type, stage.code);
		}
	}

	return handle;
}

void Renderer::DestroyBuffer(BufferHandle buffer)
{
	if (buffer.index < buffers.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_BUFFER, buffer.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid buffer {}", buffer.index);
	}
}

void Renderer::DestroyTexture(TextureHandle texture)
{
	if (texture.index < textures.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_TEXTURE, texture.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid texture {}", texture.index);
	}
}

void Renderer::DestroyPipeline(PipelineHandle pipeline)
{
	if (pipeline.index < pipelines.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_PIPELINE, pipeline.index, currentFrame });

		Pipeline* vkPipeline = AccessPipeline(pipeline);
		DestroyShaderState(vkPipeline->shaderState);
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid Pipeline {}", pipeline.index);
	}
}

void Renderer::DestroySampler(SamplerHandle sampler)
{
	if (sampler.index < samplers.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_SAMPLER, sampler.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid sampler {}", sampler.index);
	}
}

void Renderer::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle layout)
{
	if (layout.index < descriptorSetLayouts.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT, layout.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid descriptor set layout {}", layout.index);
	}
}

void Renderer::DestroyDescriptorSet(DescriptorSetHandle set)
{
	if (set.index < descriptorSets.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_DESCRIPTOR_SET, set.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid descriptor set {}", set.index);
	}
}

void Renderer::DestroyRenderPass(RenderPassHandle renderPass)
{
	if (renderPass.index < renderPasses.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_RENDER_PASS, renderPass.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid render pass {}", renderPass.index);
	}
}

void Renderer::DestroyShaderState(ShaderStateHandle shader)
{
	if (shader.index < shaders.ResourceCount)
	{
		resourceDeletionQueue.Push({ RESOURCE_DELETE_TYPE_RENDER_PASS, shader.index, currentFrame });
	}
	else
	{
		Logger::Error("Graphics error: trying to free invalid shader {}", shader.index);
	}
}

void Renderer::DestroyBufferInstant(ResourceHandle buffer)
{
	Buffer* vkBuffer = (Buffer*)buffers.GetResource(buffer);

	if (vkBuffer && vkBuffer->parentBuffer.index == INVALID_BUFFER.index)
	{
		vmaDestroyBuffer(allocator, vkBuffer->buffer, vkBuffer->allocation);
	}
	buffers.ReleaseResource(buffer);
}

void Renderer::DestroyTextureInstant(ResourceHandle texture)
{
	Texture* vkTexture = (Texture*)textures.GetResource(texture);

	if (vkTexture)
	{
		vkDestroyImageView(device, vkTexture->imageView, allocationCallbacks);
		vmaDestroyImage(allocator, vkTexture->image, vkTexture->allocation);
	}
	textures.ReleaseResource(texture);
}

void Renderer::DestroyPipelineInstant(ResourceHandle pipeline)
{
	Pipeline* vkPipeline = (Pipeline*)pipelines.GetResource(pipeline);

	if (vkPipeline)
	{
		vkDestroyPipeline(device, vkPipeline->pipeline, allocationCallbacks);

		vkDestroyPipelineLayout(device, vkPipeline->pipelineLayout, allocationCallbacks);
	}
	pipelines.ReleaseResource(pipeline);
}

void Renderer::DestroySamplerInstant(ResourceHandle sampler)
{
	Sampler* vkSampler = (Sampler*)samplers.GetResource(sampler);

	if (vkSampler)
	{
		vkDestroySampler(device, vkSampler->sampler, allocationCallbacks);
	}
	samplers.ReleaseResource(sampler);
}

void Renderer::DestroyDescriptorSetLayoutInstant(ResourceHandle layout)
{
	DesciptorSetLayout* vkDescriptorSetLayout = (DesciptorSetLayout*)descriptorSetLayouts.GetResource(layout);

	if (vkDescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(device, vkDescriptorSetLayout->descriptorSetLayout, allocationCallbacks);

		Memory::FreeSize(&vkDescriptorSetLayout->bindings);
	}
	descriptorSetLayouts.ReleaseResource(layout);
}

void Renderer::DestroyDescriptorSetInstant(ResourceHandle set)
{
	DesciptorSet* vkDescriptorSet = (DesciptorSet*)descriptorSets.GetResource(set);

	if (vkDescriptorSet)
	{
		Memory::FreeSize(&vkDescriptorSet->resources);
	}
	descriptorSets.ReleaseResource(set);
}

void Renderer::DestroyRenderPassInstant(ResourceHandle renderPass)
{
	RenderPass* vkRenderPass = (RenderPass*)renderPasses.GetResource(renderPass);

	if (vkRenderPass)
	{
		if (vkRenderPass->numRenderTargets) { vkDestroyFramebuffer(device, vkRenderPass->frameBuffer, allocationCallbacks); }
	}
	renderPasses.ReleaseResource(renderPass);
}

void Renderer::DestroyShaderStateInstant(ResourceHandle shader)
{
	ShaderState* vkShaderState = (ShaderState*)shaders.GetResource(shader);
	if (vkShaderState)
	{
		for (size_t i = 0; i < vkShaderState->activeShaders; i++)
		{
			vkDestroyShaderModule(device, vkShaderState->shaderStageInfos[i].module, allocationCallbacks);
		}
	}
	shaders.ReleaseResource(shader);
}

ShaderState* Renderer::AccessShaderState(ShaderStateHandle shader)
{
	return shaders.GetResource(shader.index);
}

Texture* Renderer::AccessTexture(TextureHandle texture)
{
	return textures.GetResource(texture.index);
}

Buffer* Renderer::AccessBuffer(BufferHandle buffer)
{
	return buffers.GetResource(buffer.index);
}

Pipeline* Renderer::AccessPipeline(PipelineHandle pipeline)
{
	return pipelines.GetResource(pipeline.index);
}

Sampler* Renderer::AccessSampler(SamplerHandle sampler)
{
	return samplers.GetResource(sampler.index);
}

DesciptorSetLayout* Renderer::AccessDescriptorSetLayout(DescriptorSetLayoutHandle layout)
{
	return descriptorSetLayouts.GetResource(layout.index);
}

DesciptorSet* Renderer::AccessDescriptorSet(DescriptorSetHandle set)
{
	return descriptorSets.GetResource(set.index);
}

RenderPass* Renderer::AccessRenderPass(RenderPassHandle renderPass)
{
	return renderPasses.GetResource(renderPass.index);
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
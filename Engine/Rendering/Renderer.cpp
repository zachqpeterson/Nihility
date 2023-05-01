#include "Renderer.hpp"

#include "CommandBuffer.hpp"
#include "Core\Logger.hpp"
#include "Containers\Vector.hpp"
#include "Platform\Platform.hpp"
#include "Math\Math.hpp"

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

	

	FlatHashMapIterator it = renderPassCache.iterator_begin();
	while (it.is_valid())
	{
		VkRenderPass vk_render_pass = renderPassCache.get(it);
		vkDestroyRenderPass(vulkan_device, vk_render_pass, vulkan_allocation_callbacks);
		renderPassCache.iterator_advance(it);
	}
	renderPassCache.shutdown();

	RenderPass* pass = AccessRenderPass(swapchainPass);
	vkDestroyRenderPass(device, pass->renderPass, allocationCallbacks);

	for (U64 i = 0; i < swapchainImageCount; ++i)
	{
		vkDestroyImageView(device, swapchainImageViews[i], allocationCallbacks);
		vkDestroyFramebuffer(device, swapchainFramebuffers[i], allocationCallbacks);
	}

	vkDestroySwapchainKHR(device, swapchain, allocationCallbacks);

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
	VkDeviceQueueCreateInfo queueInfo[1] { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
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
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_B8G8R8_UNORM,
		VK_FORMAT_R8G8B8_UNORM
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

	VkQueryPoolCreateInfo queryPoolInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,  };
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

	U8* memory;
	Memory::AllocateSize(&memory, sizeof(GPUTimestampManager) + sizeof(CommandBuffer*) * 128);

	timestampManager = (GPUTimestampManager*)(memory);
	timestampManager->Create(timeQueriesPerFrame, MAX_SWAPCHAIN_IMAGES);

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
	sc.SetSddressModeUVW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
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
	strcpy(binariesPath + Length(binariesPath), "%s\\Bin\\");
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
}

void Renderer::Update()
{

}






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

BufferHandle Renderer::CreateBuffer(const BufferCreation& creation)
{
	BufferHandle handle = { buffers.ObtainResource() };
	if (handle.index == INVALID_INDEX) { return handle; }

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
	//    mapped_data = static_cast<uint8_t *>(allocation_info.pMappedData);
	//}

	return handle;
}

TextureHandle Renderer::CreateTexture(const TextureCreation& creation)
{

}

PipelineHandle Renderer::CreatePipeline(const PipelineCreation& creation)
{

}

SamplerHandle Renderer::CreateSampler(const SamplerCreation& creation)
{

}

DescriptorSetLayoutHandle Renderer::CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& creation)
{

}

DescriptorSetHandle Renderer::CreateDescriptorSet(const DescriptorSetCreation& creation)
{

}

RenderPassHandle Renderer::CreateRenderPass(const RenderPassCreation& creation)
{

}

ShaderStateHandle Renderer::CreateShaderState(const ShaderStateCreation& creation)
{

}

void Renderer::DestroyBuffer(BufferHandle buffer)
{

}

void Renderer::DestroyTexture(TextureHandle texture)
{

}

void Renderer::DestroyPipeline(PipelineHandle pipeline)
{

}

void Renderer::DestroySampler(SamplerHandle sampler)
{

}

void Renderer::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle layout)
{

}

void Renderer::DestroyDescriptorSet(DescriptorSetHandle set)
{

}

void Renderer::DestroyRenderPass(RenderPassHandle renderPass)
{

}

void Renderer::DestroyShaderState(ShaderStateHandle shader)
{

}

void Renderer::DestroyBufferInstant(ResourceHandle buffer)
{

}

void Renderer::DestroyTextureInstant(ResourceHandle texture)
{

}

void Renderer::DestroyPipelineInstant(ResourceHandle pipeline)
{

}

void Renderer::DestroySamplerInstant(ResourceHandle sampler)
{

}

void Renderer::DestroyDescriptorSetLayoutInstant(ResourceHandle layout)
{

}

void Renderer::DestroyDescriptorSetInstant(ResourceHandle set)
{

}

void Renderer::DestroyRenderPassInstant(ResourceHandle renderPass)
{

}

void Renderer::DestroyShaderStateInstant(ResourceHandle shader)
{

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
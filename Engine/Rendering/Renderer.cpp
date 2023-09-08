#include "Renderer.hpp"

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
#include "External\LunarG\vma\vk_mem_alloc.h"

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
#endif
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
VkPhysicalDeviceFeatures			Renderer::physicalDeviceFeatures;
VkPhysicalDeviceProperties			Renderer::physicalDeviceProperties;
VkPhysicalDeviceMemoryProperties	Renderer::physicalDeviceMemoryProperties;

// DEVICE
VkInstance							Renderer::instance;
VkPhysicalDevice					Renderer::physicalDevice;
VkDevice							Renderer::device;
VkQueue								Renderer::deviceQueue;
Swapchain							Renderer::swapchain{};
U32									Renderer::queueFamilyIndex;
PFN_vkCmdPushDescriptorSetWithTemplateKHR Renderer::vkCmdPushDescriptorSetWithTemplateKHR;

VkAllocationCallbacks* Renderer::allocationCallbacks;
VkDescriptorPool					Renderer::descriptorPool;
U64									Renderer::uboAlignment;
U64									Renderer::sboAlignemnt;

bool								Renderer::bindlessSupported{ false };
bool								Renderer::pushDescriptorsSupported{ false };
bool								Renderer::meshShadingSupported{ false };

// WINDOW
U32									Renderer::frameIndex{ 0 };
U32									Renderer::currentFrame{ 1 };
U32									Renderer::previousFrame{ 0 };
U32									Renderer::absoluteFrame{ 0 };
bool								Renderer::resized{ false };

// RESOURCES
Scene* Renderer::currentScene;
VmaAllocator_T* Renderer::allocator;
CommandBufferRing					Renderer::commandBufferRing;
Buffer								Renderer::stagingBuffer;
Buffer								Renderer::vertexBuffer;
Buffer								Renderer::indexBuffer;
Buffer								Renderer::instanceBuffer;
Buffer								Renderer::meshBuffer;
Buffer								Renderer::drawCommandsBuffer;
Vector<VkDrawIndexedIndirectCommand>Renderer::drawCommands;
U32									Renderer::drawCount{ 0 };

// TIMING
VkSemaphore							Renderer::imageAcquired{ nullptr };
VkSemaphore							Renderer::queueSubmitted{ nullptr };

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

	vkDestroySemaphore(device, imageAcquired, allocationCallbacks);
	vkDestroySemaphore(device, queueSubmitted, allocationCallbacks);

	DestroyBuffer(stagingBuffer);
	DestroyBuffer(vertexBuffer);
	DestroyBuffer(indexBuffer);
	DestroyBuffer(meshBuffer);

	swapchain.Destroy();

	Resources::Shutdown();

	vmaDestroyAllocator(allocator);

#ifdef NH_DEBUG
	vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocationCallbacks);
#endif

	vkDestroyDescriptorPool(device, descriptorPool, allocationCallbacks);

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

	vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

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

	Logger::Trace("Best GPU Found: {}", physicalDeviceProperties.deviceName);

	uboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	sboAlignemnt = physicalDeviceProperties.limits.minStorageBufferOffsetAlignment;

	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

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

	U32 extensionCount = 0;
	VkValidateFR(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, 0));
	Vector<VkExtensionProperties> extensions(extensionCount, {});
	VkValidateFR(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, extensions.Data()));

	for (VkExtensionProperties& ext : extensions)
	{
		pushDescriptorsSupported |= Memory::Compare(ext.extensionName, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, 22);
		meshShadingSupported |= Memory::Compare(ext.extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME, 18);
	}

	//TODO: Remove
	pushDescriptorsSupported = false;
	meshShadingSupported = false;

	U32 deviceExtensionCount = 0;
	CSTR deviceExtensions[4];

	deviceExtensions[deviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	if (pushDescriptorsSupported) { deviceExtensions[deviceExtensionCount++] = VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME; }
	if (meshShadingSupported) { deviceExtensions[deviceExtensionCount++] = VK_EXT_MESH_SHADER_EXTENSION_NAME; }

	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr };
	VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures };

	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

	bindlessSupported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;

	VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	features.features.multiDrawIndirect = true;
	features.features.pipelineStatisticsQuery = true;
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
	deviceInfo.queueCreateInfoCount = CountOf32(queueInfo);
	deviceInfo.pQueueCreateInfos = queueInfo;
	deviceInfo.enabledExtensionCount = deviceExtensionCount;
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceInfo.pNext = &features;
	deviceInfo.pEnabledFeatures = nullptr;
	deviceInfo.flags = 0;

	VkValidateFR(vkCreateDevice(physicalDevice, &deviceInfo, allocationCallbacks, &device));

	vkGetDeviceQueue(device, queueFamilyIndex, 0, &deviceQueue);

	SetResourceName(VK_OBJECT_TYPE_QUEUE, (U64)deviceQueue, "device_queue");

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

	VkValidateFR(vkCreateDescriptorPool(device, &poolInfo, allocationCallbacks, &descriptorPool));

	if (bindlessSupported) { Resources::CreateBindless(); }

	static constexpr U16 timeQueriesPerFrame = 32;

	VkQueryPoolCreateInfo queryPoolInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, };
	queryPoolInfo.pNext = nullptr;
	queryPoolInfo.flags = 0;
	queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolInfo.queryCount = timeQueriesPerFrame * 2u * MAX_SWAPCHAIN_IMAGES;
	queryPoolInfo.pipelineStatistics = 0;

	VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &imageAcquired);
	SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (U64)imageAcquired, "image_aquired_semaphore");

	vkCreateSemaphore(device, &semaphoreInfo, allocationCallbacks, &queueSubmitted);
	SetResourceName(VK_OBJECT_TYPE_SEMAPHORE, (U64)queueSubmitted, "queue_submitted_semaphore");

	commandBufferRing.Create();

	stagingBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vertexBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	instanceBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	meshBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	drawCommandsBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Resources::CreateDefaults();

	return true;
}

bool Renderer::BeginFrame()
{
	VkResult result = swapchain.Update();
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Resize();
	}
	else if (result == VK_NOT_READY)
	{
		FrameCountersAdvance();
		return false;
	}

	result = swapchain.NextImage(frameIndex, imageAcquired);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Resize();
		FrameCountersAdvance();
		return false;
	}

	commandBufferRing.ResetPools(frameIndex);
	VkValidateFR(vkResetDescriptorPool(device, descriptorPool, 0));

	return true;
}

void Renderer::Render(CommandBuffer* commandBuffer, Pipeline* pipeline, U32 drawCount)
{
	Camera& camera = currentScene->camera;

	GlobalData globalData{};
	globalData.vp = camera.ViewProjection();
	globalData.eye = camera.Eye();

	commandBuffer->BindRenderpass(pipeline->renderpass);

	bool taskSubmit = false;

	if (taskSubmit)
	{
		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, late ? meshlatePipelineMS : meshPipelineMS);
		//
		//// TODO: double-check synchronization
		//DescriptorInfo pyramidDesc(depthSampler, depthPyramid.imageView, VK_IMAGE_LAYOUT_GENERAL);
		//DescriptorInfo descriptors[] = { dcb.buffer, db.buffer, mlb.buffer, mdb.buffer, vb.buffer, mvb.buffer, pyramidDesc };
		//// vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer, meshProgramMS.updateTemplate, meshProgramMS.layout, 0, descriptors);
		//pushDescriptors(meshProgramMS, descriptors);
		//
		//vkCmdPushConstants(commandBuffer, meshProgramMS.layout, meshProgramMS.pushConstantStages, 0, sizeof(globals), &globals);
		//vkCmdDrawMeshTasksIndirectEXT(commandBuffer, dccb.buffer, 4, 1, 0);
	}
	else
	{
		commandBuffer->BindPipeline(pipeline);

		Descriptor descriptors[]{ {drawCommandsBuffer.vkBuffer}, {meshBuffer.vkBuffer} };
		PushDescriptors(pipeline->shader, descriptors);
		vkCmdPushConstants(commandBuffer->commandBuffer, pipeline->shader->pipelineLayout, pipeline->shader->pushConstantStages, 0, sizeof(GlobalData), &globalData);

		commandBuffer->BindIndexBuffer(indexBuffer);
		commandBuffer->BindVertexBuffer(vertexBuffer);
		commandBuffer->BindInstanceBuffer(instanceBuffer);

		//TODO: Take into account physicalDeviceProperties.limits.maxDrawIndirectCount;

		if (physicalDeviceFeatures.multiDrawIndirect)
		{
			vkCmdDrawIndexedIndirect(commandBuffer->commandBuffer, drawCommandsBuffer.vkBuffer, 0, drawCount, sizeof(VkDrawIndexedIndirectCommand));
		}
		else
		{
			for (U32 i = 0; i < drawCount; ++i)
			{
				vkCmdDrawIndexedIndirect(commandBuffer->commandBuffer, drawCommandsBuffer.vkBuffer, sizeof(VkDrawIndexedIndirectCommand) * i, 1, sizeof(VkDrawIndexedIndirectCommand));
			}
		}
	}

	vkCmdEndRenderPass(commandBuffer->commandBuffer);
}

void Renderer::EndFrame()
{
	CommandBuffer* commandBuffer = GetCommandBuffer(true);

	if (currentScene)
	{
		currentScene->Update();
	}
	else
	{
		return; //TODO: Default scene?
	}

	Resources::Update();

	Render(commandBuffer, Resources::renderPipeline, (U32)currentScene->draws.Size());

	//Post Processing
	//TODO: Skybox
	//TODO: Bloom
	//TODO: Fog
	//TODO: Exposure
	//TODO: White Balancing
	//TODO: Contrast
	//TODO: Brightness
	//TODO: Color Filtering
	//TODO: Saturation
	//TODO: Tonemapping
	//TODO: Gamma

	//TODO: UI

	//VkImageMemoryBarrier2 copyBarrier = ImageBarrier(swapchain.renderTargets[frameIndex]->image,
	//	0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	//
	//commandBuffer->PipelineBarrier(VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &copyBarrier);
	//
	//VkImageCopy copyRegion{};
	//copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//copyRegion.srcSubresource.layerCount = 1;
	//copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//copyRegion.dstSubresource.layerCount = 1;
	//copyRegion.extent = { Settings::WindowWidth(), Settings::WindowHeight(), 1 };
	//
	//vkCmdCopyImage(commandBuffer->commandBuffer, Resources::renderPipeline->renderpass->outputTextures[0]->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	//	swapchain.renderTargets[frameIndex]->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
	//
	//VkImageMemoryBarrier2 presentBarrier = ImageBarrier(swapchain.renderTargets[frameIndex]->image,
	//	VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	//	0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	//
	//commandBuffer->PipelineBarrier(VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &presentBarrier);

	commandBuffer->BindRenderpass(&swapchain.renderpass);
	commandBuffer->BindPipeline(Resources::swapchainPipeline);

	Descriptor descriptors[]{ {Resources::renderPipeline->renderpass->outputTextures[0]->imageView, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, Resources::defaultSampler->sampler} };
	PushDescriptors(Resources::swapchainProgram, descriptors);

	commandBuffer->Draw(0, 3, 0, 1);
	vkCmdEndRenderPass(commandBuffer->commandBuffer);

	VkValidate(vkEndCommandBuffer(commandBuffer->commandBuffer));

	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAcquired;
	submitInfo.pWaitDstStageMask = &submitStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &queueSubmitted;

	BreakPoint;
	VkValidate(vkQueueSubmit(deviceQueue, 1, &submitInfo, nullptr));

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &queueSubmitted;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.swapchain;
	presentInfo.pImageIndices = &frameIndex;

	VkResult result = vkQueuePresentKHR(deviceQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) { Resize(); }

	FrameCountersAdvance();

	VkValidate(vkQueueWaitIdle(deviceQueue));
}

void Renderer::Resize()
{
	vkDeviceWaitIdle(device);

	swapchain.Create();

	Resources::UpdatePipelines();
	currentScene->updatePostProcess = true;

	vkDeviceWaitIdle(device);

	//TODO: Update camera here
}

void Renderer::LoadScene(const String& name)
{
	if (currentScene)
	{
		//Resources::SaveScene(currentScene);
		//TODO: Unload scene
	}

	currentScene = Resources::LoadScene(name);
	currentScene->updatePostProcess = true;
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

void Renderer::FrameCountersAdvance()
{
	previousFrame = currentFrame;
	currentFrame = (currentFrame + 1) % swapchain.imageCount;

	++absoluteFrame;
}

U32 Renderer::FrameIndex()
{
	return frameIndex;
}

U32 Renderer::CurrentFrame()
{
	return currentFrame;
}

CommandBuffer* Renderer::GetCommandBuffer(bool begin)
{
	return commandBufferRing.GetCommandBuffer(currentFrame, begin);
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

VkImageMemoryBarrier2 Renderer::ImageBarrier(VkImage image, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
	VkImageLayout oldLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkImageLayout newLayout,
	VkImageAspectFlags aspectMask, U32 baseMipLevel, U32 levelCount)
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
	result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

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

Buffer Renderer::CreateBuffer(U64 size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags)
{
	Buffer buffer{};
	buffer.size = size;
	buffer.usage = usageFlags;
	buffer.memoryProperties = memoryFlags;

	VkBufferCreateInfo info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	info.size = size;
	info.usage = usageFlags;

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { memoryInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; }
	memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
	memoryInfo.requiredFlags = memoryFlags;

	VmaAllocationInfo allocationInfo{};
	allocationInfo.pName = "buffer";
	VkValidate(vmaCreateBuffer(allocator, &info, &memoryInfo, &buffer.vkBuffer, &buffer.allocation, &allocationInfo));

	//TODO: Generate resource name?

	buffer.deviceMemory = allocationInfo.deviceMemory;

	if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		buffer.data = allocationInfo.pMappedData;
		buffer.mapped = true;
	}

	//TODO: CreateBufferWithData
	//if (data)
	//{
	//	void* mappedData;
	//	vmaMapMemory(allocator, buffer->allocation, &mappedData);
	//	Memory::Copy(mappedData, data, (U64)buffer->size);
	//	vmaUnmapMemory(allocator, buffer->allocation);
	//}

	return Move(buffer);
}

void Renderer::FillBuffer(Buffer& buffer, const void* data, U64 size, U64 offset)
{
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(currentFrame);
	vkBeginCommandBuffer(commandBuffer->commandBuffer, &beginInfo);

	Memory::Copy(stagingBuffer.data, data, size);

	VkBufferCopy region = { 0, offset, size };
	vkCmdCopyBuffer(commandBuffer->commandBuffer, stagingBuffer.vkBuffer, buffer.vkBuffer, 1, &region);

	vkEndCommandBuffer(commandBuffer->commandBuffer);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer->commandBuffer;

	vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(deviceQueue);
	vkResetCommandBuffer(commandBuffer->commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
}

U64 Renderer::UploadToBuffer(Buffer& buffer, const void* data, U64 size)
{
	FillBuffer(buffer, data, size, buffer.allocationOffset);

	U64 offset = buffer.allocationOffset;
	buffer.allocationOffset += size;

	return offset;
}

void Renderer::UploadDraw(const Mesh& mesh, U32 indexCount, U32* indices, U32 vertexCount, Vertex* vertices)
{
	VkDrawIndexedIndirectCommand drawCommand{};
	drawCommand.indexCount = indexCount;
	drawCommand.instanceCount = 1;
	drawCommand.firstIndex = (U32)Renderer::UploadToBuffer(Renderer::indexBuffer, indices, indexCount * sizeof(U32));
	drawCommand.vertexOffset = (U32)Renderer::UploadToBuffer(Renderer::vertexBuffer, vertices, vertexCount * sizeof(Vertex));
	drawCommand.firstInstance = 0;

	U32 meshIndex = Renderer::UploadToBuffer(Renderer::meshBuffer, &mesh, sizeof(Mesh)) / sizeof(Mesh);
	Renderer::UploadToBuffer(Renderer::instanceBuffer, &meshIndex, sizeof(U32));
	Renderer::UploadToBuffer(Renderer::drawCommandsBuffer, &drawCommand, sizeof(VkDrawIndexedIndirectCommand));

	drawCommands.Push(drawCommand);

	++drawCount;
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
			vmaUnmapMemory(allocator, buffer.allocation);
			buffer.data = nullptr;
			buffer.mapped = false;
		}

		vmaDestroyBuffer(allocator, buffer.vkBuffer, buffer.allocation);
	}
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
	createInfo.anisotropyEnable = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.unnormalizedCoordinates = VK_FALSE;
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
	imageInfo.flags = 0;
	imageInfo.imageType = texture->type;
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
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (texture->flags & TEXTURE_FLAG_COMPUTE) { imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT; }
	if (texture->flags & TEXTURE_FLAG_RENDER_TARGET)
	{
		if (HasDepthOrStencil(texture->format)) { imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }
		else { imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
	}

	VmaAllocationCreateInfo memoryInfo{};
	memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocationInfo allocInfo{};
	allocInfo.pName = "texture";
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &texture->image, &texture->allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, texture->name);

	VkImageViewCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	info.image = texture->image;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D; //TODO: Don't hardcode
	info.format = imageInfo.format;
	info.subresourceRange.levelCount = texture->mipmapCount;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.baseMipLevel = 0;

	if (HasDepthOrStencil(texture->format))
	{
		info.subresourceRange.aspectMask = HasDepth(texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
		info.subresourceRange.aspectMask |= HasStencil(texture->format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
	}
	else
	{
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->imageView));
	texture->mipmaps[0] = texture->imageView;

	SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, texture->name);


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

		CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(currentFrame);
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

		CommandBuffer* blitCmd = commandBufferRing.GetCommandBufferInstant(currentFrame);
		vkBeginCommandBuffer(blitCmd->commandBuffer, &beginInfo);

		info.subresourceRange.levelCount = 1;

		//TODO: Some textures could have mipmaps stored in the already
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

			//Create Texture View

			info.subresourceRange.baseMipLevel = i;

			VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->mipmaps[i]));
		}

		subresourceRange.levelCount = texture->mipmapCount;
		TransitionImage(blitCmd->commandBuffer, texture->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT); //TODO: Images can be used outside of fragment shader
		vkEndCommandBuffer(blitCmd->commandBuffer);

		submitInfo.pCommandBuffers = &blitCmd->commandBuffer;

		vkQueueSubmit(deviceQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(deviceQueue);

		vkResetCommandBuffer(blitCmd->commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
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

	return true;
}

bool Renderer::CreateCubeMap(Texture* texture, void* data, U32* layerSizes)
{
	VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = texture->format;
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
	allocInfo.pName = texture->name;
	VkValidate(vmaCreateImage(allocator, &imageInfo, &memoryInfo, &texture->image, &texture->allocation, &allocInfo));

	SetResourceName(VK_OBJECT_TYPE_IMAGE, (U64)texture->image, texture->name);

	VkBufferImageCopy bufferCopyRegions[6 * 14];
	U32 regionCount = 0;
	U32 offset = 0;

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

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipmapCount;
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

	CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(currentFrame);
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
	view.subresourceRange.levelCount = texture->mipmapCount;
	view.image = texture->image;
	VkValidateR(vkCreateImageView(device, &view, allocationCallbacks, &texture->imageView));

	if (bindlessSupported) { Resources::bindlessTexturesToUpdate.Push({ RESOURCE_UPDATE_TYPE_TEXTURE, texture->handle, currentFrame }); }

	return true;
}

bool Renderer::CreateDescriptorSetLayout(DescriptorSetLayout* descriptorSetLayout)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = pushDescriptorsSupported ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
	layoutInfo.bindingCount = descriptorSetLayout->bindingCount;
	layoutInfo.pBindings = descriptorSetLayout->bindings;

	VkValidateR(vkCreateDescriptorSetLayout(device, &layoutInfo, allocationCallbacks, &descriptorSetLayout->descriptorSetLayout));

	return true;
}

bool Renderer::CreateDescriptorUpdateTemplate(DescriptorSetLayout* descriptorSetLayout, Shader* shader)
{
	VkDescriptorUpdateTemplateEntry entries[MAX_DESCRIPTORS_PER_SET]{};

	for (U32 i = 0; i < descriptorSetLayout->bindingCount; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = descriptorSetLayout->bindings[i];
		VkDescriptorUpdateTemplateEntry& entry = entries[i];

		entry.dstBinding = binding.binding;
		entry.dstArrayElement = 0;
		entry.descriptorCount = binding.descriptorCount;
		entry.descriptorType = binding.descriptorType;
		entry.offset = sizeof(Descriptor) * i;
		entry.stride = sizeof(Descriptor);
	}

	VkDescriptorUpdateTemplateCreateInfo descriptorTemplateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO };
	descriptorTemplateInfo.pNext = nullptr;
	descriptorTemplateInfo.flags = 0;
	descriptorTemplateInfo.templateType = pushDescriptorsSupported ? VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR : VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
	descriptorTemplateInfo.descriptorSetLayout = descriptorSetLayout->descriptorSetLayout;
	descriptorTemplateInfo.pipelineBindPoint = shader->bindPoint;
	descriptorTemplateInfo.pipelineLayout = shader->pipelineLayout;
	descriptorTemplateInfo.descriptorUpdateEntryCount = descriptorSetLayout->bindingCount;
	descriptorTemplateInfo.pDescriptorUpdateEntries = entries;
	descriptorTemplateInfo.set = descriptorSetLayout->setIndex;

	VkValidateR(vkCreateDescriptorUpdateTemplate(device, &descriptorTemplateInfo, allocationCallbacks, &descriptorSetLayout->updateTemplate));

	return true;
}

void Renderer::PushDescriptors(Shader* shader, const Descriptor* descriptors)
{
	CommandBuffer* commandBuffer = GetCommandBuffer(false);

	if (pushDescriptorsSupported)
	{
		vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer->commandBuffer, shader->setLayouts[0]->updateTemplate, shader->pipelineLayout, 0, descriptors);
	}
	else
	{
		VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &shader->setLayouts[0]->descriptorSetLayout;

		VkDescriptorSet set = nullptr;
		VkValidate(vkAllocateDescriptorSets(device, &allocateInfo, &set));

		vkUpdateDescriptorSetWithTemplate(device, set, shader->setLayouts[0]->updateTemplate, descriptors);

		if (shader->useBindless)
		{
			const VkDescriptorSet sets[]{ set, Resources::bindlessDescriptorSet };
			vkCmdBindDescriptorSets(commandBuffer->commandBuffer, shader->bindPoint, shader->pipelineLayout, 0, 2, sets, 0, 0);
		}
		else
		{
			vkCmdBindDescriptorSets(commandBuffer->commandBuffer, shader->bindPoint, shader->pipelineLayout, 0, 1, &set, 0, 0);
		}
	}
};

bool Renderer::CreateRenderpass(Renderpass* renderpass, bool swapchainRenderpass)
{
	for (U32 i = 0; i < renderpass->renderTargetCount; ++i) { renderpass->output.Color(renderpass->outputTextures[i]->format); }
	if (renderpass->outputDepth) { renderpass->output.Depth(renderpass->outputDepth->format); }

	VkImageLayout colorInitial, depthInitial;

	switch (renderpass->output.colorOperation)
	{
	case VK_ATTACHMENT_LOAD_OP_LOAD: { colorInitial = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; } break;
	case VK_ATTACHMENT_LOAD_OP_CLEAR: { colorInitial = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; } break;
	case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
	default: { colorInitial = VK_IMAGE_LAYOUT_UNDEFINED; } break;
	}

	switch (renderpass->output.depthOperation)
	{
	case VK_ATTACHMENT_LOAD_OP_LOAD: { depthInitial = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; } break;
	case VK_ATTACHMENT_LOAD_OP_CLEAR: { depthInitial = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; } break;
	case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
	default: { depthInitial = VK_IMAGE_LAYOUT_UNDEFINED; } break;
	}

	VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1]{};
	VkAttachmentReference colorAttachments[MAX_IMAGE_OUTPUTS]{};
	VkAttachmentReference depthAttachment{};

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	U32 attachmentCount = 0;
	if (swapchainRenderpass)
	{
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = renderpass->outputTextures[0]->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = renderpass->output.colorOperation;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = renderpass->output.stencilOperation;
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
			attachments[attachmentCount].loadOp = renderpass->output.colorOperation;
			attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[attachmentCount].stencilLoadOp = renderpass->output.stencilOperation;
			attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[attachmentCount].finalLayout = renderpass->output.attachmentFinalLayout;

			colorAttachments[attachmentCount].attachment = attachmentCount;
			colorAttachments[attachmentCount].layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

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
		attachments[attachmentCount].loadOp = renderpass->output.depthOperation;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = renderpass->output.stencilOperation;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[attachmentCount].finalLayout = renderpass->output.attachmentFinalLayout;

		depthAttachment.attachment = attachmentCount;
		depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depthAttachment;

		if (swapchainRenderpass)
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

	if (swapchainRenderpass)
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
	renderpass->lastResize = absoluteFrame;

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

	if (swapchainRenderpass)
	{
		renderpass->tiedToFrame = true;

		for (U64 i = 0; i < swapchain.imageCount; i++)
		{
			framebufferAttachments[0] = renderpass->outputTextures[i]->imageView;
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

	if (swapchainRenderpass)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(currentFrame);
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

		U32 imageBarrierCount = 0;
		VkImageMemoryBarrier2 imageBarriers[MAX_SWAPCHAIN_IMAGES];
		for (U64 i = 0; i < swapchain.imageCount; ++i)
		{
			imageBarriers[i] = ImageBarrier(renderpass->outputTextures[i]->image, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
			++imageBarrierCount;
		}

		commandBuffer->PipelineBarrier(0, 0, nullptr, imageBarrierCount, imageBarriers);

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
		for (U32 i = 0; i < texture->mipmapCount; ++i)
		{
			vkDestroyImageView(device, texture->mipmaps[i], allocationCallbacks);
		}

		if (!texture->swapchainImage) { vmaDestroyImage(allocator, texture->image, texture->allocation); }
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
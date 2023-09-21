#include "Renderer.hpp"

#include "UI.hpp"
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
Buffer								Renderer::instanceBuffer;
Buffer								Renderer::indexBuffer;
Buffer								Renderer::materialBuffer;
Buffer								Renderer::drawCommandsBuffer;
U32									Renderer::meshDrawCount{ 0 };
U32									Renderer::uiDrawCount{ 0 };
U32									Renderer::uiInstanceOffset{ 0 };

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
	DestroyBuffer(instanceBuffer);
	DestroyBuffer(indexBuffer);
	DestroyBuffer(materialBuffer);
	DestroyBuffer(drawCommandsBuffer);

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
	materialBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	drawCommandsBuffer = CreateBuffer(MEGABYTES(128), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	uiInstanceOffset = MEGABYTES(64);

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
	VkValidateF(vkResetDescriptorPool(device, descriptorPool, 0));

	return true;
}

void Renderer::Render(CommandBuffer* commandBuffer, Pipeline* pipeline, U32 drawCount, U32 offset)
{
	if (drawCount)
	{
		Camera& camera = currentScene->camera;

		GlobalData globalData{};
		globalData.vp = camera.ViewProjection();
		globalData.eye = camera.Eye();

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

			PushDescriptors(pipeline->shader);
			if (pipeline->shader->pushConstantStages) { commandBuffer->PushConstants(pipeline->shader, 0, sizeof(GlobalData), &globalData); }

			commandBuffer->BindIndexBuffer(indexBuffer);
			if (pipeline->shader->instanceOffset) { commandBuffer->BindVertexBuffer(vertexBuffer); }
			if (pipeline->shader->instanceOffset != U8_MAX) { commandBuffer->BindInstanceBuffer(instanceBuffer); }

			commandBuffer->DrawIndexedIndirect(drawCommandsBuffer, drawCount, offset);
		}
	}
}

void Renderer::EndFrame()
{
	CommandBuffer* commandBuffer = GetCommandBuffer();
	commandBuffer->Begin();

	if (currentScene)
	{
		currentScene->Update();
	}
	else
	{
		return; //TODO: Default scene?
	}

	Resources::Update();

	commandBuffer->BeginRenderpass(Resources::renderPipeline->renderpass);

	Render(commandBuffer, Resources::renderPipeline, meshDrawCount, 0);

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

	Render(commandBuffer, Resources::uiPipeline, uiDrawCount, UI::uploadOffset);

	commandBuffer->EndRenderpass();

	VkImageMemoryBarrier2 copyBarriers[]{
		ImageBarrier(Resources::renderPipeline->renderpass->outputTextures[0]->image,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
		ImageBarrier(swapchain.renderTargets[frameIndex]->image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	};

	commandBuffer->PipelineBarrier(VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 2, copyBarriers);

	VkImageCopy copyRegion{};
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.extent = { Settings::WindowWidth(), Settings::WindowHeight(), 1 };

	commandBuffer->ImageToImage(Resources::renderPipeline->renderpass->outputTextures[0], swapchain.renderTargets[frameIndex], 1, &copyRegion);

	VkImageMemoryBarrier2 presentBarrier = ImageBarrier(swapchain.renderTargets[frameIndex]->image,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	commandBuffer->PipelineBarrier(VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &presentBarrier);
	commandBuffer->End();

	//Submit Frame
	VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkValidate(commandBuffer->Submit(deviceQueue, &submitStageMask, 1, &imageAcquired, 1, &queueSubmitted));

	VkResult result = swapchain.Present(deviceQueue, frameIndex, 1, &queueSubmitted);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) { Resize(); }

	FrameCountersAdvance();

	vkQueueWaitIdle(deviceQueue);
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

CommandBuffer* Renderer::GetCommandBuffer()
{
	return commandBufferRing.GetCommandBuffer(frameIndex);
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
	CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(currentFrame);
	commandBuffer->Begin();

	Memory::Copy(stagingBuffer.data, data, size);

	VkBufferCopy region = { 0, offset, size };
	commandBuffer->BufferToBuffer(stagingBuffer, buffer, 1, &region);
	commandBuffer->End();
	commandBuffer->Submit(deviceQueue);

	vkQueueWaitIdle(deviceQueue);

	commandBuffer->Reset();
}

U64 Renderer::UploadToBuffer(Buffer& buffer, const void* data, U64 size)
{
	FillBuffer(buffer, data, size, buffer.allocationOffset);

	U64 offset = buffer.allocationOffset;
	buffer.allocationOffset += size;

	return offset;
}

void Renderer::UploadDrawCall(U32 indexCount, U32 indexOffset, U32 vertexOffset, U32 instanceCount, U32 instanceOffset, U32 offset)
{
	VkDrawIndexedIndirectCommand drawCommand{};
	drawCommand.indexCount = indexCount;
	drawCommand.instanceCount = instanceCount;
	drawCommand.firstIndex = indexOffset;
	drawCommand.vertexOffset = vertexOffset;
	drawCommand.firstInstance = instanceOffset;

	if (offset == U32_MAX) { Renderer::UploadToBuffer(Renderer::drawCommandsBuffer, &drawCommand, sizeof(VkDrawIndexedIndirectCommand)); }
	else { FillBuffer(Renderer::drawCommandsBuffer, &drawCommand, sizeof(VkDrawIndexedIndirectCommand), offset); }
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
		CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(frameIndex);
		commandBuffer->Begin();

		Memory::Copy(stagingBuffer.data, data, texture->size);

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

		VkImageMemoryBarrier2 copyBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
		VkImageMemoryBarrier2 mipBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);

		commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
		commandBuffer->BufferToImage(stagingBuffer, texture, 1, &region);
		commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &mipBarrier);
		commandBuffer->End();
		commandBuffer->Submit(deviceQueue);

		vkQueueWaitIdle(deviceQueue);

		commandBuffer->Reset();

		CommandBuffer* blitCmd = commandBufferRing.GetCommandBufferInstant(frameIndex);
		blitCmd->Begin();

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

			blitCmd->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
			blitCmd->Blit(texture, texture, VK_FILTER_LINEAR, 1, &blitRegion);
			blitCmd->PipelineBarrier(0, 0, nullptr, 1, &mipBarrier);

			VkValidate(vkCreateImageView(device, &info, allocationCallbacks, &texture->mipmaps[i]));
		}

		texture->mipmapsGenerated = texture->mipmapCount > 1;

		VkImageMemoryBarrier2 finalBarrier;

		if (texture->flags & TEXTURE_FLAG_RENDER_TARGET)
		{
			finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, HasDepthOrStencil(texture->format) ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				HasDepthOrStencil(texture->format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, texture->mipmapCount);
		}
		else
		{
			finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, texture->mipmapCount);
		}

		blitCmd->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
		blitCmd->End();
		blitCmd->Submit(deviceQueue);

		vkQueueWaitIdle(deviceQueue);
		texture->imageLayout = finalBarrier.newLayout;

		blitCmd->Reset();
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

	CommandBuffer* commandBuffer = commandBufferRing.GetCommandBufferInstant(frameIndex);
	commandBuffer->Begin();

	Memory::Copy(stagingBuffer.data, data, texture->size);

	VkImageMemoryBarrier2 copyBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1);
	VkImageMemoryBarrier2 finalBarrier = ImageBarrier(texture->image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, texture->mipmapCount, 6);

	commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &copyBarrier);
	commandBuffer->BufferToImage(stagingBuffer, texture, regionCount, bufferCopyRegions);
	commandBuffer->PipelineBarrier(0, 0, nullptr, 1, &finalBarrier);
	commandBuffer->End();
	commandBuffer->Submit(deviceQueue);

	vkQueueWaitIdle(deviceQueue);
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	commandBuffer->Reset();

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
	descriptorTemplateInfo.descriptorSetLayout = pushDescriptorsSupported ? 0 : descriptorSetLayout->descriptorSetLayout;
	descriptorTemplateInfo.pipelineBindPoint = shader->bindPoint;
	descriptorTemplateInfo.pipelineLayout = shader->pipelineLayout;
	descriptorTemplateInfo.descriptorUpdateEntryCount = descriptorSetLayout->bindingCount;
	descriptorTemplateInfo.pDescriptorUpdateEntries = entries;
	descriptorTemplateInfo.set = descriptorSetLayout->setIndex;

	VkValidateR(vkCreateDescriptorUpdateTemplate(device, &descriptorTemplateInfo, allocationCallbacks, &descriptorSetLayout->updateTemplate));

	return true;
}

void Renderer::PushDescriptors(Shader* shader)
{
	CommandBuffer* commandBuffer = GetCommandBuffer();

	if (pushDescriptorsSupported)
	{
		//vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer->commandBuffer, shader->setLayouts[0]->updateTemplate, shader->pipelineLayout, 0, shader->descriptors);
	}
	else
	{
		VkDescriptorSet sets[]{ nullptr, Resources::bindlessDescriptorSet };
		U32 firstSet = 0;

		if (shader->descriptorCount)
		{
			VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };

			allocateInfo.descriptorPool = descriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &shader->setLayouts[0]->descriptorSetLayout;

			VkValidate(vkAllocateDescriptorSets(device, &allocateInfo, sets));

			vkUpdateDescriptorSetWithTemplate(device, sets[0], shader->setLayouts[0]->updateTemplate, shader->descriptors);
		}
		else { firstSet = 1; }

		commandBuffer->BindDescriptorSets(shader, firstSet, shader->descriptorCount + shader->useBindless, sets + firstSet);
	}
};

bool Renderer::CreateRenderpass(Renderpass* renderpass)
{
	VkAttachmentDescription attachments[MAX_IMAGE_OUTPUTS + 1]{};
	VkAttachmentReference colorAttachments[MAX_IMAGE_OUTPUTS]{};
	VkAttachmentReference depthAttachment{};

	//TODO: Support multiple subpasses
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkImageLayout initialColorLayout = renderpass->colorOperation == VK_ATTACHMENT_LOAD_OP_LOAD ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout initialDepthLayout = renderpass->depthOperation == VK_ATTACHMENT_LOAD_OP_LOAD ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;

	U32 attachmentCount = 0;
	for (U32 i = 0; i < renderpass->renderTargetCount; ++i)
	{
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = renderpass->outputTextures[i]->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = renderpass->colorOperation;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = renderpass->stencilOperation;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = initialColorLayout;
		attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		colorAttachments[attachmentCount].attachment = attachmentCount;
		colorAttachments[attachmentCount].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		++attachmentCount;
	}

	subpass.colorAttachmentCount = attachmentCount;
	subpass.pColorAttachments = colorAttachments;

	if (renderpass->outputDepth)
	{
		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = renderpass->outputDepth->format;
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = renderpass->depthOperation;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = renderpass->stencilOperation;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentCount].initialLayout = initialDepthLayout;
		attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthAttachment.attachment = attachmentCount;
		depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depthAttachment;

		++attachmentCount;
	}

	VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependencies[3]{};

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

	dependencies[2].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].dstSubpass = 0;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[2].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[2].dependencyFlags = 0;

	renderPassInfo.dependencyCount = 3;
	renderPassInfo.pDependencies = dependencies;

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

			texture->mipmaps[i] = nullptr;
		}

		texture->imageView = nullptr;

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
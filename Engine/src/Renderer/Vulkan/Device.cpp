#include "Device.hpp"

#include "Core/Logger.hpp"
#include "Core/Settings.hpp"
#include <Containers/Vector.hpp>
#include "Memory/Memory.hpp"

VkPhysicalDevice Device::physicalDevice;
VkDevice Device::logicalDevice;
SwapchainSupportInfo Device::swapchainSupport;

I32 Device::graphicsQueueIndex;
I32 Device::presentQueueIndex;
I32 Device::transferQueueIndex;
I32 Device::computeQueueIndex;
bool Device::supportsDeviceLocalHostVisible;

VkQueue Device::graphicsQueue;
VkQueue Device::presentQueue;
VkQueue Device::transferQueue;
VkQueue Device::computeQueue;

VkCommandPool Device::graphicsCommandPool;

VkPhysicalDeviceProperties Device::properties;
VkPhysicalDeviceFeatures Device::features;
VkPhysicalDeviceMemoryProperties Device::memory;

I32 Device::maxSamples;
VkFormat Device::depthFormat;
U8 Device::depthChannelCount;

bool Device::Initialize(RendererState* rendererState)
{
	if (!SelectPhysicalDevice(rendererState)) { return false; }

	Logger::Info("Creating logical device...");

	bool presentSharesGraphicsQueue = graphicsQueueIndex == presentQueueIndex;
	bool transferSharesGraphicsQueue = graphicsQueueIndex == transferQueueIndex;
	U32 indexCount = 1 + !presentSharesGraphicsQueue + !transferSharesGraphicsQueue;

	U32 indices[32];
	U8 index = 0;
	indices[index] = graphicsQueueIndex;
	if (!presentSharesGraphicsQueue) { indices[++index] = presentQueueIndex; }
	if (!transferSharesGraphicsQueue) { indices[++index] = transferQueueIndex; }

	VkDeviceQueueCreateInfo queueCreateInfos[32];
	for (U32 i = 0; i < indexCount; ++i)
	{
		queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[i].queueFamilyIndex = indices[i];
		queueCreateInfos[i].queueCount = 1;

		// TODO: Enable this for a future enhancement.
		//if (indices[i] == graphicsQueueIndex) {
		//    queueCreateInfos[i].queueCount = 2;
		//}
		queueCreateInfos[i].flags = 0;
		queueCreateInfos[i].pNext = 0;
		F32 queuePriority = 1.0f;
		queueCreateInfos[i].pQueuePriorities = &queuePriority;
	}

	// TODO: should be config driven
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;

	bool portabilityRequired = false;
	U32 availableExtensionCount = 0;
	VkExtensionProperties* availableExtensions = 0;
	VkCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &availableExtensionCount, 0));

	if (availableExtensionCount != 0)
	{
		availableExtensions = (VkExtensionProperties*)Memory::Allocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
		VkCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &availableExtensionCount, availableExtensions));
		for (U32 i = 0; i < availableExtensionCount; ++i)
		{
			if (strcmp(availableExtensions[i].extensionName, "VK_KHR_portability_subset") == 0)
			{
				Logger::Trace("Adding required extension 'VK_KHR_portability_subset'.");
				portabilityRequired = true;
				break;
			}

			//TODO:, check for maintenance3, descriptor indexing
		}
	}

	Memory::Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);

	Vector<const char*> extensionNames;
	extensionNames.Push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	if (portabilityRequired) { extensionNames.Push("VK_KHR_portability_subset"); }

	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.queueCreateInfoCount = indexCount;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = (U32)extensionNames.Size();
	deviceCreateInfo.ppEnabledExtensionNames = extensionNames.Data();
	deviceCreateInfo.flags = 0;

	VkPhysicalDeviceVulkan12Features features{};
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features.runtimeDescriptorArray = true;
	features.bufferDeviceAddress = true;
	features.descriptorIndexing = true;

	deviceCreateInfo.pNext = &features;

	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;

	VkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, rendererState->allocator, &logicalDevice));

	vkGetDeviceQueue(logicalDevice, graphicsQueueIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, presentQueueIndex, 0, &presentQueue);
	vkGetDeviceQueue(logicalDevice, transferQueueIndex, 0, &transferQueue);
	vkGetDeviceQueue(logicalDevice, computeQueueIndex, 0, &computeQueue);

	VkCommandPoolCreateInfo poolCreateInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCheck(vkCreateCommandPool(logicalDevice, &poolCreateInfo, rendererState->allocator, &graphicsCommandPool));

	return true;
}

void Device::Shutdown(RendererState* rendererState)
{
	graphicsQueue = VK_NULL_HANDLE;
	presentQueue = VK_NULL_HANDLE;
	transferQueue = VK_NULL_HANDLE;
	computeQueue = VK_NULL_HANDLE;

	Logger::Info("Destroying vulkan command pools...");
	vkDestroyCommandPool(logicalDevice, graphicsCommandPool, rendererState->allocator);

	Logger::Info("Destroying vulkan logical device...");
	if (logicalDevice)
	{
		vkDestroyDevice(logicalDevice, rendererState->allocator);
		logicalDevice = VK_NULL_HANDLE;
	}

	Logger::Info("Releasing physical device resources...");
	physicalDevice = VK_NULL_HANDLE;

	swapchainSupport.formats.Clear();
	swapchainSupport.presentModes.Clear();

	Memory::Zero(&swapchainSupport.capabilities, sizeof(swapchainSupport.capabilities));

	graphicsQueueIndex = -1;
	presentQueueIndex = -1;
	transferQueueIndex = -1;
	computeQueueIndex = -1;
}

bool Device::SelectPhysicalDevice(RendererState* rendererState)
{
	Logger::Info("Selecting physical device...");

	U32 physicalDeviceCount = 0;
	VkCheck(vkEnumeratePhysicalDevices(rendererState->instance, &physicalDeviceCount, 0));
	if (physicalDeviceCount == 0)
	{
		Logger::Fatal("No devices which support Vulkan were found.");
		return false;
	}

	VkPhysicalDevice physicalDevices[32];
	VkCheck(vkEnumeratePhysicalDevices(rendererState->instance, &physicalDeviceCount, physicalDevices));
	for (U32 i = 0; i < physicalDeviceCount; ++i)
	{
		vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

		vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

		vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

		Logger::Trace("Evaluating device: {}, index {}.", properties.deviceName, i);

		supportsDeviceLocalHostVisible = false;
		for (U32 i = 0; i < memory.memoryTypeCount; ++i)
		{
			if (((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
				((memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0))
			{
				supportsDeviceLocalHostVisible = true;
				break;
			}
		}

		// TODO: These requirements should probably be driven by engine
		// configuration.
		PhysicalDeviceRequirements requirements = {};
		requirements.graphics = true;
		requirements.present = true;
		requirements.transfer = true;
		requirements.compute = true;
		requirements.samplerAnisotropy = true; //TODO: Settings
#if PLATFORM_APPLE
		requirements.discreteGpu = false;
#else
		requirements.discreteGpu = true;
#endif
		requirements.deviceExtensionNames.Push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		PhysicalDeviceQueueFamilyInfo queueInfo = {};
		physicalDevice = physicalDevices[i];
		bool result = PhysicalDeviceMeetsRequirements(rendererState->surface, requirements, queueInfo, swapchainSupport);

		if (result)
		{
#if LOG_TRACE_ENABLED
			Logger::Trace("Selected device: '{}'.", properties.deviceName);
			switch (properties.deviceType)
			{
			default:
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				Logger::Trace("GPU type is Unknown.");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				Logger::Trace("GPU type is Integrated.");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				Logger::Trace("GPU type is Descrete.");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				Logger::Trace("GPU type is Virtual.");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				Logger::Trace("GPU type is CPU.");
				break;
			}

			Logger::Trace("GPU Driver version: {}.{}.{}",
				VK_VERSION_MAJOR(properties.driverVersion),
				VK_VERSION_MINOR(properties.driverVersion),
				VK_VERSION_PATCH(properties.driverVersion));

			Logger::Trace("Vulkan API version: {}.{}.{}",
				VK_VERSION_MAJOR(properties.apiVersion),
				VK_VERSION_MINOR(properties.apiVersion),
				VK_VERSION_PATCH(properties.apiVersion));

			for (U32 j = 0; j < memory.memoryHeapCount; ++j)
			{
				F32 memorySizeGb = (((F32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
				if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				{
					Logger::Trace("Local GPU memory: {} GB", memorySizeGb);
				}
				else
				{
					Logger::Trace("Shared System memory: {} GB", memorySizeGb);
				}
			}
#endif
			graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
			presentQueueIndex = queueInfo.presentFamilyIndex;
			transferQueueIndex = queueInfo.transferFamilyIndex;
			computeQueueIndex = queueInfo.computeFamilyIndex;

			VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
			if (counts & VK_SAMPLE_COUNT_64_BIT) { maxSamples = VK_SAMPLE_COUNT_64_BIT; }
			else if (counts & VK_SAMPLE_COUNT_32_BIT) { maxSamples = VK_SAMPLE_COUNT_32_BIT; }
			else if (counts & VK_SAMPLE_COUNT_16_BIT) { maxSamples = VK_SAMPLE_COUNT_16_BIT; }
			else if (counts & VK_SAMPLE_COUNT_8_BIT) { maxSamples = VK_SAMPLE_COUNT_8_BIT; }
			else if (counts & VK_SAMPLE_COUNT_4_BIT) { maxSamples = VK_SAMPLE_COUNT_4_BIT; }
			else if (counts & VK_SAMPLE_COUNT_2_BIT) { maxSamples = VK_SAMPLE_COUNT_2_BIT; }
			else { maxSamples = VK_SAMPLE_COUNT_1_BIT; }
			
			break;
		}
	}

	if (!physicalDevice)
	{
		Logger::Error("No physical devices were found which meet the requirements.");
		return false;
	}

	return true;
}

bool Device::PhysicalDeviceMeetsRequirements(VkSurfaceKHR surface, const PhysicalDeviceRequirements& requirements, 
	PhysicalDeviceQueueFamilyInfo& outQueueInfo, SwapchainSupportInfo& outSwapchainSupport)
{
	outQueueInfo.graphicsFamilyIndex = -1;
	outQueueInfo.presentFamilyIndex = -1;
	outQueueInfo.computeFamilyIndex = -1;
	outQueueInfo.transferFamilyIndex = -1;

	if (requirements.discreteGpu)
	{
		if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			Logger::Trace("Device is not a discrete GPU, and one is required. Skipping.");
			return false;
		}
	}

	U32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, 0);
	VkQueueFamilyProperties queueFamilies[32];
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

	U8 minTransferScore = 255;
	for (U32 i = 0; i < queueFamilyCount; ++i)
	{
		U8 currentTransferScore = 0;

		if (outQueueInfo.graphicsFamilyIndex == -1 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			outQueueInfo.graphicsFamilyIndex = i;
			++currentTransferScore;

			VkBool32 supportsPresent = VK_FALSE;
			VkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent));
			if (supportsPresent)
			{
				outQueueInfo.presentFamilyIndex = i;
				++currentTransferScore;
			}
		}

		if (outQueueInfo.computeFamilyIndex == -1 && queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			outQueueInfo.computeFamilyIndex = i;
			++currentTransferScore;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			if (currentTransferScore <= minTransferScore)
			{
				minTransferScore = currentTransferScore;
				outQueueInfo.transferFamilyIndex = i;
			}
		}
	}

	if (outQueueInfo.presentFamilyIndex == -1)
	{
		for (U32 i = 0; i < queueFamilyCount; ++i)
		{
			VkBool32 supportsPresent = VK_FALSE;
			VkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent));
			if (supportsPresent)
			{
				outQueueInfo.presentFamilyIndex = i;

				if (outQueueInfo.presentFamilyIndex != outQueueInfo.graphicsFamilyIndex)
				{
					Logger::Warn("Different queue index used for present vs graphics: {}.", i);
				}
				break;
			}
		}
	}

	Logger::Trace("Graphics | Present | Compute | Transfer | Name");
	Logger::Trace("       {} |       {} |       {} |        {} | {}",
		outQueueInfo.graphicsFamilyIndex != -1,
		outQueueInfo.presentFamilyIndex != -1,
		outQueueInfo.computeFamilyIndex != -1,
		outQueueInfo.transferFamilyIndex != -1,
		properties.deviceName);

	if ((!requirements.graphics || (requirements.graphics && outQueueInfo.graphicsFamilyIndex != -1)) &&
		(!requirements.present || (requirements.present && outQueueInfo.presentFamilyIndex != -1)) &&
		(!requirements.compute || (requirements.compute && outQueueInfo.computeFamilyIndex != -1)) &&
		(!requirements.transfer || (requirements.transfer && outQueueInfo.transferFamilyIndex != -1)))
	{
		Logger::Trace("Device meets queue requirements.");
		Logger::Trace("Graphics Family Index: {}", outQueueInfo.graphicsFamilyIndex);
		Logger::Trace("Present Family Index:  {}", outQueueInfo.presentFamilyIndex);
		Logger::Trace("Transfer Family Index: {}", outQueueInfo.transferFamilyIndex);
		Logger::Trace("Compute Family Index:  {}", outQueueInfo.computeFamilyIndex);

		QuerySwapchainSupport(surface);

		if (outSwapchainSupport.formats.Size() < 1 || outSwapchainSupport.presentModes.Size() < 1)
		{
			outSwapchainSupport.formats.Clear();
			outSwapchainSupport.presentModes.Clear();
			Logger::Trace("Required swapchain support not present, skipping device.");
			return false;
		}

		if (requirements.deviceExtensionNames.Size())
		{
			U32 availableExtensionCount = 0;
			VkExtensionProperties* availableExtensions = 0;
			VkCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &availableExtensionCount, 0));

			if (availableExtensionCount != 0)
			{
				availableExtensions = (VkExtensionProperties*)Memory::Allocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
				VkCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &availableExtensionCount, availableExtensions));

				U32 requiredExtensionCount = (U32)requirements.deviceExtensionNames.Size();
				for (U32 i = 0; i < requiredExtensionCount; ++i)
				{
					bool found = false;
					for (U32 j = 0; j < availableExtensionCount; ++j)
					{
						if (strcmp(requirements.deviceExtensionNames[i], availableExtensions[j].extensionName) == 0)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						Logger::Trace("Required extension not found: {}, skipping device.", requirements.deviceExtensionNames[i]);
						Memory::Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
						return false;
					}
				}
			}
			Memory::Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
		}

		if (requirements.samplerAnisotropy && !features.samplerAnisotropy)
		{
			Logger::Trace("Device does not support samplerAnisotropy, skipping.");
			return false;
		}

		return true;
	}

	return false;
}

void Device::QuerySwapchainSupport(VkSurfaceKHR surface)
{
	VkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainSupport.capabilities));

	U32 size;
	VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &size, nullptr));

	if (size)
	{
		swapchainSupport.formats.Resize(size);
		VkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &size, swapchainSupport.formats.Data()));
	}

	VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &size, nullptr));
	if (size)
	{
		swapchainSupport.presentModes.Resize(size);
		VkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &size, (VkPresentModeKHR*)swapchainSupport.presentModes.Data()));
	}
}

bool Device::DetectDepthFormat()
{
	static const U64 candidateCount = 3;
	static const VkFormat candidates[3] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	static const U8 sizes[3] = { 4, 4, 3 };
	static const U32 depthFlags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (U64 i = 0; i < candidateCount; ++i)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &properties);

		if ((properties.linearTilingFeatures & depthFlags) == depthFlags)
		{
			depthFormat = candidates[i];
			depthChannelCount = sizes[i];

			return true;
		}
		else if ((properties.optimalTilingFeatures & depthFlags) == depthFlags)
		{
			depthFormat = candidates[i];
			depthChannelCount = sizes[i];

			return true;
		}
	}

	return false;
}
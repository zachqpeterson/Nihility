#include "Device.hpp"

#include "Renderer.hpp"
#include "Platform/Platform.hpp"

Device::CustomQueueDescription::CustomQueueDescription(U32 index, Vector<F32> priorities) : index(index), priorities(Move(priorities)) { }

bool Device::Create()
{
	if (!CreateSurface()) { Logger::Fatal("Failed To Create Vulkan Surface!"); return false; }
	if (!SelectPhysicalDevice()) { return false; }

	Vector<CustomQueueDescription> queueDescriptions;

	for (uint32_t i = 0; i < physicalDevice.queueFamilies.Size(); ++i)
	{
		queueDescriptions.Emplace(i, Vector<F32>{ 1.0f });
	}

	Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (auto& desc : queueDescriptions)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCreateInfo.queueFamilyIndex = desc.index;
		queueCreateInfo.queueCount = (U32)desc.priorities.Size();
		queueCreateInfo.pQueuePriorities = desc.priorities.Data();
		queueCreateInfos.Push(queueCreateInfo);
	}

	bindlessSupported = physicalDevice.indexingFeatures.descriptorBindingPartiallyBound && physicalDevice.indexingFeatures.runtimeDescriptorArray;

	VkPhysicalDeviceFeatures2 features2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	features2.features.multiDrawIndirect = true;
	features2.features.drawIndirectFirstInstance = true;
	features2.features.pipelineStatisticsQuery = true;
	features2.features.shaderFloat64 = true;
	features2.features.shaderInt16 = true;
	features2.features.shaderInt64 = true; 
	features2.features.samplerAnisotropy = true;

	VkPhysicalDeviceVulkan11Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	features11.pNext = &features2;
	features11.storageBuffer16BitAccess = true;
	features11.shaderDrawParameters = true;

	VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.pNext = &features11;
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
	features13.pNext = &features12;
	features13.dynamicRendering = true;
	features13.synchronization2 = true;
	features13.maintenance4 = true;

	Vector<const C8*> extensionsToEnable{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.pNext = &features13;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = (U32)queueCreateInfos.Size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.Data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = (U32)extensionsToEnable.Size();
	deviceCreateInfo.ppEnabledExtensionNames = extensionsToEnable.Data();
	deviceCreateInfo.pEnabledFeatures = nullptr;

	VkValidateFR(vkCreateDevice(physicalDevice, &deviceCreateInfo, Renderer::allocationCallbacks, &vkDevice));

	return true;
}

void Device::Destroy()
{
	if (vkDevice) { vkDestroyDevice(vkDevice, Renderer::allocationCallbacks); }
	if (vkSurface) { vkDestroySurfaceKHR(Renderer::instance, vkSurface, Renderer::allocationCallbacks); }

	vkDevice = nullptr;
	vkSurface = nullptr;
}

bool Device::CreateSurface()
{
#ifdef NH_PLATFORM_WINDOWS
	WindowInfo wd = Platform::GetWindowInfo();
	VkWin32SurfaceCreateInfoKHR surfaceInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.pNext = nullptr;
	surfaceInfo.flags = 0;
	surfaceInfo.hinstance = wd.instance;
	surfaceInfo.hwnd = wd.window;

	VkValidateFR(vkCreateWin32SurfaceKHR(Renderer::instance, &surfaceInfo, Renderer::allocationCallbacks, &vkSurface));
#elif NH_PLATFORM_LINUX
	//TODO:
#elif NH_PLATFORM_APPLE
	//TODO:
#endif

	return true;
}

bool Device::SelectPhysicalDevice()
{
	U32 physicalDeviceCount;
	VkValidateFR(vkEnumeratePhysicalDevices(Renderer::instance, &physicalDeviceCount, nullptr));
	Vector<VkPhysicalDevice> vkPhysicalDevices(physicalDeviceCount, {});
	VkValidateFR(vkEnumeratePhysicalDevices(Renderer::instance, &physicalDeviceCount, vkPhysicalDevices.Data()));

	if (vkPhysicalDevices.Size() == 0)
	{
		Logger::Fatal("Failed To Find A Valid Physical Device!");
		return false;
	}

	U32 best = U32_MAX;
	Vector<PhysicalDevice> physicalDevices;
	for (VkPhysicalDevice& vkPhysicalDevice : vkPhysicalDevices)
	{
		PhysicalDevice physicalDevice(vkPhysicalDevice, vkSurface);
		if (physicalDevice.suitable)
		{
			if (best == U32_MAX && physicalDevice.discrete) { best = (U32)physicalDevices.Size(); }
			physicalDevices.Push(Move(physicalDevice));
		}
	}

	if (physicalDevices.Size() == 0)
	{
		Logger::Fatal("Failed To Find A Valid Physical Device!");
		return false;
	}

	if (best != U32_MAX) { physicalDevice = Move(physicalDevices[best]); }
	else { physicalDevice = Move(physicalDevices[0]); }

	return true;
}

U32 Device::GetQueueIndex(QueueType type) const
{
	switch (type)
	{
	case QueueType::Present: return physicalDevice.GetPresentQueueIndex(vkSurface);
	case QueueType::Graphics: return physicalDevice.GetFirstQueueIndex(VK_QUEUE_GRAPHICS_BIT);
	case QueueType::Compute: return physicalDevice.GetSeparateQueueIndex(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT);
	case QueueType::Transfer: return physicalDevice.GetSeparateQueueIndex(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT);
	}

	return U32_MAX;
}

U32 Device::GetDedicatedQueueIndex(QueueType type) const
{
	switch (type)
	{
	case QueueType::Compute: return physicalDevice.GetDedicatedQueueIndex(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT);
	case QueueType::Transfer: return physicalDevice.GetDedicatedQueueIndex(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT);
	}

	return U32_MAX;
}

VkQueue Device::GetQueue(QueueType type) const
{
	U32 index = GetQueueIndex(type);
	if (index == U32_MAX) { return nullptr; }

	VkQueue outQueue;
	vkGetDeviceQueue(vkDevice, index, 0, &outQueue);

	return outQueue;
}

VkQueue Device::GetDedicatedQueue(QueueType type) const
{
	U32 index = GetDedicatedQueueIndex(type);
	if (index == U32_MAX) { return nullptr; }

	VkQueue outQueue;
	vkGetDeviceQueue(vkDevice, index, 0, &outQueue);

	return outQueue;
}

Device::operator VkDevice() const
{
	return vkDevice;
}
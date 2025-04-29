#include "PhysicalDevice.hpp"

#include "Renderer.hpp"

PhysicalDevice::PhysicalDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface) : vkPhysicalDevice(vkPhysicalDevice), suitable(false)
{
	U32 queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
	queueFamilies.Resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.Data());

	vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);
	vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &features);
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);

	features2.pNext = &indexingFeatures;
	vkGetPhysicalDeviceFeatures2(vkPhysicalDevice, &features2);

	if (properties.apiVersion < VK_API_VERSION_1_3) { return; }
	if (!features.samplerAnisotropy || !features.wideLines) { return; }

	discrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	U32 propertyCount;
	vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &propertyCount, nullptr);
	availableExtensions.Resize(propertyCount);
	vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &propertyCount, availableExtensions.Data());

	dedicatedComputeQueueIndex = GetDedicatedQueueIndex(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT);
	dedicatedTransferQueueIndex = GetDedicatedQueueIndex(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT);
	separateComputeQueueIndex = GetSeparateQueueIndex(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT);
	separateTransferQueueIndex = GetSeparateQueueIndex(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_COMPUTE_BIT);

	presentQueueIndex = GetPresentQueueIndex(vkSurface);

	if (presentQueueIndex == U32_MAX) { return; }

	Vector<String> requiredExtensions{};

	if (!CheckDeviceExtensionSupport(requiredExtensions)) { return; }

	U32 surfaceFormatCount;
	VkValidateExit(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &surfaceFormatCount, nullptr));
	surfaceFormats.Resize(surfaceFormatCount);
	VkValidateExit(vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &surfaceFormatCount, surfaceFormats.Data()));

	U32 presentModeCount;
	VkValidateExit(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, nullptr));
	presentModes.Resize(presentModeCount);
	VkValidateExit(vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, presentModes.Data()));

	suitable = true;
}

PhysicalDevice::PhysicalDevice(PhysicalDevice&& other) noexcept : 
	vkPhysicalDevice(other.vkPhysicalDevice), queueFamilies(Move(other.queueFamilies)), availableExtensions(Move(other.availableExtensions)), surfaceFormats(Move(other.surfaceFormats)), presentModes(Move(other.presentModes)),
	features(other.features), features2(other.features2), properties(other.properties), memoryProperties(other.memoryProperties),
	dedicatedComputeQueueIndex(other.dedicatedComputeQueueIndex), dedicatedTransferQueueIndex(other.dedicatedTransferQueueIndex),
	separateComputeQueueIndex(other.separateComputeQueueIndex), separateTransferQueueIndex(other.separateTransferQueueIndex),
	presentQueueIndex(other.presentQueueIndex), suitable(other.suitable), discrete(other.discrete) { }

PhysicalDevice& PhysicalDevice::operator=(PhysicalDevice&& other) noexcept
{
	vkPhysicalDevice = other.vkPhysicalDevice;
	queueFamilies = Move(other.queueFamilies);
	availableExtensions = Move(other.availableExtensions);
	surfaceFormats = Move(other.surfaceFormats);
	presentModes = Move(other.presentModes);
	features = other.features;
	features2 = other.features2;
	properties = other.properties;
	memoryProperties = other.memoryProperties;
	dedicatedComputeQueueIndex = other.dedicatedComputeQueueIndex;
	dedicatedTransferQueueIndex = other.dedicatedTransferQueueIndex;
	separateComputeQueueIndex = other.separateComputeQueueIndex;
	separateTransferQueueIndex = other.separateTransferQueueIndex;
	presentQueueIndex = other.presentQueueIndex;
	suitable = other.suitable;
	discrete = other.discrete;

	return *this;
}

PhysicalDevice::~PhysicalDevice()
{
	queueFamilies.Destroy();
	availableExtensions.Destroy();
	surfaceFormats.Destroy();
	presentModes.Destroy();
}

U32 PhysicalDevice::GetFirstQueueIndex(VkQueueFlags desiredFlags) const
{
	U32 i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		if ((queueFamily.queueFlags & desiredFlags) == desiredFlags) { return i; }
		++i;
	}

	return U32_MAX;
}

U32 PhysicalDevice::GetDedicatedQueueIndex(VkQueueFlags desiredFlags, VkQueueFlags undesiredFlags) const
{
	U32 i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		if ((queueFamily.queueFlags & desiredFlags) == desiredFlags &&
			(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 && 
			(queueFamily.queueFlags & undesiredFlags) == 0)
		{
			return i;
		}

		++i;
	}

	return U32_MAX;
}

U32 PhysicalDevice::GetSeparateQueueIndex(VkQueueFlags desiredFlags, VkQueueFlags undesiredFlags) const
{
	U32 index = U32_MAX;
	U32 i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		if ((queueFamily.queueFlags & desiredFlags) == desiredFlags && 
			(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
		{
			if ((queueFamily.queueFlags & undesiredFlags) == 0) { return i; }
			else { index = i; }
		}

		++i;
	}

	return index;
}

U32 PhysicalDevice::GetPresentQueueIndex(VkSurfaceKHR vkSurface) const
{
	for (U32 i = 0; i < (U32)queueFamilies.Size(); ++i)
	{
		VkBool32 presentSupport = VK_FALSE;
		VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);
		if (res != VK_SUCCESS) { return U32_MAX; }
		if (presentSupport == VK_TRUE) { return i; }
	}
	return U32_MAX;
}

bool PhysicalDevice::CheckDeviceExtensionSupport(const Vector<String>& requiredExtensions) const
{
	for (const String& reqExt : requiredExtensions)
	{
		bool found = false;
		for (const VkExtensionProperties& availExt : availableExtensions)
		{
			if (reqExt == availExt.extensionName)
			{
				found = true;
				break;
			}
		}

		if (!found) { return false; }
	}

	return true;
}

PhysicalDevice::operator VkPhysicalDevice() const
{
	return vkPhysicalDevice;
}
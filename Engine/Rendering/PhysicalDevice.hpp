#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"

struct PhysicalDevice
{
public:
	PhysicalDevice() = default;
	PhysicalDevice(PhysicalDevice&& other) noexcept;
	PhysicalDevice& operator=(PhysicalDevice&& other) noexcept;

private:
	PhysicalDevice(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface);
	~PhysicalDevice();

	U32 GetFirstQueueIndex(VkQueueFlags desiredFlags) const;
	U32 GetDedicatedQueueIndex(VkQueueFlags desiredFlags, VkQueueFlags undesiredFlags) const;
	U32 GetSeparateQueueIndex(VkQueueFlags desiredFlags, VkQueueFlags undesiredFlags) const;
	U32 GetPresentQueueIndex(VkSurfaceKHR vkSurface) const;
	bool CheckDeviceExtensionSupport(const Vector<String>& requiredExtensions) const;

	operator VkPhysicalDevice() const;

	VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
	Vector<VkQueueFamilyProperties> queueFamilies;
	Vector<VkExtensionProperties> availableExtensions;
	Vector<VkSurfaceFormatKHR> surfaceFormats;
	Vector<VkPresentModeKHR> presentModes;

	VkPhysicalDeviceFeatures features{};
	VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceMemoryProperties memoryProperties{};

	U32 dedicatedComputeQueueIndex;
	U32 dedicatedTransferQueueIndex;
	U32 separateComputeQueueIndex;
	U32 separateTransferQueueIndex;
	U32 presentQueueIndex;

	bool suitable;
	bool discrete;

	friend class Renderer;
	friend struct Device;
	friend struct Swapchain;
	friend struct CommandBuffer;
};
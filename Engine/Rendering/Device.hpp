#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"
#include "PhysicalDevice.hpp"

enum class QueueType
{
	Present,
	Graphics,
	Compute,
	Transfer
};

struct Device
{
public:
	operator VkDevice() const;

private:
	struct CustomQueueDescription
	{
		explicit CustomQueueDescription(U32 index, Vector<F32> priorities);
		U32 index;
		Vector<F32> priorities;
	};

private:
	bool Create();
	void Destroy();

	bool CreateSurface();
	bool SelectPhysicalDevice();

	U32 GetQueueIndex(QueueType type) const;
	U32 GetDedicatedQueueIndex(QueueType type) const;
	VkQueue GetQueue(QueueType type) const;
	VkQueue GetDedicatedQueue(QueueType type) const;

	VkDevice vkDevice = VK_NULL_HANDLE;
	PhysicalDevice physicalDevice;
	VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
	Vector<VkQueueFamilyProperties> queueFamilies;

	bool bindlessSupported;

	friend class Renderer;
	friend struct Swapchain;
};
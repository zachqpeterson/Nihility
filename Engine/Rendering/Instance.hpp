#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

struct Instance
{
public:
	bool Create();
	void Destroy();

	VkInstance vkInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	operator VkInstance() const;
};
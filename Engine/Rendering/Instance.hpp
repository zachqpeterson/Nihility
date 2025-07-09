#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;

struct Instance
{
public:
	bool Create(const StringView& name, U32 version);
	void Destroy();

	VkInstance_T* vkInstance = nullptr;
	VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;

	operator VkInstance_T* () const;
};
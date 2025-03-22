#pragma once

#include "Defines.hpp"

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkAllocationCallbacks_T;
struct VkDebugUtilsMessengerCallbackDataEXT;
enum VkDebugUtilsMessageSeverityFlagBitsEXT;

struct Instance
{
public:
	bool Create();
	void Destroy();

	VkInstance_T* instance = nullptr;
	VkDebugUtilsMessengerEXT_T* debugMessenger = nullptr;

	operator VkInstance_T*() const;
};
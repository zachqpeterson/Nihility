#pragma once

#include "Defines.hpp"

struct VkInstance_T;
struct VkSurfaceKHR_T;
struct VkAllocationCallbacks;

class NH_API Renderer
{
public:


private:
	static bool Initialize(CSTR applicationName, U32 applicationVersion);
	static void Shutdown();
	static void Update();

	static bool CreateInstance();
	static bool CreateAllocator();

private:
	static CSTR appName;
	static U32 appVersion;
	static VkInstance_T* instance;
	static VkSurfaceKHR_T* surface;

	static VkAllocationCallbacks* allocator;

	STATIC_CLASS(Renderer);
	friend class Engine;
};
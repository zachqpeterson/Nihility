#pragma once

#include "Defines.hpp"

#include "Instance.hpp"

struct VkAllocationCallbacks;

class NH_API Renderer
{
public:


private:
	static bool Initialize();
	static void Shutdown();

	static VkAllocationCallbacks* allocationCallbacks;
	static Instance instance;

	friend class Engine;
	friend struct Instance;

	STATIC_CLASS(Renderer);
};
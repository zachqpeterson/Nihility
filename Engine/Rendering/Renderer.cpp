#include "Renderer.hpp"

#include "VulkanInclude.hpp"

Instance Renderer::instance;
VkAllocationCallbacks* Renderer::allocationCallbacks = nullptr;

bool Renderer::Initialize()
{
	instance.Create();

	return true;
}

void Renderer::Shutdown()
{
	instance.Destroy();
}
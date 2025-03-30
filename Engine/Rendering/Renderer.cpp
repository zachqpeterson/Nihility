#include "Renderer.hpp"

#include "VulkanInclude.hpp"

Instance Renderer::instance;
VkAllocationCallbacks* Renderer::allocationCallbacks = nullptr;

bool Renderer::Initialize()
{
	Logger::Trace("Initializing Renderer...");

	instance.Create();

	return true;
}

void Renderer::Shutdown()
{
	Logger::Trace("Cleaning Up Renderer...");

	instance.Destroy();
}
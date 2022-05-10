#include "VulkanRenderer.hpp"

#include "Memory/Memory.hpp"

#include <vulkan/vulkan.hpp>

struct RendererState
{

};

static RendererState* rendererState;

bool VulkanRenderer::Initialize()
{
    rendererState = (RendererState*)Memory::Allocate(sizeof(RendererState), MEMORY_TAG_RENDERER);

    return true;
}

void VulkanRenderer::Shutdown()
{
    //TODO: Clean up vulkan

    Memory::Free(rendererState, sizeof(RendererState), MEMORY_TAG_RENDERER);
}

void* VulkanRenderer::operator new(U64 size)
{
    return Memory::Allocate(size, MEMORY_TAG_RENDERER);
}

void VulkanRenderer::operator delete(void* p)
{
    Memory::Free(p, sizeof(VulkanRenderer), MEMORY_TAG_RENDERER);
}
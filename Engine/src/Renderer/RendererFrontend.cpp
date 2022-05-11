#include "RendererFrontend.hpp"

#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"
#include "Vulkan/VulkanRenderer.hpp"

struct RendererFrontendState
{
    Renderer* renderer;
};

static RendererFrontendState* frontendState;


bool RendererFrontend::Initialize(void* state)
{
    frontendState = (RendererFrontendState*)state;

    //Try vulkan
    frontendState->renderer = new VulkanRenderer();
    if (frontendState->renderer->Initialize()) { return true; }

    delete frontendState->renderer;
    LOG_ERROR("Vulkan isn't supported of this machine!");

    //If windows, try DirectX

    //Try OpenGL


    return false;
}

void* RendererFrontend::Shutdown()
{
    frontendState->renderer->Shutdown();
    delete frontendState->renderer;

    return frontendState;
}

const U64 RendererFrontend::GetMemoryRequirements()
{
    return sizeof(RendererFrontendState);
}
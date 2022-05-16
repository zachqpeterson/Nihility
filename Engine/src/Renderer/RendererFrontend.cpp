#include "RendererFrontend.hpp"

#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"
#include "Vulkan/VulkanRenderer.hpp"

struct FrontendState
{
    Renderer* renderer;
};

static FrontendState* frontendState;

bool RendererFrontend::Initialize(void* state)
{
    frontendState = (FrontendState*)state;

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

bool RendererFrontend::DrawFrame()
{
    ++frontendState->renderer->frameNumber;

    return
        frontendState->renderer->BeginFrame() &&
        frontendState->renderer->BeginRenderpass(0) &&
        frontendState->renderer->EndRenderpass(0) &&
        frontendState->renderer->EndFrame();
}

const U64 RendererFrontend::GetMemoryRequirements()
{
    return sizeof(FrontendState);
}
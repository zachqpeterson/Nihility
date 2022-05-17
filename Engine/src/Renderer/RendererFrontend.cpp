#include "RendererFrontend.hpp"

#include "Core/Logger.hpp"
#include "Memory/Memory.hpp"
#include "Containers/String.hpp"
#include "Vulkan/VulkanRenderer.hpp"
#include "Resources/Shader.hpp"

Renderer* RendererFrontend::renderer;

bool RendererFrontend::Initialize()
{
    //Try vulkan
    renderer = new VulkanRenderer();
    if (renderer->Initialize()) { return true; }

    delete renderer;
    LOG_ERROR("Vulkan isn't supported on this machine!");

    //If windows, try DirectX

    //Try OpenGL

    //TODO: Load resources

    return false;
}

void RendererFrontend::Shutdown()
{
    renderer->Shutdown();
    delete renderer;
}

bool RendererFrontend::DrawFrame()
{
    ++renderer->frameNumber;

    return
        renderer->BeginFrame() &&
        renderer->BeginRenderpass(0) &&
        renderer->EndRenderpass(0) &&
        renderer->EndFrame();
}

U8 RendererFrontend::GetRenderpassId(const String& name)
{
    // TODO: HACK: Need dynamic renderpasses instead of hardcoding them.
    if (name.EqualsI("Renderpass.Builtin.World"))
    {
        return BUILTIN_RENDERPASS_WORLD;
    }
    else if (name.EqualsI("Renderpass.Builtin.UI"))
    {
        return BUILTIN_RENDERPASS_UI;
    }

    LOG_ERROR("GetRenderpassId: No renderpass named '%s'.", (const char*)name);
    return INVALID_ID_U8;
}

bool RendererFrontend::CreateShader(const Shader& shader, U8 renderpassId, U8 stageCount, const Vector<String>& stageFilenames, const Vector<ShaderStageType>& stages)
{
    return renderer->CreateShader(shader, renderpassId, stageCount, stageFilenames, stages);
}

void RendererFrontend::DestroyShader(const Shader& shader)
{
    renderer->DestroyShader(shader);
}

bool RendererFrontend::InitializeShader(const Shader& shader)
{
    return renderer->InitializeShader(shader);
}

bool RendererFrontend::UseShader(const Shader& shader)
{
    return renderer->UseShader(shader);
}

bool RendererFrontend::BindGlobals(const Shader& shader)
{
    return renderer->BindGlobals(shader);
}

bool RendererFrontend::BindInstance(const Shader& shader, U32 instanceId)
{
    return renderer->BindInstance(shader, instanceId);
}

bool RendererFrontend::ApplyGlobals(const Shader& shader)
{
    return renderer->ApplyGlobals(shader);
}

bool RendererFrontend::ApplyInstance(const Shader& shader, bool needsUpdate)
{
    return renderer->ApplyInstance(shader, needsUpdate);
}

U32 RendererFrontend::AcquireInstanceResources(const Shader& shader)
{
    return renderer->AcquireInstanceResources(shader);
}

bool RendererFrontend::ReleaseInstanceResources(const Shader& shader, U32 instanceId)
{
    return renderer->ReleaseInstanceResources(shader, instanceId);
}

bool RendererFrontend::SetUniform(Shader& shader, const ShaderUniform& uniform, const void* value)
{
    return renderer->SetUniform(shader, uniform, value);
}

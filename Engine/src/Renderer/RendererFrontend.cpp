#include "RendererFrontend.hpp"

#include "Camera.hpp"
#include "Scene.hpp"

#include "Core/Logger.hpp"
#include "Core/Events.hpp"
#include "Memory/Memory.hpp"
#include "Containers/String.hpp"
#include "Containers/Vector.hpp"
#include "Vulkan/VulkanRenderer.hpp"
#include "Resources/Resources.hpp"
#include "Math/Math.hpp"

Renderer* RendererFrontend::renderer;

bool RendererFrontend::resizing;
U8 RendererFrontend::framesSinceResize;
U8 RendererFrontend::windowRenderTargetCount;
U32 RendererFrontend::framebufferWidth;
U32 RendererFrontend::framebufferHeight;

Scene* RendererFrontend::activeScene;

bool RendererFrontend::Initialize(const String& applicationName, U32 width, U32 height)
{
    Events::Subscribe("Resize", OnResize);
    framebufferWidth = width;
    framebufferHeight = height;

    resizing = false;
    framesSinceResize = 0;

    //Try vulkan
    renderer = new VulkanRenderer();
    if (!renderer->Initialize(applicationName, windowRenderTargetCount))
    {
        delete renderer;
        Logger::Error("Vulkan isn't supported on this machine!");

        //If windows, try DirectX

        //Try OpenGL

        return false;
    }

    return true;
}

void RendererFrontend::Shutdown()
{
    renderer->Shutdown();
    delete renderer;
}

bool RendererFrontend::DrawFrame()
{
    //TODO: Temporary
    static bool done = false;

    if (!done)
    {
        activeScene = (Scene*)Memory::Allocate(sizeof(Scene), MEMORY_TAG_RENDERER);
        activeScene->Create();

        MeshConfig config;
        config.name = "Mesh";
        config.MaterialName = "Tile.mat";

        config.vertices.Push(Vertex{ {-0.5f, -0.5f, 0.0f}, { 0.0f, 0.125f }, });
        config.vertices.Push(Vertex{ { 0.5f, -0.5f, 0.0f}, { 0.16666666666f, 0.125f }, });
        config.vertices.Push(Vertex{ { 0.5f,  0.5f, 0.0f}, { 0.16666666666f, 0.0f }, });
        config.vertices.Push(Vertex{ {-0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f }, });

        config.indices.Push(0);
        config.indices.Push(1);
        config.indices.Push(2);
        config.indices.Push(2);
        config.indices.Push(3);
        config.indices.Push(0);

        Mesh* mesh = Resources::CreateMesh(config);

        activeScene->DrawMesh(mesh, Matrix4::IDENTITY);
        done = true;
    }

    ++renderer->frameNumber;

    if (resizing)
    {
        if (++framesSinceResize >= 30)
        {
            activeScene->OnResize(framebufferWidth, framebufferHeight);
            renderer->OnResize({ (I32)framebufferWidth, (I32)framebufferHeight });

            framesSinceResize = 0;
            resizing = false;
        }
        else
        {
            return true;
        }
    }

    if (renderer->BeginFrame())
    {
        U8 attachmentIndex = renderer->GetWindowAttachmentIndex();

        activeScene->OnRender(renderer->frameNumber, attachmentIndex);

        if (!renderer->EndFrame())
        {
            Logger::Error("End frame failed. Application shutting down...");
            return false;
        }
    }

    return true;
}

bool RendererFrontend::BeginRenderpass(Renderpass* renderpass)
{
    return renderer->BeginRenderpass(renderpass);
}

bool RendererFrontend::EndRenderpass(Renderpass* renderpass)
{
    return renderer->EndRenderpass(renderpass);
}

bool RendererFrontend::CreateMesh(Mesh* mesh, Vector<Vertex>& vertices, Vector<U32>& indices)
{
    return renderer->CreateMesh(mesh, vertices, indices);
}

void RendererFrontend::DestroyMesh(Mesh* mesh)
{
    renderer->DestroyMesh(mesh);
}

void RendererFrontend::DrawMesh(const struct MeshRenderData& Meshdata)
{
    renderer->DrawMesh(Meshdata);
}

void RendererFrontend::CreateTexture(Texture* texture, const Vector<U8>& pixels)
{
    renderer->CreateTexture(texture, pixels);
}

void RendererFrontend::DestroyTexture(Texture* texture)
{
    renderer->DestroyTexture(texture);
}

bool RendererFrontend::CreateWritableTexture(Texture* texture)
{
    return renderer->CreateWritableTexture(texture);
}

void RendererFrontend::WriteTextureData(Texture* texture, U32 offset, U32 size, const Vector<U8>& pixels)
{
    renderer->WriteTextureData(texture, offset, size, pixels);
}

void RendererFrontend::ResizeTexture(Texture* texture, U32 width, U32 height)
{
    renderer->ResizeTexture(texture, width, height);
}

bool RendererFrontend::AcquireTextureMapResources(TextureMap& map)
{
    return renderer->AcquireTextureMapResources(map);
}

void RendererFrontend::ReleaseTextureMapResources(TextureMap& map)
{
    renderer->ReleaseTextureMapResources(map);
}

void RendererFrontend::CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext)
{
    renderer->CreateRenderpass(renderpass, hasPrev, hasNext);
}

void RendererFrontend::DestroyRenderpass(Renderpass* renderpass)
{
    renderer->DestroyRenderpass(renderpass);
}

bool RendererFrontend::CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, RenderTarget* target)
{
    return renderer->CreateRenderTarget(attachments, renderpass, width, height, target);
}

bool RendererFrontend::DestroyRenderTarget(RenderTarget* target, bool freeInternalMemory)
{
    return renderer->DestroyRenderTarget(target, freeInternalMemory);
}

Texture* RendererFrontend::GetWindowAttachment(U8 index)
{
    return renderer->GetWindowAttachment(index);
}

Texture* RendererFrontend::GetDepthAttachment()
{
    return renderer->GetDepthAttachment();
}

U32 RendererFrontend::GetWindowAttachmentIndex()
{
    return renderer->GetWindowAttachmentIndex();
}

U8 RendererFrontend::WindowRenderTargetCount()
{
    return windowRenderTargetCount;
}

bool RendererFrontend::CreateShader(Shader* shader)
{
    return renderer->CreateShader(shader);
}

void RendererFrontend::DestroyShader(Shader* shader)
{
    renderer->DestroyShader(shader);
}

bool RendererFrontend::InitializeShader(Shader* shader)
{
    return renderer->InitializeShader(shader);
}

bool RendererFrontend::UseShader(Shader* shader)
{
    return renderer->UseShader(shader);
}

bool RendererFrontend::BindShaderInstance(Shader* shader, U32 instanceId)
{
    return renderer->BindInstance(shader, instanceId);
}

bool RendererFrontend::ApplyShaderGlobals(Shader* shader)
{
    return renderer->ApplyGlobals(shader);
}

bool RendererFrontend::ApplyShaderInstance(Shader* shader, bool needsUpdate)
{
    return renderer->ApplyInstance(shader, needsUpdate);
}

U32 RendererFrontend::AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps)
{
    return renderer->AcquireInstanceResources(shader, maps);
}

bool RendererFrontend::ReleaseInstanceResources(Shader* shader, U32 instanceId)
{
    return renderer->ReleaseInstanceResources(shader, instanceId);
}

bool RendererFrontend::SetUniform(Shader* shader, Uniform& uniform, const void* value)
{
    return renderer->SetUniform(shader, uniform, value);
}

bool RendererFrontend::SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value)
{
    return renderer->SetPushConstant(shader, pushConstant, value);
}

bool RendererFrontend::OnResize(void* data)
{
    Vector2Int size = *(Vector2Int*)data;
    framebufferWidth = size.x;
    framebufferHeight = size.y;
    framesSinceResize = 0;
    resizing = true;

    return true;
}
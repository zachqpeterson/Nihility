#pragma once

#include "Defines.hpp"

#include "Resources/Resources.hpp"

template<typename> struct Vector;

struct MeshRenderData
{
    Matrix4 model;
    Mesh* mesh;
};

class RendererFrontend
{
public:
    static bool Initialize(const String& applicationName);
    static void Shutdown();

    static bool DrawFrame();
    static NH_API bool BeginRenderpass(Renderpass* renderpass);
    static NH_API bool EndRenderpass(Renderpass* renderpass);

    static bool CreateMesh(Mesh* mesh, Vector<Vertex>& vertices, Vector<U32>& indices);
    static void DestroyMesh(Mesh* mesh);
    static void DrawMesh(const struct MeshRenderData& Meshdata);

    static void CreateTexture(Texture* texture, const Vector<U8>& pixels);
    static void DestroyTexture(Texture* texture);
    static bool CreateWritableTexture(Texture* texture);
    static void WriteTextureData(Texture* texture, U32 offset, U32 size, const Vector<U8>& pixels);
    static void ResizeTexture(Texture* texture, U32 width, U32 height);

    static bool AcquireTextureMapResources(TextureMap& map);
    static void ReleaseTextureMapResources(TextureMap& map);

    static void CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext);
    static void DestroyRenderpass(Renderpass* renderpass);

    static NH_API bool CreateRenderTarget(Vector<Texture*>& attachments, struct Renderpass* renderpass, U32 width, U32 height, struct RenderTarget* target);
    static NH_API bool DestroyRenderTarget(struct RenderTarget* target, bool freeInternalMemory);
    static NH_API Texture* GetWindowAttachment(U8 index);
    static NH_API Texture* GetDepthAttachment();
    static U32 GetWindowAttachmentIndex();
    static U8 WindowRenderTargetCount();

    static bool CreateShader(Shader* shader);
    static void DestroyShader(Shader* shader);
    static bool InitializeShader(Shader* shader);
    static NH_API bool UseShader(Shader* shader);
    static bool BindShaderInstance(Shader* shader, U32 instanceId);
    static bool ApplyShaderGlobals(Shader* shader);
    static bool ApplyShaderInstance(Shader* shader, bool needsUpdate);
    static U32 AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps);
    static bool ReleaseInstanceResources(Shader* shader, U32 instanceId);
    static bool SetUniform(Shader* shader, Uniform& uniform, const void* value);
    static bool SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value);

    static bool OnResize(void* data);
    static NH_API Vector2Int WindowSize() { return { (I32)framebufferWidth, (I32)framebufferHeight }; }

private:
    RendererFrontend() = delete;

    static class Renderer* renderer;

    static bool resizing;
    static U8 framesSinceResize;
    static U8 windowRenderTargetCount;
    static U32 framebufferWidth;
    static U32 framebufferHeight;

    //TODO: Multiple scenes
    static class Scene* activeScene;
};
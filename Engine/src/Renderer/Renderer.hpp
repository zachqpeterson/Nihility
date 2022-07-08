#pragma once

#include "Defines.hpp"

template<typename> struct Vector;

class Renderer
{
public:
    virtual ~Renderer() {};

    virtual bool Initialize(const struct String& applicationName, U8& renderTargetCount) = 0;
    virtual void Shutdown() = 0;

    virtual bool BeginFrame() = 0;
    virtual bool EndFrame() = 0;
    virtual bool BeginRenderpass(struct Renderpass* renderpass) = 0;
    virtual bool EndRenderpass(Renderpass* renderpass) = 0;

    virtual bool CreateMesh(struct Mesh* mesh, Vector<Vertex>& vertices, Vector<U32>& indices) = 0;
    virtual void DestroyMesh(Mesh* mesh) = 0;
    virtual void DrawMesh(const struct MeshRenderData& Meshdata) = 0;

    virtual void CreateTexture(struct Texture* texture, const Vector<U8>& pixels) = 0;
    virtual void DestroyTexture(Texture* texture) = 0;
    virtual bool CreateWritableTexture(Texture* texture) = 0;
    virtual void WriteTextureData(Texture* texture, U32 offset, U32 size, const Vector<U8>& pixels) = 0;
    virtual void ResizeTexture(Texture* texture, U32 width, U32 height) = 0;

    virtual bool AcquireTextureMapResources(struct TextureMap& map) = 0;
    virtual void ReleaseTextureMapResources(TextureMap& map) = 0;

    virtual void CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext) = 0;
    virtual void DestroyRenderpass(Renderpass* renderpass) = 0;

    virtual bool CreateRenderTarget(Vector<Texture*>& attachments, struct Renderpass* renderpass, U32 width, U32 height, struct RenderTarget* target) = 0;
    virtual bool DestroyRenderTarget(struct RenderTarget* target, bool freeInternalMemory) = 0;
    virtual Texture* GetWindowAttachment(U8 index) = 0;
    virtual Texture* GetDepthAttachment() = 0;
    virtual U32 GetWindowAttachmentIndex() = 0;

    virtual bool CreateShader(struct Shader* shader) = 0;
    virtual void DestroyShader(Shader* shader) = 0;
    virtual bool InitializeShader(Shader* shader) = 0;
    virtual bool UseShader(Shader* shader) = 0;
    virtual bool BindInstance(Shader* shader, U32 instanceId) = 0;
    virtual bool ApplyGlobals(Shader* shader) = 0;
    virtual bool ApplyInstance(Shader* shader, bool needsUpdate) = 0;
    virtual U32 AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps) = 0;
    virtual bool ReleaseInstanceResources(Shader* shader, U32 instanceId) = 0;
    virtual bool SetUniform(Shader* shader, struct Uniform& uniform, const void* value) = 0;
    virtual bool SetPushConstant(Shader* shader, struct PushConstant& pushConstant, const void* value) = 0;

    virtual bool OnResize() = 0;

public:
    U64 frameNumber;
};
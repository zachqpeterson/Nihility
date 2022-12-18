#pragma once

#include "Defines.hpp"

template<typename> struct Vector;
struct Mesh;
struct Renderpass;
struct String;
struct MeshRenderData;
struct Texture;
struct TextureMap;
struct RenderTarget;
struct Shader;
struct Uniform;
struct PushConstant;
struct Vector2Int;

class Renderer
{
public:
    virtual ~Renderer() {};

    virtual bool Initialize(const String& applicationName, U8& renderTargetCount) = 0;
    virtual void Shutdown() = 0;

    virtual bool BeginFrame() = 0;
    virtual bool EndFrame() = 0;
    virtual bool BeginRenderpass(Renderpass* renderpass) = 0;
    virtual bool EndRenderpass(Renderpass* renderpass) = 0;

    virtual bool CreateMesh(Mesh* mesh) = 0;
    virtual bool BatchCreateMeshes(Vector<Mesh*>& meshes) = 0;
    virtual void DestroyMesh(Mesh* mesh) = 0;
    virtual void DrawMesh(const MeshRenderData& meshData) = 0;

    virtual void CreateTexture(Texture* texture, const Vector<U8>& pixels) = 0;
    virtual void DestroyTexture(Texture* texture) = 0;
    virtual bool CreateWritableTexture(Texture* texture) = 0;
    virtual void WriteTextureData(Texture* texture, const Vector<U8>& pixels) = 0;
    virtual void ResizeTexture(Texture* texture, U32 width, U32 height) = 0;

    virtual bool AcquireTextureMapResources(TextureMap& map) = 0;
    virtual void ReleaseTextureMapResources(TextureMap& map) = 0;

    virtual void CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext) = 0;
    virtual void DestroyRenderpass(Renderpass* renderpass) = 0;

    virtual bool CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, RenderTarget* target) = 0;
    virtual bool DestroyRenderTarget(RenderTarget* target, bool freeInternalMemory) = 0;
    virtual Texture* GetWindowAttachment(U8 index) = 0;
    virtual Texture* GetDepthAttachment() = 0;
    virtual U32 GetWindowAttachmentIndex() = 0;

    virtual bool CreateShader(Shader* shader) = 0;
    virtual void DestroyShader(Shader* shader) = 0;
    virtual bool InitializeShader(Shader* shader) = 0;
    virtual void UseShader(Shader* shader) = 0;
    virtual bool ApplyGlobals(Shader* shader) = 0;
    virtual bool ApplyInstance(Shader* shader, bool needsUpdate) = 0;
    virtual U32 AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps) = 0;
    virtual bool ReleaseInstanceResources(Shader* shader, U32 instanceId) = 0;
    virtual void SetGlobalUniform(Shader* shader, Uniform& uniform, const void* value) = 0;
    virtual void SetInstanceUniform(Shader* shader, Uniform& uniform, const void* value) = 0;
    virtual void SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value) = 0;

    virtual void OnResize() = 0;
    virtual Vector2Int WindowSize() = 0;
    virtual Vector2Int WindowOffset() = 0;

public:
    U64 frameNumber{ 0 };
};
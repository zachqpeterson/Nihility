#pragma once

#include "VulkanDefines.hpp"

#include "Renderer/Renderer.hpp"

template<typename> struct Vector;
struct ShaderStage;
class VulkanBuffer;

class VulkanRenderer : public Renderer
{
public:
    bool Initialize(const String& applicationName, U8& renderTargetCount) final;
    void Shutdown() final;

    bool BeginFrame() final;
    bool EndFrame() final;
    bool BeginRenderpass(Renderpass* renderpass) final;
    bool EndRenderpass(Renderpass* renderpass) final;

    bool CreateMesh(Mesh* mesh) final;
    bool BatchCreateMeshes(Vector<Mesh*>& meshes) final;
    void DestroyMesh(Mesh* mesh) final;
    void DrawMesh(const MeshRenderData& meshData) final;

    void CreateTexture(Texture* texture, const Vector<U8>& pixels) final;
    void DestroyTexture(Texture* texture) final;
    bool CreateWritableTexture(Texture* texture) final;
    void WriteTextureData(Texture* texture, const Vector<U8>& pixels) final;
    void ResizeTexture(Texture* texture, U32 width, U32 height) final;

    bool AcquireTextureMapResources(TextureMap& map) final;
    void ReleaseTextureMapResources(TextureMap& map) final;

    void CreateRenderpass(Renderpass* renderpass) final;
    void DestroyRenderpass(Renderpass* renderpass) final;

    bool CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, RenderTarget* target) final;
    bool DestroyRenderTarget(RenderTarget* target, bool freeInternalMemory) final;
    Texture* GetWindowAttachment(U8 index) final;
    Texture* GetDepthAttachment() final;
    U32 GetWindowAttachmentIndex() final;

    bool CreateShader(Shader* shader) final;
    void DestroyShader(Shader* shader) final;
    bool InitializeShader(Shader* shader) final;
    void UseShader(Shader* shader) final;
    bool ApplyGlobals(Shader* shader) final;
    bool ApplyInstance(Shader* shader, bool needsUpdate) final;
    U32  AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps) final;
    bool ReleaseInstanceResources(Shader* shader, U32 instanceId) final;
    void SetGlobalUniform(Shader* shader, Uniform& uniform, const void* value) final;
    void SetInstanceUniform(Shader* shader, Uniform& uniform, const void* value) final;
    void SetPushConstant(Shader* shader, PushConstant& pushConstant, const void* value) final;

    void OnResize() final;
    Vector2Int WindowSize() final;
    Vector2Int WindowOffset() final;

    void* operator new(U64 size);
    void operator delete(void* p);

private: //VULKAN SPECIFIC FUNCTIONS
    bool CreateInstance(const String& applicationName);
    bool CreateDebugger();
    bool CreateSurface();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    bool CreateBuffers();

    void GetPlatformExtentions(Vector<const char*>& names);
    bool RegenerateRenderTargets();

    static I32 FindMemoryIndex(U32 memoryTypeBits, VkMemoryPropertyFlags memoryFlags);
    static VkFilter ConvertFilterType(TextureFilter filter);
    static VkSamplerAddressMode ConvertRepeatType(TextureRepeat repeat);
    static bool UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, U32& outOffset, U32 size, const void* data);
    static bool UploadDataRanges(VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, Vector<struct DataUpload>& uploads);
    static void FreeDataRange(VulkanBuffer* buffer, U32 offset, U32 size);

    //Runtime
    bool RecreateSwapchain();
};
#pragma once

#include "VulkanDefines.hpp"

#include "Renderer/Renderer.hpp"

template<typename> struct Vector;

class VulkanRenderer : public Renderer
{
public:
    bool Initialize(const struct String& applicationName, U8& renderTargetCount) final;
    void Shutdown() final;

    bool BeginFrame() final;
    bool EndFrame() final;
    bool BeginRenderpass(struct Renderpass* renderpass) final;
    bool EndRenderpass(Renderpass* renderpass) final;

    bool CreateMesh(struct Mesh* mesh, Vector<Vertex>& vertices, Vector<U32>& indices) final;
    void DestroyMesh(Mesh* mesh) final;
    void DrawMesh(const struct MeshRenderData& Meshdata) final;

    void CreateTexture(struct Texture* texture, const Vector<U8>& pixels) final;
    void DestroyTexture(Texture* texture) final;
    bool CreateWritableTexture(Texture* texture) final;
    void WriteTextureData(Texture* texture, const Vector<U8>& pixels) final;
    void ResizeTexture(Texture* texture, U32 width, U32 height) final;

    bool AcquireTextureMapResources(struct TextureMap& map) final;
    void ReleaseTextureMapResources(TextureMap& map) final;

    void CreateRenderpass(Renderpass* renderpass, bool hasPrev, bool hasNext) final;
    void DestroyRenderpass(Renderpass* renderpass) final;

    bool CreateRenderTarget(Vector<Texture*>& attachments, Renderpass* renderpass, U32 width, U32 height, struct RenderTarget* target) final;
    bool DestroyRenderTarget(struct RenderTarget* target, bool freeInternalMemory) final;
    Texture* GetWindowAttachment(U8 index) final;
    Texture* GetDepthAttachment() final;
    U32 GetWindowAttachmentIndex() final;

    bool CreateShader(struct Shader* shader) final;
    void DestroyShader(Shader* shader) final;
    bool InitializeShader(Shader* shader) final;
    bool UseShader(Shader* shader) final;
    bool ApplyGlobals(Shader* shader) final;
    bool ApplyInstance(Shader* shader, bool needsUpdate) final;
    U32  AcquireInstanceResources(Shader* shader, Vector<TextureMap>& maps) final;
    bool ReleaseInstanceResources(Shader* shader, U32 instanceId) final;
    bool SetUniform(Shader* shader, struct Uniform& uniform, const void* value) final;
    bool SetPushConstant(Shader* shader, struct PushConstant& pushConstant, const void* value) final;

    void OnResize() final;

    void* operator new(U64 size);
    void operator delete(void* p);

private: //VULKAN SPECIFIC FUNCTIONS
    bool CreateInstance(const struct String& applicationName);
    bool CreateDebugger();
    bool CreateSurface();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    bool CreateBuffers();
    bool CreateShaderModule(const String& name, const String& typeStr,
        VkShaderStageFlagBits shaderStageFlag, U32 stageIndex, Vector<struct ShaderStage> shaderStages);

    void GetPlatformExtentions(Vector<const char*>& names);
    bool RegenerateRenderTargets();

    static I32 FindMemoryIndex(U32 memoryTypeBits, VkMemoryPropertyFlags memoryFlags);
    static VkFilter ConvertFilterType(TextureFilter filter);
    static VkSamplerAddressMode ConvertRepeatType(TextureRepeat repeat);
    static bool UploadDataRange(VkCommandPool pool, VkFence fence, VkQueue queue, class VulkanBuffer* buffer, U32& outOffset, U32 size, const void* data);
    static void FreeDataRange(class VulkanBuffer* buffer, U32 offset, U32 size);

    //Runtime
    bool RecreateSwapchain();
};
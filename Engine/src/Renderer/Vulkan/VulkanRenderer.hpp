#pragma once

#include "VulkanDefines.hpp"

#include "Renderer/Renderer.hpp"
#include "Containers/Vector.hpp"

class VulkanRenderer : public Renderer
{
public:
    bool Initialize() final;
    void Shutdown() final;

    bool BeginFrame() final;
    bool EndFrame() final;
    bool BeginRenderpass(U8 renderpassId) final;
    bool EndRenderpass(U8 renderpassId) final;
    void DrawMesh() final;

    bool CreateShader(const Shader& shader, U8 renderpassId, U8 stageCount, const Vector<String>& stageFilenames, const Vector<ShaderStageType>& stages) final;
    void DestroyShader(const Shader& shader) final;
    bool InitializeShader(const Shader& shader) final;
    bool UseShader(const Shader& shader) final;
    bool BindGlobals(const Shader& shader) final;
    bool BindInstance(const Shader& shader, U32 instanceId) final;
    bool ApplyGlobals(const Shader& shader) final;
    bool ApplyInstance(const Shader& shader, bool needsUpdate) final;
    U32 AcquireInstanceResources(const Shader& shader) final;
    bool ReleaseInstanceResources(const Shader& shader, U32 instanceId) final;
    bool SetUniform(const Shader& shader, const ShaderUniform& uniform, const void* value) final;

    void* operator new(U64 size);
    void operator delete(void* p);

private: //VULKAN SPECIFIC FUNCTIONS
    bool CreateInstance();
    bool CreateDebugger();
    bool CreateSurface();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    bool CreateBuffers();
    bool CreateShaderModule(const String& name, const String& typeStr,
        VkShaderStageFlagBits shaderStageFlag, U32 stageIndex, Vector<ShaderStage> shaderStages);

    void GetPlatformExtentions(Vector<const char*>& names);
    static I32 FindMemoryIndex(U32 memoryTypeBits, VkMemoryPropertyFlags memoryFlags);

    //Runtime
    bool RecreateSwapchain();
    void RecreateFramebuffers();
};
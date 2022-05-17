#pragma once

#include "VulkanDefines.hpp"
#include "Renderer/Renderer.hpp"
#include "Resources/Shader.hpp"

#include "Containers/String.hpp"
#include "Containers/Vector.hpp"

struct ShaderStage
{
    VkShaderModuleCreateInfo info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageInfo;
};

struct ShaderStageConfig
{
    VkShaderStageFlagBits stage;
    String fileName;
};

struct DescriptorState
{
    U8 generations[3];
    U32 ids[3];
};

struct DescriptorSetState
{
    VkDescriptorSet descriptorSets[3];

    DescriptorState descriptorStates[VULKAN_SHADER_MAX_BINDINGS];
};

struct DescriptorSetConfig
{
    U8 bindingCount;
    VkDescriptorSetLayoutBinding bindings[VULKAN_SHADER_MAX_BINDINGS];
};

struct InstanceState
{
    U32 id;
    U64 offset;

    DescriptorSetState descriptorSetState;

    Vector<struct Texture*> instanceTextures;
};

struct VulkanShaderConfig
{
    U8 stageCount;
    ShaderStageConfig stages[VULKAN_SHADER_MAX_STAGES];
    VkDescriptorPoolSize poolSizes[2];
    U16 maxDescriptorSetCount;

    U8 descriptorSetCount;
    DescriptorSetConfig descriptorSets[2];

    VkVertexInputAttributeDescription attributes[VULKAN_SHADER_MAX_ATTRIBUTES];
};

class VulkanShader
{
public:
    bool Create(RendererState* rendererState, U8 renderpassId, U8 stageCount, const Vector<String>& stageFilenames, const Vector<ShaderStageType>& stages);
    void Destroy(RendererState* rendererState);
    bool Initialize(RendererState* rendererState);

    bool Use(RendererState* rendererState);
    bool BindGlobals(RendererState* rendererState);
    bool ApplyGlobals(RendererState* rendererState);
    bool BindInstance(RendererState* rendererState, U32 instanceId);
    bool ApplyInstance(RendererState* rendererState, bool needsUpdate);
    U32 AcquireInstanceResources(RendererState* rendererState);
    bool ReleaseInstanceResources(RendererState* rendererState, U32 instanceId);
    bool SetUniform(RendererState* rendererState, Shader& shader, const ShaderUniform& uniform, const void* value);

public:
    void* mappedUniformBufferBlock;

    U32 id;
    VulkanShaderConfig config;

    class VulkanRenderpass* renderpass;

    ShaderStage stages[VULKAN_SHADER_MAX_STAGES];

    VkDescriptorPool descriptorPool;

    VkDescriptorSetLayout descriptorSetLayouts[2];
    VkDescriptorSet globalDescriptorSets[3];
    class VulkanBuffer* uniformBuffer;

    class VulkanPipeline* pipeline;

    bool useInstances;
    U32 instanceCount;
    InstanceState instanceStates[VULKAN_MAX_MATERIAL_COUNT];

    U64 boundUboOffset;
    U64 globalUboOffset;
    U64 globalUboStride;
    U64 uboStride;

    U32 boundInstanceId;
};
#pragma once

#include "VulkanDefines.hpp"

class VulkanPipeline
{
public:
    bool Create(
        RendererState* rendererState,
        struct Renderpass* renderpass,
        U32 stride,
        U32 attributeCount,
        VkVertexInputAttributeDescription* attributes,
        U32 descriptorSetLayoutCount,
        VkDescriptorSetLayout* descriptorSetLayouts,
        U32 stageCount,
        VkPipelineShaderStageCreateInfo* stages,
        bool isWireframe,
        bool depthTestEnabled,
        U32 pushConstantRangeCount,
        Range* pushConstantRanges);
    void Destroy(RendererState* rendererState);

    void Bind(const class VulkanCommandBuffer& commandBuffer, VkPipelineBindPoint bindPoint);

    VkPipeline handle;
    VkPipelineLayout layout;
};
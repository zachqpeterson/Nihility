#pragma once

#include "VulkanDefines.hpp"

class VulkanPipeline
{
public:
    bool Create(RendererState* rendererState,
        class VulkanRenderpass* renderpass,
        U32 stride,
        U32 attributeCount,
        VkVertexInputAttributeDescription* attributes,
        U32 descriptorSetLayoutCount,
        VkDescriptorSetLayout* descriptorSetLayouts,
        U32 stageCount,
        VkPipelineShaderStageCreateInfo* stages,
        VkViewport viewport,
        VkRect2D scissor,
        bool isWireframe,
        bool depthTestEnabled,
        U32 pushConstantRangeCount,
        Range* pushConstantRanges);
    void Destroy(RendererState* rendererState);

    void Bind(const class VulkanCommandBuffer& commandBuffer, VkPipelineBindPoint bindPoint);

    VkPipeline handle;
    VkPipelineLayout layout;
};
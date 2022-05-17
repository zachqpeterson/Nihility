#pragma once

#include "VulkanDefines.hpp"

class VulkanPipeline
{
public:
    bool Create(RendererState* rendererState);
    void Destroy(RendererState* rendererState);

    VkPipeline pipeline;
    VkPipelineLayout layout;

private:

};
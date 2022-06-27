#pragma once

#include "VulkanDefines.hpp"
#include "Math/Math.hpp"

enum RenderPassState
{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
};

class VulkanRenderpass
{
public:
    VkRenderPass handle;

    bool hasPrevPass;
    bool hasNextPass;

    RenderPassState state;
};
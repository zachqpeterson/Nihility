#pragma once

#include "VulkanDefines.hpp"
#include "Math/Math.hpp"

enum renderpassClearFlags
{
    RENDERPASS_CLEAR_NONE_FLAG = 0x0,
    RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG = 0x1,
    RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
    RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
};

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
    bool Create(RendererState* rendererState,
        Vector4 renderArea,
        Vector4 clearColor,
        F32 depth,
        U32 stencil,
        U8 clearFlags,
        bool hasPrevPass,
        bool hasNextPass);
    void Destroy(RendererState* rendererState);
    void Begin(class VulkanCommandBuffer* commandBuffer, VkFramebuffer frameBuffer);
    void End(VulkanCommandBuffer* commandBuffer);

    VkRenderPass handle;
    Vector4 renderArea;
    Vector4 clearColor;
    
private:
    F32 depth;
    U32 stencil;

    U8 clearFlags;

    bool hasPrevPass;
    bool hasNextPass;

    //RenderPassState state;
};
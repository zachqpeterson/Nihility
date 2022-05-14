#pragma once

#include "VulkanDefines.hpp"

enum CommandBufferState
{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
};

class VulkanCommandBuffer
{
public:
    void Allocate(RendererState* rendererState, VkCommandPool pool, bool isPrimary);
    void Free(RendererState* rendererState, VkCommandPool pool);
    void Begin(bool singleUse, bool renderpassContinue, bool simultaneousUse);
    void End();
    void UpdateSubmitted();
    void Reset();
    void AllocateAndBeginSingleUse(RendererState* rendererState, VkCommandPool pool);
    void EndSingleUse(RendererState* rendererState, VkCommandPool pool, VkQueue queue);
    
public:
    VkCommandBuffer handle;

    CommandBufferState state;
};

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

public:
    VkCommandBuffer buffer;

    CommandBufferState state;
};

#include "RenderingDefines.hpp"

// RENDER PASS OUTPUT

RenderPassOutput& RenderPassOutput::Reset()
{
    numColorFormats = 0;

    for (U32 i = 0; i < MAX_IMAGE_OUTPUTS; ++i)
    {
        colorFormats[i] = VK_FORMAT_UNDEFINED;
    }

    depthStencilFormat = VK_FORMAT_UNDEFINED;
    colorOperation = RENDER_PASS_OP_DONT_CARE;
    depthOperation = RENDER_PASS_OP_DONT_CARE;
    stencilOperation = RENDER_PASS_OP_DONT_CARE;

    return *this;
}

RenderPassOutput& RenderPassOutput::Color(VkFormat format)
{
    colorFormats[numColorFormats++] = format;
    return *this;
}

RenderPassOutput& RenderPassOutput::Depth(VkFormat format)
{
    depthStencilFormat = format;
    return *this;
}

RenderPassOutput& RenderPassOutput::SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil)
{
    colorOperation = color;
    depthOperation = depth;
    stencilOperation = stencil;

    return *this;
}

// GPU TIMESTAMP MANAGER

void GPUTimestampManager::Create(U16 queriesPerFrame, U16 maxFrames)
{
    this->queriesPerFrame = queriesPerFrame;

    const U32 dataPerQuery = 2;
    const U64 allocatedSize = sizeof(GPUTimestamp) * queriesPerFrame * maxFrames + sizeof(U64) * queriesPerFrame * maxFrames * dataPerQuery;
    Memory::AllocateSize(&timestamps, allocatedSize);

    timestampsData = (U64*)((U8*)timestamps + sizeof(GPUTimestamp) * queriesPerFrame * maxFrames);

    Reset();
}

void GPUTimestampManager::Destroy()
{
    Memory::FreeSize(&timestamps);
}

bool GPUTimestampManager::HasValidQueries() const
{
    return currentQuery > 0 && (depth == 0);
}

void GPUTimestampManager::Reset()
{
    currentQuery = 0;
    parentIndex = 0;
    currentFrameResolved = false;
    depth = 0;
}

U32 GPUTimestampManager::Resolve(U32 currentFrame, GPUTimestamp* timestampsToFill)
{
    memcpy(timestampsToFill, &timestamps[currentFrame * queriesPerFrame], sizeof(GPUTimestamp) * currentQuery);
    return currentQuery;
}

U32 GPUTimestampManager::Push(U32 currentFrame, const char* name)
{
    U32 queryIndex = (currentFrame * queriesPerFrame) + currentQuery;

    GPUTimestamp& timestamp = timestamps[queryIndex];
    timestamp.parentIndex = (U16)parentIndex;
    timestamp.start = queryIndex * 2;
    timestamp.end = timestamp.start + 1;
    timestamp.name = name;
    timestamp.depth = (U16)depth++;

    parentIndex = currentQuery;
    ++currentQuery;

    return (queryIndex * 2);
}

U32 GPUTimestampManager::Pop(U32 currentFrame)
{
    U32 queryIndex = (currentFrame * queriesPerFrame) + parentIndex;
    GPUTimestamp& timestamp = timestamps[queryIndex];
    parentIndex = timestamp.parentIndex;
    --depth;

    return (queryIndex * 2) + 1;
}

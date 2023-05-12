#include "RenderingDefines.hpp"

// VERTEX INPUT CREATION

VertexInputCreation& VertexInputCreation::Reset()
{
	numVertexStreams = numVertexAttributes = 0;
	return *this;
}

VertexInputCreation& VertexInputCreation::AddVertexStream(const VertexStream& stream)
{
	vertexStreams[numVertexStreams++] = stream;
	return *this;
}

VertexInputCreation& VertexInputCreation::AddVertexAttribute(const VertexAttribute& attribute)
{
	vertexAttributes[numVertexAttributes++] = attribute;
	return *this;
}

// DEPTH STENCIL CREATION

DepthStencilCreation& DepthStencilCreation::SetDepth(bool write, VkCompareOp comparisonTest)
{
	depthWriteEnable = write;
	depthComparison = comparisonTest;
	// Setting depth like this means we want to use the depth test.
	depthEnable = 1;

	return *this;
}

// BLEND STATE
BlendState& BlendState::SetColor(VkBlendFactor source, VkBlendFactor destination, VkBlendOp operation)
{
	sourceColor = source;
	destinationColor = destination;
	colorOperation = operation;
	blendEnabled = 1;

	return *this;
}

BlendState& BlendState::SetAlpha(VkBlendFactor source, VkBlendFactor destination, VkBlendOp operation)
{
	sourceAlpha = source;
	destinationAlpha = destination;
	alphaOperation = operation;
	separateBlend = 1;

	return *this;
}

BlendState& BlendState::SetColorWriteMask(ColorWriteEnableMask value)
{
	colorWriteMask = value;

	return *this;
}

// BLEND STATE CREATION

BlendStateCreation& BlendStateCreation::Reset()
{
	activeStates = 0;

	return *this;
}

BlendState& BlendStateCreation::AddBlendState()
{
	return blendStates[activeStates++];
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
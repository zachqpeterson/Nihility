#include "RenderingDefines.hpp"

#include "Renderer.hpp"

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
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

// SHADER STATE CREATION

ShaderStateCreation& ShaderStateCreation::Reset()
{
	stagesCount = 0;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::SetName(CSTR name)
{
	this->name = name;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::AddStage(CSTR code, U32 codeSize, VkShaderStageFlagBits type)
{
	stages[stagesCount].code = code;
	stages[stagesCount].codeSize = codeSize;
	stages[stagesCount].type = type;
	++stagesCount;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::SetSpvInput(bool value)
{
	spvInput = value;
	return *this;
}

// DESCRIPTOR SET LAYOUT CREATION

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::Reset()
{
	numBindings = 0;
	setIndex = 0;
	return *this;
}

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::AddBinding(const Binding& binding)
{
	bindings[numBindings++] = binding;
	return *this;
}

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::SetName(CSTR name)
{
	this->name = name;
	return *this;
}


DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::SetSetIndex(U32 index)
{
	setIndex = index;
	return *this;
}

// DESCRIPTOR SET CREATION

DescriptorSetCreation& DescriptorSetCreation::Reset()
{
	numResources = 0;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetLayout(DescriptorSetLayoutHandle layout)
{
	this->layout = layout;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::Texture(TextureHandle texture, U16 binding)
{
	// Set a default sampler
	samplers[numResources] = INVALID_SAMPLER;
	bindings[numResources] = binding;
	resources[numResources++] = texture.index;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::Buffer(BufferHandle buffer, U16 binding)
{
	samplers[numResources] = INVALID_SAMPLER;
	bindings[numResources] = binding;
	resources[numResources++] = buffer.index;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::TextureSampler(TextureHandle texture, SamplerHandle sampler, U16 binding)
{
	bindings[numResources] = binding;
	resources[numResources] = texture.index;
	samplers[numResources++] = sampler;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetName(CSTR name)
{
	this->name = name;
	return *this;
}

// BUFFER CREATION

BufferCreation& BufferCreation::Reset()
{
	size = 0;
	initialData = nullptr;

	return *this;
}

BufferCreation& BufferCreation::Set(VkBufferUsageFlags flags, ResourceUsage usage, U32 size)
{
	typeFlags = flags;
	this->usage = usage;
	this->size = size;

	return *this;
}

BufferCreation& BufferCreation::SetData(void* data)
{
	initialData = data;

	return *this;
}

BufferCreation& BufferCreation::SetName(CSTR name)
{
	this->name = name;

	return *this;
}

// TEXTURE CREATION

TextureCreation& TextureCreation::SetSize(U16 width, U16 height, U16 depth)
{
	this->width = width;
	this->height = height;
	this->depth = depth;

	return *this;
}

TextureCreation& TextureCreation::SetFlags(U8 mipmaps, U8 flags)
{
	this->mipmaps = mipmaps;
	this->flags = flags;

	return *this;
}

TextureCreation& TextureCreation::SetFormatType(VkFormat format, TextureType type)
{
	this->format = format;
	this->type = type;

	return *this;
}

TextureCreation& TextureCreation::SetName(CSTR name)
{
	this->name = name;

	return *this;
}

TextureCreation& TextureCreation::SetData(void* data)
{
	initialData = data;

	return *this;
}

// PIPELINE CREATION

PipelineCreation& PipelineCreation::AddDescriptorSetLayout(DescriptorSetLayoutHandle handle)
{
	descriptorSetLayouts[numActiveLayouts++] = handle;
	return *this;
}

RenderPassOutput& PipelineCreation::RenderPassOutput()
{
	return renderPass;
}

// SAMPLER CREATION

SamplerCreation& SamplerCreation::SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip)
{
	minFilter = min;
	magFilter = mag;
	mipFilter = mip;

	return *this;
}

SamplerCreation& SamplerCreation::SetAddressModeU(VkSamplerAddressMode u)
{
	addressModeU = u;

	return *this;
}

SamplerCreation& SamplerCreation::SetAddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v)
{
	addressModeU = u;
	addressModeV = v;

	return *this;
}

SamplerCreation& SamplerCreation::SetAddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
{
	addressModeU = u;
	addressModeV = v;
	addressModeW = w;

	return *this;
}

SamplerCreation& SamplerCreation::SetName(CSTR name)
{
	this->name = name;

	return *this;
}

// RENDER PASS CREATION

RenderPassCreation& RenderPassCreation::Reset()
{
	numRenderTargets = 0;
	depthStencilTexture = INVALID_TEXTURE;
	resize = 0;
	scaleX = 1.f;
	scaleY = 1.f;
	colorOperation = depthOperation = stencilOperation = RENDER_PASS_OP_DONT_CARE;

	return *this;
}

RenderPassCreation& RenderPassCreation::AddRenderTexture(TextureHandle texture)
{
	outputTextures[numRenderTargets++] = texture;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetScaling(F32 scaleX, F32 scaleY, U8 resize)
{
	this->scaleX = scaleX;
	this->scaleY = scaleY;
	this->resize = resize;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetDepthStencilTexture(TextureHandle texture)
{
	depthStencilTexture = texture;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetName(CSTR name)
{
	this->name = name;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetType(RenderPassType type)
{
	this->type = type;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil)
{
	colorOperation = color;
	depthOperation = depth;
	stencilOperation = stencil;

	return *this;
}

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

// EXECUTION BARRIER

ExecutionBarrier& ExecutionBarrier::Reset()
{
	numTextureBarriers = numBufferBarriers = 0;
	sourcePipelineStage = PIPELINE_STAGE_DRAW_INDIRECT;
	destinationPipelineStage = PIPELINE_STAGE_DRAW_INDIRECT;
	return *this;
}

ExecutionBarrier& ExecutionBarrier::Set(PipelineStage source, PipelineStage destination)
{
	sourcePipelineStage = source;
	destinationPipelineStage = destination;

	return *this;
}

ExecutionBarrier& ExecutionBarrier::AddImageBarrier(const TextureBarrier& textureBarrier)
{
	textureBarriers[numTextureBarriers++] = textureBarrier;

	return *this;
}

ExecutionBarrier& ExecutionBarrier::AddMemoryBarrier(const BufferBarrier& bufferBarrier)
{
	bufferBarriers[numBufferBarriers++] = bufferBarrier;

	return *this;
}
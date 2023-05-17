#include "ResourceDefines.hpp"

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

SamplerCreation& SamplerCreation::SetName(const String& name)
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

TextureCreation& TextureCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

TextureCreation& TextureCreation::SetData(void* data)
{
	initialData = data;

	return *this;
}

// BUFFER CREATION

BufferCreation& BufferCreation::Reset()
{
	size = 0;
	initialData = nullptr;

	return *this;
}

BufferCreation& BufferCreation::Set(VkBufferUsageFlags flags, ResourceUsage usage, U64 size)
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

BufferCreation& BufferCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

// DESCRIPTOR SET LAYOUT CREATION

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::Reset()
{
	bindingCount = 0;
	setIndex = 0;
	return *this;
}

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::AddBinding(const DescriptorBinding& binding)
{
	bindings[bindingCount++] = binding;
	return *this;
}

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::SetName(const String& name)
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
	resourceCount = 0;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetLayout(DescriptorSetLayout* layout)
{
	this->layout = layout;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetTexture(Texture* texture, U16 binding)
{
	//TODO: Set a default sampler
	samplers[resourceCount] = nullptr;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = texture;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetBuffer(Buffer* buffer, U16 binding)
{
	samplers[resourceCount] = nullptr;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = buffer;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetTextureSampler(Texture* texture, Sampler* sampler, U16 binding)
{
	samplers[resourceCount] = sampler;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = texture;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetName(const String& name)
{
	this->name = name;
	return *this;
}

// SHADER STATE CREATION

ShaderStateCreation& ShaderStateCreation::Reset()
{
	stagesCount = 0;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::AddStage(const String& path, VkShaderStageFlagBits type)
{
	stages[stagesCount].path = path;
	stages[stagesCount].type = type;
	++stagesCount;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::SetSpvInput(bool value)
{
	spvInput = value;
	return *this;
}

// RENDER PASS OUTPUT

RenderPassOutput& RenderPassOutput::Reset()
{
	colorFormatCount = 0;

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
	colorFormats[colorFormatCount++] = format;
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

// RENDER PASS CREATION

RenderPassCreation& RenderPassCreation::Reset()
{
	renderTargetCount = 0;
	depthStencilTexture = nullptr;
	resize = 0;
	scaleX = 1.f;
	scaleY = 1.f;
	colorOperation = depthOperation = stencilOperation = RENDER_PASS_OP_DONT_CARE;

	return *this;
}

RenderPassCreation& RenderPassCreation::AddRenderTexture(Texture* texture)
{
	outputTextures[renderTargetCount++] = texture;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetScaling(F32 scaleX, F32 scaleY, U8 resize)
{
	this->scaleX = scaleX;
	this->scaleY = scaleY;
	this->resize = resize;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetDepthStencilTexture(Texture* texture)
{
	depthStencilTexture = texture;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetName(const String& name)
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

// PIPELINE CREATION

PipelineCreation& PipelineCreation::AddDescriptorSetLayout(DescriptorSetLayout* handle)
{
	descriptorSetLayouts[activeLayoutCount++] = handle;
	return *this;
}

RenderPassOutput& PipelineCreation::GetRenderPassOutput()
{
	return renderPass;
}

// EXECUTION BARRIER

ExecutionBarrier& ExecutionBarrier::Reset()
{
	textureBarrierCount = bufferBarrierCount = 0;
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
	textureBarriers[textureBarrierCount++] = textureBarrier;

	return *this;
}

ExecutionBarrier& ExecutionBarrier::AddMemoryBarrier(const BufferBarrier& bufferBarrier)
{
	bufferBarriers[bufferBarrierCount++] = bufferBarrier;

	return *this;
}
#include "ResourceDefines.hpp"

#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"

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
	name.Destroy();

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

BufferCreation& BufferCreation::SetParent(Buffer* parent, U64 offset)
{
	parentBuffer = parent;
	this->offset = offset;

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

DescriptorSetLayoutCreation& DescriptorSetLayoutCreation::AddBindingAtIndex(const DescriptorBinding& binding, U32 index)
{
	bindings[index] = binding;
	bindingCount = (index + 1) > bindingCount ? (index + 1) : bindingCount;
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
	DescriptorSetResource resource{};
	resource.imageInfo.image = texture->imageView;
	resource.imageInfo.layout = texture->format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	samplers[resourceCount] = nullptr;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = resource;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetTexture(RenderTarget& texture, U16 binding)
{
	DescriptorSetResource resource{};
	resource.imageInfo.image = texture.imageView;
	resource.imageInfo.layout = texture.format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	samplers[resourceCount] = nullptr;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = resource;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetBuffer(Buffer* buffer, U16 binding)
{
	DescriptorSetResource resource{};
	if (buffer->parentBuffer) { resource.bufferInfo.buffer = buffer->parentBuffer->buffer; }
	else { resource.bufferInfo.buffer = buffer->buffer; }
	resource.bufferInfo.size = buffer->size;
	resource.bufferInfo.offset = (U32)buffer->globalOffset;
	resource.bufferInfo.type = buffer->usage == RESOURCE_USAGE_DYNAMIC ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	samplers[resourceCount] = nullptr;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = resource;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetTextureSampler(Texture* texture, Sampler* sampler, U16 binding)
{
	DescriptorSetResource resource{};
	resource.imageInfo.image = texture->imageView;
	resource.imageInfo.layout = texture->format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	samplers[resourceCount] = sampler;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = resource;
	return *this;
}

DescriptorSetCreation& DescriptorSetCreation::SetTextureSampler(RenderTarget& texture, Sampler* sampler, U16 binding)
{
	DescriptorSetResource resource{};
	resource.imageInfo.image = texture.imageView;
	resource.imageInfo.layout = texture.format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	samplers[resourceCount] = sampler;
	bindings[resourceCount] = binding;
	resources[resourceCount++] = resource;
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
	name.Destroy();

	return *this;
}

ShaderStateCreation& ShaderStateCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

ShaderStateCreation& ShaderStateCreation::AddStage(CSTR name, VkShaderStageFlagBits type)
{
	stages[stagesCount].name = name;
	stages[stagesCount].type = type;
	++stagesCount;

	return *this;
}

// SHADER STATE

void ShaderState::SetSpecializationData(const SpecializationData& data)
{
	Memory::Copy(specializationInfos[data.stage].specializationBuffer + specializationInfos[data.stage].specializationData[data.index].offset,
		data.data, specializationInfos[data.stage].specializationData[data.index].size);
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
	width = 0;
	height = 0;
	renderTargetCount = 0;
	type = RENDER_PASS_TYPE_GEOMETRY;

	depthStencilTexture = {};
	sampler = nullptr ;

	colorOperation = RENDER_PASS_OP_DONT_CARE;
	depthOperation = RENDER_PASS_OP_DONT_CARE;
	stencilOperation = RENDER_PASS_OP_DONT_CARE;

	name.Clear();

	return *this;
}

RenderPassCreation& RenderPassCreation::AddRenderTarget(const RenderTarget& texture)
{
	outputTextures[renderTargetCount++] = texture;

	return *this;
}

RenderPassCreation& RenderPassCreation::SetDepthStencilTexture(const RenderTarget& texture)
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

PipelineCreation& PipelineCreation::AddBlendState(const BlendState& blendState)
{
	blendStates[blendStateCount++] = blendState;

	return *this;
}

BlendState& PipelineCreation::AddBlendState()
{
	return blendStates[blendStateCount++] = {};
}

PipelineCreation& PipelineCreation::AddSpecializationData(const SpecializationData& data)
{
	specializationData[specializationCount++] = data;

	return *this;
}

// PROGRAM CREATION

ProgramCreation& ProgramCreation::Reset()
{
	prePassCount = 0;
	postPassCount = 0;
	name.Clear();

	return *this;
}

ProgramCreation& ProgramCreation::SetName(const String& name_)
{
	name = name_;

	return *this;
}

ProgramCreation& ProgramCreation::AddPrePass(const PipelineCreation& pass)
{
	prePasses[prePassCount++] = pass;

	return *this;
}

ProgramCreation& ProgramCreation::AddPostPass(const PipelineCreation& pass)
{
	postPasses[postPassCount++] = pass;

	return *this;
}

// MATERIAL CREATION

MaterialCreation& MaterialCreation::Reset()
{
	program = nullptr;
	name.Clear();
	renderIndex = U32_MAX;
	return *this;
}

MaterialCreation& MaterialCreation::SetProgram(Program* program_)
{
	program = program_;
	return *this;
}

MaterialCreation& MaterialCreation::SetRenderIndex(U32 render_index_)
{
	renderIndex = render_index_;
	return *this;
}

MaterialCreation& MaterialCreation::SetName(const String& name_)
{
	name = name_;
	return *this;
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

// PROGRAM

void Program::RunPrePasses()
{

}

void Program::RunPostPasses(CommandBuffer* commands)
{
	RenderPass* renderPass = nullptr;

	for (U8 i = 0; i < postPassCount; ++i)
	{
		Pipeline* pipeline = postPasses[i];

		if (pipeline->renderPass != renderPass)
		{
			if(renderPass != nullptr) {} //TODO: End renderpass
			
			renderPass = pipeline->renderPass;
			commands->BindPass(pipeline->renderPass);
		}

		DescriptorSetCreation dsCreation{};
		dsCreation.SetLayout(pipeline->descriptorSetLayouts[0]);

		U32 textureIndex = 0;

		for (U32 i = 0; i < pipeline->descriptorSetLayouts[0]->bindingCount; ++i)
		{
			switch (pipeline->descriptorSetLayouts[0]->bindings[i].type)
			{
				//TODO: Indicate we want an output texture instead of some other texture
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
				//TODO: Use previous renderpass
				dsCreation.SetTextureSampler(Renderer::offscreenPass->outputTextures[textureIndex++], renderPass->sampler, i);
			} break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
				//TODO:
				BreakPoint;
			} break;
			}
		}
		
		DescriptorSet* descriptorSet = commands->CreateDescriptorSet(dsCreation);

		commands->BindPipeline(pipeline);
		commands->BindDescriptorSet(&descriptorSet, 1, nullptr, 0);

		commands->Draw(TOPOLOGY_TYPE_TRIANGLE, 0, 3, 0, 1);
	}

	if (renderPass != nullptr) {} //TODO: End renderpass
}
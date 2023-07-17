#include "ResourceDefines.hpp"

#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Resources\Resources.hpp"

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

// RENDER PASS OUTPUT

RenderpassOutput& RenderpassOutput::Reset()
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

RenderpassOutput& RenderpassOutput::Color(VkFormat format)
{
	colorFormats[colorFormatCount++] = format;
	return *this;
}

RenderpassOutput& RenderpassOutput::Depth(VkFormat format)
{
	depthStencilFormat = format;
	return *this;
}

RenderpassOutput& RenderpassOutput::SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil)
{
	colorOperation = color;
	depthOperation = depth;
	stencilOperation = stencil;

	return *this;
}

// RENDER PASS CREATION

RenderpassCreation& RenderpassCreation::Reset()
{
	width = 0;
	height = 0;
	renderTargetCount = 0;
	type = RENDERPASS_TYPE_GEOMETRY;

	depthStencilTexture = {};

	colorOperation = RENDER_PASS_OP_DONT_CARE;
	depthOperation = RENDER_PASS_OP_DONT_CARE;
	stencilOperation = RENDER_PASS_OP_DONT_CARE;

	name.Clear();

	return *this;
}

RenderpassCreation& RenderpassCreation::AddRenderTarget(Texture* texture)
{
	outputTextures[renderTargetCount++] = texture;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetDepthStencilTexture(Texture* texture)
{
	depthStencilTexture = texture;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetType(RenderpassType type)
{
	this->type = type;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil)
{
	colorOperation = color;
	depthOperation = depth;
	stencilOperation = stencil;

	return *this;
}

RenderpassCreation& RenderpassCreation::AddClearColor(const Vector4& color)
{
	clears[clearCount++].color = { color.x, color.y, color.z, color.w };

	return *this;
}

RenderpassCreation& RenderpassCreation::AddClearDepth(F32 depth)
{
	clears[clearCount++].depthStencil = { depth, 0 };

	return *this;
}

// PROGRAM CREATION

ProgramCreation& ProgramCreation::Reset()
{
	passCount = 0;
	name.Clear();

	return *this;
}

ProgramCreation& ProgramCreation::SetName(const String& name_)
{
	name = name_;

	return *this;
}

ProgramCreation& ProgramCreation::AddPass(Pipeline* pass)
{
	passes[passCount++] = pass;

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

Renderpass* Program::prevRenderpass{ nullptr };

void Program::RunPasses(CommandBuffer* commands)
{
	Renderpass* renderpass = nullptr;

	U8 frame = Renderer::GetFrameIndex();

	for (U8 i = 0; i < passCount; ++i)
	{
		Pipeline* pipeline = passes[i];
		commands->BindPipeline(pipeline);

		if (pipeline->renderpass != renderpass)
		{
			if (renderpass != nullptr) { prevRenderpass = renderpass; }

			renderpass = pipeline->renderpass;
			commands->BindPass(pipeline->renderpass);
		}

		commands->BindDescriptorSet(pipeline->descriptorSets[frame], pipeline->descriptorSetCount, nullptr, 0);

		if (pipeline->vertexBufferCount)
		{
			for (U8 i = 0; i < pipeline->vertexBufferCount; ++i)
			{
				commands->BindVertexBuffer(pipeline->vertexBuffers[i], i);
			}

			if (pipeline->indexBuffer)
			{
				commands->BindIndexBuffer(pipeline->indexBuffer);
				commands->DrawIndexed((U32)(pipeline->indexBuffer->size / 2), 1, 0, 0, 0);
			}
			else
			{
				commands->Draw(0, (U32)pipeline->vertexBuffers[0]->size / pipeline->shader.vertexSize, 0, 1);
			}
		}
		else
		{
			commands->Draw(0, 3, 0, 1);
		}
	}

	if (renderpass != nullptr) {}
}

void Program::DrawMesh(CommandBuffer* commands, Mesh& mesh, Buffer* constantBuffer)
{
	//TODO: Move to renderer
	Buffer* buffers[MAX_DESCRIPTORS_PER_SET]{ constantBuffer, mesh.materialBuffer };

	Resources::UpdateDescriptorSet(passes[0]->descriptorSets[Renderer::GetFrameIndex()][0], nullptr, buffers);

	passes[0]->indexBuffer = mesh.indexBuffer;
	passes[0]->vertexBuffers[0] = mesh.positionBuffer;
	passes[0]->vertexBuffers[1] = mesh.tangentBuffer;
	passes[0]->vertexBuffers[2] = mesh.normalBuffer;
	passes[0]->vertexBuffers[3] = mesh.texcoordBuffer;
	passes[0]->vertexBufferCount = 4;

	RunPasses(commands);
}
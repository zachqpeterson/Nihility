#pragma once

#include "Rendering\RenderingDefines.hpp"

typedef U32 ResourceHandle;
NH_HEADER_STATIC constexpr Traits<ResourceHandle>::Base INVALID_HANDLE = Traits<ResourceHandle>::MaxValue;

//TODO: Operators
struct SamplerHandle { ResourceHandle index; };
struct TextureHandle { ResourceHandle index; };
struct BufferHandle { ResourceHandle index; };
struct DescriptorSetLayoutHandle { ResourceHandle index; };
struct DescriptorSetHandle { ResourceHandle index; };
struct ShaderStateHandle { ResourceHandle index; };
struct RenderPassHandle { ResourceHandle index; };
struct PipelineHandle { ResourceHandle index; };

NH_HEADER_STATIC constexpr SamplerHandle				INVALID_SAMPLER{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr TextureHandle				INVALID_TEXTURE{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr BufferHandle					INVALID_BUFFER{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr DescriptorSetLayoutHandle	INVALID_LAYOUT{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr DescriptorSetHandle			INVALID_SET{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr ShaderStateHandle			INVALID_SHADER{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr RenderPassHandle				INVALID_PASS{ INVALID_HANDLE };
NH_HEADER_STATIC constexpr PipelineHandle				INVALID_PIPELINE{ INVALID_HANDLE };

enum ResourceDeleteType
{
	RESOURCE_DELETE_TYPE_BUFFER,
	RESOURCE_DELETE_TYPE_TEXTURE,
	RESOURCE_DELETE_TYPE_PIPELINE,
	RESOURCE_DELETE_TYPE_SAMPLER,
	RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT,
	RESOURCE_DELETE_TYPE_DESCRIPTOR_SET,
	RESOURCE_DELETE_TYPE_RENDER_PASS,
	RESOURCE_DELETE_TYPE_SHADER_STATE,

	RESOURCE_DELETE_TYPE_COUNT
};

struct ResourceUpdate
{
	ResourceDeleteType	type;
	ResourceHandle		handle;
	U32					currentFrame;
};

struct DescriptorSetUpdate
{
	DescriptorSetHandle	descriptorSet;
	U32					frameIssued = 0;
};

struct Sampler
{
	VkSampler				sampler;

	VkFilter				minFilter = VK_FILTER_NEAREST;
	VkFilter				magFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode		mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode	addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	SamplerHandle			handle;
	CSTR					name = nullptr;
};

struct Texture
{
	VkImage				image;
	VkImageView			imageView;
	VkFormat			format;
	VkImageLayout		imageLayout;
	VmaAllocation_T*	allocation;

	U16					width = 1;
	U16					height = 1;
	U16					depth = 1;
	U8					mipmaps = 1;
	U8					flags = 0;

	TextureType			type = TEXTURE_TYPE_2D;

	Sampler*			sampler = nullptr;

	TextureHandle		handle;
	CSTR				name = nullptr;
};

struct Buffer
{
	VkBuffer			buffer;
	VmaAllocation_T*	allocation;
	VkDeviceMemory		deviceMemory;
	VkDeviceSize		deviceSize;

	VkBufferUsageFlags	typeFlags = 0;
	ResourceUsage		usage = RESOURCE_USAGE_IMMUTABLE;
	U32					size = 0;
	U32					globalOffset = 0;	// Offset into global constant, if dynamic

	BufferHandle		handle;
	BufferHandle		parentBuffer;

	CSTR				name = nullptr;
};

struct DesciptorSetLayout
{
	VkDescriptorSetLayout			descriptorSetLayout;

	VkDescriptorSetLayoutBinding*	binding = nullptr;
	DescriptorBinding*				bindings = nullptr;
	U16								numBindings = 0;
	U16								setIndex = 0;

	DescriptorSetLayoutHandle		handle;
};

struct DesciptorSet
{
	VkDescriptorSet				descriptorSet;

	ResourceHandle*				resources = nullptr;
	SamplerHandle*				samplers = nullptr;
	U16*						bindings = nullptr;

	const DesciptorSetLayout*	layout = nullptr;
	U32							numResources = 0;

	DescriptorSetHandle			handle;
};

struct ShaderState
{
	VkPipelineShaderStageCreateInfo	shaderStageInfos[MAX_SHADER_STAGES];

	U32								activeShaders = 0;
	bool							graphicsPipeline = false;

	ShaderStateHandle				handle;
	CSTR							name = nullptr;
};

struct RenderPassOutput
{
	RenderPassOutput&	Reset();
	RenderPassOutput&	Color(VkFormat format);
	RenderPassOutput&	Depth(VkFormat format);
	RenderPassOutput&	SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

	VkFormat			colorFormats[MAX_IMAGE_OUTPUTS];
	VkFormat			depthStencilFormat;
	U32					numColorFormats;

	RenderPassOperation	colorOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation	depthOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation	stencilOperation = RENDER_PASS_OP_DONT_CARE;
};

struct RenderPass
{
	VkRenderPass		renderPass;
	VkFramebuffer		frameBuffer;

	RenderPassOutput	output;

	TextureHandle		outputTextures[MAX_IMAGE_OUTPUTS];
	TextureHandle		outputDepth;

	RenderPassType		type;

	F32					scaleX = 1.f;
	F32					scaleY = 1.f;
	U16					width = 0;
	U16					height = 0;
	U16					dispatchX = 0;
	U16					dispatchY = 0;
	U16					dispatchZ = 0;

	U8					resize = 0;
	U8					numRenderTargets = 0;

	RenderPassHandle	handle;
	CSTR				name = nullptr;
};

struct Pipeline
{
	VkPipeline					pipeline;
	VkPipelineLayout			pipelineLayout;

	VkPipelineBindPoint			bindPoint;

	ShaderStateHandle			shaderState;

	const DesciptorSetLayout*	descriptorSetLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	DescriptorSetLayoutHandle	descriptorSetLayoutHandles[MAX_DESCRIPTOR_SET_LAYOUTS];
	U32							numActiveLayouts = 0;

	DepthStencilCreation		depthStencil;
	BlendStateCreation			blendState;
	RasterizationCreation		rasterization;

	PipelineHandle				handle;
	bool						graphicsPipeline = true;
};

//Creation

struct SamplerCreation
{
	SamplerCreation&		SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip);
	SamplerCreation&		SetAddressModeU(VkSamplerAddressMode u);
	SamplerCreation&		SetAddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v);
	SamplerCreation&		SetAddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
	SamplerCreation&		SetName(CSTR name);

	VkFilter				minFilter = VK_FILTER_NEAREST;
	VkFilter				magFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode		mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode	addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	CSTR					name = nullptr;
};

struct TextureCreation
{
	TextureCreation&	SetSize(U16 width, U16 height, U16 depth);
	TextureCreation&	SetFlags(U8 mipmaps, U8 flags);
	TextureCreation&	SetFormatType(VkFormat format, TextureType type);
	TextureCreation&	SetName(CSTR name);
	TextureCreation&	SetData(void* data);

	void*				initialData = nullptr;
	U16					width = 1;
	U16					height = 1;
	U16					depth = 1;
	U8					mipmaps = 1;
	U8					flags = 0;    // TextureFlags bitmasks

	VkFormat			format = VK_FORMAT_UNDEFINED;
	TextureType			type = TEXTURE_TYPE_2D;

	CSTR				name = nullptr;
};

struct NH_API BufferCreation
{
	BufferCreation&		Reset();
	BufferCreation&		Set(VkBufferUsageFlags flags, ResourceUsage usage, U32 size);
	BufferCreation&		SetData(void* data);
	BufferCreation&		SetName(CSTR name);

	VkBufferUsageFlags	typeFlags = 0;
	ResourceUsage		usage = RESOURCE_USAGE_IMMUTABLE;
	U32					size = 0;
	void*				initialData = nullptr;

	CSTR				name = nullptr;
};

struct NH_API DescriptorSetLayoutCreation
{
	struct Binding
	{
		VkDescriptorType			type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		U16							start = 0;
		U16							count = 0;
		CSTR						name = nullptr;
	};

	DescriptorSetLayoutCreation&	Reset();
	DescriptorSetLayoutCreation&	AddBinding(const Binding& binding);
	DescriptorSetLayoutCreation&	SetName(CSTR name);
	DescriptorSetLayoutCreation&	SetSetIndex(U32 index);

	Binding							bindings[MAX_DESCRIPTORS_PER_SET];
	U32								numBindings = 0;
	U32								setIndex = 0;

	CSTR							name = nullptr;
};

struct DescriptorSetCreation
{
	DescriptorSetCreation&		Reset();
	DescriptorSetCreation&		SetLayout(DescriptorSetLayoutHandle layout);
	DescriptorSetCreation&		Texture(TextureHandle texture, U16 binding);
	DescriptorSetCreation&		Buffer(BufferHandle buffer, U16 binding);
	DescriptorSetCreation&		TextureSampler(TextureHandle texture, SamplerHandle sampler, U16 binding);   // TODO: separate samplers from textures
	DescriptorSetCreation&		SetName(CSTR name);

	ResourceHandle				resources[MAX_DESCRIPTORS_PER_SET];
	SamplerHandle				samplers[MAX_DESCRIPTORS_PER_SET];
	U16							bindings[MAX_DESCRIPTORS_PER_SET];

	DescriptorSetLayoutHandle	layout;
	U32							numResources = 0;

	CSTR						name = nullptr;
};

struct NH_API ShaderStateCreation
{
	ShaderStateCreation&	Reset();
	ShaderStateCreation&	SetName(CSTR name);
	ShaderStateCreation&	AddStage(CSTR path, VkShaderStageFlagBits type);
	ShaderStateCreation&	SetSpvInput(bool value);

	ShaderStage				stages[MAX_SHADER_STAGES];

	CSTR					name = nullptr;

	U32						stagesCount = 0;
	U32						spvInput = 0;
};

struct RenderPassCreation
{
	RenderPassCreation&		Reset();
	RenderPassCreation&		AddRenderTexture(TextureHandle texture);
	RenderPassCreation&		SetScaling(F32 scaleX, F32 scaleY, U8 resize);
	RenderPassCreation&		SetDepthStencilTexture(TextureHandle texture);
	RenderPassCreation&		SetName(CSTR name);
	RenderPassCreation&		SetType(RenderPassType type);
	RenderPassCreation&		SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

	U16						numRenderTargets = 0;
	RenderPassType			type = RENDER_PASS_TYPE_GEOMETRY;

	TextureHandle			outputTextures[MAX_IMAGE_OUTPUTS];
	TextureHandle			depthStencilTexture;

	F32						scaleX = 1.f;
	F32						scaleY = 1.f;
	U8						resize = 1;

	RenderPassOperation		colorOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation		depthOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation		stencilOperation = RENDER_PASS_OP_DONT_CARE;

	CSTR					name = nullptr;
};

struct NH_API PipelineCreation
{
	PipelineCreation&			AddDescriptorSetLayout(DescriptorSetLayoutHandle handle);
	RenderPassOutput&			GetRenderPassOutput();

	RasterizationCreation		rasterization;
	DepthStencilCreation		depthStencil;
	BlendStateCreation			blendState;
	VertexInputCreation			vertexInput;
	ShaderStateCreation			shaders;

	RenderPassOutput			renderPass;
	DescriptorSetLayoutHandle	descriptorSetLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	const ViewportState*		viewport = nullptr;

	U32							numActiveLayouts = 0;

	CSTR						name = nullptr;
};

struct MapBufferParameters
{
	BufferHandle	buffer;
	U32				offset = 0;
	U32				size = 0;
};

struct TextureBarrier
{
	TextureHandle texture;
};

struct BufferBarrier
{
	BufferHandle buffer;
};

struct ExecutionBarrier
{
	ExecutionBarrier& Reset();
	ExecutionBarrier& Set(PipelineStage source, PipelineStage destination);
	ExecutionBarrier& AddImageBarrier(const TextureBarrier& textureBarrier);
	ExecutionBarrier& AddMemoryBarrier(const BufferBarrier& bufferBarrier);

	PipelineStage	sourcePipelineStage;
	PipelineStage	destinationPipelineStage;

	U32				newBarrierExperimental = U32_MAX;
	U32				loadOperation = 0;

	U32				numTextureBarriers;
	U32				numBufferBarriers;

	TextureBarrier	textureBarriers[8];
	BufferBarrier	bufferBarriers[8];
};
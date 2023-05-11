#pragma once

#include "Rendering\RenderingDefines.hpp"

typedef U32 ResourceHandle;
NH_HEADER_STATIC constexpr Traits<ResourceHandle>::Base INVALID_ID = Traits<ResourceHandle>::MaxValue;

//TODO: Operators
struct SamplerHandle { ResourceHandle index; };
struct TextureHandle { ResourceHandle index; };
struct BufferHandle { ResourceHandle index; };
struct DescriptorSetLayoutHandle { ResourceHandle index; };
struct DescriptorSetHandle { ResourceHandle index; };
struct ShaderStateHandle { ResourceHandle index; };
struct RenderPassHandle { ResourceHandle index; };
struct PipelineHandle { ResourceHandle index; };

NH_HEADER_STATIC constexpr SamplerHandle				INVALID_SAMPLER{ INVALID_INDEX };
NH_HEADER_STATIC constexpr TextureHandle				INVALID_TEXTURE{ INVALID_INDEX };
NH_HEADER_STATIC constexpr BufferHandle					INVALID_BUFFER{ INVALID_INDEX };
NH_HEADER_STATIC constexpr DescriptorSetLayoutHandle	INVALID_LAYOUT{ INVALID_INDEX };
NH_HEADER_STATIC constexpr DescriptorSetHandle			INVALID_SET{ INVALID_INDEX };
NH_HEADER_STATIC constexpr ShaderStateHandle			INVALID_SHADER{ INVALID_INDEX };
NH_HEADER_STATIC constexpr RenderPassHandle				INVALID_PASS{ INVALID_INDEX };
NH_HEADER_STATIC constexpr PipelineHandle				INVALID_PIPELINE{ INVALID_INDEX };

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
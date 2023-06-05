#pragma once

#include "Rendering\RenderingDefines.hpp"
#include "Containers\String.hpp"
#include "Containers\Hashmap.hpp"

enum ResourceUpdateType
{
	RESOURCE_UPDATE_TYPE_BUFFER,
	RESOURCE_UPDATE_TYPE_TEXTURE,
	RESOURCE_UPDATE_TYPE_PIPELINE,
	RESOURCE_UPDATE_TYPE_SAMPLER,
	RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET_LAYOUT,
	RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET,
	RESOURCE_UPDATE_TYPE_RENDER_PASS,
	RESOURCE_UPDATE_TYPE_SHADER_STATE,

	RESOURCE_UPDATE_TYPE_COUNT
};

enum AlphaMode
{
	ALPHA_MODE_OPAQUE,
	ALPHA_MODE_MASK,
	ALPHA_MODE_TRANSPARENT,
};

//Creation

struct NH_API SamplerCreation
{
	SamplerCreation& SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip);
	SamplerCreation& SetAddressModeU(VkSamplerAddressMode u);
	SamplerCreation& SetAddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v);
	SamplerCreation& SetAddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
	SamplerCreation& SetName(const String& name);

	VkFilter				minFilter = VK_FILTER_NEAREST;
	VkFilter				magFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode		mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode	addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	String					name{ NO_INIT };
};

struct NH_API TextureCreation
{
	TextureCreation& SetSize(U16 width, U16 height, U16 depth);
	TextureCreation& SetFlags(U8 mipmaps, U8 flags);
	TextureCreation& SetFormatType(VkFormat format, TextureType type);
	TextureCreation& SetName(const String& name);
	TextureCreation& SetData(void* data);

	void* initialData = nullptr;
	U16					width = 1;
	U16					height = 1;
	U16					depth = 1;
	U8					mipmaps = 1;
	U8					flags = 0;    // TextureFlags bitmasks

	VkFormat			format = VK_FORMAT_UNDEFINED;
	TextureType			type = TEXTURE_TYPE_2D;

	String				name{ NO_INIT };
};

struct NH_API DescriptorSetLayoutCreation
{
	DescriptorSetLayoutCreation& Reset();
	DescriptorSetLayoutCreation& AddBinding(const DescriptorBinding& binding);
	DescriptorSetLayoutCreation& AddBindingAtIndex(const DescriptorBinding& binding, U32 index);
	DescriptorSetLayoutCreation& SetName(const String& name);
	DescriptorSetLayoutCreation& SetSetIndex(U32 index);

	DescriptorBinding				bindings[MAX_DESCRIPTORS_PER_SET];
	U32								bindingCount = 0;
	U32								setIndex = 0;

	String							name{ NO_INIT };
};

struct NH_API ShaderStateCreation
{
	ShaderStateCreation& Reset();
	ShaderStateCreation& SetName(const String& name);
	ShaderStateCreation& AddStage(CSTR name, VkShaderStageFlagBits type);
	ShaderStateCreation& SetSpvInput(bool value);

	ShaderStage				stages[MAX_SHADER_STAGES];

	String					name{ NO_INIT };

	U32						stagesCount = 0;
};

// Resources

struct Sampler
{
	String					name{ NO_INIT };
	HashHandle				handle;
	U32						sceneID;

	VkFilter				minFilter = VK_FILTER_NEAREST;
	VkFilter				magFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode		mipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;

	VkSamplerAddressMode	addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkSampler				sampler;
};

struct Texture
{
	String				name{ NO_INIT };
	HashHandle			handle;
	U32					sceneID;

	U16					width = 1;
	U16					height = 1;
	U16					depth = 1;
	U8					mipmaps = 1;
	U8					flags = 0;

	TextureType			type = TEXTURE_TYPE_2D;

	VkImage				image;
	VkImageView			imageView;
	VkFormat			format;
	VkImageLayout		imageLayout;
	VmaAllocation_T* allocation;

	Sampler* sampler{ nullptr };
};

struct Buffer
{
	String				name{ NO_INIT };
	HashHandle			handle;
	U32					sceneID;

	Buffer* parentBuffer;

	VkBufferUsageFlags	typeFlags = 0;
	ResourceUsage		usage = RESOURCE_USAGE_IMMUTABLE;
	U64					size = 0;
	U64					globalOffset = 0;	// Offset into global constant, if dynamic

	VkBuffer			buffer;
	VmaAllocation_T* allocation;
	VkDeviceMemory		deviceMemory;
	VkDeviceSize		deviceSize;
};

struct NH_API BufferCreation
{
	BufferCreation& Reset();
	BufferCreation& Set(VkBufferUsageFlags flags, ResourceUsage usage, U64 size);
	BufferCreation& SetData(void* data);
	BufferCreation& SetName(const String& name);
	BufferCreation& SetParent(Buffer* parent, U64 offset);

	VkBufferUsageFlags	typeFlags{ 0 };
	ResourceUsage		usage{ RESOURCE_USAGE_IMMUTABLE };
	U64					size{ 0 };
	U64					offset{ 0 };
	void* initialData{ nullptr };
	Buffer* parentBuffer{ nullptr };

	String				name{ NO_INIT };
};

struct DescriptorSetLayout
{
	String							name{ NO_INIT };
	HashHandle						handle;

	VkDescriptorSetLayout			descriptorSetLayout;

	VkDescriptorSetLayoutBinding* binding = nullptr;
	DescriptorBinding* bindings = nullptr;
	U16								bindingCount = 0;
	U16								setIndex = 0;
};

struct DescriptorSet
{
	String						name{ NO_INIT };
	HashHandle					handle;

	VkDescriptorSet				descriptorSet;

	void** resources = nullptr;
	Sampler** samplers = nullptr;
	U16* bindings = nullptr;

	DescriptorSetLayout* layout = nullptr;
	U32						resourceCount = 0;
};

struct ShaderState
{
	String							name{ NO_INIT };
	String							entry{ NO_INIT };
	HashHandle						handle;

	VkPipelineShaderStageCreateInfo	shaderStageInfos[MAX_SHADER_STAGES];

	U32								activeShaders{ 0 };
	bool							graphicsPipeline{ false };

	U32								setCount{ 0 };
	DescriptorSetLayoutCreation		sets[MAX_SET_COUNT];

	U32								vertexStreamCount{ 0 };
	U32								vertexAttributeCount{ 0 };
	VertexStream					vertexStreams[MAX_VERTEX_STREAMS];
	VertexAttribute					vertexAttributes[MAX_VERTEX_ATTRIBUTES];
};

struct RenderPassOutput
{
	RenderPassOutput& Reset();
	RenderPassOutput& Color(VkFormat format);
	RenderPassOutput& Depth(VkFormat format);
	RenderPassOutput& SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

	VkFormat			colorFormats[MAX_IMAGE_OUTPUTS];
	VkFormat			depthStencilFormat;
	U32					colorFormatCount;

	RenderPassOperation	colorOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation	depthOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation	stencilOperation = RENDER_PASS_OP_DONT_CARE;
};

struct RenderPass
{
	String				name{ NO_INIT };
	HashHandle			handle;

	VkRenderPass		renderPass;
	VkFramebuffer		frameBuffer;

	RenderPassOutput	output;

	Texture* outputTextures[MAX_IMAGE_OUTPUTS];
	Texture* outputDepth;

	RenderPassType		type;

	F32					scaleX = 1.f;
	F32					scaleY = 1.f;
	U16					width = 0;
	U16					height = 0;
	U16					dispatchX = 0;
	U16					dispatchY = 0;
	U16					dispatchZ = 0;

	U8					resize = 0;
	U8					renderTargetCount = 0;
};

struct Pipeline
{
	String						name{ NO_INIT };
	HashHandle					handle;

	VkPipeline					pipeline;
	VkPipelineLayout			pipelineLayout;

	VkPipelineBindPoint			bindPoint;

	ShaderState* shaderState;

	DescriptorSetLayout* descriptorSetLayouts[MAX_DESCRIPTOR_SET_LAYOUTS];
	U32							activeLayoutCount = 0;

	DepthStencilCreation		depthStencil;
	BlendStateCreation			blendState;
	RasterizationCreation		rasterization;

	bool						graphicsPipeline = true;
};

struct ProgramPass
{
	Pipeline* pipeline{ nullptr };
	DescriptorSetLayout* descriptorSetLayout{ nullptr };
};

struct Program
{
	String				name;
	Vector<ProgramPass>	passes;
	U32					poolIndex;
};

struct NH_API DescriptorSetCreation
{
	DescriptorSetCreation& Reset();
	DescriptorSetCreation& SetLayout(DescriptorSetLayout* layout);
	DescriptorSetCreation& SetTexture(Texture* texture, U16 binding);
	DescriptorSetCreation& SetBuffer(Buffer* buffer, U16 binding);
	DescriptorSetCreation& SetTextureSampler(Texture* texture, Sampler* sampler, U16 binding);   // TODO: separate samplers from textures
	DescriptorSetCreation& SetName(const String& name);

	void* resources[MAX_DESCRIPTORS_PER_SET];
	Sampler* samplers[MAX_DESCRIPTORS_PER_SET];
	U16						bindings[MAX_DESCRIPTORS_PER_SET];

	DescriptorSetLayout* layout;
	U32						resourceCount = 0;

	String					name{ NO_INIT };
};

struct NH_API RenderPassCreation
{
	RenderPassCreation& Reset();
	RenderPassCreation& AddRenderTexture(Texture* texture);
	RenderPassCreation& SetScaling(F32 scaleX, F32 scaleY, U8 resize);
	RenderPassCreation& SetDepthStencilTexture(Texture* texture);
	RenderPassCreation& SetName(const String& name);
	RenderPassCreation& SetType(RenderPassType type);
	RenderPassCreation& SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

	U16						renderTargetCount = 0;
	RenderPassType			type = RENDER_PASS_TYPE_GEOMETRY;

	Texture* outputTextures[MAX_IMAGE_OUTPUTS];
	Texture* depthStencilTexture;

	F32						scaleX = 1.f;
	F32						scaleY = 1.f;
	U8						resize = 1;

	RenderPassOperation		colorOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation		depthOperation = RENDER_PASS_OP_DONT_CARE;
	RenderPassOperation		stencilOperation = RENDER_PASS_OP_DONT_CARE;

	String					name{ NO_INIT };
};

struct NH_API PipelineCreation
{
	RenderPassOutput& GetRenderPassOutput();

	RasterizationCreation		rasterization;
	DepthStencilCreation		depthStencil;
	BlendStateCreation			blendState;
	ShaderStateCreation			shaders;

	RenderPassOutput			renderPass;
	const ViewportState* viewport = nullptr;

	String						name{ NO_INIT };
};

struct NH_API ProgramCreation
{
	PipelineCreation	pipelineCreation;
};

struct NH_API MaterialCreation
{
	MaterialCreation& Reset();
	MaterialCreation& SetProgram(Program* program);
	MaterialCreation& SetName(const String& name);
	MaterialCreation& SetRenderIndex(U32 renderIndex);

	Program* program{ nullptr };
	String		name{ NO_INIT };
	U32			renderIndex{ U32_MAX };

}; // struct MaterialCreation

//
//
struct NH_API Material
{
	Program* program{ nullptr };

	U32			renderIndex;
	U32			poolIndex;

	String		name{ NO_INIT };
};

struct UniformData
{
	Matrix4 vp;
	Vector4 eye;
	Vector4 light;
	F32		lightRange;
	F32		lightIntensity;
};

struct MeshData
{
	Matrix4		model;
	Matrix4		modelInv;

	Vector4Int	textures; // diffuse, roughness, normal, occlusion
	Vector4		baseColorFactor;
	Vector4		metalRoughOcclFactor;
	Vector4		emissiveFactor;
	F32			alphaCutoff;
	F32			unused[3];
	U32			flags;
};

struct NH_API MeshDraw
{
	Material* material;

	Buffer* indexBuffer;
	Buffer* positionBuffer;
	Buffer* tangentBuffer;
	Buffer* normalBuffer;
	Buffer* texcoordBuffer;
	Buffer* materialBuffer;

	U32			primitiveCount;

	//These are HashHandles, used in bindless resources
	U16			diffuseTextureIndex{ U16_MAX };
	U16			metalRoughOcclTextureIndex{ U16_MAX };
	U16			normalTextureIndex{ U16_MAX };
	U16			emissivityTextureIndex{ U16_MAX };

	Vector4		baseColorFactor{ Vector4::One };
	Vector4		metalRoughOcclFactor{ Vector4::One }; //TODO: Look into making these Vector3
	Vector4		emissiveFactor{ Vector4::Zero };

	//TODO: Transform component
	Vector3		position{ Vector3::Zero };
	Quaternion3	rotation{ Quaternion3::Identity };
	Vector3		scale{ Vector3::One };

	F32			alphaCutoff{ 0.0f };
	U32			flags{ 0 };
};

struct NH_API Transform
{
	Vector3 position;
	Vector3 scale;
	Quaternion3 rotation;

	void CalculateMatrix(Matrix4& matrix)
	{
		matrix.Set(position, rotation, scale);
	}
};

struct ResourceUpdate
{
	ResourceUpdateType	type;
	HashHandle			handle;
	U32					currentFrame;
};

struct DescriptorSetUpdate
{
	DescriptorSet* descriptorSet;
	U32				frameIssued = 0;
};

struct MapBufferParameters
{
	Buffer* buffer;
	U64		offset = 0;
	U64		size = 0;
};

struct TextureBarrier
{
	Texture* texture;
};

struct BufferBarrier
{
	Buffer* buffer;
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

	U32				textureBarrierCount;
	U32				bufferBarrierCount;

	TextureBarrier	textureBarriers[8];
	BufferBarrier	bufferBarriers[8];
};
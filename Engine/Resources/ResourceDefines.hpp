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

enum KTXType
{
	KTX_TYPE_COMPRESSED = 0x0,
	KTX_TYPE_BYTE = 0x1400,
	KTX_TYPE_UNSIGNED_BYTE = 0x1401,
	KTX_TYPE_SHORT = 0x1402,
	KTX_TYPE_UNSIGNED_SHORT = 0x1403,
	KTX_TYPE_INT = 0x1404,
	KTX_TYPE_UNSIGNED_INT = 0x1405,
	KTX_TYPE_FLOAT = 0x1406,
	KTX_TYPE_DOUBLE = 0x140A,
	KTX_TYPE_HALF_FLOAT = 0x140B,
	KTX_TYPE_UNSIGNED_BYTE_3_3_2 = 0x8032,
	KTX_TYPE_UNSIGNED_SHORT_4_4_4_4 = 0x8033,
	KTX_TYPE_UNSIGNED_SHORT_5_5_5_1 = 0x8034,
	KTX_TYPE_UNSIGNED_INT_8_8_8_8 = 0x8035,
	KTX_TYPE_UNSIGNED_INT_10_10_10_2 = 0x8036,
	KTX_TYPE_UNSIGNED_BYTE_2_3_3_REV = 0x8362,
	KTX_TYPE_UNSIGNED_SHORT_5_6_5 = 0x8363,
	KTX_TYPE_UNSIGNED_SHORT_5_6_5_REV = 0x8364,
	KTX_TYPE_UNSIGNED_SHORT_4_4_4_4_REV = 0x8365,
	KTX_TYPE_UNSIGNED_SHORT_1_5_5_5_REV = 0x8366,
	KTX_TYPE_UNSIGNED_INT_8_8_8_8_REV = 0x8367,
	KTX_TYPE_UNSIGNED_INT_2_10_10_10_REV = 0x8368,
	KTX_TYPE_UNSIGNED_INT_24_8 = 0x84FA,
	KTX_TYPE_UNSIGNED_INT_5_9_9_9_REV = 0x8C3E,
	KTX_TYPE_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B,
	KTX_TYPE_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD
};

enum KTXFormat
{
	KTX_FORMAT_RED = 0x1903,
	KTX_FORMAT_GREEN = 0x1904,
	KTX_FORMAT_BLUE = 0x1905,
	KTX_FORMAT_ALPHA = 0x1906,
	KTX_FORMAT_RGB = 0x1907,
	KTX_FORMAT_RGBA = 0x1908,
	KTX_FORMAT_LUMINANCE = 0x1909,
	KTX_FORMAT_LUMINANCE_ALPHA = 0x190A,
	KTX_FORMAT_ABGR = 0x8000,
	KTX_FORMAT_INTENSITY = 0x8049,
	KTX_FORMAT_BGR = 0x80E0,
	KTX_FORMAT_BGRA = 0x80E1,
	KTX_FORMAT_RG = 0x8227,
	KTX_FORMAT_RG_INTEGER = 0x8228,
	KTX_FORMAT_SRGB = 0x8C40,
	KTX_FORMAT_SRGB_ALPHA = 0x8C42,
	KTX_FORMAT_SLUMINANCE_ALPHA = 0x8C44,
	KTX_FORMAT_SLUMINANCE = 0x8C46,
	KTX_FORMAT_RED_INTEGER = 0x8D94,
	KTX_FORMAT_GREEN_INTEGER = 0x8D95,
	KTX_FORMAT_BLUE_INTEGER = 0x8D96,
	KTX_FORMAT_ALPHA_INTEGER = 0x8D97,
	KTX_FORMAT_RGB_INTEGER = 0x8D98,
	KTX_FORMAT_RGBA_INTEGER = 0x8D99,
	KTX_FORMAT_BGR_INTEGER = 0x8D9A,
	KTX_FORMAT_BGRA_INTEGER = 0x8D9B,
	KTX_FORMAT_RED_SNORM = 0x8F90,
	KTX_FORMAT_RG_SNORM = 0x8F91,
	KTX_FORMAT_RGB_SNORM = 0x8F92,
	KTX_FORMAT_RGBA_SNORM = 0x8F93,

	KTX_FORMAT_INT_ALPHA4 = 0x803B,
	KTX_FORMAT_INT_ALPHA8 = 0x803C,
	KTX_FORMAT_INT_ALPHA12 = 0x803D,
	KTX_FORMAT_INT_ALPHA16 = 0x803E,
	KTX_FORMAT_INT_LUMINANCE4 = 0x803F,
	KTX_FORMAT_INT_LUMINANCE8 = 0x8040,
	KTX_FORMAT_INT_LUMINANCE12 = 0x8041,
	KTX_FORMAT_INT_LUMINANCE16 = 0x8042,
	KTX_FORMAT_INT_LUMINANCE4_ALPHA4 = 0x8043,
	KTX_FORMAT_INT_LUMINANCE6_ALPHA2 = 0x8044,
	KTX_FORMAT_INT_LUMINANCE8_ALPHA8 = 0x8045,
	KTX_FORMAT_INT_LUMINANCE12_ALPHA4 = 0x8046,
	KTX_FORMAT_INT_LUMINANCE12_ALPHA12 = 0x8047,
	KTX_FORMAT_INT_LUMINANCE16_ALPHA16 = 0x8048,
	KTX_FORMAT_INT_INTENSITY4 = 0x804A,
	KTX_FORMAT_INT_INTENSITY8 = 0x804B,
	KTX_FORMAT_INT_INTENSITY12 = 0x804C,
	KTX_FORMAT_INT_INTENSITY16 = 0x804D,
	KTX_FORMAT_INT_RGB2 = 0x804E,
	KTX_FORMAT_INT_RGB4 = 0x804F,
	KTX_FORMAT_INT_RGB5 = 0x8050,
	KTX_FORMAT_INT_RGB8 = 0x8051,
	KTX_FORMAT_INT_RGB10 = 0x8052,
	KTX_FORMAT_INT_RGB12 = 0x8053,
	KTX_FORMAT_INT_RGB16 = 0x8054,
	KTX_FORMAT_INT_RGBA2 = 0x8055,
	KTX_FORMAT_INT_RGBA4 = 0x8056,
	KTX_FORMAT_INT_RGB5_A1 = 0x8057,
	KTX_FORMAT_INT_RGBA8 = 0x8058,
	KTX_FORMAT_INT_RGB10_A2 = 0x8059,
	KTX_FORMAT_INT_RGBA12 = 0x805A,
	KTX_FORMAT_INT_RGBA16 = 0x805B,
	KTX_FORMAT_INT_R8 = 0x8229,
	KTX_FORMAT_INT_R16 = 0x822A,
	KTX_FORMAT_INT_RG8 = 0x822B,
	KTX_FORMAT_INT_RG16 = 0x822C,
	KTX_FORMAT_INT_R16F = 0x822D,
	KTX_FORMAT_INT_R32F = 0x822E,
	KTX_FORMAT_INT_RG16F = 0x822F,
	KTX_FORMAT_INT_RG32F = 0x8230,
	KTX_FORMAT_INT_R8I = 0x8231,
	KTX_FORMAT_INT_R8UI = 0x8232,
	KTX_FORMAT_INT_R16I = 0x8233,
	KTX_FORMAT_INT_R16UI = 0x8234,
	KTX_FORMAT_INT_R32I = 0x8235,
	KTX_FORMAT_INT_R32UI = 0x8236,
	KTX_FORMAT_INT_RG8I = 0x8237,
	KTX_FORMAT_INT_RG8UI = 0x8238,
	KTX_FORMAT_INT_RG16I = 0x8239,
	KTX_FORMAT_INT_RG16UI = 0x823A,
	KTX_FORMAT_INT_RG32I = 0x823B,
	KTX_FORMAT_INT_RG32UI = 0x823C,
	KTX_FORMAT_INT_RGBA32F = 0x8814,
	KTX_FORMAT_INT_RGB32F = 0x8815,
	KTX_FORMAT_INT_RGBA16F = 0x881A,
	KTX_FORMAT_INT_RGB16F = 0x881B,
	KTX_FORMAT_INT_R11F_G11F_B10F = 0x8C3A,
	KTX_FORMAT_INT_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B,
	KTX_FORMAT_INT_RGB9_E5 = 0x8C3D,
	KTX_FORMAT_INT_SRGB8 = 0x8C41,
	KTX_FORMAT_INT_SRGB8_ALPHA8 = 0x8C43,
	KTX_FORMAT_INT_SLUMINANCE8_ALPHA8 = 0x8C45,
	KTX_FORMAT_INT_SLUMINANCE8 = 0x8C47,
	KTX_FORMAT_INT_RGB565 = 0x8D62,
	KTX_FORMAT_INT_RGBA32UI = 0x8D70,
	KTX_FORMAT_INT_RGB32UI = 0x8D71,
	KTX_FORMAT_INT_RGBA16UI = 0x8D76,
	KTX_FORMAT_INT_RGB16UI = 0x8D77,
	KTX_FORMAT_INT_RGBA8UI = 0x8D7C,
	KTX_FORMAT_INT_RGB8UI = 0x8D7D,
	KTX_FORMAT_INT_RGBA32I = 0x8D82,
	KTX_FORMAT_INT_RGB32I = 0x8D83,
	KTX_FORMAT_INT_RGBA16I = 0x8D88,
	KTX_FORMAT_INT_RGB16I = 0x8D89,
	KTX_FORMAT_INT_RGBA8I = 0x8D8E,
	KTX_FORMAT_INT_RGB8I = 0x8D8F,
	KTX_FORMAT_INT_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
	KTX_FORMAT_INT_R8_SNORM = 0x8F94,
	KTX_FORMAT_INT_RG8_SNORM = 0x8F95,
	KTX_FORMAT_INT_RGB8_SNORM = 0x8F96,
	KTX_FORMAT_INT_RGBA8_SNORM = 0x8F97,
	KTX_FORMAT_INT_R16_SNORM = 0x8F98,
	KTX_FORMAT_INT_RG16_SNORM = 0x8F99,
	KTX_FORMAT_INT_RGB16_SNORM = 0x8F9A,
	KTX_FORMAT_INT_RGBA16_SNORM = 0x8F9B,
	KTX_FORMAT_INT_ALPHA8_SNORM = 0x9014,
	KTX_FORMAT_INT_LUMINANCE8_SNORM = 0x9015,
	KTX_FORMAT_INT_LUMINANCE8_ALPHA8_SNORM = 0x9016,
	KTX_FORMAT_INT_INTENSITY8_SNORM = 0x9017,
	KTX_FORMAT_INT_ALPHA16_SNORM = 0x9018,
	KTX_FORMAT_INT_LUMINANCE16_SNORM = 0x9019,
	KTX_FORMAT_INT_LUMINANCE16_ALPHA16_SNORM = 0x901A,
	KTX_FORMAT_INT_INTENSITY16_SNORM = 0x901B,

	KTX_FORMAT_PALETTE4_RGB8_OES = 0x8B90,
	KTX_FORMAT_PALETTE4_RGBA8_OES = 0x8B91,
	KTX_FORMAT_PALETTE4_R5_G6_B5_OES = 0x8B92,
	KTX_FORMAT_PALETTE4_RGBA4_OES = 0x8B93,
	KTX_FORMAT_PALETTE4_RGB5_A1_OES = 0x8B94,
	KTX_FORMAT_PALETTE8_RGB8_OES = 0x8B95,
	KTX_FORMAT_PALETTE8_RGBA8_OES = 0x8B96,
	KTX_FORMAT_PALETTE8_R5_G6_B5_OES = 0x8B97,
	KTX_FORMAT_PALETTE8_RGBA4_OES = 0x8B98,
	KTX_FORMAT_PALETTE8_RGB5_A1_OES = 0x8B99
};

enum KTXCompression
{
	KTX_COMPRESSION_RGB_S3TC_DXT1 = 0x83F0,
	KTX_COMPRESSION_RGBA_S3TC_DXT1 = 0x83F1,
	KTX_COMPRESSION_RGBA_S3TC_DXT3 = 0x83F2,
	KTX_COMPRESSION_RGBA_S3TC_DXT5 = 0x83F3,
	KTX_COMPRESSION_3DC_X_AMD = 0x87F9,
	KTX_COMPRESSION_3DC_XY_AMD = 0x87FA,
	KTX_COMPRESSION_ATC_RGBA_INTERPOLATED_ALPHA = 0x87EE,
	KTX_COMPRESSION_SRGB_PVRTC_2BPPV1 = 0x8A54,
	KTX_COMPRESSION_SRGB_PVRTC_4BPPV1 = 0x8A55,
	KTX_COMPRESSION_SRGB_ALPHA_PVRTC_2BPPV1 = 0x8A56,
	KTX_COMPRESSION_SRGB_ALPHA_PVRTC_4BPPV1 = 0x8A57,
	KTX_COMPRESSION_RGB_PVRTC_4BPPV1 = 0x8C00,
	KTX_COMPRESSION_RGB_PVRTC_2BPPV1 = 0x8C01,
	KTX_COMPRESSION_RGBA_PVRTC_4BPPV1 = 0x8C02,
	KTX_COMPRESSION_RGBA_PVRTC_2BPPV1 = 0x8C03,
	KTX_COMPRESSION_SRGB_S3TC_DXT1 = 0x8C4C,
	KTX_COMPRESSION_SRGB_ALPHA_S3TC_DXT1 = 0x8C4D,
	KTX_COMPRESSION_SRGB_ALPHA_S3TC_DXT3 = 0x8C4E,
	KTX_COMPRESSION_SRGB_ALPHA_S3TC_DXT5 = 0x8C4F,
	KTX_COMPRESSION_LUMINANCE_LATC1 = 0x8C70,
	KTX_COMPRESSION_SIGNED_LUMINANCE_LATC1 = 0x8C71,
	KTX_COMPRESSION_LUMINANCE_ALPHA_LATC2 = 0x8C72,
	KTX_COMPRESSION_SIGNED_LUMINANCE_ALPHA_LATC2 = 0x8C73,
	KTX_COMPRESSION_ATC_RGB = 0x8C92,
	KTX_COMPRESSION_ATC_RGBA_EXPLICIT_ALPHA = 0x8C93,
	KTX_COMPRESSION_RED_RGTC1 = 0x8DBB,
	KTX_COMPRESSION_SIGNED_RED_RGTC1 = 0x8DBC,
	KTX_COMPRESSION_RED_GREEN_RGTC2 = 0x8DBD,
	KTX_COMPRESSION_SIGNED_RED_GREEN_RGTC2 = 0x8DBE,
	KTX_COMPRESSION_ETC1_RGB8_OES = 0x8D64,
	KTX_COMPRESSION_RGBA_BPTC_UNORM = 0x8E8C,
	KTX_COMPRESSION_SRGB_ALPHA_BPTC_UNORM = 0x8E8D,
	KTX_COMPRESSION_RGB_BPTC_SIGNED_FLOAT = 0x8E8E,
	KTX_COMPRESSION_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F,
	KTX_COMPRESSION_R11_EAC = 0x9270,
	KTX_COMPRESSION_SIGNED_R11_EAC = 0x9271,
	KTX_COMPRESSION_RG11_EAC = 0x9272,
	KTX_COMPRESSION_SIGNED_RG11_EAC = 0x9273,
	KTX_COMPRESSION_RGB8_ETC2 = 0x9274,
	KTX_COMPRESSION_SRGB8_ETC2 = 0x9275,
	KTX_COMPRESSION_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276,
	KTX_COMPRESSION_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,
	KTX_COMPRESSION_RGBA8_ETC2_EAC = 0x9278,
	KTX_COMPRESSION_SRGB8_ALPHA8_ETC2_EAC = 0x9279,
	KTX_COMPRESSION_SRGB_ALPHA_PVRTC_2BPPV2 = 0x93F0,
	KTX_COMPRESSION_SRGB_ALPHA_PVRTC_4BPPV2 = 0x93F1,
	KTX_COMPRESSION_RGBA_ASTC_4x4 = 0x93B0,
	KTX_COMPRESSION_RGBA_ASTC_5x4 = 0x93B1,
	KTX_COMPRESSION_RGBA_ASTC_5x5 = 0x93B2,
	KTX_COMPRESSION_RGBA_ASTC_6x5 = 0x93B3,
	KTX_COMPRESSION_RGBA_ASTC_6x6 = 0x93B4,
	KTX_COMPRESSION_RGBA_ASTC_8x5 = 0x93B5,
	KTX_COMPRESSION_RGBA_ASTC_8x6 = 0x93B6,
	KTX_COMPRESSION_RGBA_ASTC_8x8 = 0x93B7,
	KTX_COMPRESSION_RGBA_ASTC_10x5 = 0x93B8,
	KTX_COMPRESSION_RGBA_ASTC_10x6 = 0x93B9,
	KTX_COMPRESSION_RGBA_ASTC_10x8 = 0x93BA,
	KTX_COMPRESSION_RGBA_ASTC_10x10 = 0x93BB,
	KTX_COMPRESSION_RGBA_ASTC_12x10 = 0x93BC,
	KTX_COMPRESSION_RGBA_ASTC_12x12 = 0x93BD,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_4x4 = 0x93D0,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_5x4 = 0x93D1,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_5x5 = 0x93D2,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_6x5 = 0x93D3,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_6x6 = 0x93D4,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_8x5 = 0x93D5,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_8x6 = 0x93D6,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_8x8 = 0x93D7,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_10x5 = 0x93D8,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_10x6 = 0x93D9,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_10x8 = 0x93DA,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_10x10 = 0x93DB,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_12x10 = 0x93DC,
	KTX_COMPRESSION_SRGB8_ALPHA8_ASTC_12x12 = 0x93DD
};

struct Sampler
{
	void Destroy() { name.Destroy(); }

	String					name{ NO_INIT };
	HashHandle				handle{ U64_MAX };
	U32						sceneID{ U32_MAX };

	VkFilter				minFilter{ VK_FILTER_NEAREST };
	VkFilter				magFilter{ VK_FILTER_NEAREST };
	VkSamplerMipmapMode		mipFilter{ VK_SAMPLER_MIPMAP_MODE_NEAREST };

	VkSamplerAddressMode	addressModeU{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkSamplerAddressMode	addressModeV{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkSamplerAddressMode	addressModeW{ VK_SAMPLER_ADDRESS_MODE_REPEAT };

	VkBorderColor			border{ VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE };

	VkSampler				sampler{ nullptr };
};

struct SamplerCreation
{
	void Destroy() { name.Destroy(); }

	SamplerCreation& SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip);
	SamplerCreation& SetAddressModeU(VkSamplerAddressMode u);
	SamplerCreation& SetAddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v);
	SamplerCreation& SetAddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
	SamplerCreation& SetName(const String& name);

	VkFilter				minFilter{ VK_FILTER_NEAREST };
	VkFilter				magFilter{ VK_FILTER_NEAREST };
	VkSamplerMipmapMode		mipFilter{ VK_SAMPLER_MIPMAP_MODE_NEAREST };

	VkSamplerAddressMode	addressModeU{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkSamplerAddressMode	addressModeV{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
	VkSamplerAddressMode	addressModeW{ VK_SAMPLER_ADDRESS_MODE_REPEAT };

	VkBorderColor			border{ VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE };

	String					name{ NO_INIT };
};

struct Texture
{
	void Destroy() { name.Destroy(); }

	String				name{ NO_INIT };
	HashHandle			handle{ U64_MAX };
	U32					sceneID{ U32_MAX };

	U64					size{ 0 };
	U16					width{ 1 };
	U16					height{ 1 };
	U16					depth{ 1 };
	U8					mipmaps{ 1 };
	U8					flags{ 0 };

	TextureType			type{ TEXTURE_TYPE_2D };

	VkImage				image{ nullptr };
	VkImageView			imageView{ nullptr };
	VkFormat			format;
	VkImageLayout		imageLayout;
	VmaAllocation_T* allocation{ nullptr };

	Sampler* sampler{ nullptr };

	bool				swapchainImage{ false };
};

struct TextureCreation
{
	void Destroy() { name.Destroy(); }

	TextureCreation& SetSize(U16 width, U16 height, U16 depth);
	TextureCreation& SetFlags(U8 mipmaps, U8 flags);
	TextureCreation& SetFormatType(VkFormat format, TextureType type);
	TextureCreation& SetName(const String& name);
	TextureCreation& SetData(void* data);

	void* initialData{ nullptr };
	U16					width{ 1 };
	U16					height{ 1 };
	U16					depth{ 1 };
	U8					mipmaps{ 1 };
	U8					flags{ 0 };

	VkFormat			format{ VK_FORMAT_UNDEFINED };
	TextureType			type{ TEXTURE_TYPE_2D };

	String				name{ NO_INIT };
};

struct Buffer
{
	void Destroy() { name.Destroy(); }

	String				name{ NO_INIT };
	HashHandle			handle{ U64_MAX };
	U32					sceneID{ U32_MAX };

	Buffer* parentBuffer{ nullptr };

	VkBufferUsageFlags	typeFlags{ 0 };
	ResourceUsage		usage{ RESOURCE_USAGE_IMMUTABLE };
	U64					size{ 0 };
	U64					globalOffset{ 0 };

	VkBuffer			buffer{ nullptr };
	VmaAllocation_T* allocation{ nullptr };
	VkDeviceMemory		deviceMemory{ nullptr };
	VkDeviceSize		deviceSize{ 0 };
};

struct BufferCreation
{
	void Destroy() { name.Destroy(); }

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

struct DescriptorBinding
{
	VkDescriptorType	type;
	U16					binding{ 0 };
	U16					count{ 0 };
};

struct DescriptorSetLayout
{
	HashHandle						handle{ U64_MAX };

	VkDescriptorSetLayout			descriptorSetLayout{ nullptr };

	VkDescriptorSetLayoutBinding	vkBindings[MAX_DESCRIPTORS_PER_SET]{};
	DescriptorBinding				bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8								bindingCount{ 0 };
	U8								setIndex{ 0 };
};

struct DescriptorSetLayoutCreation
{
	DescriptorBinding	bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8					bindingCount{ 0 };
	U8					setIndex{ 0 };
};

struct DescriptorSet
{
	HashHandle				handle{ U64_MAX };

	VkDescriptorSet			descriptorSet{ nullptr };
	DescriptorSetLayout* layout{ nullptr };

	U32						offsetsCache[MAX_DESCRIPTORS_PER_SET]{};
	DescriptorBinding		bindings[MAX_DESCRIPTORS_PER_SET]{};
	U8						bindingCount{ 0 };
};

struct ShaderCreation
{
	String					name{ NO_INIT };
	VkShaderStageFlagBits	stages[MAX_SHADER_STAGES];
	U8						stageCount{ 0 };
};

struct RenderpassOutput
{
	RenderpassOutput& Reset();
	RenderpassOutput& Color(VkFormat format);
	RenderpassOutput& Depth(VkFormat format);
	RenderpassOutput& SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

	VkFormat			colorFormats[MAX_IMAGE_OUTPUTS]{ VK_FORMAT_UNDEFINED };
	VkFormat			depthStencilFormat{ VK_FORMAT_UNDEFINED };
	U32					colorFormatCount{ 0 };

	RenderPassOperation	colorOperation{ RENDER_PASS_OP_DONT_CARE };
	RenderPassOperation	depthOperation{ RENDER_PASS_OP_DONT_CARE };
	RenderPassOperation	stencilOperation{ RENDER_PASS_OP_DONT_CARE };
};

struct Renderpass
{
	void Destroy() { name.Destroy(); }

	String				name{ NO_INIT };
	HashHandle			handle;

	VkRenderPass		renderpass{ nullptr };
	VkFramebuffer		frameBuffers[MAX_IMAGE_OUTPUTS]{ nullptr };

	Texture* outputTextures[MAX_IMAGE_OUTPUTS]{ nullptr };
	Texture* outputDepth{ nullptr };
	VkClearValue		clears[MAX_IMAGE_OUTPUTS + 1]{};
	U8					clearCount{ 0 };
	Viewport			viewport{};

	RenderpassOutput	output{};
	RenderpassType		type{ RENDERPASS_TYPE_GEOMETRY };

	U16					width{ 0 };
	U16					height{ 0 };
	U8					renderTargetCount{ 0 };
};

struct RenderPassCreation
{
	void Destroy() { name.Destroy(); }

	RenderPassCreation& Reset();
	RenderPassCreation& AddRenderTarget(Texture* texture);
	RenderPassCreation& SetDepthStencilTexture(Texture* texture);
	RenderPassCreation& SetName(const String& name);
	RenderPassCreation& SetType(RenderpassType type);
	RenderPassCreation& SetOperations(RenderPassOperation color, RenderPassOperation depth, RenderPassOperation stencil);

	U16					width{ 0 };
	U16					height{ 0 };
	U8					renderTargetCount{ 0 };
	RenderpassType		type{ RENDERPASS_TYPE_GEOMETRY };

	Texture* outputTextures[MAX_IMAGE_OUTPUTS]{ nullptr };
	Texture* depthStencilTexture{ nullptr };
	//TODO: Pass in Viewport info

	RenderPassOperation	colorOperation{ RENDER_PASS_OP_DONT_CARE };
	RenderPassOperation	depthOperation{ RENDER_PASS_OP_DONT_CARE };
	RenderPassOperation	stencilOperation{ RENDER_PASS_OP_DONT_CARE };

	String				name{ NO_INIT };
};

struct CommandBuffer;
struct Mesh;
struct Pipeline;

struct Program
{
	void Destroy() { name.Destroy(); }

	void RunPasses(CommandBuffer* commands);
	void DrawMesh(CommandBuffer* commands, Mesh& mesh, Buffer* constantBuffer);

	String		name{ NO_INIT };
	HashHandle	handle;

	Pipeline* passes[MAX_PROGRAM_PASSES];
	U8			passCount;

	static Renderpass* prevRenderpass;
};

struct ProgramCreation
{
	void Destroy() { name.Destroy(); }
	ProgramCreation& Reset();

	ProgramCreation& SetName(const String& name);
	ProgramCreation& AddPass(Pipeline* pass);

	String		name{ NO_INIT };

	Pipeline* passes[MAX_PROGRAM_PASSES];
	U8			passCount;
};

struct Material
{
	void Destroy() { name.Destroy(); }

	String		name{ NO_INIT };
	HashHandle	handle;

	U32			renderIndex;
	U32			poolIndex;

	Program* program{ nullptr };
};

struct MaterialCreation
{
	void Destroy() { name.Destroy(); }

	MaterialCreation& Reset();
	MaterialCreation& SetProgram(Program* program);
	MaterialCreation& SetName(const String& name);
	MaterialCreation& SetRenderIndex(U32 renderIndex);

	Program* program{ nullptr };
	String		name{ NO_INIT };
	U32			renderIndex{ U32_MAX };
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
	Vector3		metalRoughOcclFactor;
	Vector3		emissiveFactor;
	F32			alphaCutoff;
	F32			unused[3];
	U32			flags;
};

struct NH_API Mesh
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
	//TODO: Move these to Material
	U16			diffuseTextureIndex{ U16_MAX };
	U16			metalRoughOcclTextureIndex{ U16_MAX };
	U16			normalTextureIndex{ U16_MAX };
	U16			emissivityTextureIndex{ U16_MAX };

	Vector4		baseColorFactor{ Vector4::One };
	Vector3		metalRoughOcclFactor{ Vector3::One };
	Vector3		emissiveFactor{ Vector3::Zero };

	F32			alphaCutoff{ 0.0f };
	U32			flags{ 0 };

	//TODO: Transform component
	Vector3		position{ Vector3::Zero };
	Vector3		scale{ Vector3::One };
	Quaternion3	rotation{ Quaternion3::Identity };
};

struct Skybox
{
	String name{ NO_INIT };

	Buffer* indexBuffer{ nullptr };
	Buffer* vertexBuffer{ nullptr };

	Texture* texture{ nullptr };
};

struct SkyboxCreation
{
	String name{ NO_INIT };

	U16 vertexCount{ 0 };
	U16 indexCount{ 0 };

	String textureName{ NO_INIT };
	String binaryName{ NO_INIT };
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
	U32				frameIssued{ 0 };
};

struct MapBufferParameters
{
	Buffer* buffer;
	U64		offset{ 0 };
	U64		size{ 0 };
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
	U32				loadOperation{ 0 };

	U32				textureBarrierCount;
	U32				bufferBarrierCount;

	TextureBarrier	textureBarriers[8];
	BufferBarrier	bufferBarriers[8];
};
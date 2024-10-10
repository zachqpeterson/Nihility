#include "Resources.hpp"

#include "Font.hpp"
#include "Scene.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Rendering\RenderingDefines.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Math\Math.hpp"

#include "External\Assimp\cimport.h"
#include "External\Assimp\scene.h"
#include "External\Assimp\postprocess.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "External\stb_image.h"

#define STB_VORBIS_NO_STDIO
#include "External\stb_vorbis.h"

import Core;
import Containers;
import Audio;

#undef near
#undef far

constexpr U32 ASSIMP_IMPORT_FLAGS =
aiProcess_CalcTangentSpace |
aiProcess_JoinIdenticalVertices |
aiProcess_Triangulate |
aiProcess_RemoveComponent |
aiProcess_GenSmoothNormals |
aiProcess_ValidateDataStructure |
aiProcess_ImproveCacheLocality |
aiProcess_RemoveRedundantMaterials |
aiProcess_FindInvalidData |
aiProcess_GenUVCoords |
aiProcess_FindInstances |
aiProcess_OptimizeMeshes |
aiProcess_OptimizeGraph |
aiProcess_FlipUVs |
aiProcess_EmbedTextures;

#if NH_BIG_ENDIAN
#define WAV_RIFF 'RIFF'
#define WAV_DATA 'data'
#define WAV_FMT 'fmt '
#define WAV_WAVE 'WAVE'
#define WAV_XWMA 'XWMA'
#define WAV_DPDS 'dpds'
#define WAV_LIST 'LIST'
#else
#define WAV_RIFF 'FFIR'
#define WAV_DATA 'atad'
#define WAV_FMT ' tmf'
#define WAV_WAVE 'EVAW'
#define WAV_XWMA 'AMWX'
#define WAV_DPDS 'sdpd'
#define WAV_LIST 'TSIL'
#endif

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

	KTX_FORMAT_ALPHA4 = 0x803B,
	KTX_FORMAT_ALPHA8 = 0x803C,
	KTX_FORMAT_ALPHA12 = 0x803D,
	KTX_FORMAT_ALPHA16 = 0x803E,
	KTX_FORMAT_LUMINANCE4 = 0x803F,
	KTX_FORMAT_LUMINANCE8 = 0x8040,
	KTX_FORMAT_LUMINANCE12 = 0x8041,
	KTX_FORMAT_LUMINANCE16 = 0x8042,
	KTX_FORMAT_LUMINANCE4_ALPHA4 = 0x8043,
	KTX_FORMAT_LUMINANCE6_ALPHA2 = 0x8044,
	KTX_FORMAT_LUMINANCE8_ALPHA8 = 0x8045,
	KTX_FORMAT_LUMINANCE12_ALPHA4 = 0x8046,
	KTX_FORMAT_LUMINANCE12_ALPHA12 = 0x8047,
	KTX_FORMAT_LUMINANCE16_ALPHA16 = 0x8048,
	KTX_FORMAT_INTENSITY4 = 0x804A,
	KTX_FORMAT_INTENSITY8 = 0x804B,
	KTX_FORMAT_INTENSITY12 = 0x804C,
	KTX_FORMAT_INTENSITY16 = 0x804D,
	KTX_FORMAT_R3_G3_B2 = 0x2A10,
	KTX_FORMAT_RGB2 = 0x804E,
	KTX_FORMAT_RGB4 = 0x804F,
	KTX_FORMAT_RGB5 = 0x8050,
	KTX_FORMAT_RGB8 = 0x8051,
	KTX_FORMAT_RGB10 = 0x8052,
	KTX_FORMAT_RGB12 = 0x8053,
	KTX_FORMAT_RGB16 = 0x8054,
	KTX_FORMAT_RGBA2 = 0x8055,
	KTX_FORMAT_RGBA4 = 0x8056,
	KTX_FORMAT_RGB5_A1 = 0x8057,
	KTX_FORMAT_RGBA8 = 0x8058,
	KTX_FORMAT_RGB10_A2 = 0x8059,
	KTX_FORMAT_RGB10_A2UI = 0x906F,
	KTX_FORMAT_RGBA12 = 0x805A,
	KTX_FORMAT_RGBA16 = 0x805B,
	KTX_FORMAT_R8 = 0x8229,
	KTX_FORMAT_R16 = 0x822A,
	KTX_FORMAT_RG8 = 0x822B,
	KTX_FORMAT_RG16 = 0x822C,
	KTX_FORMAT_R16F = 0x822D,
	KTX_FORMAT_R32F = 0x822E,
	KTX_FORMAT_RG16F = 0x822F,
	KTX_FORMAT_RG32F = 0x8230,
	KTX_FORMAT_R8I = 0x8231,
	KTX_FORMAT_R8UI = 0x8232,
	KTX_FORMAT_R16I = 0x8233,
	KTX_FORMAT_R16UI = 0x8234,
	KTX_FORMAT_R32I = 0x8235,
	KTX_FORMAT_R32UI = 0x8236,
	KTX_FORMAT_RG8I = 0x8237,
	KTX_FORMAT_RG8UI = 0x8238,
	KTX_FORMAT_RG16I = 0x8239,
	KTX_FORMAT_RG16UI = 0x823A,
	KTX_FORMAT_RG32I = 0x823B,
	KTX_FORMAT_RG32UI = 0x823C,
	KTX_FORMAT_RGBA32F = 0x8814,
	KTX_FORMAT_RGB32F = 0x8815,
	KTX_FORMAT_RGBA16F = 0x881A,
	KTX_FORMAT_RGB16F = 0x881B,
	KTX_FORMAT_R11F_G11F_B10F = 0x8C3A,
	KTX_FORMAT_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B,
	KTX_FORMAT_RGB9_E5 = 0x8C3D,
	KTX_FORMAT_SLUMINANCE8_ALPHA8 = 0x8C45,
	KTX_FORMAT_SLUMINANCE8 = 0x8C47,
	KTX_FORMAT_RGB565 = 0x8D62,
	KTX_FORMAT_RGBA32UI = 0x8D70,
	KTX_FORMAT_RGB32UI = 0x8D71,
	KTX_FORMAT_RGBA16UI = 0x8D76,
	KTX_FORMAT_RGB16UI = 0x8D77,
	KTX_FORMAT_RGBA8UI = 0x8D7C,
	KTX_FORMAT_RGB8UI = 0x8D7D,
	KTX_FORMAT_RGBA32I = 0x8D82,
	KTX_FORMAT_RGB32I = 0x8D83,
	KTX_FORMAT_RGBA16I = 0x8D88,
	KTX_FORMAT_RGB16I = 0x8D89,
	KTX_FORMAT_RGBA8I = 0x8D8E,
	KTX_FORMAT_RGB8I = 0x8D8F,
	KTX_FORMAT_FLOAT_32_UNSIGNED_INT_24_8_REV = 0x8DAD,
	KTX_FORMAT_R8_SNORM = 0x8F94,
	KTX_FORMAT_RG8_SNORM = 0x8F95,
	KTX_FORMAT_RGB8_SNORM = 0x8F96,
	KTX_FORMAT_RGBA8_SNORM = 0x8F97,
	KTX_FORMAT_R16_SNORM = 0x8F98,
	KTX_FORMAT_RG16_SNORM = 0x8F99,
	KTX_FORMAT_RGB16_SNORM = 0x8F9A,
	KTX_FORMAT_RGBA16_SNORM = 0x8F9B,
	KTX_FORMAT_SR8 = 0x8FBD,
	KTX_FORMAT_SRG8 = 0x8FBE,
	KTX_FORMAT_SRGB8 = 0x8C41,
	KTX_FORMAT_SRGB8_ALPHA8 = 0x8C43,
	KTX_FORMAT_ALPHA8_SNORM = 0x9014,
	KTX_FORMAT_LUMINANCE8_SNORM = 0x9015,
	KTX_FORMAT_LUMINANCE8_ALPHA8_SNORM = 0x9016,
	KTX_FORMAT_INTENSITY8_SNORM = 0x9017,
	KTX_FORMAT_ALPHA16_SNORM = 0x9018,
	KTX_FORMAT_LUMINANCE16_SNORM = 0x9019,
	KTX_FORMAT_LUMINANCE16_ALPHA16_SNORM = 0x901A,
	KTX_FORMAT_INTENSITY16_SNORM = 0x901B,

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

enum KTXFormatType
{
	KTX_FORMAT_TYPE_NONE = 0x00000000,
	KTX_FORMAT_TYPE_PACKED = 0x00000001,
	KTX_FORMAT_TYPE_COMPRESSED = 0x00000002,
	KTX_FORMAT_TYPE_PALETTIZED = 0x00000004,
	KTX_FORMAT_TYPE_DEPTH = 0x00000008,
	KTX_FORMAT_TYPE_STENCIL = 0x00000010,
};

#pragma pack(push, 1)

struct KTXHeader11
{
	KTXType	type;
	U32	typeSize;
	KTXFormat format;
	KTXCompression internalFormat;
	KTXFormat baseInternalFormat;
	U32	pixelWidth;
	U32	pixelHeight;
	U32	pixelDepth;
	U32 arrayElementCount;
	U32	faceCount;
	U32	mipmapLevelCount;
	U32	keyValueDataSize;
};

struct KTXHeader20
{
	VkFormat format;
	U32 typeSize;
	U32 pixelWidth;
	U32 pixelHeight;
	U32 pixelDepth;
	U32 layerCount;
	U32 faceCount;
	U32 levelCount;
	U32 superCompressionScheme;
	U32 dfdByteOffset;
	U32 dfdByteLength;
	U32 kvdByteOffset;
	U32 kvdByteLength;
	U64 sgdByteOffset;
	U64 sgdByteLength;
};

struct KTXLevel
{
	U64 byteOffset;
	U64 byteLength;
	U64 uncompressedByteLength;
};

struct KTXInfo
{
	U32 flags;
	U32 paletteSizeInBits;
	U32 blockSizeInBits;
	U32 blockWidth;			// in texels
	U32 blockHeight;		// in texels
	U32 blockDepth;			// in texels
};

#pragma pack(pop)

struct MP3Header
{
	U16 sync : 12;
	U16 version : 1;
	U16 layer : 2;
	U16 errorProtection : 1;

	U8 bitRate : 4;
	U8 frequency : 2;
	U8 padded : 1;
	U8 unknown : 1;

	U8 mode : 2;
	U8 intensityStereo : 1;
	U8 msStereo : 1;
	U8 copywrited : 1;
	U8 original : 1;
	U8 emphasis : 2;
};

//nhtex
//nhsky
//nhaud
//nhmat
//nhmsh
//nhmdl
//nhshd
//nhscn
//nhfnt

constexpr U32 TEXTURE_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SKYBOX_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 AUDIO_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MATERIAL_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MESH_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MODEL_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SHADER_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SCENE_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 FONT_VERSION = MakeVersionNumber(0, 1, 0);

Hashmap<String, Pair<Texture, U64>>			Resources::textures(512);
Hashmap<String, Pair<Skybox, U64>>			Resources::skyboxes(32);
Hashmap<String, Pair<Font, U64>>			Resources::fonts(32);
Hashmap<String, Pair<AudioClip, U64>>		Resources::audioClips(512);
Hashmap<String, Pair<Shader, U64>>			Resources::shaders(128);
Hashmap<String, Pair<Pipeline, U64>>		Resources::pipelines(256);
Hashmap<String, Pair<MaterialEffect, U64>>	Resources::materialEffects(256);
Hashmap<String, Pair<Material, U64>>		Resources::materials(256);
Hashmap<String, Pair<Mesh, U64>>			Resources::meshes(512);
Hashmap<String, Pair<Model, U64>>			Resources::models(256);
Hashmap<String, Scene>						Resources::scenes(128);

Queue<ResourceUpdate>			Resources::bindlessTexturesToUpdate;

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	if (!File::Exists("materials/default_material.nhmat"))
	{
		MaterialInfo info{};
		info.name = "materials/default_material.nhmat";
		ResourceRef<Material> mat = CreateMaterial(info);
		SaveMaterial(mat);
	}

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Shutting Down Resources...");

	vkDeviceWaitIdle(Renderer::device);

	CleanupHashmap(textures, Renderer::DestroyTextureInstant);

	textures.Destroy();
	skyboxes.Destroy();
	fonts.Destroy();
	audioClips.Destroy();
	shaders.Destroy();
	pipelines.Destroy();
	materialEffects.Destroy();
	materials.Destroy();
	meshes.Destroy();
	models.Destroy();
	scenes.Destroy();

	bindlessTexturesToUpdate.Destroy();
}

void Resources::Update()
{
	if (bindlessTexturesToUpdate.Size())
	{
		VkWriteDescriptorSet* bindlessDescriptorWrites;
		Memory::AllocateArray(&bindlessDescriptorWrites, bindlessTexturesToUpdate.Size());

		VkDescriptorImageInfo* bindlessImageInfo;
		Memory::AllocateArray(&bindlessImageInfo, bindlessTexturesToUpdate.Size());

		U32 currentWriteIndex = 0;

		while (!bindlessTexturesToUpdate.Empty())
		{
			ResourceUpdate textureToUpdate;
			bindlessTexturesToUpdate.Pop(textureToUpdate);

			//TODO: Maybe check frame
			{
				ResourceRef<Texture> texture = Resources::GetTexture(textureToUpdate.handle);
				if (texture->image == nullptr) { continue; }

				VkWriteDescriptorSet& descriptorWrite = bindlessDescriptorWrites[currentWriteIndex];
				descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.dstArrayElement = (U32)textureToUpdate.handle;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.dstSet = Shader::bindlessDescriptorSet;
				descriptorWrite.dstBinding = Shader::bindlessTextureBinding;

				VkDescriptorImageInfo& descriptorImageInfo = bindlessImageInfo[currentWriteIndex];

				descriptorImageInfo.sampler = texture->sampler.vkSampler;
				descriptorImageInfo.imageView = texture->imageView;
				descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptorWrite.pImageInfo = &descriptorImageInfo;

				++currentWriteIndex;
			}
		}

		if (currentWriteIndex) { vkUpdateDescriptorSets(Renderer::device, currentWriteIndex, bindlessDescriptorWrites, 0, nullptr); }

		Memory::Free(&bindlessDescriptorWrites);
		Memory::Free(&bindlessImageInfo);
	}
}

ResourceRef<Texture> Resources::CreateTexture(const TextureInfo& info, const SamplerInfo& samplerInfo)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Texture, U64>* pair = textures.Request(info.name, handle);
	Texture* texture = &pair->a;

	if (!texture->Name().Blank()) { return pair; }

	*texture = {};

	texture->name = info.name;
	texture->handle = handle;
	texture->width = info.width;
	texture->height = info.height;
	texture->size = info.width * info.height * 4;
	texture->depth = info.depth;
	texture->flags = info.flags;
	texture->format = info.format;
	texture->mipmapCount = info.mipmapCount;
	texture->type = info.type;

	if (!Renderer::CreateTexture(texture, info.initialData))
	{
		Logger::Error("Failed To Create Texture!");
		textures.Remove(texture->Handle());
		texture->Destroy();
		return nullptr;
	}

	texture->sampler.minFilter = samplerInfo.minFilter;
	texture->sampler.magFilter = samplerInfo.magFilter;
	texture->sampler.mipFilter = samplerInfo.mipFilter;

	texture->sampler.boundsModeU = samplerInfo.boundsModeU;
	texture->sampler.boundsModeV = samplerInfo.boundsModeV;
	texture->sampler.boundsModeW = samplerInfo.boundsModeW;

	texture->sampler.border = samplerInfo.border;

	texture->sampler.reductionMode = samplerInfo.reductionMode;

	return pair;
}

ResourceRef<Texture> Resources::CreateSwapchainTexture(VkImage image, VkFormat format, U8 index)
{
	String name{ "SwapchainTexture", index };

	HashHandle handle;
	Pair<Texture, U64>* pair = textures.Request(name, handle);
	Texture* texture = &pair->a;

	*texture = {};

	texture->name = name;
	texture->handle = handle;
	texture->swapchainImage = true;
	texture->format = format;
	texture->image = image;

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.pNext = nullptr;
	viewInfo.flags = 0;
	viewInfo.image = texture->image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(Renderer::device, &viewInfo, Renderer::allocationCallbacks, &texture->imageView) != VK_SUCCESS) { textures.Remove(handle); texture->Destroy(); return nullptr; }

	texture->mipmaps[0] = texture->imageView;

	Renderer::SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, "Swapchain_ImageView");

	return pair;
}

ResourceRef<Shader> Resources::CreateShader(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Shader, U64>* pair = shaders.Request(name, handle);
	Shader* shader = &pair->a;

	if (!shader->Name().Blank()) { return pair; }

	*shader = {};

	shader->name = name;
	shader->handle = handle;

	if (!shader->Create(name))
	{
		Logger::Error("Failed To Create Shader!");
		shaders.Remove(shader->Handle());
		return nullptr;
	}

	return pair;
}

ResourceRef<MaterialEffect> Resources::CreateMaterialEffect(const String& name, Vector<ResourceRef<Pipeline>>&& pipelines)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<MaterialEffect, U64>* pair = materialEffects.Request(name, handle);
	MaterialEffect* materialEffect = &pair->a;

	if (!materialEffect->Name().Blank()) { return pair; }

	*materialEffect = {};

	materialEffect->name = name;
	materialEffect->handle = handle;
	materialEffect->processing = Move(pipelines);

	return pair;
}

ResourceRef<MaterialEffect> Resources::CreateMaterialEffect(const String& name, const Vector<ResourceRef<Pipeline>>& pipelines)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<MaterialEffect, U64>* pair = materialEffects.Request(name, handle);
	MaterialEffect* materialEffect = &pair->a;

	if (!materialEffect->Name().Blank()) { return pair; }

	*materialEffect = {};

	materialEffect->name = name;
	materialEffect->handle = handle;
	materialEffect->processing = pipelines;

	return pair;
}

ResourceRef<Material> Resources::CreateMaterial(const MaterialInfo& info)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Material, U64>* pair = materials.Request(info.name, handle);
	Material* material = &pair->a;

	if (!material->name.Blank()) { return pair; }

	*material = {};

	material->name = info.name;
	material->handle = handle;

	if (info.diffuseTexture) { material->data.diffuseTextureIndex = (U32)info.diffuseTexture->handle; }
	if (info.armTexture) { material->data.armTextureIndex = (U32)info.armTexture->handle; }
	if (info.normalTexture) { material->data.normalTextureIndex = (U32)info.normalTexture->handle; }
	if (info.emissivityTexture) { material->data.emissivityTextureIndex = (U32)info.emissivityTexture->handle; }

	material->effect = info.effect;
	material->data.baseColorFactor = info.baseColorFactor;
	material->data.metalRoughFactor = info.metalRoughFactor;
	material->data.emissiveFactor = info.emissiveFactor;
	material->data.alphaCutoff = info.alphaCutoff;
	material->data.flags = info.flags;

	if (info.effect) { material->effect = info.effect; }
	else
	{
		if (material->data.baseColorFactor.w < 1.0f) { material->effect = GetMaterialEffect("pbrTransparentEffect"); }
		else { material->effect = GetMaterialEffect("pbrOpaqueEffect"); }
	}


	VkBufferCopy region{};
	region.dstOffset = sizeof(MaterialData) * handle;
	region.size = sizeof(MaterialData);
	region.srcOffset = 0;

	Renderer::FillBuffer(Renderer::materialBuffer, sizeof(MaterialData), &material->data, 1, &region);

	return pair;
}

ResourceRef<Mesh> Resources::CreateMesh(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Mesh, U64>* pair = meshes.Request(name, handle);
	Mesh* mesh = &pair->a;

	if (!mesh->Name().Blank()) { return pair; }

	*mesh = {};

	mesh->name = name;
	mesh->handle = handle;

	return pair;
}

Scene* Resources::CreateScene(const String& name, CameraType cameraType)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Scene* scene = scenes.Request(name, handle);

	if (!scene->name.Blank()) { return scene; }

	*scene = {};

	scene->name = name;
	scene->handle = handle;

	scene->Create(cameraType);

	return scene;
}

bool Resources::RecreateSwapchainTexture(ResourceRef<Texture>& texture, VkImage image)
{
	vkDestroyImageView(Renderer::device, texture->imageView, Renderer::allocationCallbacks);

	texture->image = image;

	VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.pNext = nullptr;
	viewInfo.flags = 0;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = (VkFormat)texture->format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkValidateFR(vkCreateImageView(Renderer::device, &viewInfo, Renderer::allocationCallbacks, &texture->imageView));

	texture->mipmaps[0] = texture->imageView;

	Renderer::SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, "Swapchain_ImageView");

	return true;
}

bool Resources::RecreateTexture(ResourceRef<Texture>& texture, U16 width, U16 height, U16 depth)
{
	if (texture->lastResize < Renderer::AbsoluteFrame())
	{
		Texture deleteTexture;
		deleteTexture.imageView = texture->imageView;
		deleteTexture.image = texture->image;
		deleteTexture.allocation = texture->allocation;
		deleteTexture.width = texture->width;
		deleteTexture.height = texture->height;
		deleteTexture.depth = texture->depth;
		deleteTexture.sampler = texture->sampler;
		deleteTexture.mipmapCount = texture->mipmapCount;
		Copy(deleteTexture.mipmaps, texture->mipmaps, deleteTexture.mipmapCount);

		texture->width = width;
		texture->height = height;
		texture->depth = depth;

		if (!Renderer::CreateTexture((Texture*)texture, nullptr))
		{
			Logger::Error("Failed To Recreate Pipeline!");
			texture->width = deleteTexture.width;
			texture->height = deleteTexture.height;
			texture->depth = deleteTexture.depth;

			return false;
		}

		Renderer::DestroyTextureInstant(&deleteTexture);
		texture->lastResize = Renderer::AbsoluteFrame();

		return true;
	}

	return false;
}

bool SetAtlasPositions()
{
	U8 x = 0;
	U8 y = 0;

	for (C8 i = 0; i < 96; ++i)
	{
		Font::atlasPositions[i] = { x, y };

		++x &= 7;
		y += x == 0;
	}

	return true;
}

ResourceRef<Font> Resources::LoadFont(const String& path)
{
	static bool b = SetAtlasPositions();

	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Font, U64>* pair = fonts.Request(path, handle);
	Font* font = &pair->a;

	if (!font->Name().Blank()) { return pair; }

	*font = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		font->name = path;
		font->handle = handle;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Font"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Font!", path);
			fonts.Remove(handle);
			fonts.Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		reader.Read(font->ascent);
		reader.Read(font->descent);
		reader.Read(font->lineGap);

		for (U32 i = 0; i < 96; ++i)
		{
			reader.Read(font->glyphs[i]);
		}

		String textureName = path.GetFileName().Appended("_texture");

		Texture* texture = &textures.Request(textureName, handle)->a;
		*texture = {};

		reader.Read(font->glyphSize);
		reader.Read(font->width);
		reader.Read(font->height);

		texture->name = Move(textureName);
		texture->handle = handle;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->format = VK_FORMAT_R32G32B32A32_SFLOAT;
		texture->mipmapCount = 1;
		texture->width = font->width;
		texture->height = font->height;
		texture->size = texture->width * texture->height * 4 * sizeof(F32);

		texture->sampler.minFilter = FILTER_TYPE_NEAREST;
		texture->sampler.magFilter = FILTER_TYPE_NEAREST;
		texture->sampler.mipFilter = SAMPLER_MIPMAP_MODE_NEAREST;

		if (!Renderer::CreateTexture(texture, reader.Pointer()))
		{
			Logger::Error("Failed To Create Texture: {}!", texture->Name());
			fonts.Remove(font->Handle());
			textures.Remove(handle);
			textures.Destroy();
			fonts.Destroy();
			return nullptr;
		}

		font->texture = texture;

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	fonts.Remove(handle);
	return nullptr;
}

ResourceRef<AudioClip> Resources::LoadAudio(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<AudioClip, U64>* pair = audioClips.Request(path, handle);
	AudioClip* audioClip = &pair->a;

	if (!audioClip->Name().Blank()) { return pair; }

	*audioClip = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		audioClip->name = path;
		audioClip->handle = handle;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Audio"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Audio!", path);
			audioClips.Remove(handle);
			audioClip->Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		reader.Read(audioClip->format);
		reader.Read(audioClip->size);

		Memory::AllocateSize(&audioClip->buffer, audioClip->size);

		Copy(audioClip->buffer, reader.Pointer(), audioClip->size);

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	audioClips.Remove(handle);
	return nullptr;
}

U8 GetFormatSize(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_UNDEFINED: return 0;
	case VK_FORMAT_R4G4_UNORM_PACK8: return 1;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return 2;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return 2;
	case VK_FORMAT_R5G6B5_UNORM_PACK16: return 2;
	case VK_FORMAT_B5G6R5_UNORM_PACK16: return 2;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return 2;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return 2;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return 2;
	case VK_FORMAT_R8_UNORM: return 1;
	case VK_FORMAT_R8_SNORM: return 1;
	case VK_FORMAT_R8_USCALED: return 1;
	case VK_FORMAT_R8_SSCALED: return 1;
	case VK_FORMAT_R8_UINT: return 1;
	case VK_FORMAT_R8_SINT: return 1;
	case VK_FORMAT_R8_SRGB: return 1;
	case VK_FORMAT_R8G8_UNORM: return 2;
	case VK_FORMAT_R8G8_SNORM: return 2;
	case VK_FORMAT_R8G8_USCALED: return 2;
	case VK_FORMAT_R8G8_SSCALED: return 2;
	case VK_FORMAT_R8G8_UINT: return 2;
	case VK_FORMAT_R8G8_SINT: return 2;
	case VK_FORMAT_R8G8_SRGB: return 2;
	case VK_FORMAT_R8G8B8_UNORM: return 3;
	case VK_FORMAT_R8G8B8_SNORM: return 3;
	case VK_FORMAT_R8G8B8_USCALED: return 3;
	case VK_FORMAT_R8G8B8_SSCALED: return 3;
	case VK_FORMAT_R8G8B8_UINT: return 3;
	case VK_FORMAT_R8G8B8_SINT: return 3;
	case VK_FORMAT_R8G8B8_SRGB: return 3;
	case VK_FORMAT_B8G8R8_UNORM: return 3;
	case VK_FORMAT_B8G8R8_SNORM: return 3;
	case VK_FORMAT_B8G8R8_USCALED: return 3;
	case VK_FORMAT_B8G8R8_SSCALED: return 3;
	case VK_FORMAT_B8G8R8_UINT: return 3;
	case VK_FORMAT_B8G8R8_SINT: return 3;
	case VK_FORMAT_B8G8R8_SRGB: return 3;
	case VK_FORMAT_R8G8B8A8_UNORM: return 4;
	case VK_FORMAT_R8G8B8A8_SNORM: return 4;
	case VK_FORMAT_R8G8B8A8_USCALED: return 4;
	case VK_FORMAT_R8G8B8A8_SSCALED: return 4;
	case VK_FORMAT_R8G8B8A8_UINT: return 4;
	case VK_FORMAT_R8G8B8A8_SINT: return 4;
	case VK_FORMAT_R8G8B8A8_SRGB: return 4;
	case VK_FORMAT_B8G8R8A8_UNORM: return 4;
	case VK_FORMAT_B8G8R8A8_SNORM: return 4;
	case VK_FORMAT_B8G8R8A8_USCALED: return 4;
	case VK_FORMAT_B8G8R8A8_SSCALED: return 4;
	case VK_FORMAT_B8G8R8A8_UINT: return 4;
	case VK_FORMAT_B8G8R8A8_SINT: return 4;
	case VK_FORMAT_B8G8R8A8_SRGB: return 4;
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return 4;
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return 4;
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return 4;
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return 4;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32: return 4;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32: return 4;
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return 4;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return 4;
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return 4;
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return 4;
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return 4;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32: return 4;
	case VK_FORMAT_A2R10G10B10_SINT_PACK32: return 4;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return 4;
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return 4;
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return 4;
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return 4;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32: return 4;
	case VK_FORMAT_A2B10G10R10_SINT_PACK32: return 4;
	case VK_FORMAT_R16_UNORM: return 2;
	case VK_FORMAT_R16_SNORM: return 2;
	case VK_FORMAT_R16_USCALED: return 2;
	case VK_FORMAT_R16_SSCALED: return 2;
	case VK_FORMAT_R16_UINT: return 2;
	case VK_FORMAT_R16_SINT: return 2;
	case VK_FORMAT_R16_SFLOAT: return 2;
	case VK_FORMAT_R16G16_UNORM: return 4;
	case VK_FORMAT_R16G16_SNORM: return 4;
	case VK_FORMAT_R16G16_USCALED: return 4;
	case VK_FORMAT_R16G16_SSCALED: return 4;
	case VK_FORMAT_R16G16_UINT: return 4;
	case VK_FORMAT_R16G16_SINT: return 4;
	case VK_FORMAT_R16G16_SFLOAT: return 4;
	case VK_FORMAT_R16G16B16_UNORM: return 6;
	case VK_FORMAT_R16G16B16_SNORM: return 6;
	case VK_FORMAT_R16G16B16_USCALED: return 6;
	case VK_FORMAT_R16G16B16_SSCALED: return 6;
	case VK_FORMAT_R16G16B16_UINT: return 6;
	case VK_FORMAT_R16G16B16_SINT: return 6;
	case VK_FORMAT_R16G16B16_SFLOAT: return 6;
	case VK_FORMAT_R16G16B16A16_UNORM: return 8;
	case VK_FORMAT_R16G16B16A16_SNORM: return 8;
	case VK_FORMAT_R16G16B16A16_USCALED: return 8;
	case VK_FORMAT_R16G16B16A16_SSCALED: return 8;
	case VK_FORMAT_R16G16B16A16_UINT: return 8;
	case VK_FORMAT_R16G16B16A16_SINT: return 8;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return 8;
	case VK_FORMAT_R32_UINT: return 4;
	case VK_FORMAT_R32_SINT: return 4;
	case VK_FORMAT_R32_SFLOAT: return 4;
	case VK_FORMAT_R32G32_UINT: return 8;
	case VK_FORMAT_R32G32_SINT: return 8;
	case VK_FORMAT_R32G32_SFLOAT: return 8;
	case VK_FORMAT_R32G32B32_UINT: return 12;
	case VK_FORMAT_R32G32B32_SINT: return 12;
	case VK_FORMAT_R32G32B32_SFLOAT: return 12;
	case VK_FORMAT_R32G32B32A32_UINT: return 16;
	case VK_FORMAT_R32G32B32A32_SINT: return 16;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
	case VK_FORMAT_R64_UINT: return 8;
	case VK_FORMAT_R64_SINT: return 8;
	case VK_FORMAT_R64_SFLOAT: return 8;
	case VK_FORMAT_R64G64_UINT: return 16;
	case VK_FORMAT_R64G64_SINT: return 16;
	case VK_FORMAT_R64G64_SFLOAT: return 16;
	case VK_FORMAT_R64G64B64_UINT: return 24;
	case VK_FORMAT_R64G64B64_SINT: return 24;
	case VK_FORMAT_R64G64B64_SFLOAT: return 24;
	case VK_FORMAT_R64G64B64A64_UINT: return 32;
	case VK_FORMAT_R64G64B64A64_SINT: return 32;
	case VK_FORMAT_R64G64B64A64_SFLOAT: return 32;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return 4;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return 4;
	case VK_FORMAT_D16_UNORM: return 2;
	case VK_FORMAT_X8_D24_UNORM_PACK32: return 4;
	case VK_FORMAT_D32_SFLOAT: return 4;
	case VK_FORMAT_S8_UINT: return 1;
	case VK_FORMAT_D16_UNORM_S8_UINT: return 3;
	case VK_FORMAT_D24_UNORM_S8_UINT: return 4;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: return 5;
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return 0;
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return 0;
	case VK_FORMAT_BC2_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC2_SRGB_BLOCK: return 0;
	case VK_FORMAT_BC3_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC3_SRGB_BLOCK: return 0;
	case VK_FORMAT_BC4_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC4_SNORM_BLOCK: return 0;
	case VK_FORMAT_BC5_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC5_SNORM_BLOCK: return 0;
	case VK_FORMAT_BC6H_UFLOAT_BLOCK: return 0;
	case VK_FORMAT_BC6H_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_BC7_UNORM_BLOCK: return 0;
	case VK_FORMAT_BC7_SRGB_BLOCK: return 0;
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return 0;
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return 0;
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return 0;
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return 0;
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return 0;
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return 0;
	case VK_FORMAT_EAC_R11_UNORM_BLOCK: return 0;
	case VK_FORMAT_EAC_R11_SNORM_BLOCK: return 0;
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return 0;
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return 0;
	case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return 0;
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return 0;
	case VK_FORMAT_G8B8G8R8_422_UNORM: return 4;
	case VK_FORMAT_B8G8R8G8_422_UNORM: return 4;
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return 0;
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: return 0;
	case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM: return 0;
	case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: return 0;
	case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: return 0;
	case VK_FORMAT_R10X6_UNORM_PACK16: return 2;
	case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: return 4;
	case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return 8;
	case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return 8;
	case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return 8;
	case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: return 6;
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return 6;
	case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: return 6;
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: return 6;
	case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return 6;
	case VK_FORMAT_R12X4_UNORM_PACK16: return 2;
	case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: return 4;
	case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: return 8;
	case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return 8;
	case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return 8;
	case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: return 6;
	case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: return 6;
	case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: return 6;
	case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return 6;
	case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return 6;
	case VK_FORMAT_G16B16G16R16_422_UNORM: return 0;
	case VK_FORMAT_B16G16R16G16_422_UNORM: return 0;
	case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM: return 0;
	case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: return 0;
	case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM: return 0;
	case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: return 0;
	case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return 0;
	case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM: return 0;
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: return 0;
	case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: return 0;
	case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM: return 0;
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16: return 0;
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16: return 0;
	case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK: return 0;
	case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return 0;
	case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return 0;
	case VK_FORMAT_R16G16_S10_5_NV: return 0;
	case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR: return 0;
	case VK_FORMAT_A8_UNORM_KHR: return 0;
	default: return 0;
	}
}

ResourceRef<Texture> Resources::LoadTexture(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Texture, U64>* pair = textures.Request(path, handle);
	Texture* texture = &pair->a;

	if (!texture->Name().Blank()) { return pair; }

	*texture = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		texture->name = path;
		texture->handle = handle;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Texture"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Texture!", path);
			textures.Remove(handle);
			texture->Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		reader.Read(texture->usage);
		reader.ReadCount((U8*)&texture->sampler, sizeof(SamplerInfo));

		reader.Read(texture->width);
		reader.Read(texture->height);
		reader.Read(texture->format);
		reader.Read(texture->mipmapCount);
		texture->size = texture->width * texture->height * GetFormatSize((VkFormat)texture->format);

		if (!Renderer::CreateTexture(texture, reader.Pointer()))
		{
			Logger::Error("Failed To Create Texture: {}!", path);
			textures.Remove(handle);
			texture->Destroy();
			return nullptr;
		}

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	textures.Remove(handle);
	return nullptr;
}

ResourceRef<Skybox> Resources::LoadSkybox(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Skybox, U64>* pair = skyboxes.Request(path, handle);
	Skybox* skybox = &pair->a;

	if (!skybox->Name().Blank()) { return pair; }

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		skybox->name = path;
		skybox->handle = handle;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Skybox"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Skybox!", path);
			skyboxes.Remove(handle);
			skybox->Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		U16 resolution;
		VkFormat format;

		reader.Read(resolution);
		reader.Read(format);
		U32 faceSize = resolution * resolution * GetFormatSize(format);

		String textureName = path.GetFileName().Appended("_texture");

		skybox->texture = textures.Request(textureName, handle);

		*(skybox->texture) = {};

		skybox->texture->name = Move(textureName);
		skybox->texture->handle = handle;
		skybox->texture->width = resolution;
		skybox->texture->height = resolution;
		skybox->texture->type = VK_IMAGE_TYPE_2D;
		skybox->texture->format = format;
		skybox->texture->flags = 0;
		skybox->texture->depth = 1;
		skybox->texture->mipmapCount = 1;
		skybox->texture->size = faceSize * 6;

		if (!Renderer::CreateCubemap((Texture*)skybox->texture, reader.Pointer(), &faceSize))
		{
			Logger::Error("Failed To Create Cubemap: {}!", path);
			skyboxes.Remove(skybox->Handle());
			textures.Remove(handle);
			skybox->Destroy();
			skybox->texture->Destroy();
			return nullptr;
		}

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	skyboxes.Remove(handle);
	return nullptr;
}

ResourceRef<Shader> Resources::LoadShader(const String& path, ShaderStageType stage)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Shader, U64>* pair = shaders.Request(path, handle);
	Shader* shader = &pair->a;

	if (!shader->name.Blank()) { return pair; }

	*shader = {};

	shader->name = path;
	shader->handle = handle;
	shader->stage = stage;

	if (!shader->Create(path))
	{
		Logger::Error("Failed To Create Shader!");
		shaders.Remove(shader->Handle());
		return nullptr;
	}

	return pair;
}

ResourceRef<Pipeline> Resources::LoadPipeline(const String& path, U8 pushConstantCount, PushConstant* pushConstants)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Pipeline, U64>* pair = pipelines.Request(path, handle);
	Pipeline* pipeline = &pair->a;

	if (!pipeline->name.Blank()) { return pair; }

	*pipeline = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		pipeline->name = path;
		pipeline->handle = handle;

		String data;
		file.ReadAll(data);
		file.Close();

		I64 index = 0;

		do
		{
			I64 value = data.IndexOfNot(' ', data.IndexOf('=', index + 1) + 1);

			switch (Hash::StringHash(data.Data() + index, data.IndexOf('=', index) - index))
			{
			case "cull"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "NONE"_Hash: { pipeline->cullMode = CULL_MODE_NONE; } break;
				case "FRONT"_Hash: { pipeline->cullMode = CULL_MODE_FRONT_BIT; } break;
				case "BACK"_Hash: { pipeline->cullMode = CULL_MODE_BACK_BIT; } break;
				case "BOTH"_Hash: { pipeline->cullMode = CULL_MODE_FRONT_AND_BACK; } break;
				default: { Logger::Warn("Unknown Cull Mode, Defaulting To NONE!"); pipeline->cullMode = CULL_MODE_NONE; }
				}
			} break;
			case "front"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "CLOCKWISE"_Hash: { pipeline->frontMode = FRONT_FACE_MODE_CLOCKWISE; } break;
				case "COUNTER"_Hash: { pipeline->frontMode = FRONT_FACE_MODE_COUNTER_CLOCKWISE; } break;
				default: { Logger::Warn("Unknown Front Mode, Defaulting To COUNTER!"); pipeline->frontMode = FRONT_FACE_MODE_COUNTER_CLOCKWISE; }
				}
			} break;
			case "fill"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "SOLID"_Hash: { pipeline->fillMode = POLYGON_MODE_FILL; } break;
				case "LINE"_Hash: { pipeline->fillMode = POLYGON_MODE_LINE; } break;
				case "POINT"_Hash: { pipeline->fillMode = POLYGON_MODE_POINT; } break;
				default: { Logger::Warn("Unknown Fill Mode, Defaulting To SOLID!"); pipeline->fillMode = POLYGON_MODE_FILL; }
				}
			} break;
			case "depth"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "NONE"_Hash: {
					pipeline->depthEnable = false;
					pipeline->depthWriteEnable = false;
					pipeline->depthComparison = COMPARE_OP_ALWAYS;
				} break;
				case "LESS_EQUAL"_Hash: {
					pipeline->depthEnable = true;
					pipeline->depthWriteEnable = true;
					pipeline->depthComparison = COMPARE_OP_LESS_OR_EQUAL;
				} break;
				case "LESS"_Hash: {
					pipeline->depthEnable = true;
					pipeline->depthWriteEnable = true;
					pipeline->depthComparison = COMPARE_OP_LESS;
				} break;
				case "GREATER_EQUAL"_Hash: {
					pipeline->depthEnable = true;
					pipeline->depthWriteEnable = true;
					pipeline->depthComparison = COMPARE_OP_GREATER_OR_EQUAL;
				} break;
				case "GREATER"_Hash: {
					pipeline->depthEnable = true;
					pipeline->depthWriteEnable = true;
					pipeline->depthComparison = COMPARE_OP_GREATER;
				} break;
				case "EQUAL"_Hash: {
					pipeline->depthEnable = true;
					pipeline->depthWriteEnable = true;
					pipeline->depthComparison = COMPARE_OP_EQUAL;
				} break;
				default: {
					Logger::Warn("Unknown Depth Compare, Defaulting To LESS!");
					pipeline->depthEnable = true;
					pipeline->depthWriteEnable = true;
					pipeline->depthComparison = COMPARE_OP_LESS;
				} break;
				}
			} break;
			case "blend"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "ADD"_Hash: { pipeline->blendMode = BLEND_MODE_ADD; } break;
				case "SUB"_Hash: { pipeline->blendMode = BLEND_MODE_SUB; } break;
				case "NONE"_Hash: { pipeline->blendMode = BLEND_MODE_NONE; } break;
				default: { Logger::Warn("Unknown Blend Mode, Defaulting To ADD!"); pipeline->blendMode = BLEND_MODE_ADD; } break;
				}
			} break;
			case "topology"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "POINTS"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_POINT_LIST; } break;
				case "LINES"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_LINE_LIST; } break;
				case "LINE_STRIP"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_LINE_STRIP; } break;
				case "TRIANGLES"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_TRIANGLE_LIST; } break;
				case "TRIANGLE_STRIP"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_TRIANGLE_STRIP; } break;
				case "TRIANGLE_FAN"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_TRIANGLE_FAN; } break;
				case "LINES_ADJ"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_LINE_LIST_WITH_ADJACENCY; } break;
				case "LINE_STRIP_ADJ"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_LINE_STRIP_WITH_ADJACENCY; } break;
				case "TRIANGLES_ADJ"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_TRIANGLE_LIST_WITH_ADJACENCY; } break;
				case "TRIANGLE_STRIP_ADJ"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_TRIANGLE_STRIP_WITH_ADJACENCY; } break;
				case "PATCHES"_Hash: { pipeline->topologyMode = TOPOLOGY_MODE_PATCH_LIST; } break;
				default: { Logger::Warn("Unknown Topology Mode, Defaulting To LINES!"); pipeline->topologyMode = TOPOLOGY_MODE_LINE_LIST; } break;
				}
			} break;
			case "clear"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "COLOR"_Hash: { pipeline->clearTypes |= CLEAR_TYPE_COLOR; } break;
				case "DEPTH"_Hash: { pipeline->clearTypes |= CLEAR_TYPE_DEPTH; } break;
				case "STENCIL"_Hash: { pipeline->clearTypes |= CLEAR_TYPE_STENCIL; } break;
				default: { Logger::Warn("Unknown Blend Mode, Ignoring!"); } break;
				}
			} break;
			case "useIndices"_Hash: {
				if (data.CompareN("FALSE", value)) { pipeline->useIndices = false; }
			} break;
			case "vertexCount"_Hash: {
				pipeline->vertexCount = data.ToType<U8>(value);
			} break;
			case "depthBiasConstant"_Hash: {
				pipeline->depthBiasConstant = data.ToType<F32>(value);
				pipeline->depthBiasEnable = true;
			} break;
			case "depthBiasClamp"_Hash: {
				pipeline->depthBiasClamp = data.ToType<F32>(value);
				pipeline->depthBiasEnable = true;
			} break;
			case "depthBiasSlope"_Hash: {
				pipeline->depthBiasSlope = data.ToType<F32>(value);
				pipeline->depthBiasEnable = true;
			} break;
			case "type"_Hash: {
				I64 size = data.IndexOf('\n', value) - value - 1;

				switch (Hash::StringHash(data.Data() + value, size))
				{
				case "PRE_OPAQUE"_Hash: { pipeline->type |= PIPELINE_TYPE_PRE_PROCESSING_OPAQUE; } break;
				case "PRE_TRANSPARENT"_Hash: { pipeline->type |= PIPELINE_TYPE_PRE_PROCESSING_TRANSPARENT; } break;
				case "PRE"_Hash: { pipeline->type |= PIPELINE_TYPE_PRE_PROCESSING_OPAQUE | PIPELINE_TYPE_PRE_PROCESSING_TRANSPARENT; } break;
				case "MESH_OPAQUE"_Hash: { pipeline->type |= PIPELINE_TYPE_MESH_OPAQUE; } break;
				case "MESH_TRANSPARENT"_Hash: { pipeline->type |= PIPELINE_TYPE_PRE_PROCESSING_TRANSPARENT; } break;
				case "MESH"_Hash: { pipeline->type |= PIPELINE_TYPE_MESH_OPAQUE | PIPELINE_TYPE_PRE_PROCESSING_TRANSPARENT; } break;
				case "POST_MESH"_Hash: { pipeline->type |= PIPELINE_TYPE_POST_PROCESSING_MESH; } break;
				case "POST_RENDER"_Hash: { pipeline->type |= PIPELINE_TYPE_POST_PROCESSING_RENDER; } break;
				case "UI"_Hash: { pipeline->type |= PIPELINE_TYPE_UI; } break;
				default: { Logger::Warn("Unknown Pipeline Type, Ignoring!"); } break;
				}
			} break;
			case "renderOrder"_Hash: {
				pipeline->renderOrder = data.ToType<I32>(value);
			} break;
			case "size"_Hash: {
				pipeline->sizeX = data.ToType<U32>(value);
				I64 next = data.IndexOfNot(' ', data.IndexOf(',', value)) + 1;
				pipeline->sizeY = data.ToType<U32>(next);

				value = next;
			} break;
			case "vertex"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_VERTEX_BIT)); } break;
			case "control"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_TESSELLATION_CONTROL_BIT)); } break;
			case "evaluation"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_TESSELLATION_EVALUATION_BIT)); } break;
			case "geometry"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_GEOMETRY_BIT)); } break;
			case "fragment"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_FRAGMENT_BIT)); } break;
			case "compute"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_COMPUTE_BIT)); } break;
			case "task"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_TASK_BIT_EXT)); } break;
			case "mesh"_Hash: { pipeline->shaders.Push(LoadShader(data.SubString(value, data.IndexOf('\n', value) - value - 1), SHADER_STAGE_MESH_BIT_EXT)); } break;
			default: { Logger::Warn("Unknown Setting, Ignoring!"); }
			}

			index = data.IndexOf('\n', value + 1);
		} while (index++ != -1);

		if (!pipeline->Create(pushConstantCount, pushConstants))
		{
			Logger::Error("Failed To Create Shader!");
			shaders.Remove(pipeline->Handle());
			return nullptr;
		}

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	pipelines.Remove(handle);
	return nullptr;
}

ResourceRef<Material> Resources::LoadMaterial(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Material, U64>* pair = materials.Request(path, handle);
	Material* material = &pair->a;

	if (!material->name.Blank()) { return pair; }

	*material = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		material->name = path;
		material->handle = handle;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Material"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Material!", path);
			materials.Remove(handle);
			material->Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		String texturePath = reader.ReadString();
		if (!texturePath.Compare("NULL")) { material->data.diffuseTextureIndex = (U32)LoadTexture(texturePath)->handle; }

		texturePath = reader.ReadString();
		if (!texturePath.Compare("NULL")) { material->data.armTextureIndex = (U32)LoadTexture(texturePath)->handle; }

		texturePath = reader.ReadString();
		if (!texturePath.Compare("NULL")) { material->data.normalTextureIndex = (U32)LoadTexture(texturePath)->handle; }

		texturePath = reader.ReadString();
		if (!texturePath.Compare("NULL")) { material->data.emissivityTextureIndex = (U32)LoadTexture(texturePath)->handle; }

		reader.Read(material->data.baseColorFactor);
		reader.Read(material->data.metalRoughFactor);
		reader.Read(material->data.emissiveFactor);
		reader.Read(material->data.alphaCutoff);
		reader.Read(material->data.flags);

		if (material->data.baseColorFactor.w < 1.0f) { material->effect = GetMaterialEffect("pbrTransparentEffect"); }
		else { material->effect = GetMaterialEffect("pbrOpaqueEffect"); }

		VkBufferCopy region{};
		region.dstOffset = sizeof(MaterialData) * handle;
		region.size = sizeof(MaterialData);
		region.srcOffset = 0;

		Renderer::FillBuffer(Renderer::materialBuffer, sizeof(MaterialData), &material->data, 1, &region);

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);
	materials.Remove(handle);
	return nullptr;
}

ResourceRef<Mesh> Resources::LoadMesh(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Mesh, U64>* pair = meshes.Request(path, handle);
	Mesh* mesh = &pair->a;

	if (!mesh->Name().Blank()) { return pair; }

	*mesh = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		mesh->name = path;
		mesh->handle = handle;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Mesh"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Mesh!", path);
			meshes.Remove(handle);
			mesh->Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		reader.Read(mesh->vertexCount);
		reader.Read(mesh->indicesSize);
		bool hasTangents, hasTexcoords;
		reader.Read(hasTangents);
		reader.Read(hasTexcoords);

		U32 verticesSize = sizeof(Vector3) * mesh->vertexCount;

		VertexBuffer positionBuffer;
		positionBuffer.type = VERTEX_TYPE_POSITION;
		positionBuffer.size = verticesSize;
		positionBuffer.stride = sizeof(Vector3);
		Memory::AllocateSize(&positionBuffer.buffer, verticesSize);
		Copy((Vector3*)positionBuffer.buffer, (Vector3*)reader.Pointer(), mesh->vertexCount);
		mesh->buffers.Push(positionBuffer);
		reader.Seek(verticesSize);

		VertexBuffer normalBuffer;
		normalBuffer.type = VERTEX_TYPE_NORMAL;
		normalBuffer.size = verticesSize;
		normalBuffer.stride = sizeof(Vector3);
		Memory::AllocateSize(&normalBuffer.buffer, verticesSize);
		Copy((Vector3*)normalBuffer.buffer, (Vector3*)reader.Pointer(), mesh->vertexCount);
		mesh->buffers.Push(normalBuffer);
		reader.Seek(verticesSize);

		if (hasTangents)
		{
			VertexBuffer tangentBuffer;
			tangentBuffer.type = VERTEX_TYPE_TANGENT;
			tangentBuffer.size = verticesSize;
			tangentBuffer.stride = sizeof(Vector3);
			Memory::AllocateSize(&tangentBuffer.buffer, verticesSize);
			Copy((Vector3*)tangentBuffer.buffer, (Vector3*)reader.Pointer(), mesh->vertexCount);
			mesh->buffers.Push(tangentBuffer);
			reader.Seek(verticesSize);
		}

		if (hasTexcoords)
		{
			VertexBuffer texcoordBuffer;
			texcoordBuffer.type = VERTEX_TYPE_TEXCOORD;
			texcoordBuffer.size = verticesSize;
			texcoordBuffer.stride = sizeof(Vector3);
			Memory::AllocateSize(&texcoordBuffer.buffer, verticesSize);
			Copy((Vector3*)texcoordBuffer.buffer, (Vector3*)reader.Pointer(), mesh->vertexCount);
			mesh->buffers.Push(texcoordBuffer);
			reader.Seek(verticesSize);
		}

		//TODO: Store index count instead of size
		Memory::AllocateSize(&mesh->indices, mesh->indicesSize);
		Copy((U32*)mesh->indices, (U32*)reader.Pointer(), mesh->indicesSize / sizeof(U32));

		reader.Seek(mesh->indicesSize);

		reader.Read(mesh->mass);
		reader.Read(mesh->centerOfMass);
		reader.Read(mesh->invInertiaMatrix);

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);
	meshes.Remove(handle);
	return nullptr;
}

ResourceRef<Model> Resources::LoadModel(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pair<Model, U64>* pair = models.Request(path, handle);
	Model* model = &pair->a;

	if (!model->Name().Blank()) { return pair; }

	*model = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		model->name = path;
		model->handle = handle;

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Model"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Model!", path);
			models.Remove(handle);
			model->Destroy();
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		U32 meshCount;
		reader.Read(meshCount);

		for (U32 i = 0; i < meshCount; ++i)
		{
			String path = reader.ReadString();

			ResourceRef<Mesh> mesh = LoadMesh(path);

			U32 instanceCount;
			reader.Read(instanceCount);

			for (U32 j = 0; j < instanceCount; ++j)
			{
				MeshInstance instance{};

				instance.mesh = mesh;

				path = reader.ReadString();
				instance.material = LoadMaterial(path);

				Matrix4 matrix;
				reader.Read(matrix);
				model->matrices.Push(matrix);

				U32 materialIndex = (U32)instance.material->handle;
				Copy((U32*)&instance.instanceData, &materialIndex, 1);

				model->meshes.Push(Move(instance));
			}
		}

		return pair;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);
	models.Remove(handle);
	return nullptr;
}

Scene* Resources::LoadScene(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Scene* scene = scenes.Request(path, handle);

	if (!scene->name.Blank()) { return scene; }

	*scene = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		I64 extIndex = path.LastIndexOf('.') + 1;

		bool success = false;

		scene->name = path;
		scene->handle = handle;

		//U32 bufferCount;
		//U32 textureCount;
		//U32 samplerCount;
		//U32 meshCount;
		//
		//file.Read(bufferCount);
		//file.Read(samplerCount);
		//file.Read(textureCount);
		//file.Read(meshCount);
		//
		//scene->buffers.Reserve(bufferCount);
		//scene->textures.Reserve(textureCount);
		//scene->samplers.Reserve(samplerCount);
		//scene->meshes.Reserve(meshCount);
		//
		//String str{};
		//void* bufferData = nullptr;
		//U32 bufferLength = 0;
		//VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		//
		//for (U32 i = 0; i < bufferCount; ++i)
		//{
		//	file.ReadString(str);
		//
		//	BufferCreation bufferCreation{};
		//	bufferCreation.SetName(str).Set(flags, RESOURCE_USAGE_IMMUTABLE, 0);
		//	Buffer* buffer = LoadBuffer(bufferCreation);
		//	buffer->sceneID = i;
		//
		//	scene->buffers.Push(buffer);
		//}
		//
		////TODO: Check for no samplers, use default sampler
		//for (U32 i = 0; i < samplerCount; ++i)
		//{
		//	SamplerInfo samplerInfo{};
		//	samplerInfo.SetName("Sampler"); //TODO: Unique name!
		//	file.Read((I32&)samplerInfo.minFilter);
		//	file.Read((I32&)samplerInfo.magFilter);
		//	file.Read((I32&)samplerInfo.mipFilter);
		//	file.Read((I32&)samplerInfo.addressModeU);
		//	file.Read((I32&)samplerInfo.addressModeV);
		//	file.Read((I32&)samplerInfo.addressModeW);
		//	file.Read((I32&)samplerInfo.border);
		//
		//	Sampler* sampler = CreateSampler(samplerInfo);
		//	sampler->sceneID = i;
		//
		//	scene->samplers.Push(sampler);
		//}
		//
		//for (U32 i = 0; i < textureCount; ++i)
		//{
		//	file.ReadString(str);
		//	U32 samplerID = 0;
		//	file.Read(samplerID);
		//
		//	Texture* texture = LoadTexture(str, true);
		//	texture->sceneID = i;
		//	texture->sampler = scene->samplers[samplerID];
		//
		//	scene->textures.Push(texture);
		//}
		//
		//file.ReadString(str);
		//scene->skybox = LoadSkybox(str);
		//
		//scene->camera = {};
		//bool perspective;
		//F32 near, far;
		//Vector3 position, rotation;
		//file.Read(perspective);
		//file.Read(near);
		//file.Read(far);
		//file.Read(position.x);
		//file.Read(position.y);
		//file.Read(position.z);
		//file.Read(rotation.x);
		//file.Read(rotation.y);
		//file.Read(rotation.z);
		//
		//scene->camera.SetPosition(position);
		//scene->camera.SetRotation(rotation);
		//
		//if (perspective)
		//{
		//	F32 fov, aspect;
		//	file.Read(fov);
		//	file.Read(aspect);
		//
		//	scene->camera.SetPerspective(near, far, fov, aspect);
		//}
		//else
		//{
		//	F32 width, height, zoom;
		//	file.Read(width);
		//	file.Read(height);
		//	file.Read(zoom);
		//
		//	scene->camera.SetOrthograpic(near, far, width, height, zoom);
		//}
		//
		//for (U32 i = 0; i < meshCount; ++i)
		//{
		//	Mesh mesh{};
		//	BufferCreation bufferCreation{};
		//	U32 id;
		//	U64 offset;
		//	U64 size;
		//
		//	file.Read(id);
		//	file.Read(offset);
		//	file.Read(size);
		//	bufferCreation.Reset().SetName("texcoord_buffer").Set(flags, RESOURCE_USAGE_IMMUTABLE, size).SetParent(scene->buffers[id], offset);
		//	mesh.texcoordBuffer = CreateBuffer(bufferCreation);
		//
		//	file.Read(id);
		//	file.Read(offset);
		//	file.Read(size);
		//	bufferCreation.Reset().SetName("normal_buffer").Set(flags, RESOURCE_USAGE_IMMUTABLE, size).SetParent(scene->buffers[id], offset);
		//	mesh.normalBuffer = CreateBuffer(bufferCreation);
		//
		//	file.Read(id);
		//	file.Read(offset);
		//	file.Read(size);
		//	bufferCreation.Reset().SetName("tangent_buffer").Set(flags, RESOURCE_USAGE_IMMUTABLE, size).SetParent(scene->buffers[id], offset);
		//	mesh.tangentBuffer = CreateBuffer(bufferCreation);
		//
		//	file.Read(id);
		//	file.Read(offset);
		//	file.Read(size);
		//	bufferCreation.Reset().SetName("position_buffer").Set(flags, RESOURCE_USAGE_IMMUTABLE, size).SetParent(scene->buffers[id], offset);
		//	mesh.positionBuffer = CreateBuffer(bufferCreation);
		//
		//	file.Read(id);
		//	file.Read(offset);
		//	file.Read(size);
		//	bufferCreation.Reset().SetName("index_buffer").Set(flags, RESOURCE_USAGE_IMMUTABLE, size).SetParent(scene->buffers[id], offset);
		//	mesh.indexBuffer = CreateBuffer(bufferCreation);
		//
		//	mesh.primitiveCount = (U32)(mesh.indexBuffer->size / sizeof(U16));
		//
		//	file.Read(id);
		//	mesh.diffuseTextureIndex = (U16)scene->textures[id]->handle;
		//	file.Read(id);
		//	mesh.metalRoughOcclTextureIndex = (U16)scene->textures[id]->handle;
		//	file.Read(id);
		//	mesh.normalTextureIndex = (U16)scene->textures[id]->handle;
		//	file.Read(id);
		//	mesh.emissivityTextureIndex = (U16)scene->textures[id]->handle;
		//
		//	file.Read(mesh.baseColorFactor.x);
		//	file.Read(mesh.baseColorFactor.y);
		//	file.Read(mesh.baseColorFactor.z);
		//	file.Read(mesh.baseColorFactor.w);
		//	file.Read(mesh.metalRoughOcclFactor.x);
		//	file.Read(mesh.metalRoughOcclFactor.y);
		//	file.Read(mesh.metalRoughOcclFactor.z);
		//	file.Read(mesh.emissiveFactor.x);
		//	file.Read(mesh.emissiveFactor.y);
		//	file.Read(mesh.emissiveFactor.z);
		//	file.Read(mesh.flags);
		//	file.Read(mesh.alphaCutoff);
		//
		//	Vector3 euler;
		//	file.Read(mesh.position.x);
		//	file.Read(mesh.position.y);
		//	file.Read(mesh.position.z);
		//	file.Read(euler.x);
		//	file.Read(euler.y);
		//	file.Read(euler.z);
		//	file.Read(mesh.scale.x);
		//	file.Read(mesh.scale.y);
		//	file.Read(mesh.scale.z);
		//
		//	mesh.rotation = Quaternion3(euler);
		//
		//	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(MeshData)).SetName("material"); //TODO: Unique name
		//	mesh.materialBuffer = CreateBuffer(bufferCreation);
		//	mesh.material = AccessDefaultMaterial(); //TODO: Checks for transparency and culling
		//
		//	scene->meshes.Push(mesh);
		//}
		//
		//scene->Create();

		if (!success)
		{
			scenes.Remove(path);
			scene->Destroy();
			file.Close();
			return nullptr;
		}

		file.Close();
		return scene;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);

	scenes.Remove(path);
	return nullptr;
}

Binary Resources::LoadBinary(const String& path)
{
	File file{ path, FILE_OPEN_RESOURCE_READ };
	if (file.Opened())
	{
		Binary binary{};

		binary.size = (U32)file.Size();
		Memory::AllocateSize(&binary.data, binary.size);

		file.ReadCount((U8*)binary.data, binary.size);
		file.Close();

		return binary;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);

	return {};
}

String Resources::LoadBinaryString(const String& path)
{
	File file{ path, FILE_OPEN_RESOURCE_READ };
	if (file.Opened())
	{
		String string;
		file.ReadAll(string);
		file.Close();

		return Move(string);
	}

	Logger::Error("Failed To Find Or Open File: {}", path);

	return {};
}

void Resources::SaveMaterial(const ResourceRef<Material>& material)
{
	File file(material->name, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Material");
		file.Write(MATERIAL_VERSION);

		if (material->data.diffuseTextureIndex != U16_MAX) { file.Write(GetTexture(material->data.diffuseTextureIndex)->Name()); }
		else { file.Write("NULL"); }
		if (material->data.armTextureIndex != U16_MAX) { file.Write(GetTexture(material->data.armTextureIndex)->Name()); }
		else { file.Write("NULL"); }
		if (material->data.normalTextureIndex != U16_MAX) { file.Write(GetTexture(material->data.normalTextureIndex)->Name()); }
		else { file.Write("NULL"); }
		if (material->data.emissivityTextureIndex != U16_MAX) { file.Write(GetTexture(material->data.emissivityTextureIndex)->Name()); }
		else { file.Write("NULL"); }

		file.Write(material->data.baseColorFactor);
		file.Write(material->data.metalRoughFactor);
		file.Write(material->data.emissiveFactor);
		file.Write(material->data.alphaCutoff);
		file.Write(material->data.flags);

		file.Close();
	}
}

void Resources::SaveScene(Scene* scene)
{
	//File file(scene->name, FILE_OPEN_RESOURCE_WRITE);
	//if (file.Opened())
	//{
	//	file.Write((U32)scene->buffers.Size());
	//	file.Write((U32)scene->samplers.Size());
	//	file.Write((U32)scene->textures.Size());
	//	file.Write((U32)scene->meshes.Size());
	//
	//	for (Buffer* buffer : scene->buffers) { file.Write(buffer->name); }
	//	for (Sampler* sampler : scene->samplers)
	//	{
	//		file.Write((I32)sampler->minFilter);
	//		file.Write((I32)sampler->magFilter);
	//		file.Write((I32)sampler->mipFilter);
	//		file.Write((I32)sampler->addressModeU);
	//		file.Write((I32)sampler->addressModeV);
	//		file.Write((I32)sampler->addressModeW);
	//		file.Write((I32)sampler->border);
	//	}
	//	for (Texture* texture : scene->textures) { file.Write(texture->name); file.Write(texture->sampler->sceneID); }
	//
	//	file.Write(scene->skybox->name);
	//
	//	file.Write(scene->camera.perspective);
	//	file.Write(scene->camera.nearPlane);
	//	file.Write(scene->camera.farPlane);
	//	file.Write(scene->camera.position.x);
	//	file.Write(scene->camera.position.y);
	//	file.Write(scene->camera.position.z);
	//	Vector3 rotation = scene->camera.Euler();
	//	file.Write(rotation.x);
	//	file.Write(rotation.y);
	//	file.Write(rotation.z);
	//
	//	if (scene->camera.perspective)
	//	{
	//		file.Write(scene->camera.fov);
	//		file.Write(scene->camera.aspectRatio);
	//	}
	//	else
	//	{
	//		file.Write(scene->camera.viewportWidth);
	//		file.Write(scene->camera.viewportHeight);
	//		file.Write(scene->camera.zoom);
	//	}
	//
	//	for (const Mesh& mesh : scene->meshes)
	//	{
	//		file.Write(mesh.texcoordBuffer->parentBuffer->sceneID);
	//		file.Write(mesh.texcoordBuffer->globalOffset);
	//		file.Write(mesh.texcoordBuffer->size);
	//		file.Write(mesh.normalBuffer->parentBuffer->sceneID);
	//		file.Write(mesh.normalBuffer->globalOffset);
	//		file.Write(mesh.normalBuffer->size);
	//		file.Write(mesh.tangentBuffer->parentBuffer->sceneID);
	//		file.Write(mesh.tangentBuffer->globalOffset);
	//		file.Write(mesh.tangentBuffer->size);
	//		file.Write(mesh.positionBuffer->parentBuffer->sceneID);
	//		file.Write(mesh.positionBuffer->globalOffset);
	//		file.Write(mesh.positionBuffer->size);
	//		file.Write(mesh.indexBuffer->parentBuffer->sceneID);
	//		file.Write(mesh.indexBuffer->globalOffset);
	//		file.Write(mesh.indexBuffer->size);
	//
	//		file.Write(AccessTexture(mesh.diffuseTextureIndex)->sceneID);
	//		file.Write(AccessTexture(mesh.metalRoughOcclTextureIndex)->sceneID);
	//		file.Write(AccessTexture(mesh.normalTextureIndex)->sceneID);
	//		file.Write(AccessTexture(mesh.emissivityTextureIndex)->sceneID);
	//
	//		file.Write(mesh.baseColorFactor.x);
	//		file.Write(mesh.baseColorFactor.y);
	//		file.Write(mesh.baseColorFactor.z);
	//		file.Write(mesh.baseColorFactor.w);
	//		file.Write(mesh.metalRoughOcclFactor.x);
	//		file.Write(mesh.metalRoughOcclFactor.y);
	//		file.Write(mesh.metalRoughOcclFactor.z);
	//		file.Write(mesh.emissiveFactor.x);
	//		file.Write(mesh.emissiveFactor.y);
	//		file.Write(mesh.emissiveFactor.z);
	//		file.Write(mesh.flags);
	//		file.Write(mesh.alphaCutoff);
	//
	//		Vector3 rotation = mesh.rotation.Euler();
	//		file.Write(mesh.position.x);
	//		file.Write(mesh.position.y);
	//		file.Write(mesh.position.z);
	//		file.Write(rotation.x);
	//		file.Write(rotation.y);
	//		file.Write(rotation.z);
	//		file.Write(mesh.scale.x);
	//		file.Write(mesh.scale.y);
	//		file.Write(mesh.scale.z);
	//	}
	//
	//	file.Close();
	//}
}

void Resources::SaveBinary(const String& path, U32 size, void* data)
{
	File file{ path, FILE_OPEN_RESOURCE_WRITE };
	if (file.Opened())
	{
		file.Write(data, (U32)size);
		file.Close();
	}

	Logger::Error("Failed To Find Or Open File: {}", path);
}

ResourceRef<Texture> Resources::GetTexture(const String& name)
{
	return textures.Get(name);
}

ResourceRef<Texture> Resources::GetTexture(HashHandle handle)
{
	return textures.Obtain(handle);
}

ResourceRef<MaterialEffect> Resources::GetMaterialEffect(const String& name)
{
	return materialEffects.Get(name);
}

ResourceRef<MaterialEffect> Resources::GetMaterialEffect(HashHandle handle)
{
	return materialEffects.Obtain(handle);
}

void Resources::DestroyBinary(Binary& binary)
{
	Memory::Free(&binary.data);
	binary.data = nullptr;
	binary.size = 0;
}

U8 Resources::MipmapCount(U16 width, U16 height)
{
	return (U8)DegreeOfTwo(Math::Min(width, height));
}

String Resources::UploadFile(const String& path)
{
	I64 fileExtension = path.LastIndexOf('.');

	if (fileExtension++ == -1)
	{
		//TODO: No file extension
		Logger::Error("Can't Upload File That Has No Extension!");
		return {};
	}

	switch (Hash::StringHashCI(path.Data() + fileExtension, path.Size() - fileExtension))
	{
		//Images
	case "jpg"_Hash:
	case "jpeg"_Hash:
	case "png"_Hash:
	case "bmp"_Hash:
	case "tga"_Hash:
	case "jfif"_Hash:
	case "tiff"_Hash:
	case "ktx"_Hash:
	case "ktx2"_Hash:
	case "pnm"_Hash:
	case "ppm"_Hash:
	case "pgm"_Hash:
	case "sbsar"_Hash: {
		return Move(UploadTexture(path));
	} break;

					 //Skyboxes
	case "hdr"_Hash: {
		return Move(UploadSkybox(path));
	} break;

				   //Fonts
	case "ttf"_Hash:
	case "otf"_Hash: {
		return Move(UploadFont(path));
	} break;

				   //Models
	case "obj"_Hash:
	case "gltf"_Hash:
	case "glb"_Hash:
	case "fbx"_Hash:
	case "blend"_Hash:
	case "pmx"_Hash: {
		return Move(UploadModel(path));
	} break;

				   //Audio
	case "wav"_Hash:
	case "mp3"_Hash:
	case "ogg"_Hash:
	case "opus"_Hash:
	case "flac"_Hash: {
		return Move(UploadAudio(path));
	} break;

	default: {
		Logger::Error("Unknown File Extension: !");
		return {};
	}
	}
}

String Resources::UploadFont(const String& path)
{
	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		U8* data;
		Memory::AllocateSize(&data, file.Size());
		file.ReadCount(data, (U32)file.Size());
		file.Close();

		Font font;
		F32* atlas = FontLoader::LoadFont(data, font);

		Memory::Free(&data);

		String newPath = path.GetFileName().Surround("fonts/", ".nhfnt");
		file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

		file.Write("NH Font");
		file.Write(FONT_VERSION);

		file.Write(font.ascent);
		file.Write(font.descent);
		file.Write(font.lineGap);

		for (U32 i = 0; i < 96; ++i)
		{
			file.Write(font.glyphs[i]);
		}

		file.Write(font.glyphSize);
		file.Write(font.width);
		file.Write(font.height);

		file.WriteCount(atlas, font.width * font.height * 4);

		file.Close();

		return newPath;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	return {};
}

String Resources::UploadAudio(const String& path)
{
	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		I64 extension = path.LastIndexOf('.');

		if (Compare(path.Data() + extension + 1, "wav", 3))
		{
			DataReader reader{ file };
			file.Close();

			String newPath = path.GetFileName().Surround("audio/", ".nhaud");
			file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

			file.Write("NH Audio");
			file.Write(AUDIO_VERSION);

			U32 chunkType;
			U32 chunkSize;
			U32 fileType;

			AudioFormat format;

			bool finished = false;
			while (!finished)
			{
				reader.Read(chunkType);
				reader.Read(chunkSize);

				switch (chunkType)
				{
				case WAV_RIFF: {
					reader.Read(fileType);
					if (fileType != WAV_WAVE)
					{
						Logger::Error("Invalid WAV File: '{}'!", path);
						return {};
					}
				} break;
				case WAV_FMT: {
					reader.ReadSize(&format, chunkSize);
					file.Write(format);
				} break;
				case WAV_DATA: {
					file.Write(chunkSize);
					file.WriteCount(reader.Pointer(), chunkSize);
					finished = true;
				} break;
				default: {
					reader.Seek(chunkSize);
				} break;
				}
			}

			file.Close();
			return Move(newPath);
		}
		else if (Compare(path.Data() + extension + 1, "ogg", 3))
		{
			DataReader reader{ file };
			file.Close();

			String newPath = path.GetFileName().Surround("audio/", ".nhaud");
			file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

			file.Write("NH Audio");
			file.Write(AUDIO_VERSION);

			I32 channelCount;
			I32 sampleRate;
			I16* data;
			I32 samples = stb_vorbis_decode_memory(reader.Pointer(), (I32)reader.Size(), &channelCount, &sampleRate, &data);

			AudioFormat format{};
			format.formatTag = 1;
			format.channelCount = channelCount;
			format.samplesPerSec = sampleRate;
			format.avgBytesPerSec = sampleRate * 2 * channelCount;
			format.blockAlign = channelCount * 2;
			format.bitsPerSample = 16;
			format.extraSize = 0;

			file.Write(format);

			U32 size = samples * 2 * channelCount;
			file.Write(size);
			file.Write(data, size);

			free(data);

			file.Close();
			return Move(newPath);
		}
		else if (Compare(path.Data() + extension + 1, "mp3", 3))
		{
			DataReader reader{ file };
			file.Close();
		}
		else
		{
			Logger::Error("Unknown Audio Format!");
			return {};
		}
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	return {};
}

String Resources::UploadSkybox(const String& path)
{
	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		DataReader reader{ file };
		file.Close();

		I32 width, height;
		U8* textureData = stbi_load_from_memory(reader.Data(), (I32)reader.Size(), &width, &height, nullptr, 4);

		U32 resolution = width / 4;
		U32 faceSize = resolution * resolution * 4;
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		U8* result;
		Memory::AllocateSize(&result, faceSize * 6);
		U32 coordX = 0;
		U32 coordY = 0;

		for (U32 i = 0; i < 6; ++i)
		{
			U8* face = result + faceSize * i;

			for (U32 y = 0; y < resolution; ++y)
			{
				for (U32 x = 0; x < resolution; ++x)
				{
					U8* pixel = face + (y * resolution + x) * 4;

					F32 u = ((F32)x / resolution) * 2.0f - 1.0f;
					F32 v = ((F32)y / resolution) * 2.0f - 1.0f;

					Vector3 dir;

					switch (i)
					{
					case 0: { dir = { 1.0f, -v, -u }; } break;	//right +x
					case 1: { dir = { -1.0f, -v, u }; } break;	//left -x
					case 2: { dir = { u, 1.0f, v }; } break;	//top +y
					case 3: { dir = { u, -1.0f, -v }; } break;	//bottom -y
					case 4: { dir = { u, -v, 1.0f }; } break;	//front +z
					case 5: { dir = { -u, -v, -1.0f }; } break;	//back -z
					}

					dir.Normalize();

					F32 lon = -Math::Atan2(dir.z, dir.x);
					F32 lat = Math::Atan(dir.y / dir.xz().Magnitude());
					Vector2 uv{ (lon + PI_F) / TWO_PI_F, 1.0f - (lat + HALF_PI_F) / PI_F };

					coordX = (U32)(uv.x * (width - 1));
					coordY = (U32)(uv.y * (height - 1));

					Copy((U32*)pixel, (U32*)textureData + (coordY * width + coordX), 1);
				}
			}
		}

		String newPath = path.GetFileName().Surround("textures/", ".nhsky");

		file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

		file.Write("NH Skybox");
		file.Write(SKYBOX_VERSION);

		file.Write((U16)resolution);
		file.Write(format);

		file.Write(result, faceSize * 6);

		Memory::Free(&textureData);
		Memory::Free(&result);

		file.Close();

		return Move(newPath);
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	return {};
}

String Resources::UploadTexture(const String& path, const TextureUpload& upload)
{
	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		U32 fileSize = (U32)file.Size();
		U8* fileData;
		Memory::AllocateSize(&fileData, fileSize);
		file.ReadCount(fileData, fileSize);
		file.Close();

		I32 width;
		I32 height;

		U8* textureData = stbi_load_from_memory(fileData, fileSize, &width, &height, nullptr, 4);
		Memory::Free(&fileData);

		if (!textureData)
		{
			Logger::Error("Failed To Convert Image!");
			return {};
		}

		String newPath = path.GetFileName().Surround("textures/", ".nhtex");

		file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

		file.Write("NH Texture");
		file.Write(TEXTURE_VERSION);

		file.Write(upload.usage);
		file.Write(upload.samplerInfo);

		VkFormat format;

		switch (upload.usage)
		{
		case TEXTURE_USAGE_COLOR: { format = VK_FORMAT_R8G8B8A8_UNORM; } break;
		case TEXTURE_USAGE_CALCULATION: { format = VK_FORMAT_R8G8B8A8_UNORM; } break;
		case TEXTURE_USAGE_MASK: { format = VK_FORMAT_R8_UNORM; } break;
		}

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(format);
		file.Write(MipmapCount(width, height));

		if (upload.usage == TEXTURE_USAGE_MASK)
		{
			for (I32 i = 0; i < width * height; ++i)
			{
				file.Write(textureData + i * 4 + 3);
			}
		}
		else
		{
			file.Write(textureData, width * height * 4);
		}

		Memory::Free(&textureData);

		file.Close();

		return Move(newPath);
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	return {};
}

String Resources::UploadTexture(const String& name, U32 index, const aiTexture* textureInfo, TextureUsage usage)
{
	VkFormat format;

	switch (usage)
	{
	case TEXTURE_USAGE_COLOR: { format = VK_FORMAT_R8G8B8A8_UNORM; } break;
	case TEXTURE_USAGE_CALCULATION: { format = VK_FORMAT_R8G8B8A8_UNORM; } break;
	case TEXTURE_USAGE_MASK: { format = VK_FORMAT_R8_UNORM; } break;
	}

	I32 width;
	I32 height;
	U8* textureData = stbi_load_from_memory((U8*)textureInfo->pcData, textureInfo->mWidth, &width, &height, nullptr, 4);

	if (!textureData)
	{
		Logger::Error("Failed To Convert Image!");
		return {};
	}

	String path = textureInfo->mFilename.C_Str();

	if (path.Blank()) { path = { "textures/", name, index, ".nhtex" }; }
	else { path = path.GetFileName().Surround("textures/", ".nhtex"); }

	File file(path, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Texture");
		file.Write(TEXTURE_VERSION);

		file.Write(usage);
		SamplerInfo sampler = {};
		file.Write(sampler);

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(format);
		file.Write(MipmapCount(width, height));

		if (usage == TEXTURE_USAGE_MASK)
		{
			for (I32 i = 0; i < width * height; ++i)
			{
				file.Write(textureData + i * 4 + 3);
			}
		}
		else
		{
			file.Write(textureData, width * height * 4);
		}

		Memory::Free(&textureData);

		file.Close();

		return Move(path);
	}

	Logger::Error("Failed To Create File: {}!", path);
	return {};
}

String Resources::UploadTextures(const String& name, U32 index, const aiTexture* textureInfo0, const aiTexture* textureInfo1, const aiTexture* textureInfo2, U8 def0, U8 def1, U8 def2, TextureUsage usage)
{
	VkFormat format;

	switch (usage)
	{
	case TEXTURE_USAGE_COLOR: { format = VK_FORMAT_R8G8B8A8_UNORM; } break;
	case TEXTURE_USAGE_CALCULATION: { format = VK_FORMAT_R8G8B8A8_UNORM; } break;
	case TEXTURE_USAGE_MASK: { Logger::Error("Can't Combine Textures Into A Mask!"); return {}; } break;
	}

	I32 width0 = 0;
	I32 height0 = 0;
	U8* textureData0 = nullptr;
	if (textureInfo0) { textureData0 = stbi_load_from_memory((U8*)textureInfo0->pcData, textureInfo0->mWidth, &width0, &height0, nullptr, 3); }

	I32 width1 = 0;
	I32 height1 = 0;
	U8* textureData1 = nullptr;
	if (textureInfo1) { textureData1 = stbi_load_from_memory((U8*)textureInfo1->pcData, textureInfo1->mWidth, &width1, &height1, nullptr, 3); }

	I32 width2 = 0;
	I32 height2 = 0;
	U8* textureData2 = nullptr;
	if (textureInfo2) { textureData2 = stbi_load_from_memory((U8*)textureInfo2->pcData, textureInfo2->mWidth, &width2, &height2, nullptr, 3); }

	String path = { "textures/", name, index, ".nhtex" };

	U32 width = 0;
	U32 height = 0;

	if (textureData0) { width = width0; height = height0; }
	else if (textureData1) { width = width1; height = height1; }
	else if (textureData2) { width = width2; height = height2; }
	else { Logger::Error("Failed To Load All Textures For Combined Texture!"); return {}; }

	U8* buffer;
	Memory::AllocateSize(&buffer, width * height * 4);

	File file(path, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Texture");
		file.Write(TEXTURE_VERSION);

		file.Write(usage);
		SamplerInfo sampler = {};
		file.Write(sampler);

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(format);
		file.Write(MipmapCount(width, height));

		if (textureData0)
		{
			if (textureData1)
			{
				if (textureData2)
				{
					for (U32 i = 0; i < width * height; ++i)
					{
						Copy(buffer + i * 4, textureData0 + i * 3, 1);
						Copy(buffer + i * 4 + 1, textureData1 + i * 3 + 1, 1);
						Copy(buffer + i * 4 + 2, textureData2 + i * 3 + 2, 1);
					}
				}
				else
				{
					for (U32 i = 0; i < width * height; ++i)
					{
						Copy(buffer + i * 4, textureData0 + i * 3, 1);
						Copy(buffer + i * 4 + 1, textureData1 + i * 3 + 1, 1);
						Copy(buffer + i * 4 + 2, &def2, 1);
					}
				}
			}
			else if (textureData2)
			{
				for (U32 i = 0; i < width * height; ++i)
				{
					Copy(buffer + i * 4, textureData0 + i * 3, 1);
					Copy(buffer + i * 4 + 1, &def1, 1);
					Copy(buffer + i * 4 + 2, textureData2 + i * 3 + 2, 1);
				}
			}
			else
			{
				for (U32 i = 0; i < width * height; ++i)
				{
					Copy(buffer + i * 4, textureData0 + i * 3, 1);
					Copy(buffer + i * 4 + 1, &def1, 1);
					Copy(buffer + i * 4 + 2, &def2, 1);
				}
			}
		}
		else if (textureData1)
		{
			if (textureData2)
			{
				for (U32 i = 0; i < width * height; ++i)
				{
					Copy(buffer + i * 4, &def0, 1);
					Copy(buffer + i * 4 + 1, textureData1 + i * 3 + 1, 1);
					Copy(buffer + i * 4 + 2, textureData2 + i * 3 + 2, 1);
				}
			}
			else
			{
				for (U32 i = 0; i < width * height; ++i)
				{
					Copy(buffer + i * 4, &def0, 1);
					Copy(buffer + i * 4 + 1, textureData1 + i * 3 + 1, 1);
					Copy(buffer + i * 4 + 2, &def2, 1);
				}
			}
		}
		else
		{
			for (U32 i = 0; i < width * height; ++i)
			{
				Copy(buffer + i * 4, &def0, 1);
				Copy(buffer + i * 4 + 1, &def1, 1);
				Copy(buffer + i * 4 + 2, textureData2 + i * 3 + 2, 1);
			}
		}

		file.Write(buffer, width * height * 4);
		file.Close();

		if (textureData0) { Memory::Free(&textureData0); }
		if (textureData1) { Memory::Free(&textureData1); }
		if (textureData2) { Memory::Free(&textureData2); }

		Memory::Free(&buffer);

		return Move(path);
	}

	Logger::Error("Failed To Create File: {}!", path);
	return {};
}

U8* Resources::LoadKTX(DataReader& reader, U32& faceCount, U32& faceSize, U32& resolution, VkFormat& format)
{
	static constexpr U8 FileIdentifier11[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr U8 FileIdentifier20[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr U32 EndiannessIdentifier = 0x04030201;

	U8* identifier = reader.Pointer();
	reader.Seek(CountOf32(FileIdentifier11));

	if (Compare(identifier, FileIdentifier11, 12))
	{
		U32 endianness;
		reader.Read(endianness);

		if (endianness != EndiannessIdentifier)
		{
			//TODO: Don't be lazy
			Logger::Error("Too Lazy to Flip Endianness!");
			return nullptr;
		}

		KTXHeader11 header;
		reader.Read(header);

		U8 compression = 0;
		if (header.type == 0 || header.format == 0)
		{
			if (header.type + header.format != 0) { return nullptr; }
			compression = 1;

			Logger::Error("KTX Textures With Compression Not Yet Supported!");
			return nullptr;
		}

		if (header.format == header.internalFormat) { return nullptr; }
		if (header.pixelWidth == 0 || (header.pixelDepth > 0 && header.pixelHeight == 0)) { return nullptr; }

		U16 dimension = 0;
		if (header.pixelDepth > 0)
		{
			if (header.arrayElementCount > 0)
			{
				Logger::Error("3D Array Textures Are Not Yet Supported!");
				return nullptr;
			}
			dimension = 3;
		}
		else if (header.pixelHeight > 0) { dimension = 2; }
		else { dimension = 1; }

		if (header.faceCount == 6) { if (dimension != 2) { return nullptr; } }
		else if (header.faceCount != 1) { return nullptr; }

		KTXInfo info{};
		GetKTXInfo(header.internalFormat, info);

		reader.Seek(header.keyValueDataSize);

		U32 elementCount = Math::Max(1u, header.arrayElementCount);
		U32 depth = Math::Max(1u, header.pixelDepth);
		U32 dataSize = (U32)(reader.Size() - reader.Position() - header.mipmapLevelCount * sizeof(U32));

		U8* data;
		Memory::AllocateSize(&data, dataSize);

		U32 faceLodSize;
		reader.Read(faceLodSize);

		faceCount = header.faceCount;
		faceSize = faceLodSize;
		resolution = header.pixelWidth;
		format = GetKTXFormat(header.type, header.format);

		if (format == VK_FORMAT_UNDEFINED) { return nullptr; }

		return reader.Pointer();
	}
	else if (Compare(identifier, FileIdentifier20, 12))
	{
		KTXHeader20 header;
		reader.Read(header);

		KTXLevel* level = (KTXLevel*)reader.Pointer();
		reader.Seek(sizeof(KTXLevel) * header.levelCount);

		U32 dfdTotalSize;
		reader.Read(dfdTotalSize);

		reader.Seek(dfdTotalSize + header.kvdByteLength);

		if (header.superCompressionScheme != 0)
		{
			Logger::Error("KTX Textures With Compression Not Yet Supported!");
			return nullptr;
		}

		U32 dataSize = (U32)level[0].uncompressedByteLength;
		U8* data;
		Memory::AllocateSize(&data, dataSize);

		faceCount = header.faceCount;
		faceSize = dataSize / header.faceCount;
		resolution = header.pixelWidth;
		format = header.format;

		return reader.Pointer();
	}

	Logger::Error("Texture Is Not a KTX!");

	return nullptr;
}

void Resources::GetKTXInfo(U32 internalFormat, KTXInfo& info)
{
	switch (internalFormat)
	{
	case KTX_FORMAT_R8:
	case KTX_FORMAT_R8_SNORM:
	case KTX_FORMAT_R8UI:
	case KTX_FORMAT_R8I:
	case KTX_FORMAT_SR8: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_RG8:
	case KTX_FORMAT_RG8_SNORM:
	case KTX_FORMAT_RG8UI:
	case KTX_FORMAT_RG8I:
	case KTX_FORMAT_SRG8: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 16;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_RGB8:
	case KTX_FORMAT_RGB8_SNORM:
	case KTX_FORMAT_RGB8UI:
	case KTX_FORMAT_RGB8I:
	case KTX_FORMAT_SRGB8: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 24;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_RGBA8:
	case KTX_FORMAT_RGBA8_SNORM:
	case KTX_FORMAT_RGBA8UI:
	case KTX_FORMAT_RGBA8I:
	case KTX_FORMAT_SRGB8_ALPHA8: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_R16:
	case KTX_FORMAT_R16_SNORM:
	case KTX_FORMAT_R16UI:
	case KTX_FORMAT_R16I:
	case KTX_FORMAT_R16F: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 16;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_RG16:
	case KTX_FORMAT_RG16_SNORM:
	case KTX_FORMAT_RG16UI:
	case KTX_FORMAT_RG16I:
	case KTX_FORMAT_RG16F: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_RGB16:
	case KTX_FORMAT_RGB16_SNORM:
	case KTX_FORMAT_RGB16UI:
	case KTX_FORMAT_RGB16I:
	case KTX_FORMAT_RGB16F: {
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 48;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
	} break;
	case KTX_FORMAT_RGBA16:
	case KTX_FORMAT_RGBA16_SNORM:
	case KTX_FORMAT_RGBA16UI:
	case KTX_FORMAT_RGBA16I:
	case KTX_FORMAT_RGBA16F:
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 64;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_R32UI:
	case KTX_FORMAT_R32I:
	case KTX_FORMAT_R32F:
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RG32UI:
	case KTX_FORMAT_RG32I:
	case KTX_FORMAT_RG32F:
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 64;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB32UI:
	case KTX_FORMAT_RGB32I:
	case KTX_FORMAT_RGB32F:
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 12 * 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGBA32UI:
	case KTX_FORMAT_RGBA32I:
	case KTX_FORMAT_RGBA32F:
		info.flags = KTX_FORMAT_TYPE_NONE;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 16 * 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_R3_G3_B2:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB4:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 12;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB5:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 16;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB565:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 16;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB10:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB12:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 36;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGBA2:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGBA4:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 16;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGBA12:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 48;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB5_A1:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB10_A2:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_RGB10_A2UI:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_R11F_G11F_B10F:
	case KTX_FORMAT_RGB9_E5:
		info.flags = KTX_FORMAT_TYPE_PACKED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 32;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_COMPRESSION_ETC1_RGB8_OES:
	case KTX_COMPRESSION_RGB8_ETC2:
	case KTX_COMPRESSION_SRGB8_ETC2:
	case KTX_COMPRESSION_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
	case KTX_COMPRESSION_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		info.flags = KTX_FORMAT_TYPE_COMPRESSED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 64;
		info.blockWidth = 4;
		info.blockHeight = 4;
		info.blockDepth = 1;
		break;
	case KTX_COMPRESSION_RGBA8_ETC2_EAC:
	case KTX_COMPRESSION_SRGB8_ALPHA8_ETC2_EAC:
		info.flags = KTX_FORMAT_TYPE_COMPRESSED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 128;
		info.blockWidth = 4;
		info.blockHeight = 4;
		info.blockDepth = 1;
		break;
	case KTX_COMPRESSION_R11_EAC:
	case KTX_COMPRESSION_SIGNED_R11_EAC:
		info.flags = KTX_FORMAT_TYPE_COMPRESSED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 64;
		info.blockWidth = 4;
		info.blockHeight = 4;
		info.blockDepth = 1;
		break;
	case KTX_COMPRESSION_RG11_EAC:
	case KTX_COMPRESSION_SIGNED_RG11_EAC:
		info.flags = KTX_FORMAT_TYPE_COMPRESSED;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 128;
		info.blockWidth = 4;
		info.blockHeight = 4;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_PALETTE4_RGB8_OES:
		info.flags = KTX_FORMAT_TYPE_PALETTIZED;
		info.paletteSizeInBits = 16 * 24;
		info.blockSizeInBits = 4;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_PALETTE4_RGBA8_OES:
		info.flags = KTX_FORMAT_TYPE_PALETTIZED;
		info.paletteSizeInBits = 16 * 32;
		info.blockSizeInBits = 4;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_PALETTE4_R5_G6_B5_OES:
	case KTX_FORMAT_PALETTE4_RGBA4_OES:
	case KTX_FORMAT_PALETTE4_RGB5_A1_OES:
		info.flags = KTX_FORMAT_TYPE_PALETTIZED;
		info.paletteSizeInBits = 16 * 16;
		info.blockSizeInBits = 4;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_PALETTE8_RGB8_OES:
		info.flags = KTX_FORMAT_TYPE_PALETTIZED;
		info.paletteSizeInBits = 256 * 24;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_PALETTE8_RGBA8_OES:
		info.flags = KTX_FORMAT_TYPE_PALETTIZED;
		info.paletteSizeInBits = 256 * 32;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	case KTX_FORMAT_PALETTE8_R5_G6_B5_OES:
	case KTX_FORMAT_PALETTE8_RGBA4_OES:
	case KTX_FORMAT_PALETTE8_RGB5_A1_OES:
		info.flags = KTX_FORMAT_TYPE_PALETTIZED;
		info.paletteSizeInBits = 256 * 16;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	default:
		info.flags = 0;
		info.paletteSizeInBits = 0;
		info.blockSizeInBits = 8;
		info.blockWidth = 1;
		info.blockHeight = 1;
		info.blockDepth = 1;
		break;
	}
}

VkFormat Resources::GetKTXFormat(KTXType type, KTXFormat format)
{
	switch (format)
	{
	case KTX_FORMAT_RGB: {
		switch (type)
		{
		case KTX_TYPE_BYTE: return VK_FORMAT_R8G8B8_SINT;
		case KTX_TYPE_UNSIGNED_BYTE: return VK_FORMAT_R8G8B8_UINT;
		case KTX_TYPE_SHORT: return VK_FORMAT_R16G16B16_SINT;
		case KTX_TYPE_UNSIGNED_SHORT: return VK_FORMAT_R16G16B16_UINT;
		case KTX_TYPE_INT: return VK_FORMAT_R32G32B32_SINT;
		case KTX_TYPE_UNSIGNED_INT: return VK_FORMAT_R32G32B32_UINT;
		case KTX_TYPE_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
		case KTX_TYPE_DOUBLE: return VK_FORMAT_R64G64B64_SFLOAT;
		case KTX_TYPE_HALF_FLOAT: return VK_FORMAT_R16G16B16_SFLOAT;
		}
	} break;
	case KTX_FORMAT_RGBA: {
		switch (type)
		{
		case KTX_TYPE_BYTE: return VK_FORMAT_R8G8B8A8_SINT;
		case KTX_TYPE_UNSIGNED_BYTE: return VK_FORMAT_R8G8B8A8_UINT;
		case KTX_TYPE_SHORT: return VK_FORMAT_R16G16B16A16_SINT;
		case KTX_TYPE_UNSIGNED_SHORT: return VK_FORMAT_R16G16B16A16_UINT;
		case KTX_TYPE_INT: return VK_FORMAT_R32G32B32A32_SINT;
		case KTX_TYPE_UNSIGNED_INT: return VK_FORMAT_R32G32B32A32_UINT;
		case KTX_TYPE_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case KTX_TYPE_DOUBLE: return VK_FORMAT_R64G64B64A64_SFLOAT;
		case KTX_TYPE_HALF_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		}
	} break;
	}

	Logger::Error("Unknown KTX Format!");
	return VK_FORMAT_UNDEFINED;
}

struct InstanceUpload
{
	Matrix4 model;
	String material;
};

struct MeshUpload
{
	String path;

	U32 instanceCount{ 0 };
	InstanceUpload instances[32];
};

struct ModelUpload
{
	U32 materialCount{ 0 };
	String materials[32];

	U32 meshCount{ 0 };
	MeshUpload meshes[32];
};

String Resources::UploadModel(const String& path)
{
	const aiScene* scene = aiImportFile(path.Data(), ASSIMP_IMPORT_FLAGS);

	if (!scene) { Logger::Error("Failed To Import Model: {}!", path); return {}; }

	ModelUpload model{};

	String name = scene->mName.data;

	if (name.Blank()) { name = path.GetFileName(); }

	for (U32 i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* materialInfo = scene->mMaterials[i];
		model.materials[model.materialCount++] = Move(ParseAssimpMaterial(name + i, materialInfo, scene));
	}

	for (U32 i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* meshInfo = scene->mMeshes[i];
		model.meshes[model.meshCount++].path = ParseAssimpMesh(name + i, meshInfo);
	}

	ParseAssimpModel(model, scene);

	aiReleaseImport(scene);

	String newPath{ "models/", name, ".nhmdl" };

	File file(newPath, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Model");
		file.Write(MODEL_VERSION);

		file.Write(model.meshCount);

		for (U32 i = 0; i < model.meshCount; ++i)
		{
			MeshUpload& mesh = model.meshes[i];

			file.Write(mesh.path);
			file.Write(mesh.instanceCount);

			for (U32 j = 0; j < mesh.instanceCount; ++j)
			{
				file.Write(mesh.instances[j].material);
				file.Write(mesh.instances[j].model);
			}
		}

		file.Close();
		return Move(newPath);
	}

	Logger::Error("Failed To Upload Model: {}", name);
	return {};
}

String Resources::ParseAssimpMaterial(const String& name, const aiMaterial* materialInfo, const aiScene* scene)
{
	String path = { "materials/", name, ".nhmat" };

	File file(path, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Material");
		file.Write(MATERIAL_VERSION);

		aiReturn ret;

		aiString texturePath;
		U32 textureIndex = 0;

		auto WriteTexture = [&](TextureUsage usage) {
			const aiTexture* texture = scene->GetEmbeddedTexture(texturePath.C_Str());
			if (texture) { file.Write(UploadTexture(name, textureIndex++, texture, usage)); }
			else { file.Write(UploadTexture(texturePath.C_Str())); }
		};

		//Diffuse Color
		if ((ret = materialInfo->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath)) == aiReturn_SUCCESS) { WriteTexture(TEXTURE_USAGE_COLOR); }
		else if ((ret = materialInfo->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath)) == aiReturn_SUCCESS) { WriteTexture(TEXTURE_USAGE_COLOR); }
		else { file.Write("NULL"); }

		//Metalic Roughness AO
		const aiTexture* texture0 = nullptr;
		const aiTexture* texture1 = nullptr;
		const aiTexture* texture2 = nullptr;
		if ((ret = materialInfo->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath)) == aiReturn_SUCCESS) { texture0 = scene->GetEmbeddedTexture(texturePath.C_Str()); }
		if ((ret = materialInfo->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath)) == aiReturn_SUCCESS) { texture1 = scene->GetEmbeddedTexture(texturePath.C_Str()); }
		if ((ret = materialInfo->GetTexture(aiTextureType_METALNESS, 0, &texturePath)) == aiReturn_SUCCESS) { texture2 = scene->GetEmbeddedTexture(texturePath.C_Str()); }

		if (texture0 || texture1 || texture2) { file.Write(UploadTextures(name, textureIndex++, texture0, texture1, texture2, 127ui8, 127ui8, 0ui8, TEXTURE_USAGE_CALCULATION)); }
		else { file.Write("NULL"); }

		//Normal
		if ((ret = materialInfo->GetTexture(aiTextureType_NORMALS, 0, &texturePath)) == aiReturn_SUCCESS) { WriteTexture(TEXTURE_USAGE_CALCULATION); }
		else { file.Write("NULL"); }

		//Emissive Color
		if ((ret = materialInfo->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath)) == aiReturn_SUCCESS) { WriteTexture(TEXTURE_USAGE_COLOR); }
		else if ((ret = materialInfo->GetTexture(aiTextureType_EMISSION_COLOR, 0, &texturePath)) == aiReturn_SUCCESS) { WriteTexture(TEXTURE_USAGE_COLOR); }
		else { file.Write("NULL"); }

		aiColor4D color{ 1.0f, 1.0f, 1.0f, 1.0f };
		ret = materialInfo->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		ai_real metallic{ 0.5f };
		ret = materialInfo->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		ai_real roughness{ 0.5f };
		ret = materialInfo->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		ai_real transparency{ 0.0f };
		ret = materialInfo->Get(AI_MATKEY_TRANSMISSION_FACTOR, transparency);

		file.Write(color.r);
		file.Write(color.g);
		file.Write(color.b);
		file.Write(1.0f - transparency);
		file.Write(metallic);
		file.Write(roughness);
		file.Write(0.0f);
		file.Write(0.0f);
		file.Write(Vector4Zero);
		file.Write(0.0f);
		file.Write(0u);

		file.Close();
	}

	return Move(path);
}

F32 ComputeInertiaProduct(const Vector3& p0, const Vector3& p1, const Vector3& p2, U32 i, U32 j)
{
	return 2.0f * p0[i] * p0[j] + p1[i] * p2[j] + p2[i] * p1[j] +
		2.0f * p1[i] * p1[j] + p0[i] * p2[j] + p2[i] * p0[j] +
		2.0f * p2[i] * p2[j] + p0[i] * p1[j] + p1[i] * p0[j];
}

F32 ComputeInertiaMoment(const Vector3& p0, const Vector3& p1, const Vector3& p2, U32 i)
{
	return p0[i] * p0[i] + p1[i] * p2[i] +
		p1[i] * p1[i] + p0[i] * p2[i] +
		p2[i] * p2[i] + p0[i] * p1[i];
}

String Resources::ParseAssimpMesh(const String& name, const aiMesh* meshInfo)
{
	String path = { "meshes/", name, ".nhmsh" };

	File file(path, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Mesh");
		file.Write(MESH_VERSION);

		U32 vertexCount = meshInfo->mNumVertices;

		U32 faceSize = meshInfo->mFaces[0].mNumIndices * sizeof(U32);

		file.Write(vertexCount);
		file.Write(meshInfo->mNumFaces * faceSize);
		file.Write(meshInfo->HasTangentsAndBitangents());
		file.Write(meshInfo->HasTextureCoords(0));

		U32 totalSize = (sizeof(Vector3) * 4 + sizeof(Vector2)) * vertexCount + meshInfo->mNumFaces * faceSize;
		U8* buffer;
		Memory::AllocateSize(&buffer, totalSize);
		U8* it = buffer;

		Copy((aiVector3D*)it, meshInfo->mVertices, vertexCount);
		it += sizeof(Vector3) * vertexCount;
		Copy((aiVector3D*)it, meshInfo->mNormals, vertexCount);
		it += sizeof(Vector3) * vertexCount;

		if (meshInfo->HasTangentsAndBitangents())
		{
			Copy((aiVector3D*)it, meshInfo->mTangents, vertexCount);
			it += sizeof(Vector3) * vertexCount;
		}

		if (meshInfo->HasTextureCoords(0))
		{
			Copy((aiVector3D*)it, meshInfo->mTextureCoords[0], vertexCount);
			it += sizeof(Vector3) * vertexCount;

			//for (U32 i = 0; i < vertexCount; ++i)
			//{
			//	Copy((aiVector2D*)it, meshInfo->mTextureCoords[0] + i, 1);
			//	it += sizeof(Vector2);
			//}
		}

		const F32 density = 1.0f; //TODO: pass in density

		F32 mass = 0.0f;
		Vector3 centerOfMass = Vector3Zero;
		F32 Ia = 0.0f, Ib = 0.0f, Ic = 0.0f, Iap = 0.0f, Ibp = 0.0f, Icp = 0.0f;

		for (U32 i = 0; i < meshInfo->mNumFaces; ++i)
		{
			Vector3 p0 = TypePun<Vector3>(meshInfo->mVertices[meshInfo->mFaces[i].mIndices[0]]);
			Vector3 p1 = TypePun<Vector3>(meshInfo->mVertices[meshInfo->mFaces[i].mIndices[1]]);
			Vector3 p2 = TypePun<Vector3>(meshInfo->mVertices[meshInfo->mFaces[i].mIndices[2]]);

			F32 det = p0.Dot(p1.Cross(p2));
			F32 tetVolume = det / 6.0f;
			F32 tetMass = density * tetVolume;
			Vector3 tetCenterOfMass = (p0 + p1 + p2) / 4.0f;

			Ia += det * (ComputeInertiaMoment(p0, p1, p2, 1) + ComputeInertiaMoment(p0, p1, p2, 2));
			Ib += det * (ComputeInertiaMoment(p0, p1, p2, 0) + ComputeInertiaMoment(p0, p1, p2, 2));
			Ic += det * (ComputeInertiaMoment(p0, p1, p2, 0) + ComputeInertiaMoment(p0, p1, p2, 1));
			Iap += det * ComputeInertiaProduct(p0, p1, p2, 1, 2);
			Ibp += det * ComputeInertiaProduct(p0, p1, p2, 0, 1);
			Icp += det * ComputeInertiaProduct(p0, p1, p2, 0, 2);

			centerOfMass += tetMass * tetCenterOfMass;
			mass += tetMass;

			Copy((U32*)it, meshInfo->mFaces[i].mIndices, meshInfo->mFaces[0].mNumIndices);
			it += faceSize;
		}

		centerOfMass /= mass;
		Ia = density * Ia / 60.0f - mass * (centerOfMass.y * centerOfMass.y + centerOfMass.z * centerOfMass.z);
		Ib = density * Ib / 60.0f - mass * (centerOfMass.x * centerOfMass.x + centerOfMass.z * centerOfMass.z);
		Ic = density * Ic / 60.0f - mass * (centerOfMass.x * centerOfMass.x + centerOfMass.y * centerOfMass.y);
		Iap = density * Iap / 120.0f - mass * (centerOfMass.y * centerOfMass.z);
		Ibp = density * Ibp / 120.0f - mass * (centerOfMass.x * centerOfMass.y);
		Icp = density * Icp / 120.0f - mass * (centerOfMass.x * centerOfMass.z);

		Matrix3 inertiaMatrix{
			Ia, -Ibp, -Icp,
			-Ibp, Ib, -Iap,
			-Icp, -Iap, Ic
		};

		file.Write(buffer, totalSize);

		file.Write(mass);
		file.Write(centerOfMass);
		file.Write(inertiaMatrix.Inverse());

		file.Close();
	}

	return Move(path);
}

void Resources::ParseAssimpModel(ModelUpload& model, const aiScene* scene)
{
	Stack<aiNode*> nodes(32);

	nodes.Push(scene->mRootNode);

	aiNode* node;
	while (nodes.Pop(node))
	{
		for (U32 i = 0; i < node->mNumMeshes; ++i)
		{
			MeshUpload& draw = model.meshes[node->mMeshes[i]];

			draw.instances[draw.instanceCount].material = model.materials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex];
			draw.instances[draw.instanceCount++].model = *reinterpret_cast<Matrix4*>(&node->mTransformation.Transpose()); //TODO: It seems this is broken sometimes
		}

		for (U32 i = 0; i < node->mNumChildren; ++i)
		{
			nodes.Push(node->mChildren[i]);
		}
	}
}
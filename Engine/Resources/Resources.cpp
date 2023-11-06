#include "Resources.hpp"

#include "Font.hpp"
#include "Platform\Audio.hpp"
#include "Settings.hpp"
#include "Rendering\RenderingDefines.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Core\Logger.hpp"
#include "Core\DataReader.hpp"
#include "Math\Color.hpp"
#include "Containers\Stack.hpp"

#include "External\Assimp\cimport.h"
#include "External\Assimp\scene.h"
#include "External\Assimp\postprocess.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "External\stb_image.h"

#undef near
#undef far

#define ASSIMP_IMPORT_FLAGS (					\
	aiProcess_CalcTangentSpace				|	\
    aiProcess_JoinIdenticalVertices			|	\
    aiProcess_Triangulate					|	\
	aiProcess_RemoveComponent				|	\
    aiProcess_GenSmoothNormals				|	\
    aiProcess_ValidateDataStructure			|	\
    aiProcess_ImproveCacheLocality			|	\
    aiProcess_RemoveRedundantMaterials		|	\
    aiProcess_FindInvalidData				|	\
    aiProcess_GenUVCoords					|	\
	aiProcess_FindInstances					|	\
    aiProcess_OptimizeMeshes				|	\
    aiProcess_OptimizeGraph					|	\
	aiProcess_FlipUVs						|   \
    aiProcess_EmbedTextures)

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
//nhskb
//nhaud
//nhmat
//nhmsh
//nhmdl
//nhshd
//nhscn
//nhfnt
//nhbin

constexpr U32 TEXTURE_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SKYBOX_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 AUDIO_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MATERIAL_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MESH_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MODEL_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SHADER_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SCENE_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 FONT_VERSION = MakeVersionNumber(0, 1, 0);

F32 skyboxVertices[]{
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
};

U32 skyboxIndices[]{
	1, 2, 6, 6, 5, 1, //Right
	0, 4, 7, 7, 3, 0, //Left
	4, 5, 6, 6, 7, 4, //Top
	0, 3, 2, 2, 1, 0, //Bottom
	0, 1, 5, 5, 4, 0, //Back
	3, 7, 6, 6, 2, 3, //Front
};

Texture* Resources::dummyTexture;
PipelineGraph Resources::defaultPipelineGraph;
Pipeline* Resources::meshPipeline;
Pipeline* Resources::postProcessPipeline;
Pipeline* Resources::skyboxPipeline;

Hashmap<String, Texture>		Resources::textures{ 512 };
Hashmap<String, Font>			Resources::fonts{ 32 };
Hashmap<String, AudioClip>		Resources::audioClips{ 512 };
Hashmap<String, Renderpass>		Resources::renderpasses{ 32 };
Hashmap<String, Shader>			Resources::shaders{ 128 };
Hashmap<String, Pipeline>		Resources::pipelines{ 128 };
Hashmap<String, Model>			Resources::models{ 128 };
Hashmap<String, Skybox>			Resources::skyboxes{ 32 };
Hashmap<String, Scene>			Resources::scenes{ 128 };

Queue<ResourceUpdate>			Resources::bindlessTexturesToUpdate;

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	TextureInfo dummyTextureInfo{};
	dummyTextureInfo.SetName("dummy_texture");
	dummyTextureInfo.SetFormatType(FORMAT_TYPE_R8G8B8A8_UNORM, IMAGE_TYPE_2D);
	dummyTextureInfo.SetSize(1, 1, 1);
	U32 zero = 0;
	dummyTextureInfo.SetData(&zero);
	dummyTexture = CreateTexture(dummyTextureInfo);

	PushConstant pushConstant{ 0, sizeof(CameraData), &Renderer::cameraData };
	Shader* meshProgram = CreateShader("shaders/Pbr.nhshd", 1, &pushConstant);
	meshProgram->AddDescriptor({ Renderer::materialBuffer.vkBuffer });

	PipelineInfo info{};
	info.name = "mesh_pipeline";
	info.shader = meshProgram;
	defaultPipelineGraph.AddPipeline(info);

	info.name = "skybox_pipeline";
	info.shader = CreateShader("shaders/Skybox.nhshd", 1, &pushConstant);
	info.vertexBufferSize = sizeof(F32) * CountOf32(skyboxVertices);
	info.instanceBufferSize = sizeof(I32) * 128;
	info.indexBufferSize = sizeof(U32) * CountOf32(skyboxIndices);
	info.drawBufferSize = sizeof(VkDrawIndexedIndirectCommand);
	defaultPipelineGraph.AddPipeline(info);

	pushConstant = { 0, sizeof(PostProcessData), &Renderer::postProcessData };
	info.shader = CreateShader("shaders/PostProcess.nhshd", 1, &pushConstant);
	info.name = "postprocess_pipeline";
	info.vertexBufferSize = 0;
	info.instanceBufferSize = 0;
	info.indexBufferSize = 0;
	info.drawBufferSize = 0;
	defaultPipelineGraph.AddPipeline(info);

	defaultPipelineGraph.Create("default");

	meshPipeline = defaultPipelineGraph.GetPipeline(0, 0);
	skyboxPipeline = defaultPipelineGraph.GetPipeline(0, 1);
	postProcessPipeline = defaultPipelineGraph.GetPipeline(0, 2);

	skyboxPipeline->UploadVertices(sizeof(F32) * CountOf32(skyboxVertices), skyboxVertices);
	skyboxPipeline->UploadIndices(sizeof(U32) * CountOf32(skyboxIndices), skyboxIndices);

	Renderer::pipelineGraph = &defaultPipelineGraph;

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	CleanupHashmap(textures, Renderer::DestroyTextureInstant);
	CleanupHashmap(renderpasses, Renderer::DestroyRenderPassInstant);

	textures.Destroy();
	renderpasses.Destroy();
	fonts.Destroy();
	audioClips.Destroy();
	shaders.Destroy();
	pipelines.Destroy();
	models.Destroy();
	skyboxes.Destroy();
	scenes.Destroy();

	bindlessTexturesToUpdate.Destroy();

	defaultPipelineGraph.Destroy();
}

void Resources::Update()
{
	if (bindlessTexturesToUpdate.Size())
	{
		VkWriteDescriptorSet* bindlessDescriptorWrites;
		Memory::AllocateArray(&bindlessDescriptorWrites, bindlessTexturesToUpdate.Size());

		VkDescriptorImageInfo* bindlessImageInfo;
		Memory::AllocateArray(&bindlessImageInfo, bindlessTexturesToUpdate.Size());

		Texture* dummyTexture = Resources::AccessDummyTexture();

		U32 currentWriteIndex = 0;

		while (bindlessTexturesToUpdate.Size())
		{
			ResourceUpdate textureToUpdate;
			bindlessTexturesToUpdate.Pop(textureToUpdate);

			//TODO: Maybe check frame
			{
				Texture* texture = Resources::AccessTexture(textureToUpdate.handle);
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
				descriptorImageInfo.imageView = texture->format != VK_FORMAT_UNDEFINED ? texture->imageView : dummyTexture->imageView;
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

void Resources::UseSkybox(Skybox* skybox)
{
	VkDrawIndexedIndirectCommand drawCommand{};
	drawCommand.indexCount = CountOf32(skyboxIndices);
	drawCommand.instanceCount = 1;
	drawCommand.firstIndex = 0;
	drawCommand.vertexOffset = 0;
	drawCommand.firstInstance = 0;

	skyboxPipeline->UpdateDrawCall(CountOf32(skyboxIndices), 0, 0, 1, 0, 0);
	skyboxPipeline->drawCount = 1;
}

Texture* Resources::CreateTexture(const TextureInfo& info, const SamplerInfo& samplerInfo)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Texture* texture = textures.Request(info.name, handle);

	if (!texture->name.Blank()) { return texture; }

	*texture = {};

	texture->name = info.name;
	texture->width = info.width;
	texture->height = info.height;
	texture->size = info.width * info.height * 4;
	texture->depth = info.depth;
	texture->flags = info.flags;
	texture->format = info.format;
	texture->mipmapCount = info.mipmapCount;
	texture->type = info.type;
	texture->handle = handle;

	if (!Renderer::CreateTexture(texture, info.initialData))
	{
		Logger::Error("Failed To Create Texture!");
		textures.Remove(texture->handle);
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

	return texture;
}

Texture* Resources::CreateSwapchainTexture(VkImage image, VkFormat format, U8 index)
{
	String name{ "SwapchainTexture", index };

	HashHandle handle;
	Texture* texture = textures.Request(name, handle);

	*texture = {};

	texture->name = name;
	texture->swapchainImage = true;
	texture->format = format;
	texture->image = image;
	texture->handle = handle;

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

	return texture;
}

Renderpass* Resources::CreateRenderpass(const RenderpassInfo& info, const Vector<PipelineInfo>& pipelines)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Renderpass* renderpass = renderpasses.Request(info.name, handle);

	if (!renderpass->name.Blank()) { return renderpass; }

	*renderpass = {};

	renderpass->name = info.name;
	renderpass->handle = handle;
	renderpass->renderTargetCount = (U8)info.renderTargetCount;
	renderpass->depthStencilTarget = info.depthStencilTarget;
	renderpass->colorLoadOp = info.colorLoadOp;
	renderpass->depthLoadOp = info.depthLoadOp;
	renderpass->stencilLoadOp = info.stencilLoadOp;

	bool first = true;
	for (const PipelineInfo& pipeline : pipelines)
	{
		if (pipeline.shader->subpass.inputAttachmentCount)
		{
			if (first)
			{
				Logger::Error("First Shader In Renderpass Cannot Have Input Attachments!");
				renderpasses.Remove(renderpass->handle);
				renderpass->Destroy();
				return nullptr;
			}

			renderpass->subpasses[renderpass->subpassCount++] = pipeline.shader->subpass;
		}

		first = false;
	}

	for (U32 i = 0; i < info.renderTargetCount; ++i)
	{
		renderpass->renderTargets[i] = info.renderTargets[i];
	}

	if (!Renderer::CreateRenderpass(renderpass))
	{
		Logger::Error("Failed To Create Renderpass!");
		renderpasses.Remove(renderpass->handle);
		renderpass->Destroy();
		return nullptr;
	}

	return renderpass;
}

Shader* Resources::CreateShader(const String& name, U8 pushConstantCount, PushConstant* pushConstants)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Shader* shader = shaders.Request(name, handle);

	if (!shader->name.Blank()) { return shader; }

	*shader = {};

	shader->name = name;
	shader->handle = handle;

	if (!shader->Create(name, pushConstantCount, pushConstants))
	{
		Logger::Error("Failed To Create Shader!");
		shaders.Remove(shader->handle);
		shader->Destroy();
		return nullptr;
	}

	return shader;
}

Pipeline* Resources::CreatePipeline(const PipelineInfo& info, Renderpass* renderpass)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Pipeline* pipeline = pipelines.Request(info.name, handle);

	if (!pipeline->name.Blank()) { return pipeline; }

	*pipeline = {};

	pipeline->name = info.name;
	pipeline->handle = handle;
	pipeline->subpass = info.subpass;

	if (!pipeline->Create(info, renderpass))
	{
		Logger::Error("Failed To Create Pipeline!");
		pipelines.Remove(handle);
		pipeline->Destroy();
		return nullptr;
	}

	return pipeline;
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

bool Resources::RecreateSwapchainTexture(Texture* texture, VkImage image)
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

bool Resources::RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth)
{
	Texture deleteTexture;
	deleteTexture.imageView = texture->imageView;
	deleteTexture.image = texture->image;
	deleteTexture.allocation = texture->allocation;
	deleteTexture.width = texture->width;
	deleteTexture.height = texture->height;
	deleteTexture.depth = texture->depth;

	texture->width = width;
	texture->height = height;
	texture->depth = depth;

	if (!Renderer::CreateTexture(texture, nullptr))
	{
		Logger::Error("Failed To Recreate Pipeline!");
		texture->width = deleteTexture.width;
		texture->height = deleteTexture.height;
		texture->depth = deleteTexture.depth;

		return false;
	}

	Renderer::DestroyTextureInstant(&deleteTexture);

	return true;
}

void Resources::RecreateRenderpass(Renderpass* renderpass)
{
	Renderer::RecreateRenderpass(renderpass);
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

Font* Resources::LoadFont(const String& path)
{
	static bool b = SetAtlasPositions();

	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Font* font = fonts.Request(path, handle);

	if (!font->name.Blank()) { return font; }

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

		Texture* texture = textures.Request(textureName, handle);
		*texture = {};

		reader.Read(texture->width);
		reader.Read(texture->height);

		texture->name = Move(textureName);
		texture->handle = handle;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->format = VK_FORMAT_R32G32B32A32_SFLOAT;
		texture->mipmapCount = 1;
		texture->size = texture->width * texture->height * 4 * sizeof(F32);

		texture->sampler.minFilter = FILTER_TYPE_NEAREST;
		texture->sampler.magFilter = FILTER_TYPE_NEAREST;
		texture->sampler.mipFilter = SAMPLER_MIPMAP_MODE_NEAREST;

		if (!Renderer::CreateTexture(texture, reader.Pointer()))
		{
			Logger::Error("Failed To Create Texture: {}!", texture->name);
			fonts.Remove(font->handle);
			textures.Remove(handle);
			textures.Destroy();
			fonts.Destroy();
			return nullptr;
		}

		font->texture = texture;

		return font;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	fonts.Remove(handle);
	return nullptr;
}

AudioClip* Resources::LoadAudio(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	AudioClip* audioClip = audioClips.Request(path, handle);

	if (!audioClip->name.Blank()) { return audioClip; }

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

		Memory::Copy(audioClip->buffer, reader.Pointer(), audioClip->size);

		return audioClip;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	audioClips.Remove(handle);
	return nullptr;
}

Texture* Resources::LoadTexture(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Texture* texture = textures.Request(path, handle);

	if (!texture->name.Blank()) { return texture; }

	*texture = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		texture->name = path;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->handle = handle;

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

		reader.ReadSize(&texture->sampler, sizeof(SamplerInfo));

		reader.Read(texture->width);
		reader.Read(texture->height);
		reader.Read(texture->format);
		reader.Read(texture->mipmapCount);
		texture->size = texture->width * texture->height * 4;

		if (!Renderer::CreateTexture(texture, reader.Pointer()))
		{
			Logger::Error("Failed To Create Texture: {}!", path);
			textures.Remove(handle);
			texture->Destroy();
			return nullptr;
		}

		return texture;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	textures.Remove(handle);
	return nullptr;
}

Model* Resources::LoadModel(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Model* model = models.Request(path, handle);

	if (!model->name.Blank()) { return model; }

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

		U32 textureCount;
		reader.Read(textureCount);

		U32 textureIndices[32]{};

		TextureInfo info{};
		info.depth = 1;
		info.format = VK_FORMAT_R8G8B8A8_UNORM;

		for (U32 i = 0; i < textureCount; ++i)
		{
			textureIndices[i] = (U32)LoadTexture(reader.ReadString())->handle;
		}

		U32 materialCount;
		reader.Read(materialCount);

		U32 materialIndices[32]{};

		for (U32 i = 0; i < materialCount; ++i)
		{
			Material material;
			reader.Read(material);

			if (material.diffuseTextureIndex != U16_MAX) { material.diffuseTextureIndex = textureIndices[material.diffuseTextureIndex]; }
			if (material.normalTextureIndex != U16_MAX) { material.normalTextureIndex = textureIndices[material.normalTextureIndex]; }
			if (material.metalRoughOcclTextureIndex != U16_MAX) { material.metalRoughOcclTextureIndex = textureIndices[material.metalRoughOcclTextureIndex]; }
			if (material.emissivityTextureIndex != U16_MAX) { material.emissivityTextureIndex = textureIndices[material.emissivityTextureIndex]; }

			materialIndices[i] = (U32)(Renderer::UploadToBuffer(Renderer::materialBuffer, sizeof(Material), &material) / sizeof(Material));
		}

		U32 meshCount;
		reader.Read(meshCount);

		model->meshes.Resize(meshCount);

		for (U32 i = 0; i < meshCount; ++i)
		{
			DrawCall& draw = model->meshes[i];

			U32 verticesSize;
			reader.Read(verticesSize);
			draw.vertexOffset = (U32)((meshPipeline->UploadVertices(verticesSize, reader.Pointer()) / sizeof(Vertex)));
			reader.Seek(verticesSize);

			U32 indicesSize;
			reader.Read(indicesSize);
			draw.indexCount = indicesSize / sizeof(U32);
			draw.indexOffset = (U32)(meshPipeline->UploadIndices(indicesSize, reader.Pointer()) / sizeof(U32));
			reader.Seek(indicesSize);

			U32 instanceCount;
			reader.Read(instanceCount);
			draw.instances.Resize(instanceCount);

			for (U32 j = 0; j < instanceCount; ++j)
			{
				U16 index;
				reader.Read(index);
				draw.instances[j].materialIndex = materialIndices[index];
				reader.Read(draw.instances[j].model);
			}

			U32 instanceOffset = (U32)(meshPipeline->UploadInstances((U32)sizeof(MeshInstance) * (U32)draw.instances.Size(), draw.instances.Data()) / sizeof(MeshInstance));
			meshPipeline->UploadDrawCall(draw.indexCount, draw.indexOffset, draw.vertexOffset, (U32)draw.instances.Size(), instanceOffset);
		}

		return model;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);
	models.Remove(handle);
	return nullptr;
}

Skybox* Resources::LoadSkybox(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Skybox* skybox = skyboxes.Request(path, handle);

	if (!skybox->name.Blank()) { return skybox; }

	*skybox = {};

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

		U32 faceCount;
		U32 faceSize;

		reader.Read(faceCount);
		reader.Read(faceSize);

		String textureName = path.GetFileName().Appended("_texture");

		Texture* texture = textures.Request(textureName, handle);
		*texture = {};

		reader.Read((U32&)texture->width);
		texture->height = texture->width;
		reader.Read(texture->format);

		texture->name = Move(textureName);
		texture->handle = handle;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->mipmapCount = 1;
		texture->size = faceSize * faceCount;

		if (!Renderer::CreateCubemap(texture, reader.Pointer(), &faceSize))
		{
			Logger::Error("Failed To Create Cubemap!", path);
			textures.Remove(handle);
			skyboxes.Remove(skybox->handle);
			texture->Destroy();
			skybox->Destroy();
			return {};
		}

		skybox->texture = texture;

		skybox->instance = skyboxPipeline->UploadInstances(sizeof(U32), (U32*)&texture->handle);

		return skybox;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	skyboxes.Remove(skybox->handle);
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

void Resources::SaveScene(const Scene* scene)
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

Texture* Resources::AccessDummyTexture()
{
	return dummyTexture;
}

Texture* Resources::AccessTexture(const String& name)
{
	return textures.Get(name);
}

Renderpass* Resources::AccessRenderpass(const String& name)
{
	return renderpasses.Get(name);
}

Pipeline* Resources::AccessPipeline(const String& name)
{
	return pipelines.Get(name);
}

Texture* Resources::AccessTexture(HashHandle handle)
{
	return textures.Obtain(handle);
}

Renderpass* Resources::AccessRenderpass(HashHandle handle)
{
	return renderpasses.Obtain(handle);
}

Pipeline* Resources::AccessPipeline(HashHandle handle)
{
	return pipelines.Obtain(handle);
}

void Resources::DestroyTexture(Texture* texture)
{
	if (texture->handle != U64_MAX)
	{
		Renderer::DestroyTextureInstant(textures.Obtain(texture->handle));
		textures.Remove(texture->handle);

		texture->handle = U64_MAX;
	}
}

void Resources::DestroyRenderpass(Renderpass* renderpass)
{
	if (renderpass->handle != U64_MAX)
	{
		Renderer::DestroyRenderPassInstant(renderpasses.Obtain(renderpass->handle));
		renderpasses.Remove(renderpass->handle);

		renderpass->handle = U64_MAX;
	}
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
		U16 width;
		U16 height;
		F32* atlas = FontLoader::LoadFont(data, font, width, height);

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

		file.Write(width);
		file.Write(height);

		file.WriteCount(atlas, width * height * 4);

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

		if (Memory::Compare(path.Data() + extension + 1, "wav", 3))
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
					reader.ReadSize(format, chunkSize);
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
		else if (Memory::Compare(path.Data() + extension + 1, "mp3", 3))
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

String Resources::UploadTexture(const String& path, const SamplerInfo& samplerInfo)
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

		file.Write(samplerInfo);

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(VK_FORMAT_R8G8B8A8_UNORM);
		file.Write(MipmapCount(width, height));
		file.Write(textureData, width * height * 4);
		Memory::Free(&textureData);

		file.Close();

		return Move(newPath);
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	return {};
}

String Resources::UploadTexture(const aiTexture* textureInfo)
{
	I32 width;
	I32 height;
	U8* textureData = stbi_load_from_memory((U8*)textureInfo->pcData, textureInfo->mWidth, &width, &height, nullptr, 4);

	if (!textureData)
	{
		Logger::Error("Failed To Convert Image!");
		return {};
	}

	String name = textureInfo->mFilename.C_Str();

	if (name.Blank()) { name = String::RandomString(16).Surround("textures/", ".nhtex"); }
	else { name = name.GetFileName().Surround("textures/", ".nhtex"); }

	File file(name, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Texture");
		file.Write(TEXTURE_VERSION);

		SamplerInfo sampler = {};
		file.Write(sampler);

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(VK_FORMAT_R8G8B8A8_UNORM);
		file.Write(MipmapCount(width, height));
		file.Write(textureData, width * height * 4);
		Memory::Free(&textureData);

		file.Close();

		return Move(name);
	}

	return {};
}

String Resources::UploadSkybox(const String& path)
{
	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		I64 extIndex = path.LastIndexOf('.') + 1;
		DataReader reader{ file };
		file.Close();

		U8* imageData = nullptr;
		U32 faceCount;
		U32 faceSize;
		U32 resolution;
		VkFormat format;

		if (path.CompareN("ktx", extIndex) || path.CompareN("ktx2", extIndex) || path.CompareN("ktx1", extIndex)) { imageData = LoadKTX(reader, faceCount, faceSize, resolution, format); }
		else { imageData = LoadHDRToCube(reader, faceSize, resolution, format); faceCount = 6; }

		if (imageData == nullptr) { return {}; }

		String newPath = path.GetFileName().Surround("textures/", ".nhskb");

		file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

		file.Write("NH Skybox");
		file.Write(SKYBOX_VERSION);
		file.Write(faceCount);
		file.Write(faceSize);
		file.Write(resolution);
		file.Write(format);
		file.WriteCount(imageData, faceSize * faceCount);

		file.Close();

		return Move(newPath);
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);
	return {};
}

U8* Resources::LoadKTX(DataReader& reader, U32& faceCount, U32& faceSize, U32& resolution, VkFormat& format)
{
	static constexpr U8 FileIdentifier11[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr U8 FileIdentifier20[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr U32 EndiannessIdentifier = 0x04030201;

	U8* identifier = reader.Pointer();
	reader.Seek(CountOf32(FileIdentifier11));

	if (Memory::Compare(identifier, FileIdentifier11, 12))
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
	else if (Memory::Compare(identifier, FileIdentifier20, 12))
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

U8* Resources::LoadHDRToCube(DataReader& reader, U32& faceSize, U32& resolution, VkFormat& format)
{
	U32 resultChannelCount = 4;

	I32 x, y, channelCount;
	U8* data = stbi_load_from_memory(reader.Data(), (I32)reader.Size(), &x, &y, &channelCount, 0);

	resolution = x / 4;
	format = VK_FORMAT_R8G8B8A8_UINT;
	faceSize = resolution * resolution * resultChannelCount;

	U8* result;
	Memory::AllocateSize(&result, faceSize * 6);

	Vector3 startRightUp[6][3]{
		{ { 1.0f, -1.0f, -1.0f }, { 0.0f,  0.0f,  1.0f }, { 0.0f,  1.0f,  0.0f } },	// right
		{ {-1.0f, -1.0f,  1.0f }, { 0.0f,  0.0f, -1.0f }, { 0.0f,  1.0f,  0.0f } },	// left
		{ {-1.0f,  1.0f, -1.0f }, { 1.0f,  0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },	// up
		{ {-1.0f, -1.0f,  1.0f }, { 1.0f,  0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },	// down
		{ {-1.0f, -1.0f, -1.0f }, { 1.0f,  0.0f,  0.0f }, { 0.0f,  1.0f,  0.0f } },	// front
		{ { 1.0f, -1.0f,  1.0f }, {-1.0f,  0.0f,  0.0f }, { 0.0f,  1.0f,  0.0f } },	// back
	};

	for (U32 i = 0; i < 6; ++i)
	{
		Vector3& start = startRightUp[i][0];
		Vector3& right = startRightUp[i][1];
		Vector3& up = startRightUp[i][2];

		U8* face = result + faceSize * i;
		Vector3 pixelDirection3d;
		for (U32 row = 0; row < resolution; ++row)
		{
			for (U32 col = 0; col < resolution; ++col)
			{
				F32 colDir = (F32)col * 2.0f + 0.5f;
				F32 rowDir = (F32)row * 2.0f + 0.5f;

				pixelDirection3d.x = start.x + colDir / (F32)resolution * right.x + rowDir / (F32)resolution * up.x;
				pixelDirection3d.y = start.y + colDir / (F32)resolution * right.y + rowDir / (F32)resolution * up.y;
				pixelDirection3d.z = start.z + colDir / (F32)resolution * right.z + rowDir / (F32)resolution * up.z;

				F32 azimuth = Math::Atan2(pixelDirection3d.x, -pixelDirection3d.z) + PI_F;
				F32 elevation = Math::Atan(pixelDirection3d.y / Math::Sqrt(pixelDirection3d.x * pixelDirection3d.x + pixelDirection3d.z * pixelDirection3d.z)) + HALF_PI_F;

				F32 colHdri = (azimuth / HALF_PI_F) * x;
				F32 rowHdri = (elevation / PI_F) * y;

				U32 colNearest = Math::Clamp((I32)colHdri, 0, x - 1);
				U32 rowNearest = Math::Clamp((I32)rowHdri, 0, x - 1);

				face[(col + resolution * row) * resultChannelCount] = data[colNearest * channelCount + x * rowNearest * channelCount];
				face[(col + resolution * row) * resultChannelCount + 1] = data[colNearest * channelCount + x * rowNearest * channelCount + 1];
				face[(col + resolution * row) * resultChannelCount + 2] = data[colNearest * channelCount + x * rowNearest * channelCount + 2];
				face[(col + resolution * row) * resultChannelCount + 3] = 255;
			}
		}
	}

	return result;
}

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
		model.materials[model.materialCount++] = ParseAssimpMaterial(model, materialInfo, scene);
	}

	for (U32 i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* meshInfo = scene->mMeshes[i];
		model.meshes[model.meshCount] = ParseAssimpMesh(meshInfo);
		model.meshes[model.meshCount++].materialIndex = meshInfo->mMaterialIndex;
	}

	ParseAssimpModel(model, scene);

	aiReleaseImport(scene);

	String newPath = name;
	newPath.Surround("models/", ".nhmdl");

	File file(newPath, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write("NH Model");
		file.Write(MODEL_VERSION);

		file.Write(model.textureCount);

		for (U32 i = 0; i < model.textureCount; ++i) { file.Write(model.textures[i]); }

		file.Write(model.materialCount);

		for (U32 i = 0; i < model.materialCount; ++i) { file.Write(model.materials[i]); }

		file.Write(model.meshCount);

		for (U32 i = 0; i < model.meshCount; ++i)
		{
			MeshUpload& mesh = model.meshes[i];

			file.Write(mesh.verticesSize);
			file.Write(mesh.vertices, mesh.verticesSize);
			file.Write(mesh.indicesSize);
			file.Write(mesh.indices, mesh.indicesSize);
			file.Write(mesh.instanceCount);

			for (U32 j = 0; j < mesh.instanceCount; ++j)
			{
				file.Write(mesh.materialIndex);
				file.Write(mesh.instances[j]);
			}
		}

		file.Close();
		return Move(newPath);
	}

	Logger::Error("Failed To Upload Model: {}", name);
	return {};
}

Material Resources::ParseAssimpMaterial(ModelUpload& model, const aiMaterial* materialInfo, const aiScene* scene)
{
	Material material{};

	aiReturn ret;

	for (U32 i = 0; i <= AI_TEXTURE_TYPE_MAX; ++i)
	{
		aiString texturePath;
		ret = materialInfo->GetTexture((aiTextureType)i, 0, &texturePath, nullptr, nullptr, nullptr, nullptr, nullptr);
		if (ret == aiReturn_SUCCESS)
		{
			U16 index = model.textureCount;
			const aiTexture* texture = scene->GetEmbeddedTexture(texturePath.C_Str());
			if (texture) { model.textures[model.textureCount++] = UploadTexture(texture); }
			else { model.textures[model.textureCount++] = UploadTexture(texturePath.C_Str()); }

			switch (i)
			{
			case aiTextureType_DIFFUSE: { material.diffuseTextureIndex = index; } break;
			case aiTextureType_EMISSIVE: { material.emissivityTextureIndex = index; } break;
			case aiTextureType_NORMALS: { material.normalTextureIndex = index; } break;
			case aiTextureType_SHININESS: {} break;
			case aiTextureType_BASE_COLOR: { material.diffuseTextureIndex = index; } break;
			case aiTextureType_EMISSION_COLOR: { material.emissivityTextureIndex = index; } break;
			case aiTextureType_METALNESS: {} break;			//TODO: Combine these textures
			case aiTextureType_DIFFUSE_ROUGHNESS: {} break;
			case aiTextureType_AMBIENT_OCCLUSION: {} break;

			case aiTextureType_NONE:
			case aiTextureType_SPECULAR:
			case aiTextureType_AMBIENT:
			case aiTextureType_HEIGHT:
			case aiTextureType_OPACITY:
			case aiTextureType_DISPLACEMENT:
			case aiTextureType_LIGHTMAP:
			case aiTextureType_REFLECTION:
			case aiTextureType_NORMAL_CAMERA:
			case aiTextureType_UNKNOWN:
			case aiTextureType_SHEEN:
			case aiTextureType_CLEARCOAT:
			case aiTextureType_TRANSMISSION:
			default: { Logger::Warn("Unknown Texture Usage '{}'!", i); }
			}
		}
	}

	aiColor4D color{ 1.0f, 1.0f, 1.0f, 1.0f };
	ret = materialInfo->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	ai_real roughness{ 0.5f };
	ret = materialInfo->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
	ai_real metallic{ 0.5f };
	ret = materialInfo->Get(AI_MATKEY_METALLIC_FACTOR, metallic);

	material.baseColorFactor.x = color.r;
	material.baseColorFactor.y = color.g;
	material.baseColorFactor.z = color.b;
	material.baseColorFactor.w = color.a;

	material.metalRoughFactor.x = metallic;
	material.metalRoughFactor.y = roughness;

	return material;
}

MeshUpload Resources::ParseAssimpMesh(const aiMesh* meshInfo)
{
	MeshUpload mesh{};

	U32 vertexCount = meshInfo->mNumVertices;
	mesh.verticesSize = vertexCount * sizeof(Vertex);
	Memory::AllocateSize(&mesh.vertices, mesh.verticesSize);

	if (meshInfo->HasTangentsAndBitangents())
	{
		if (meshInfo->HasTextureCoords(0))
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				Vertex& vertex = mesh.vertices[i];

				vertex.position = *reinterpret_cast<Vector3*>(&meshInfo->mVertices[i]);
				vertex.normal = *reinterpret_cast<Vector3*>(&meshInfo->mNormals[i]);
				vertex.tangent = *reinterpret_cast<Vector3*>(&meshInfo->mTangents[i]);
				vertex.bitangent = *reinterpret_cast<Vector3*>(&meshInfo->mBitangents[i]);
				vertex.texcoord = *reinterpret_cast<Vector2*>(&meshInfo->mTextureCoords[0][i]);
			}
		}
		else
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				Vertex& vertex = mesh.vertices[i];

				vertex.position = *reinterpret_cast<Vector3*>(&meshInfo->mVertices[i]);
				vertex.normal = *reinterpret_cast<Vector3*>(&meshInfo->mNormals[i]);
				vertex.tangent = *reinterpret_cast<Vector3*>(&meshInfo->mTangents[i]);
				vertex.bitangent = *reinterpret_cast<Vector3*>(&meshInfo->mBitangents[i]);
			}
		}
	}
	else
	{
		if (meshInfo->HasTextureCoords(0))
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				Vertex& vertex = mesh.vertices[i];

				vertex.position = *reinterpret_cast<Vector3*>(&meshInfo->mVertices[i]);
				vertex.normal = *reinterpret_cast<Vector3*>(&meshInfo->mNormals[i]);
				vertex.texcoord = *reinterpret_cast<Vector2*>(&meshInfo->mTextureCoords[0][i]);
				vertex.tangent = {};
				vertex.bitangent = {};
			}
		}
		else
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				Vertex& vertex = mesh.vertices[i];

				vertex.position = *reinterpret_cast<Vector3*>(&meshInfo->mVertices[i]);
				vertex.normal = *reinterpret_cast<Vector3*>(&meshInfo->mNormals[i]);
				vertex.tangent = {};
				vertex.bitangent = {};
			}
		}
	}

	U32 faceSize = meshInfo->mFaces[0].mNumIndices * sizeof(U32);

	mesh.indicesSize = meshInfo->mNumFaces * faceSize;
	Memory::AllocateSize(&mesh.indices, mesh.indicesSize);

	U8* it = (U8*)mesh.indices;

	for (U32 i = 0; i < meshInfo->mNumFaces; ++i)
	{
		Memory::Copy(it, meshInfo->mFaces[i].mIndices, faceSize);
		it += faceSize;
	}

	return mesh;
}

void Resources::ParseAssimpModel(ModelUpload& model, const aiScene* scene)
{
	Stack<aiNode*> nodes{ 32 };

	nodes.Push(scene->mRootNode);

	while (nodes.Size())
	{
		aiNode* node = nodes.Pop();

		for (U32 i = 0; i < node->mNumMeshes; ++i)
		{
			MeshUpload& draw = model.meshes[node->mMeshes[i]];

			draw.instances[draw.instanceCount++] = *reinterpret_cast<Matrix4*>(&node->mTransformation.Transpose());
		}

		for (U32 i = 0; i < node->mNumChildren; ++i)
		{
			nodes.Push(node->mChildren[i]);
		}
	}
}
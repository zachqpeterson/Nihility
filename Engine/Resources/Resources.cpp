#include "Resources.hpp"

#include "Core\Logger.hpp"
#include "Core\File.hpp"

#define TEXTURES_PATH "textures/"
#define AUDIO_PATH "audio/"
#define SHADERS_PATH "shaders/"
#define MATERIALS_PATH "materials/"
#define MODELS_PATH "models/"
#define FONTS_PATH "fonts/"

//Hashmap<Texture*> Resources::textures(1024);

#pragma pack(push, 1)

struct BMPHeader
{
	U16 signature;
	U32 fileSize;
	U16 reserved1;
	U16 reserved2;
	U32 imageOffset;
};

struct DIBHeader
{
	U32 headerSize;
	I32 imageWidth;
	I32 imageHeight;
	U16 imagePlanes;
	U16 imageBitCount;
	U32 imageCompression;
	U32 imageSize;
	I32 biXPelsPerMeter;
	I32 biYPelsPerMeter;
	U32 colorsUsed;
	U32 importantColor;
	U32 extraRead;
	U32 redMask;
	U32 greenMask;
	U32 blueMask;
	U32 alphaMask;
};

#pragma pack(pop)

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	buffers.Create();
	textures.Create();
	renderPasses.Create();
	descriptorSetLayouts.Create();
	pipelines.Create();
	shaders.Create();
	descriptorSets.Create();
	samplers.Create();

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	pipelines.Destroy();
	buffers.Destroy();
	shaders.Destroy();
	textures.Destroy();
	samplers.Destroy();
	descriptorSetLayouts.Destroy();
	descriptorSets.Destroy();
	renderPasses.Destroy();
}

//Texture* Resources::LoadTexture(String& name)
//{
//	Texture* texture = nullptr;
//
//	if (textures.Get(name, texture)) { return texture; }
//
//	String path(TEXTURES_PATH, name);
//
//	File file(path, FILE_OPEN_RESOURCE);
//	if (file.Opened())
//	{
//		//TODO: Find file type, redirect to method
//
//		
//
//	}
//
//	Logger::Error("Failed to find or open file: {}", path);
//
//	return nullptr;
//}

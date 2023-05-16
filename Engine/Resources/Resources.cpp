#include "Resources.hpp"

#include "Rendering\Renderer.hpp"
#include "Core\Logger.hpp"
#include "Core\File.hpp"

#define TEXTURES_PATH "textures/"
#define AUDIO_PATH "audio/"
#define SHADERS_PATH "shaders/"
#define MATERIALS_PATH "materials/"
#define MODELS_PATH "models/"
#define FONTS_PATH "fonts/"

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

	//TODO: DummyResources

	TextureCreation textureCreation{ };
	U32 zeroValue = 0;
	textureCreation.SetName("dummy_texture").SetSize(1, 1, 1).SetFormatType(VK_FORMAT_R8G8B8A8_UNORM, TEXTURE_TYPE_2D).SetFlags(1, 0).SetData(&zeroValue);
	TextureHandle dummyTexture = CreateTexture(textureCreation);

	SamplerCreation samplerCreation{ };
	samplerCreation.minFilter = VK_FILTER_LINEAR;
	samplerCreation.magFilter = VK_FILTER_LINEAR;
	samplerCreation.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreation.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerHandle dummySampler = CreateSampler(samplerCreation);

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

Texture* Resources::LoadTexture(const TextureCreation& info)
{
	Texture* texture = &textures.GetInsert(info.name);

	if (!texture->name.Blank()) { return texture; }

	String path(TEXTURES_PATH, info.name);

	File file(path, FILE_OPEN_RESOURCE);
	if (file.Opened())
	{
		//TODO: Find file type, redirect to method, load data
		void* data = nullptr;

		Renderer::CreateTexture(texture, data);

		return texture;
	}

	Logger::Error("Failed to find or open file: {}", path);

	return nullptr;
}

Sampler* Resources::CreateSampler(const SamplerCreation& info)
{
	Sampler* sampler = &samplers.GetInsert(info.name);

	if (!sampler->name.Blank()) { return sampler; }

	sampler->addressModeU = info.addressModeU;
	sampler->addressModeV = info.addressModeV;
	sampler->addressModeW = info.addressModeW;
	sampler->minFilter = info.minFilter;
	sampler->magFilter = info.magFilter;
	sampler->mipFilter = info.mipFilter;
	sampler->name = info.name;

	Renderer::CreateSampler(sampler);

	return sampler;
}

Buffer* Resources::CreateBuffer(const BufferCreation& info, void* data)
{
	Buffer* buffer = &buffers.GetInsert(info.name);

	if (!buffer->name.Blank()) { return buffer; }

	buffer->name = info.name;
	buffer->size = info.size;
	buffer->typeFlags = info.typeFlags;
	buffer->usage = info.usage;
	buffer->globalOffset = 0;
	buffer->parentBuffer = nullptr;

	Renderer::CreateBuffer(buffer, data);

	return buffer;
}

DescriptorSetLayout* Resources::CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& info)
{
	DescriptorSetLayout* descriptorSetLayout = &descriptorSetLayouts.GetInsert(info.name);

	if (!descriptorSetLayout->name.Blank()) { return descriptorSetLayout; }

	// TODO: add support for multiple sets.
	// Create flattened binding list
	descriptorSetLayout->numBindings = (U16)info.numBindings;
	U8* memory;
	Memory::AllocateSize(&memory, (sizeof(VkDescriptorSetLayoutBinding) + sizeof(DescriptorBinding)) * info.numBindings);
	descriptorSetLayout->bindings = (DescriptorBinding*)memory;
	descriptorSetLayout->binding = (VkDescriptorSetLayoutBinding*)(memory + sizeof(DescriptorBinding) * info.numBindings);
	descriptorSetLayout->setIndex = (U16)info.setIndex;

	Renderer::CreateDescriptorSetLayout(descriptorSetLayout);

	return descriptorSetLayout;
}

DescriptorSet* Resources::CreateDescriptorSet(const DescriptorSetCreation& info)
{
	DescriptorSet* descriptorSet = &descriptorSets.GetInsert(info.name);

	if (!descriptorSet->name.Blank()) { return descriptorSet; }
}

Sampler* Resources::GetDummySampler()
{

}

bool Resources::LoadBinary(const String& name, String& result)
{
	String path;
	name.Prepended(path, SHADERS_PATH);

	File file{ path, FILE_OPEN_RESOURCE };

	if (file.Opened())
	{
		file.ReadAll(result);
		file.Close();

		return true;
	}

	return false;
}

bool Resources::LoadBinary(const String& name, void** result)
{
	String path;
	name.Prepended(path, SHADERS_PATH);

	File file{ path, FILE_OPEN_RESOURCE };

	if (file.Opened())
	{
		file.ReadAll(result);
		file.Close();

		return true;
	}

	return false;
}
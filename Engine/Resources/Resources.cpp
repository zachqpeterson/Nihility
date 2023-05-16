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

	//TextureCreation textureCreation{ };
	//U32 zeroValue = 0;
	//textureCreation.SetName("dummy_texture").SetSize(1, 1, 1).SetFormatType(VK_FORMAT_R8G8B8A8_UNORM, TEXTURE_TYPE_2D).SetFlags(1, 0).SetData(&zeroValue);
	//TextureHandle dummyTexture = CreateTexture(textureCreation);
	//

	dummyTexture.name = "dummy_texture";
	dummyTexture.width = 1;
	dummyTexture.height = 1;
	dummyTexture.depth = 1;
	dummyTexture.type = TEXTURE_TYPE_2D;
	dummyTexture.format = VK_FORMAT_R8G8B8A8_UNORM;
	dummyTexture.mipmaps = 1;
	dummyTexture.flags = 0;

	U32 zero = 0;
	Renderer::CreateTexture(&dummyTexture, &zero);

	dummySampler.name = "dummy_sampler";
	dummySampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	dummySampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	dummySampler.minFilter = VK_FILTER_LINEAR;
	dummySampler.magFilter = VK_FILTER_LINEAR;

	Renderer::CreateSampler(&dummySampler);

	dummyAttributeBuffer.name = "dummy_attribute_buffer";
	dummyAttributeBuffer.size = sizeof(Vector4) * 3;
	dummyAttributeBuffer.typeFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	dummyAttributeBuffer.usage = RESOURCE_USAGE_IMMUTABLE;
	dummyAttributeBuffer.globalOffset = 0;
	dummyAttributeBuffer.parentBuffer = nullptr;

	Vector4 dummyData[3]{};
	Renderer::CreateBuffer(&dummyAttributeBuffer, dummyData);

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

Texture* Resources::CreateTexture(const TextureCreation& info)
{
	Texture* texture = &textures.Request(info.name);

	if (!texture->name.Blank()) { return texture; }

	texture->name = info.name;
	texture->width = info.width;
	texture->height = info.height;
	texture->depth = info.depth;
	texture->flags = info.flags;
	texture->format = info.format;
	texture->mipmaps = info.mipmaps;
	texture->type = info.type;

	Renderer::CreateTexture(texture, info.initialData);
}

Texture* Resources::LoadTexture(const String& name)
{
	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }

	TextureCreation info{};

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
	Sampler* sampler = &samplers.Request(info.name);

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

Buffer* Resources::CreateBuffer(const BufferCreation& info)
{
	Buffer* buffer = &buffers.Request(info.name);

	if (!buffer->name.Blank()) { return buffer; }

	buffer->name = info.name;
	buffer->size = info.size;
	buffer->typeFlags = info.typeFlags;
	buffer->usage = info.usage;
	buffer->globalOffset = 0;
	buffer->parentBuffer = nullptr;

	Renderer::CreateBuffer(buffer, info.initialData);

	return buffer;
}

DescriptorSetLayout* Resources::CreateDescriptorSetLayout(const DescriptorSetLayoutCreation& info)
{
	DescriptorSetLayout* descriptorSetLayout = &descriptorSetLayouts.Request(info.name);

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
	DescriptorSet* descriptorSet = &descriptorSets.Request(info.name);

	if (!descriptorSet->name.Blank()) { return descriptorSet; }

	return descriptorSet;
}

RenderPass* Resources::CreateRenderPass(const RenderPassCreation& info)
{
	RenderPass* renderPass = &renderPasses.Request(info.name);

	if (!renderPass->name.Blank()) { return renderPass; }

	return renderPass;
}

Pipeline* Resources::CreatePipeline(const PipelineCreation& info)
{
	Pipeline* pipeline = &pipelines.Request(info.name);

	if (!pipeline->name.Blank()) { return pipeline; }

	return pipeline;
}

Sampler* Resources::AccessDummySampler()
{
	return &dummySampler;
}

Texture* Resources::AccessDummyTexture()
{
	return &dummyTexture;
}

Buffer* Resources::AccessDummyAttributeBuffer()
{
	&dummyAttributeBuffer;
}

Sampler* Resources::AccessSampler(const String& name)
{

}

Texture* Resources::AccessTexture(const String& name)
{

}

DescriptorSetLayout* Resources::AccessDescriptorSetLayout(const String& name)
{

}

DescriptorSet* Resources::AccessDescriptorSet(const String& name)
{

}

ShaderState* Resources::AccessShaderState(const String& name)
{

}

RenderPass* Resources::AccessRenderPass(const String& name)
{

}

Pipeline* Resources::AccessPipeline(const String& name)
{

}

void Resources::DestroyBuffer(Buffer* buffer)
{

}

void Resources::DestroyTexture(Texture* texture)
{

}

void Resources::DestroyPipeline(Pipeline* pipeline)
{

}

void Resources::DestroySampler(Sampler* sampler)
{

}

void Resources::DestroyDescriptorSetLayout(DescriptorSetLayout* layout)
{

}

void Resources::DestroyDescriptorSet(DescriptorSet* set)
{

}

void Resources::DestroyRenderPass(RenderPass* renderPass)
{

}

void Resources::DestroyShaderState(ShaderState* shader)
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
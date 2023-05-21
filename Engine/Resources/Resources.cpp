#include "Resources.hpp"

#include "Rendering\Renderer.hpp"
#include "Core\Logger.hpp"
#include "Core\File.hpp"

#include "External/json.hpp"

#define TEXTURES_PATH "textures/"
#define AUDIO_PATH "audio/"
#define SHADERS_PATH "shaders/"
#define MATERIALS_PATH "materials/"
#define MODELS_PATH "models/"
#define FONTS_PATH "fonts/"
#define SCENES_PATH "scenes/"
#define BINARIES_PATH "scenes/"

#define BYTECAST(x) ((U8)((x) & 255))

#pragma pack(push, 1)

struct BMPHeader
{
	U16 signature;
	U32 fileSize;
	U16 reserved1;
	U16 reserved2;
	U32 imageOffset;
};

struct BMPInfo
{
	U32 infoSize;
	I32 imageWidth;
	I32 imageHeight;

	U16 imagePlanes;
	U16 imageBitCount;
	U32 imageCompression;

	U32 imageSize;
	I32 XPixelsPerMeter;
	I32 YPixelsPerMeter;

	U32 colorsUsed;
	U32 importantColor;

	U32 redMask;
	U32 greenMask;
	U32 blueMask;
	U32 alphaMask;

	U32 unused[17];

	U32 extraRead;
};

constexpr U64 size = sizeof(BMPInfo);

#pragma pack(pop)

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	resourceDeletionQueue.Reserve(16);

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

	while (resourceDeletionQueue.Size())
	{
		ResourceDeletion resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == -1) { continue; }

		switch (resourceDeletion.type)
		{
		case RESOURCE_DELETE_TYPE_SAMPLER: { Renderer::DestroySamplerInstant(&samplers.Obtain(resourceDeletion.handle)); samplers.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_TEXTURE: { Renderer::DestroyTextureInstant(&textures.Obtain(resourceDeletion.handle)); textures.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_BUFFER: { Renderer::DestroyBufferInstant(&buffers.Obtain(resourceDeletion.handle)); buffers.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT: { Renderer::DestroyDescriptorSetLayoutInstant(&descriptorSetLayouts.Obtain(resourceDeletion.handle)); descriptorSetLayouts.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET: { Renderer::DestroyDescriptorSetInstant(&descriptorSets.Obtain(resourceDeletion.handle)); descriptorSets.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_SHADER_STATE: { Renderer::DestroyShaderStateInstant(&shaders.Obtain(resourceDeletion.handle)); shaders.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_RENDER_PASS: { Renderer::DestroyRenderPassInstant(&renderPasses.Obtain(resourceDeletion.handle)); renderPasses.Remove(resourceDeletion.handle); } break;
		case RESOURCE_DELETE_TYPE_PIPELINE: { Renderer::DestroyPipelineInstant(&pipelines.Obtain(resourceDeletion.handle)); pipelines.Remove(resourceDeletion.handle); } break;
		}
	}

	pipelines.Destroy();
	buffers.Destroy();
	shaders.Destroy();
	textures.Destroy();
	samplers.Destroy();
	descriptorSetLayouts.Destroy();
	descriptorSets.Destroy();
	renderPasses.Destroy();

	resourceDeletionQueue.Destroy();
}

void Resources::Update()
{
	while (resourceDeletionQueue.Size())
	{
		ResourceDeletion resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == Renderer::currentFrame)
		{
			switch (resourceDeletion.type)
			{
			case RESOURCE_DELETE_TYPE_SAMPLER: { Renderer::DestroySamplerInstant(&samplers.Obtain(resourceDeletion.handle)); samplers.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_TEXTURE: { Renderer::DestroyTextureInstant(&textures.Obtain(resourceDeletion.handle)); textures.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_BUFFER: { Renderer::DestroyBufferInstant(&buffers.Obtain(resourceDeletion.handle)); buffers.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT: { Renderer::DestroyDescriptorSetLayoutInstant(&descriptorSetLayouts.Obtain(resourceDeletion.handle)); descriptorSetLayouts.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_DESCRIPTOR_SET: { Renderer::DestroyDescriptorSetInstant(&descriptorSets.Obtain(resourceDeletion.handle)); descriptorSets.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_SHADER_STATE: { Renderer::DestroyShaderStateInstant(&shaders.Obtain(resourceDeletion.handle)); shaders.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_RENDER_PASS: { Renderer::DestroyRenderPassInstant(&renderPasses.Obtain(resourceDeletion.handle)); renderPasses.Remove(resourceDeletion.handle); } break;
			case RESOURCE_DELETE_TYPE_PIPELINE: { Renderer::DestroyPipelineInstant(&pipelines.Obtain(resourceDeletion.handle)); pipelines.Remove(resourceDeletion.handle); } break;
			}
		}
	}
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

	return texture;
}

bool Resources::RecreateTexture(Texture* texture, U16 width, U16 height, U16 depth)
{
	Texture deleteTexture;
	deleteTexture.imageView = texture->imageView;
	deleteTexture.image = texture->image;
	deleteTexture.allocation = texture->allocation;

	texture->width = width;
	texture->height = height;
	texture->depth = depth;

	Renderer::CreateTexture(texture, nullptr);

	Renderer::DestroyTextureInstant(&deleteTexture);

	return true;
}

Texture* Resources::LoadTexture(const String& name)
{
	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }

	TextureCreation info{};

	String path;
	name.Prepended(path, TEXTURES_PATH);

	File file(path, FILE_OPEN_RESOURCE);
	if (file.Opened())
	{
		texture->name = name;
		texture->format = VK_FORMAT_R8G8B8A8_UNORM;
		texture->type = TEXTURE_TYPE_2D;
		texture->flags = 0;
		texture->mipmaps = 1;
		texture->depth = 1;

		String ext{};
		path.SubString(ext, path.LastIndexOf('.') + 1);
		ext.ToLower();

		void* data = nullptr;

		if (ext == "bmp") { data = LoadBMP(texture, file); }
		else if (ext == "png") { data = LoadPNG(texture, file); }
		else if (ext == "jpg" || ext == "jpeg") { data = LoadJPG(texture, file); }
		else if (ext == "psd") { data = LoadPSD(texture, file); }
		else if (ext == "tiff") { data = LoadTIFF(texture, file); }
		else if (ext == "tga") { data = LoadTGA(texture, file); }
		else { Logger::Error("Unknown Texture Extension {}!", ext); textures.Remove(name); return nullptr; }

		if (data == nullptr)
		{
			textures.Remove(name);
			return nullptr;
		}

		Renderer::CreateTexture(texture, data);

		free(data);
		file.Close();

		return texture;
	}

	Logger::Error("Failed to find or open file: {}", path);

	textures.Remove(name);
	return nullptr;
}

U32 HighBit(U32 z)
{
	U32 n = 0;
	if (z == 0) { return 0; }
	if (z >= 0x10000) { n += 16; z >>= 16; }
	if (z >= 0x00100) { n += 8; z >>= 8; }
	if (z >= 0x00010) { n += 4; z >>= 4; }
	if (z >= 0x00004) { n += 2; z >>= 2; }
	if (z >= 0x00002) { n += 1; }
	return n;
}

U32 BitCount(U32 a)
{
	a = (a & 0x55555555) + ((a >> 1) & 0x55555555);
	a = (a & 0x33333333) + ((a >> 2) & 0x33333333);
	a = (a + (a >> 4)) & 0x0f0f0f0f;
	a = (a + (a >> 8));
	a = (a + (a >> 16));
	return a & 0xff;
}

U32 ShiftSigned(U32 v, I32 shift, I32 bits)
{
	static U32 mulTable[9] = {
	   0,
	   0xff, 0x55, 0x49, 0x11,
	   0x21, 0x41, 0x81, 0x01,
	};

	static U32 shiftTable[9] = {
	   0, 0,0,1,0,2,4,6,0,
	};

	if (shift < 0) { v <<= -shift; }
	else { v >>= shift; }
	v >>= (8 - bits);

	return (v * mulTable[bits]) >> shiftTable[bits];
}

void* Resources::LoadBMP(Texture* texture, File& file)
{
	BMPHeader header{};
	BMPInfo info{};
	file.Read(header);

	if (header.signature != 0x4D42)
	{
		Logger::Error("Texture Is Not a BMP!");
		return nullptr;
	}

	file.Read(info.infoSize);
	info.extraRead = 14;
	U32 notRead = 12;

	if (info.infoSize == 12) { file.Read((I16&)info.imageWidth); file.Read((I16&)info.imageHeight); notRead = 8; }
	else { file.Read(info.imageWidth); file.Read(info.imageHeight); }

	file.Read(&info.imagePlanes, info.infoSize - notRead);

	if (info.imagePlanes != 1) { Logger::Error("Invalid BMP!"); return nullptr; }

	if (info.imageCompression != 0 && (info.imageCompression != 3 || (info.imageBitCount != 16 && info.imageBitCount != 32)))
	{
		Logger::Error("RLE Compressed BMPs Not Yet Supported!");
		return nullptr;
	}

	texture->width = info.imageWidth;
	texture->height = info.imageHeight;

	U32 pSize = 0;
	I32 width;
	I32 pad;

	if (info.infoSize == 12 && info.imageBitCount < 24) { pSize = (header.imageOffset - info.extraRead - 24) / 3; }
	else if (info.imageBitCount < 16) { pSize = (header.imageOffset - info.extraRead - info.infoSize) >> 2; }

	U8* data = (U8*)malloc(info.imageWidth * info.imageHeight * 4); //TODO: Go through Memory

	if (info.imageBitCount < 16)
	{
		if (pSize == 0 || pSize > 256)
		{
			Logger::Error("Invalid BMP!");
			free(data);
			return nullptr;
		}

		U8* palette;
		Memory::AllocateSize(&palette, pSize);

		if (info.infoSize != 12)
		{
			for (U32 i = 0; i < pSize; ++i)
			{
				file.Read(palette[i]);
				file.Read(palette[++i]);
				file.Read(palette[++i]);
				file.Seek(1);
			}
		}
		else
		{
			for (U32 i = 0; i < pSize; ++i)
			{
				file.Read(palette[i]);
				file.Read(palette[++i]);
				file.Read(palette[++i]);
			}
		}

		file.SeekFromStart(header.imageOffset);

		if (info.imageBitCount == 1) { width = (info.imageWidth + 7) >> 3; }
		else if (info.imageBitCount == 4) { width = (info.imageWidth + 1) >> 1; }
		else if (info.imageBitCount == 8) { width = info.imageWidth; }
		else
		{
			Logger::Error("Invalid BMP!");
			free(data);
			Memory::FreeSize(&palette);
			return nullptr;
		}

		pad = (-width) & 3;

		switch (info.imageBitCount)
		{
		case 1:
		{
			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				I8 bitOffset = 7;
				U8 v;
				file.Read(v);

				I32 height = info.imageHeight * j;

				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					U8 index = (v >> bitOffset) & 0x1;
					data[height + i] = palette[index * 3];
					data[height + ++i] = palette[index * 3 + 1];
					data[height + ++i] = palette[index * 3 + 2];
					data[height + ++i] = 255;
					if ((--bitOffset) < 0 && i + 1 != info.imageWidth)
					{
						bitOffset = 7;
						file.Read(v);
					}
				}
				file.Seek(pad);
			}
		} break;
		case 4:
		{
			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				I32 height = info.imageHeight * j;

				for (I32 i = 0; i < info.imageWidth; i += 2)
				{
					U8 index0;
					file.Read(index0);
					U8 index1 = index0 & 15;
					index0 >>= 4;
					data[height + i] = palette[index0 * 3];
					data[height + ++i] = palette[index0 * 3 + 1];
					data[height + ++i] = palette[index0 * 3 + 2];
					data[height + ++i] = 255;
					if (i + 1 >= info.imageWidth) { break; }
					data[height + ++i] = palette[index1 * 3];
					data[height + ++i] = palette[index1 * 3 + 1];
					data[height + ++i] = palette[index1 * 3 + 2];
					data[height + ++i] = 255;
				}
				file.Seek(pad);
			}
		} break;
		case 8:
		{
			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				I32 height = info.imageHeight * j;

				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					U8 v;
					file.Read(v);
					data[height + i] = palette[v * 3];
					data[height + ++i] = palette[v * 3 + 1];
					data[height + ++i] = palette[v * 3 + 2];
					data[height + ++i] = 255;
				}
				file.Seek(pad);
			}
		} break;
		}

		Memory::FreeSize(&palette);
	}
	else
	{
		int rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0, acount = 0;
		U8 easy = 0;

		file.SeekFromStart(header.imageOffset);

		if (info.imageBitCount == 24) { width = 3 * info.imageWidth; }
		else if (info.imageBitCount == 16) { width = 2 * info.imageWidth; }
		else { width = 0; }

		pad = (-width) & 3;

		if (info.imageBitCount == 24) { easy = 1; }
		else if (info.imageBitCount == 32 && info.blueMask == 0xff && info.greenMask == 0xff00 && info.redMask == 0x00ff0000 && info.alphaMask == 0xff000000) { easy = 2; }

		if (!easy)
		{
			if (!info.redMask || !info.greenMask || !info.blueMask)
			{
				Logger::Error("Invalid BMP!");
				free(data);
				return nullptr;
			}

			rshift = HighBit(info.redMask) - 7;
			gshift = HighBit(info.greenMask) - 7;
			bshift = HighBit(info.blueMask) - 7;
			ashift = HighBit(info.alphaMask) - 7;

			rcount = BitCount(info.redMask);
			gcount = BitCount(info.greenMask);
			bcount = BitCount(info.blueMask);
			acount = BitCount(info.alphaMask);

			if (rcount > 8 || gcount > 8 || bcount > 8 || acount > 8)
			{
				Logger::Error("Invalid BMP!");
				free(data);
				return nullptr;
			}
		}

		for (I32 j = 0; j < info.imageHeight; ++j)
		{
			if (easy)
			{
				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					U8 red;
					U8 green;
					U8 blue;
					U8 alpha;
					file.Read(blue);
					file.Read(green);
					file.Read(red);
					if (easy == 2) { file.Read(alpha); }
					else { alpha = 255; }
					data[i] = red;
					data[++i] = green;
					data[++i] = blue;
					data[++i] = alpha;
				}
			}
			else
			{
				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					U32 v;
					info.imageBitCount == 16 ? file.Read((U16&)v) : file.Read(v);
					U32 alpha;
					data[i] = BYTECAST(ShiftSigned(v & info.redMask, rshift, rcount));
					data[++i] = BYTECAST(ShiftSigned(v & info.greenMask, gshift, gcount));
					data[++i] = BYTECAST(ShiftSigned(v & info.blueMask, bshift, bcount));
					alpha = (info.alphaMask ? ShiftSigned(v & info.alphaMask, ashift, acount) : 255);
					data[++i] = BYTECAST(alpha);
				}
			}

			file.Seek(pad);
		}
	}

	return data;
}

void* Resources::LoadPNG(Texture* texture, File& file)
{
	Logger::Error("PNG Texture Format Not Yet Supported!");

	return nullptr;
}

void* Resources::LoadJPG(Texture* texture, File& file)
{
	Logger::Error("JPG Texture Format Not Yet Supported!");

	return nullptr;
}

void* Resources::LoadPSD(Texture* texture, File& file)
{
	Logger::Error("PSD Texture Format Not Yet Supported!");

	return nullptr;
}

void* Resources::LoadTIFF(Texture* texture, File& file)
{
	Logger::Error("TIFF Texture Format Not Yet Supported!");

	return nullptr;
}

void* Resources::LoadTGA(Texture* texture, File& file)
{
	Logger::Error("TGA Texture Format Not Yet Supported!");

	return nullptr;
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
	descriptorSetLayout->bindingCount = (U16)info.bindingCount;
	U8* memory;
	Memory::AllocateSize(&memory, (sizeof(VkDescriptorSetLayoutBinding) + sizeof(DescriptorBinding)) * info.bindingCount);
	descriptorSetLayout->bindings = (DescriptorBinding*)memory;
	descriptorSetLayout->binding = (VkDescriptorSetLayoutBinding*)(memory + sizeof(DescriptorBinding) * info.bindingCount);
	descriptorSetLayout->setIndex = (U16)info.setIndex;

	for (U32 i = 0; i < info.bindingCount; ++i)
	{
		descriptorSetLayout->bindings[i] = info.bindings[i];
	}

	Renderer::CreateDescriptorSetLayout(descriptorSetLayout);

	return descriptorSetLayout;
}

DescriptorSet* Resources::CreateDescriptorSet(const DescriptorSetCreation& info)
{
	DescriptorSet* descriptorSet = &descriptorSets.Request(info.name);

	if (!descriptorSet->name.Blank()) { return descriptorSet; }

	descriptorSet->name = info.name;
	descriptorSet->resourceCount = info.resourceCount;
	descriptorSet->layout = info.layout;

	U8* memory;
	Memory::AllocateSize(&memory, (sizeof(void*) + sizeof(Sampler*) + sizeof(U16)) * info.resourceCount);
	descriptorSet->resources = (void**)memory;
	descriptorSet->samplers = (Sampler**)(memory + sizeof(void*) * info.resourceCount);
	descriptorSet->bindings = (U16*)(memory + (sizeof(void*) + sizeof(Sampler*)) * info.resourceCount);

	for (U32 i = 0; i < info.resourceCount; ++i)
	{
		descriptorSet->resources[i] = info.resources[i];
		descriptorSet->samplers[i] = info.samplers[i];
		descriptorSet->bindings[i] = info.bindings[i];
	}

	Renderer::CreateDescriptorSet(descriptorSet);

	return descriptorSet;
}

ShaderState* Resources::CreateShaderState(const ShaderStateCreation& info)
{
	ShaderState* shaderState = &shaders.Request(info.name);

	if (!shaderState->name.Blank()) { return shaderState; }

	if (info.stagesCount == 0 || info.stages == nullptr)
	{
		Logger::Error("Shader {} does not contain shader stages!", info.name);
		return shaderState;
	}

	if (!Renderer::CreateShaderState(shaderState, info))
	{
		shaders.Remove(info.name);

		return nullptr;
	}

	return shaderState;
}

RenderPass* Resources::CreateRenderPass(const RenderPassCreation& info)
{
	RenderPass* renderPass = &renderPasses.Request(info.name);

	if (!renderPass->name.Blank()) { return renderPass; }

	renderPass->name = info.name;
	renderPass->type = info.type;
	renderPass->dispatchX = 0;
	renderPass->dispatchY = 0;
	renderPass->dispatchZ = 0;
	renderPass->frameBuffer = nullptr;
	renderPass->renderPass = nullptr;
	renderPass->scaleX = info.scaleX;
	renderPass->scaleY = info.scaleY;
	renderPass->resize = info.resize;
	renderPass->renderTargetCount = (U8)info.renderTargetCount;
	renderPass->outputDepth = info.depthStencilTexture;

	for (U32 i = 0; i < info.renderTargetCount; ++i)
	{
		Texture* texture = info.outputTextures[i];

		renderPass->width = texture->width;
		renderPass->height = texture->height;
		renderPass->outputTextures[i] = texture;
	}

	Renderer::CreateRenderPass(renderPass, info.colorOperation, info.depthOperation, info.stencilOperation);

	return renderPass;
}

Pipeline* Resources::CreatePipeline(const PipelineCreation& info)
{
	Pipeline* pipeline = &pipelines.Request(info.name);

	if (!pipeline->name.Blank()) { return pipeline; }

	pipeline->name = info.name;
	pipeline->activeLayoutCount = info.activeLayoutCount;
	pipeline->shaderState = CreateShaderState(info.shaders);
	pipeline->depthStencil = info.depthStencil;
	pipeline->blendState = info.blendState;
	pipeline->rasterization = info.rasterization;
	pipeline->graphicsPipeline = true;

	for (U32 i = 0; i < info.activeLayoutCount; ++i)
	{
		pipeline->descriptorSetLayouts[i] = info.descriptorSetLayouts[i];
	}

	Renderer::CreatePipeline(pipeline, info.renderPass, info.vertexInput);

	return pipeline;
}

glTF* Resources::LoadScene(const String& name)
{
	using json = nlohmann::json;

	glTF* gltf = &scenes.Request(name);

	if (!gltf->name.Blank()) { return gltf; }

	String path{};
	name.Prepended(path, SCENES_PATH);

	File file(path, FILE_OPEN_RESOURCE);
	if (file.Opened())
	{
		gltf->name = name;

		//C8* data;
		//file.ReadAll(&data);
		//
		//json gltfData = json::parse(data);
		//
		//for (auto properties : gltfData.items())
		//{
		//	if (properties.key() == "asset")
		//	{
		//		json& jsonAsset = gltfData["asset"];
		//
		//		gltf->asset.copyright = jsonAsset.value("copyright", "").c_str();
		//		gltf->asset.generator = jsonAsset.value("generator", "").c_str();
		//		gltf->asset.minVersion = jsonAsset.value("minVersion", "").c_str();
		//		gltf->asset.version = jsonAsset.value("version", "").c_str();
		//	}
		//	else if (properties.key() == "scene")
		//	{
		//		gltf->scene = gltfData.value("version", I32_MAX);
		//	}
		//	else if (properties.key() == "scenes")
		//	{
		//		json& scenes = gltfData["scenes"];
		//
		//		U64 sceneCount = scenes.size();
		//		gltf->sceneCount = sceneCount;
		//		Memory::AllocateArray(&gltf->scenes, sceneCount);
		//
		//		for (U64 i = 0; i < sceneCount; ++i)
		//		{
		//			Scene& scene = gltf->scenes[i];
		//			json& data = scenes[i];
		//
		//			json& jsonArray = data.at("nodes");
		//
		//			scene.nodeCount = jsonArray.size();
		//
		//			if (scene.nodeCount)
		//			{
		//				Memory::AllocateArray(&scene.nodes, scene.nodeCount);
		//
		//				for (U64 i = 0; i < scene.nodeCount; ++i) { scene.nodes[i] = jsonArray.at(i); }
		//			}
		//		}
		//	}
		//	else if (properties.key() == "buffers")
		//	{
		//		json& buffers = gltfData["buffers"];
		//
		//		U64 bufferCount = buffers.size();
		//		Memory::AllocateArray(&gltf->buffers, bufferCount);
		//		gltf->bufferCount = bufferCount;
		//
		//		for (U64 i = 0; i < bufferCount; ++i)
		//		{
		//			json& data = buffers[i];
		//			BufferRef& buffer = gltf->buffers[i];
		//
		//			buffer.uri = data.value("uri", "").c_str();
		//			buffer.byteLength = data.value("byteLength", I32_MAX);
		//			buffer.name = data.value("name", "").c_str();
		//		}
		//	}
		//	else if (properties.key() == "bufferViews")
		//	{
		//		json& buffers = gltfData["bufferViews"];
		//
		//		U64 bufferCount = buffers.size();
		//		Memory::AllocateArray(&gltf->bufferViews, bufferCount);
		//		gltf->bufferViewCount = bufferCount;
		//
		//		for (U64 i = 0; i < bufferCount; ++i)
		//		{
		//			json& data = buffers[i];
		//			BufferView& view = gltf->bufferViews[i];
		//
		//			view.buffer = data.value("buffer", I32_MAX);
		//			view.byteLength = data.value("byteLength", I32_MAX);
		//			view.byteOffset = data.value("byteOffset", I32_MAX);
		//			view.byteStride = data.value("byteStride", I32_MAX);
		//			view.target = data.value("target", I32_MAX);
		//			view.name = data.value("name", "").c_str();
		//		}
		//	}
		//	else if (properties.key() == "nodes")
		//	{
		//		json& array = gltfData["nodes"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->nodes, arrayCount);
		//		gltf->nodeCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Node& node = gltf->nodes[i];
		//
		//			node.camera = data.value("camera", I32_MAX);
		//			node.mesh = data.value("mesh", I32_MAX);
		//			node.skin = data.value("skin", I32_MAX);
		//			node.name = data.value("name", "").c_str();
		//
		//			{
		//				json& jsonArray = data.at("children");
		//
		//				node.childrenCount = jsonArray.size();
		//
		//				if (node.childrenCount)
		//				{
		//					Memory::AllocateArray(&node.children, node.childrenCount);
		//
		//					for (U64 i = 0; i < node.childrenCount; ++i) { node.children[i] = jsonArray.at(i); }
		//				}
		//			}
		//			{
		//				json& jsonArray = data.at("matrix");
		//
		//				node.matrixCount = jsonArray.size();
		//
		//				if (node.matrixCount)
		//				{
		//					Memory::AllocateArray(&node.matrix, node.matrixCount);
		//
		//					for (U64 i = 0; i < node.matrixCount; ++i) { node.matrix[i] = jsonArray.at(i); }
		//				}
		//			}
		//			{
		//				json& jsonArray = data.at("rotation");
		//
		//				node.rotationCount = jsonArray.size();
		//
		//				if (node.rotationCount)
		//				{
		//					Memory::AllocateArray(&node.rotation, node.rotationCount);
		//
		//					for (U64 i = 0; i < node.rotationCount; ++i) { node.rotation[i] = jsonArray.at(i); }
		//				}
		//			}
		//			{
		//				json& jsonArray = data.at("scale");
		//
		//				node.scaleCount = jsonArray.size();
		//
		//				if (node.scaleCount)
		//				{
		//					Memory::AllocateArray(&node.scale, node.scaleCount);
		//
		//					for (U64 i = 0; i < node.scaleCount; ++i) { node.scale[i] = jsonArray.at(i); }
		//				}
		//			}
		//			{
		//				json& jsonArray = data.at("translation");
		//
		//				node.translationCount = jsonArray.size();
		//
		//				if (node.translationCount)
		//				{
		//					Memory::AllocateArray(&node.translation, node.translationCount);
		//
		//					for (U64 i = 0; i < node.translationCount; ++i) { node.translation[i] = jsonArray.at(i); }
		//				}
		//			}
		//			{
		//				json& jsonArray = data.at("weights");
		//
		//				node.weightCount = jsonArray.size();
		//
		//				if (node.weightCount)
		//				{
		//					Memory::AllocateArray(&node.weights, node.weightCount);
		//
		//					for (U64 i = 0; i < node.weightCount; ++i) { node.weights[i] = jsonArray.at(i); }
		//				}
		//			}
		//		}
		//	}
		//	else if (properties.key() == "meshes")
		//	{
		//		json& array = gltfData["meshes"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->meshes, arrayCount);
		//		gltf->meshCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Mesh& mesh = gltf->meshes[i];
		//
		//			{
		//				json& array = data["primitives"];
		//
		//				U64 arrayCount = array.size();
		//				Memory::AllocateArray(&mesh.primitives, arrayCount);
		//				mesh.primitiveCount = arrayCount;
		//
		//				for (U64 i = 0; i < arrayCount; ++i)
		//				{
		//					json& data = array[i];
		//					MeshPrimitive& primitive = mesh.primitives[i];
		//
		//					primitive.indices = data.value("indices", I32_MAX);
		//					primitive.material = data.value("material", I32_MAX);
		//					primitive.mode = data.value("mode", I32_MAX);
		//
		//					json& attributes = data["attributes"];
		//
		//					Memory::AllocateArray(&primitive.attributes, attributes.size());
		//					primitive.attributeCount = attributes.size();
		//
		//					U32 index = 0;
		//					for (auto jsonAttribute : attributes.items())
		//					{
		//						MeshPrimitive::Attribute& attribute = primitive.attributes[index];
		//
		//						attribute.key = jsonAttribute.key().c_str();
		//						attribute.accessorIndex = jsonAttribute.value();
		//
		//						++index;
		//					}
		//				}
		//			}
		//
		//			mesh.name = data.value("name", "").c_str();
		//
		//			{
		//				json& jsonArray = data.at("weights");
		//
		//				mesh.weightCount = jsonArray.size();
		//
		//				if (mesh.weightCount)
		//				{
		//					Memory::AllocateArray(&mesh.weights, mesh.weightCount);
		//
		//					for (U64 i = 0; i < mesh.weightCount; ++i) { mesh.weights[i] = jsonArray.at(i); }
		//				}
		//			}
		//		}
		//	}
		//	else if (properties.key() == "accessors")
		//	{
		//		json& array = gltfData["accessors"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->accessors, arrayCount);
		//		gltf->accessorCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Accessor& accessor = gltf->accessors[i];
		//
		//			accessor.bufferView = data.value("bufferView", I32_MAX);
		//			accessor.byteOffset = data.value("byteOffset", I32_MAX);
		//			accessor.componentType = data.value("componentType", I32_MAX);
		//			accessor.count = data.value("count", I32_MAX);
		//			accessor.sparse = data.value("sparse", I32_MAX);
		//
		//			{
		//				json& jsonArray = data.at("max");
		//
		//				accessor.maxCount = jsonArray.size();
		//
		//				if (accessor.maxCount)
		//				{
		//					Memory::AllocateArray(&accessor.max, accessor.maxCount);
		//
		//					for (U64 i = 0; i < accessor.maxCount; ++i) { accessor.max[i] = jsonArray.at(i); }
		//				}
		//			}
		//
		//			{
		//				json& jsonArray = data.at("min");
		//
		//				accessor.minCount = jsonArray.size();
		//
		//				if (accessor.minCount)
		//				{
		//					Memory::AllocateArray(&accessor.min, accessor.minCount);
		//
		//					for (U64 i = 0; i < accessor.minCount; ++i) { accessor.min[i] = jsonArray.at(i); }
		//				}
		//			}
		//
		//			accessor.normalized = data.value("normalized", false);
		//
		//			std::string value = data.value("type", "").c_str();
		//			if (value == "SCALAR") { accessor.type = Accessor::Scalar; }
		//			else if (value == "VEC2") { accessor.type = Accessor::Vec2; }
		//			else if (value == "VEC3") { accessor.type = Accessor::Vec3; }
		//			else if (value == "VEC4") { accessor.type = Accessor::Vec4; }
		//			else if (value == "MAT2") { accessor.type = Accessor::Mat2; }
		//			else if (value == "MAT3") { accessor.type = Accessor::Mat3; }
		//			else if (value == "MAT4") { accessor.type = Accessor::Mat4; }
		//			else { ASSERT(false); }
		//		}
		//	}
		//	else if (properties.key() == "materials")
		//	{
		//		json& array = gltfData["materials"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->materials, arrayCount);
		//		gltf->materialCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Material& material = gltf->materials[i];
		//
		//			{
		//				json& jsonArray = data.at("emissiveFactor");
		//
		//				material.emissiveFactorCount = jsonArray.size();
		//
		//				if (material.emissiveFactorCount)
		//				{
		//					Memory::AllocateArray(&material.emissiveFactor, material.emissiveFactorCount);
		//
		//					for (U64 i = 0; i < material.emissiveFactorCount; ++i) { material.emissiveFactor[i] = jsonArray.at(i); }
		//				}
		//			}
		//
		//			material.alphaCutoff = data.value("alphaCutoff", F32_MAX);
		//			material.alphaMode = data.value("alphaMode", "").c_str();
		//			material.doubleSided = data.value("doubleSided", false);
		//
		//			{
		//				auto it = data.find("emissiveTexture");
		//
		//				material.emissiveTexture.index = it->value("index", I32_MAX);
		//				material.emissiveTexture.texCoord = it->value("texCoord", I32_MAX);
		//			}
		//
		//			{
		//				auto it = data.find("normalTexture");
		//
		//				material.normalTexture.index = it->value("index", I32_MAX);
		//				material.normalTexture.texCoord = it->value("texCoord", I32_MAX);
		//				material.normalTexture.scale = it->value("scale", F32_MAX);
		//			}
		//
		//			{
		//				auto it = data.find("occlusionTexture");
		//
		//				material.occlusionTexture.index = it->value("index", I32_MAX);
		//				material.occlusionTexture.texCoord = it->value("texCoord", I32_MAX);
		//				material.occlusionTexture.strength = it->value("strength", F32_MAX);
		//			}
		//
		//			{
		//				auto it = data.find("pbrMetallicRoughness");
		//
		//
		//				{
		//					json& jsonArray = it->at("baseColorFactor");
		//
		//					material.pbrMetallicRoughness.baseColorFactorCount = jsonArray.size();
		//
		//					if (material.pbrMetallicRoughness.baseColorFactorCount)
		//					{
		//						Memory::AllocateArray(&material.pbrMetallicRoughness.baseColorFactor, material.pbrMetallicRoughness.baseColorFactorCount);
		//
		//						for (U64 i = 0; i < material.pbrMetallicRoughness.baseColorFactorCount; ++i) { material.pbrMetallicRoughness.baseColorFactor[i] = jsonArray.at(i); }
		//					}
		//				}
		//
		//				{
		//					auto it2 = it->find("baseColorTexture");
		//
		//					material.pbrMetallicRoughness.baseColorTexture.index = it2->value("index", I32_MAX);
		//					material.pbrMetallicRoughness.baseColorTexture.texCoord = it2->value("texCoord", I32_MAX);
		//				}
		//
		//				material.pbrMetallicRoughness.metallicFactor = it->value("metallicFactor", F32_MAX);
		//
		//				{
		//					auto it2 = it->find("metallicRoughnessTexture");
		//
		//					material.pbrMetallicRoughness.metallicRoughnessTexture.index = it2->value("index", I32_MAX);
		//					material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord = it2->value("texCoord", I32_MAX);
		//				}
		//
		//				material.pbrMetallicRoughness.roughnessFactor = it->value("roughnessFactor", F32_MAX);
		//			}
		//
		//			material.name = data.value("name", "").c_str();
		//		}
		//	}
		//	else if (properties.key() == "textures")
		//	{
		//		json& array = gltfData["textures"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->textures, arrayCount);
		//		gltf->textureCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			TextureRef& texture = gltf->textures[i];
		//
		//			texture.sampler = data.value("sampler", I32_MAX);
		//			texture.source = data.value("source", I32_MAX);
		//			texture.name = data.value("name", "").c_str();
		//		}
		//	}
		//	else if (properties.key() == "images")
		//	{
		//		json array = gltfData["images"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->images, arrayCount);
		//		gltf->imageCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Image& image = gltf->images[i];
		//
		//			image.bufferView = data.value("bufferView", I32_MAX);
		//			image.mimeType = data.value("mimeType", "").c_str();
		//			image.uri = data.value("uri", "").c_str();
		//		}
		//	}
		//	else if (properties.key() == "samplers")
		//	{
		//		json array = gltfData["samplers"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->samplers, arrayCount);
		//		gltf->samplerCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			SamplerRef& sampler = gltf->samplers[i];
		//
		//			sampler.magFilter = data.value("magFilter", I32_MAX);
		//			sampler.minFilter = data.value("minFilter", I32_MAX);
		//			sampler.wrapS = data.value("wrapS", I32_MAX);
		//			sampler.wrapT = data.value("wrapT", I32_MAX);
		//		}
		//	}
		//	else if (properties.key() == "skins")
		//	{
		//		json array = gltfData["skins"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->skins, arrayCount);
		//		gltf->skinCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Skin& skin = gltf->skins[i];
		//
		//			skin.skeletonRootNodeIndex = data.value("skeleton", I32_MAX);
		//			skin.inverseBindMatricesBufferIndex = data.value("inverseBindMatrices", I32_MAX);
		//
		//			{
		//				json& jsonArray = data.at("baseColorFactor");
		//
		//				skin.jointCount = jsonArray.size();
		//
		//				if (skin.jointCount)
		//				{
		//					Memory::AllocateArray(&skin.joints, skin.jointCount);
		//
		//					for (U64 i = 0; i < skin.jointCount; ++i) { skin.joints[i] = jsonArray.at(i); }
		//				}
		//			}
		//		}
		//	}
		//	else if (properties.key() == "animations")
		//	{
		//		json array = gltfData["animations"];
		//
		//		U64 arrayCount = array.size();
		//		Memory::AllocateArray(&gltf->animations, arrayCount);
		//		gltf->animationCount = arrayCount;
		//
		//		for (U64 i = 0; i < arrayCount; ++i)
		//		{
		//			json& data = array[i];
		//			Animation& animation = gltf->animations[i];
		//
		//			json& jsonArray = data.at("samplers");
		//			if (jsonArray.is_array())
		//			{
		//				U64 count = jsonArray.size();
		//
		//				AnimationSampler* values;
		//				Memory::AllocateArray(&values, count);
		//
		//				for (U64 i = 0; i < count; ++i)
		//				{
		//					json& element = jsonArray.at(i);
		//					AnimationSampler& sampler = values[i];
		//
		//					sampler.inputKeyframeBufferIndex = element.value("input", I32_MAX);
		//					sampler.outputKeyframeBufferIndex = element.value("output", I32_MAX);
		//
		//					String value = element.value("interpolation", "").c_str();
		//					if (value == "LINEAR") { sampler.interpolation = AnimationSampler::Linear; }
		//					else if (value == "STEP") { sampler.interpolation = AnimationSampler::Step; }
		//					else if (value == "CUBICSPLINE") { sampler.interpolation = AnimationSampler::CubicSpline; }
		//					else { sampler.interpolation = AnimationSampler::Linear; }
		//				}
		//
		//				animation.samplers = values;
		//				animation.samplerCount = count;
		//			}
		//
		//			jsonArray = data.at("channels");
		//			if (jsonArray.is_array())
		//			{
		//				U64 count = jsonArray.size();
		//
		//				AnimationChannel* values;
		//				Memory::AllocateArray(&values, count);
		//
		//				for (U64 i = 0; i < count; ++i)
		//				{
		//					json& element = jsonArray.at(i);
		//					AnimationChannel& channel = values[i];
		//
		//					channel.sampler = element.value("sampler", I32_MAX);
		//
		//					json& target = element.at("target");
		//					channel.targetNode = target.value("node", I32_MAX);
		//
		//					String targetPath = target.value("path", "").c_str();
		//					if (targetPath == "scale")
		//					{
		//						channel.targetType = AnimationChannel::Scale;
		//					}
		//					else if (targetPath == "rotation")
		//					{
		//						channel.targetType = AnimationChannel::Rotation;
		//					}
		//					else if (targetPath == "translation")
		//					{
		//						channel.targetType = AnimationChannel::Translation;
		//					}
		//					else if (targetPath == "weights")
		//					{
		//						channel.targetType = AnimationChannel::Weights;
		//					}
		//					else
		//					{
		//						Logger::Fatal("Error parsing target path {}", targetPath);
		//						channel.targetType = AnimationChannel::Count;
		//					}
		//				}
		//
		//				animation.channels = values;
		//				animation.channelCount = count;
		//			}
		//		}
		//	}
		//}

		return gltf;
	}

	Logger::Error("Failed to find or open file: {}", path);
	scenes.Remove(name);

	return nullptr;
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
	return &dummyAttributeBuffer;
}

Sampler* Resources::AccessSampler(const String& name)
{
	Sampler* sampler = &samplers.Request(name);

	if (!sampler->name.Blank()) { return sampler; }

	return nullptr;
}

Texture* Resources::AccessTexture(const String& name)
{
	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }

	return nullptr;
}

DescriptorSetLayout* Resources::AccessDescriptorSetLayout(const String& name)
{
	DescriptorSetLayout* descriptorSetLayout = &descriptorSetLayouts.Request(name);

	if (!descriptorSetLayout->name.Blank()) { return descriptorSetLayout; }

	return nullptr;
}

DescriptorSet* Resources::AccessDescriptorSet(const String& name)
{
	DescriptorSet* descriptorSet = &descriptorSets.Request(name);

	if (!descriptorSet->name.Blank()) { return descriptorSet; }

	return nullptr;
}

ShaderState* Resources::AccessShaderState(const String& name)
{
	ShaderState* shaderState = &shaders.Request(name);

	if (!shaderState->name.Blank()) { return shaderState; }

	return nullptr;
}

RenderPass* Resources::AccessRenderPass(const String& name)
{
	RenderPass* renderPass = &renderPasses.Request(name);

	if (!renderPass->name.Blank()) { return renderPass; }

	return nullptr;
}

Pipeline* Resources::AccessPipeline(const String& name)
{
	Pipeline* pipeline = &pipelines.Request(name);

	if (!pipeline->name.Blank()) { return pipeline; }

	return nullptr;
}

void Resources::DestroySampler(Sampler* sampler)
{
	HashHandle handle = samplers.GetHandle(sampler->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_SAMPLER;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", sampler->name);
	}
}

void Resources::DestroyTexture(Texture* texture)
{
	HashHandle handle = textures.GetHandle(texture->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_TEXTURE;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", texture->name);
	}
}

void Resources::DestroyBuffer(Buffer* buffer)
{
	HashHandle handle = buffers.GetHandle(buffer->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_BUFFER;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", buffer->name);
	}
}

void Resources::DestroyDescriptorSetLayout(DescriptorSetLayout* layout)
{
	HashHandle handle = descriptorSetLayouts.GetHandle(layout->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_DESCRIPTOR_SET_LAYOUT;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", layout->name);
	}
}

void Resources::DestroyDescriptorSet(DescriptorSet* set)
{
	HashHandle handle = descriptorSets.GetHandle(set->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_DESCRIPTOR_SET;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", set->name);
	}
}

void Resources::DestroyShaderState(ShaderState* shader)
{
	HashHandle handle = shaders.GetHandle(shader->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_SHADER_STATE;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", shader->name);
	}
}

void Resources::DestroyRenderPass(RenderPass* renderPass)
{
	HashHandle handle = renderPasses.GetHandle(renderPass->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_RENDER_PASS;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", renderPass->name);
	}
}

void Resources::DestroyPipeline(Pipeline* pipeline)
{
	HashHandle handle = pipelines.GetHandle(pipeline->name);

	if (handle != U64_MAX)
	{
		ResourceDeletion deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_DELETE_TYPE_PIPELINE;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);

		DestroyShaderState(pipeline->shaderState);
	}
	else
	{
		Logger::Error("Resource '{}' doesn't exist!", pipeline->name);
	}
}

bool Resources::LoadBinary(const String& name, String& result)
{
	String path;
	name.Prepended(path, BINARIES_PATH);

	File file{ path, FILE_OPEN_RESOURCE };

	if (file.Opened())
	{
		file.ReadAll(result);
		file.Close();

		return true;
	}

	return false;
}

U32 Resources::LoadBinary(const String& name, void** result)
{
	String path;
	name.Prepended(path, BINARIES_PATH);

	File file{ path, FILE_OPEN_RESOURCE };

	if (file.Opened())
	{
		U32 read = file.ReadAll(result);
		file.Close();

		return read;
	}

	return 0;
}
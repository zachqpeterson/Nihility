#include "Resources.hpp"

#include "Rendering\Renderer.hpp"
#include "Core\Logger.hpp"
#include "Core\File.hpp"
#include "Math\Color.hpp"

#define TEXTURES_PATH "textures/"
#define AUDIO_PATH "audio/"
#define SHADERS_PATH "shaders/"
#define MATERIALS_PATH "materials/"
#define MODELS_PATH "models/"
#define FONTS_PATH "fonts/"
#define SCENES_PATH "scenes/"
#define BINARIES_PATH "binaries/"

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

Sampler* Resources::dummySampler;
Texture* Resources::dummyTexture;
Buffer* Resources::dummyAttributeBuffer;
Sampler* Resources::defaultSampler;

Hashmap<String, Sampler>				Resources::samplers{ 32, {} };
Hashmap<String, Texture>				Resources::textures{ 512, {} };
Hashmap<String, Buffer>					Resources::buffers{ 4096, {} };
Hashmap<String, DescriptorSetLayout>	Resources::descriptorSetLayouts{ 128, {} };
Hashmap<String, DescriptorSet>			Resources::descriptorSets{ 256, {} };
Hashmap<String, ShaderState>			Resources::shaders{ 128, {} };
Hashmap<String, RenderPass>				Resources::renderPasses{ 256, {} };
Hashmap<String, Pipeline>				Resources::pipelines{ 128, {} };
Hashmap<String, Program>				Resources::programs{ 128, {} };
Hashmap<String, Material>				Resources::materials{ 128, {} };
Hashmap<String, Scene>					Resources::scenes{ 128, {} };

Queue<ResourceUpdate>					Resources::resourceDeletionQueue{};

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	resourceDeletionQueue.Reserve(16);

	TextureCreation dummyTextureInfo{};
	dummyTextureInfo.SetName("dummy_texture");
	dummyTextureInfo.SetFlags(1, 0);
	dummyTextureInfo.SetFormatType(VK_FORMAT_R8G8B8A8_UNORM, TEXTURE_TYPE_2D);
	dummyTextureInfo.SetSize(1, 1, 1);
	U32 zero = 0;
	dummyTextureInfo.SetData(&zero);
	dummyTexture = Resources::CreateTexture(dummyTextureInfo);

	SamplerCreation dummySamplerInfo{};
	dummySamplerInfo.SetName("dummy_sampler");
	dummySamplerInfo.SetAddressModeUV(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	dummySamplerInfo.SetMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST);
	dummySampler = Resources::CreateSampler(dummySamplerInfo);

	BufferCreation dummyAttributeBufferInfo{};
	dummyAttributeBufferInfo.SetName("dummy_attribute_buffer");
	dummyAttributeBufferInfo.Set(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, RESOURCE_USAGE_IMMUTABLE, sizeof(Vector4) * 3);
	Vector4 dummyData[3]{};
	dummyAttributeBufferInfo.SetData(dummyData);
	dummyAttributeBuffer = Resources::CreateBuffer(dummyAttributeBufferInfo);

	SamplerCreation defaultSamplerInfo{};
	defaultSamplerInfo.SetName("default_sampler");
	defaultSamplerInfo.SetAddressModeUVW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	defaultSamplerInfo.SetMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST);
	defaultSampler = Resources::CreateSampler(defaultSamplerInfo);

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	Hashmap<String, Sampler>::Iterator end0 = samplers.end();
	for (auto it = samplers.begin(); it != end0; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_SAMPLER, it->handle }); } }

	Hashmap<String, Texture>::Iterator end1 = textures.end();
	for (auto it = textures.begin(); it != end1; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_TEXTURE, it->handle }); } }

	Hashmap<String, Buffer>::Iterator end2 = buffers.end();
	for (auto it = buffers.begin(); it != end2; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_BUFFER, it->handle }); } }

	Hashmap<String, DescriptorSetLayout>::Iterator end3 = descriptorSetLayouts.end();
	for (auto it = descriptorSetLayouts.begin(); it != end3; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET_LAYOUT, it->handle }); } }

	Hashmap<String, DescriptorSet>::Iterator end4 = descriptorSets.end();
	for (auto it = descriptorSets.begin(); it != end4; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET, it->handle }); } }

	Hashmap<String, ShaderState>::Iterator end5 = shaders.end();
	for (auto it = shaders.begin(); it != end5; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_SHADER_STATE, it->handle }); } }

	Hashmap<String, RenderPass>::Iterator end6 = renderPasses.end();
	for (auto it = renderPasses.begin(); it != end6; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_RENDER_PASS, it->handle }); } }

	Hashmap<String, Pipeline>::Iterator end7 = pipelines.end();
	for (auto it = pipelines.begin(); it != end7; ++it) { if (it.Valid()) { resourceDeletionQueue.Push({ RESOURCE_UPDATE_TYPE_PIPELINE, it->handle }); } }

	while (resourceDeletionQueue.Size())
	{
		ResourceUpdate resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == -1) { continue; }

		switch (resourceDeletion.type)
		{
		case RESOURCE_UPDATE_TYPE_SAMPLER: { Renderer::DestroySamplerInstant(&samplers.Obtain(resourceDeletion.handle)); samplers.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_TEXTURE: { Renderer::DestroyTextureInstant(&textures.Obtain(resourceDeletion.handle)); textures.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_BUFFER: { Renderer::DestroyBufferInstant(&buffers.Obtain(resourceDeletion.handle)); buffers.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET_LAYOUT: { Renderer::DestroyDescriptorSetLayoutInstant(&descriptorSetLayouts.Obtain(resourceDeletion.handle)); descriptorSetLayouts.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET: { Renderer::DestroyDescriptorSetInstant(&descriptorSets.Obtain(resourceDeletion.handle)); descriptorSets.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_SHADER_STATE: { Renderer::DestroyShaderStateInstant(&shaders.Obtain(resourceDeletion.handle)); shaders.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_RENDER_PASS: { Renderer::DestroyRenderPassInstant(&renderPasses.Obtain(resourceDeletion.handle)); renderPasses.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_PIPELINE: { Renderer::DestroyPipelineInstant(&pipelines.Obtain(resourceDeletion.handle)); pipelines.Remove(resourceDeletion.handle); } break;
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
		ResourceUpdate resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == Renderer::currentFrame)
		{
			switch (resourceDeletion.type)
			{
			case RESOURCE_UPDATE_TYPE_SAMPLER: { Renderer::DestroySamplerInstant(&samplers.Obtain(resourceDeletion.handle)); samplers.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_TEXTURE: { Renderer::DestroyTextureInstant(&textures.Obtain(resourceDeletion.handle)); textures.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_BUFFER: { Renderer::DestroyBufferInstant(&buffers.Obtain(resourceDeletion.handle)); buffers.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET_LAYOUT: { Renderer::DestroyDescriptorSetLayoutInstant(&descriptorSetLayouts.Obtain(resourceDeletion.handle)); descriptorSetLayouts.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET: { Renderer::DestroyDescriptorSetInstant(&descriptorSets.Obtain(resourceDeletion.handle)); descriptorSets.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_SHADER_STATE: { Renderer::DestroyShaderStateInstant(&shaders.Obtain(resourceDeletion.handle)); shaders.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_RENDER_PASS: { Renderer::DestroyRenderPassInstant(&renderPasses.Obtain(resourceDeletion.handle)); renderPasses.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_PIPELINE: { Renderer::DestroyPipelineInstant(&pipelines.Obtain(resourceDeletion.handle)); pipelines.Remove(resourceDeletion.handle); } break;
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
	sampler->handle = samplers.GetHandle(info.name);

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
	texture->handle = textures.GetHandle(info.name);

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

Texture* Resources::LoadTexture(const String& name, bool generateMipMaps)
{
	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }

	TextureCreation info{};

	String path;
	name.Prepended(path, TEXTURES_PATH);

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		texture->name = name;
		texture->format = VK_FORMAT_R8G8B8A8_UNORM;
		texture->type = TEXTURE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->handle = textures.GetHandle(name);

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

		U32 mipLevels = 1;
		if (generateMipMaps)
		{
			U32 w = texture->width;
			U32 h = texture->height;

			while (w > 1 && h > 1)
			{
				w /= 2;
				h /= 2;

				++mipLevels;
			}
		}

		texture->mipmaps = mipLevels;

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

	U8* data = (U8*)calloc(1, info.imageWidth * info.imageHeight * 4); //TODO: Go through Memory

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
		I32 rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0, acount = 0;
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

		switch (easy)
		{
		case 0: {
			U32 pixel;
			U32 alpha;
			U32 index = 0;

			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					info.imageBitCount == 16 ? file.Read((U16&)pixel) : file.Read(pixel);
					data[index++] = BYTECAST(ShiftSigned(pixel & info.redMask, rshift, rcount));
					data[index++] = BYTECAST(ShiftSigned(pixel & info.greenMask, gshift, gcount));
					data[index++] = BYTECAST(ShiftSigned(pixel & info.blueMask, bshift, bcount));
					alpha = (info.alphaMask ? ShiftSigned(pixel & info.alphaMask, ashift, acount) : 255);
					data[index++] = BYTECAST(alpha);
				}

				file.Seek(pad);
			}
		} break;
		case 1: {
			U8 red;
			U8 green;
			U8 blue;
			U32 index;

			for (I32 j = info.imageHeight - 1; j >= 0; --j)
			{
				index = j * info.imageWidth * 4;

				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					file.Read(blue);
					file.Read(green);
					file.Read(red);

					data[index++] = red;
					data[index++] = green;
					data[index++] = blue;
					data[index++] = 255;
				}

				file.Seek(pad);
			}
		} break;
		case 2: {
			U8 red;
			U8 green;
			U8 blue;
			U8 alpha;
			U32 index = 0;

			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					file.Read(blue);
					file.Read(green);
					file.Read(red);
					file.Read(alpha);

					data[index++] = red;
					data[index++] = green;
					data[index++] = blue;
					data[index++] = alpha;
				}

				file.Seek(pad);
			}
		} break;
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
	buffer->handle = buffers.GetHandle(info.name);

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
	descriptorSetLayout->handle = descriptorSetLayouts.GetHandle(info.name);

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
	descriptorSet->handle = descriptorSets.GetHandle(info.name);

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

	shaderState->name = info.name;
	shaderState->handle = shaders.GetHandle(info.name);

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
	renderPass->handle = renderPasses.GetHandle(info.name);

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
	pipeline->shaderState = CreateShaderState(info.shaders);
	pipeline->depthStencil = info.depthStencil;
	pipeline->blendState = info.blendState;
	pipeline->rasterization = info.rasterization;
	pipeline->graphicsPipeline = true;
	pipeline->handle = pipelines.GetHandle(info.name);

	String cachePath("{}{}.cache", SHADERS_PATH, info.name);

	Renderer::CreatePipeline(pipeline, info.renderPass, cachePath);

	return pipeline;
}

Program* Resources::CreateProgram(const ProgramCreation& info)
{
	Program* program = &programs.Request(info.pipelineCreation.name);

	if (!program->name.Blank()) { return program; }

	const U32 PassCount = 1;
	program->passes.Resize(PassCount, {});
	program->name = info.pipelineCreation.name;

	String pipelineCachePath;

	for (U32 i = 0; i < PassCount; ++i)
	{
		ProgramPass& pass = program->passes[i];

		pass.pipeline = CreatePipeline(info.pipelineCreation);
		pass.descriptorSetLayout = pass.pipeline->descriptorSetLayouts[0];
	}

	return program;
}

Material* Resources::CreateMaterial(const MaterialCreation& info)
{
	Material* material = &materials.Request(info.name);

	if (!material->name.Blank()) { return material; }

	material->program = info.program;
	material->name = info.name;
	material->renderIndex = info.renderIndex;

	return material;
}

Scene* Resources::LoadScene(const String& name)
{
	Scene* scene = &scenes.Request(name);

	if (!scene->name.Blank()) { return scene; }

	scene->name = name;

	String path;
	name.Prepended(path, TEXTURES_PATH);

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		//TODO: Load
	}

	return scene;
}

void Resources::SaveScene(const Scene* scene)
{
	String path;
	scene->name.Prepended(path, TEXTURES_PATH);

	File file(path, FILE_OPEN_RESOURCE_WRITE);
	if (file.Opened())
	{
		file.Write(scene->name);
		for (Buffer* buffer : scene->buffers) { file.Write(buffer->name); }
		for (Texture* texture : scene->images) { file.Write(texture->name); }
		//TODO: Samplers

		for (const MeshDraw& mesh : scene->meshDraws)
		{
			//TODO: Don't hardcode buffer indices, if parent buffer, write sceneID, if child buffer, write parent's sceneID
			file.Write(0);
			file.Write(mesh.texcoordOffset);
			file.Write(mesh.texcoordBuffer->size);
			file.Write(0);
			file.Write(mesh.normalOffset);
			file.Write(mesh.normalBuffer->size);
			file.Write(0);
			file.Write(mesh.tangentOffset);
			file.Write(mesh.tangentBuffer->size);
			file.Write(0);
			file.Write(mesh.positionOffset);
			file.Write(mesh.positionBuffer->size);
			file.Write(0);
			file.Write(mesh.indexOffset);
			file.Write(mesh.indexBuffer->size);

			file.Write(AccessTexture(mesh.diffuseTextureIndex)->sceneID);
			file.Write(AccessTexture(mesh.metalRoughOcclTextureIndex)->sceneID);
			file.Write(AccessTexture(mesh.normalTextureIndex)->sceneID);
			file.Write(AccessTexture(mesh.emissivityTextureIndex)->sceneID);

			file.Write(mesh.baseColorFactor.x);
			file.Write(mesh.baseColorFactor.y);
			file.Write(mesh.baseColorFactor.z);
			file.Write(mesh.baseColorFactor.w);
			file.Write(mesh.metalRoughOcclFactor.x);
			file.Write(mesh.metalRoughOcclFactor.y);
			file.Write(mesh.metalRoughOcclFactor.z);
			file.Write(mesh.emissiveFactor.x);
			file.Write(mesh.emissiveFactor.y);
			file.Write(mesh.emissiveFactor.z);
			file.Write(mesh.flags);
			file.Write(mesh.alphaCutoff);

			Vector3 rotation = mesh.rotation.Euler();
			file.Write(mesh.position.x);
			file.Write(mesh.position.y);
			file.Write(mesh.position.z);
			file.Write(rotation.x);
			file.Write(rotation.y);
			file.Write(rotation.z);
			file.Write(mesh.scale.x);
			file.Write(mesh.scale.y);
			file.Write(mesh.scale.z);
		}

		file.Close();
	}
}

Sampler* Resources::AccessDummySampler()
{
	return dummySampler;
}

Texture* Resources::AccessDummyTexture()
{
	return dummyTexture;
}

Buffer* Resources::AccessDummyAttributeBuffer()
{
	return dummyAttributeBuffer;
}

Sampler* Resources::AccessDefaultSampler()
{
	return defaultSampler;
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

Sampler* Resources::AccessSampler(HashHandle handle)
{
	return &samplers.Obtain(handle);
}

Texture* Resources::AccessTexture(HashHandle handle)
{
	return &textures.Obtain(handle);
}

DescriptorSetLayout* Resources::AccessDescriptorSetLayout(HashHandle handle)
{
	return &descriptorSetLayouts.Obtain(handle);
}

DescriptorSet* Resources::AccessDescriptorSet(HashHandle handle)
{
	return &descriptorSets.Obtain(handle);
}

ShaderState* Resources::AccessShaderState(HashHandle handle)
{
	return &shaders.Obtain(handle);
}

RenderPass* Resources::AccessRenderPass(HashHandle handle)
{
	return &renderPasses.Obtain(handle);
}

Pipeline* Resources::AccessPipeline(HashHandle handle)
{
	return &pipelines.Obtain(handle);
}

void Resources::DestroySampler(Sampler* sampler)
{
	HashHandle handle = samplers.GetHandle(sampler->name);

	if (handle != U64_MAX)
	{
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_SAMPLER;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_TEXTURE;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_BUFFER;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET_LAYOUT;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_SHADER_STATE;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_RENDER_PASS;
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
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_PIPELINE;
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

	File file{ path, FILE_OPEN_RESOURCE_READ };

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

	File file{ path, FILE_OPEN_RESOURCE_READ };

	if (file.Opened())
	{
		U32 read = file.ReadAll(result);
		file.Close();

		return read;
	}

	return 0;
}

#if defined(_MSC_VER)
#include <spirv-headers\spirv.h>
#else
#include <spirv_cross\spirv.h>
#endif

struct Member
{
	U32         idIndex;
	U32         offset;

	String		name;
};

struct Id
{
	SpvOp           op;
	U32             set;
	U32             binding;
	U32				location;

	// For integers and floats
	U8              width;
	U8              sign;

	// For arrays, vectors and matrices
	U32             typeIndex;
	U32             count;

	// For variables
	SpvStorageClass storageClass;

	// For constants
	U32             value;

	// For structs
	String			name;
	Vector<Member>	members;
};

VkShaderStageFlags ParseExecutionModel(SpvExecutionModel model)
{
	switch (model)
	{
	case SpvExecutionModelVertex: return VK_SHADER_STAGE_VERTEX_BIT;
	case SpvExecutionModelGeometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case SpvExecutionModelFragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SpvExecutionModelKernel: return VK_SHADER_STAGE_COMPUTE_BIT;
	}

	return 0;
}

void Resources::ParseSPIRV(VkShaderModuleCreateInfo& shaderInfo, ShaderState* shaderState)
{
	const U32* data = shaderInfo.pCode;
	if (data == nullptr || data[0] != 0x07230203 || size == 0 || size % 4 != 0) { return; }
	U32 spvWordCount = (U32)(shaderInfo.codeSize / 4);

	U32 idBound = data[3];
	Vector<Id> ids{ idBound, {} };

	SpvExecutionModel stage{ SpvExecutionModelMax };

	U64 wordIndex = 5;
	U16 wordCount;
	SpvOp op;

	while ((op = (SpvOp)(data[wordIndex] & 0xFF)) != SpvOpEntryPoint) { wordIndex += (data[wordIndex] >> 16); } //Skip to entry point

	//TODO: There could be multiple entry points/excecution modes
	stage = (SpvExecutionModel)data[wordIndex + 1];	//Shader type
	shaderState->entry = (C8*)(data + (wordIndex + 3));			//Entry point name
	wordIndex += (U16)(data[wordIndex] >> 16);
	wordIndex += (U16)(data[wordIndex] >> 16); //Skip excecution mode

	while (wordIndex < spvWordCount)
	{
		op = (SpvOp)(data[wordIndex] & 0xFF);
		wordCount = (U16)(data[wordIndex] >> 16);

		switch (op)
		{
		case SpvOpDecorate: {
			Id& id = ids[data[wordIndex + 1]];

			SpvDecoration decoration = (SpvDecoration)data[wordIndex + 2];

			switch (decoration)
			{
			case SpvDecorationBinding: { id.binding = data[wordIndex + 3]; } break;
			case SpvDecorationDescriptorSet: { id.set = data[wordIndex + 3]; } break;
			case SpvDecorationLocation: { id.location = data[wordIndex + 3]; } break;
			}
		} break;
		case SpvOpMemberDecorate: {
			Id& id = ids[data[wordIndex + 1]];

			U32 memberIndex = data[wordIndex + 2];

			if (!id.members.Size()) { id.members.Resize(64, {}); }

			Member& member = id.members[memberIndex];

			SpvDecoration decoration = (SpvDecoration)data[wordIndex + 3];
			switch (decoration)
			{
			case (SpvDecorationOffset): { member.offset = data[wordIndex + 4]; } break;
			}
		} break;
		case SpvOpName: {
			Id& id = ids[data[wordIndex + 1]];
			id.name = (C8*)(data + (wordIndex + 2));
		} break;
		case SpvOpMemberName: {
			Id& id = ids[data[wordIndex + 1]];
			U32 memberIndex = data[wordIndex + 2];

			if (!id.members.Size()) { id.members.Resize(64, {}); }

			Member& member = id.members[memberIndex];
			member.name = (char*)(data + (wordIndex + 3));
		} break;
		case SpvOpTypeInt: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.width = (U8)data[wordIndex + 2];
			id.sign = (U8)data[wordIndex + 3];
		} break;
		case SpvOpTypeFloat: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.width = (U8)data[wordIndex + 2];
		} break;
		case SpvOpTypeVector: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.typeIndex = data[wordIndex + 2];
			id.count = data[wordIndex + 3];
		} break;
		case SpvOpTypeMatrix: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.typeIndex = data[wordIndex + 2];
			id.count = data[wordIndex + 3];
		} break;
		case SpvOpTypeImage: {
			//TODO: Might not need
		} break;
		case SpvOpTypeSampler: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
		}
		case SpvOpTypeSampledImage: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
		} break;
		case SpvOpTypeArray: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.typeIndex = data[wordIndex + 2];
			id.count = data[wordIndex + 3];
		} break;
		case SpvOpTypeRuntimeArray: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.typeIndex = data[wordIndex + 2];
		} break;
		case SpvOpTypeStruct: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;

			if (wordCount > 2)
			{
				for (U16 memberIndex = 0; memberIndex < wordCount - 2; ++memberIndex)
				{
					id.members[memberIndex].idIndex = data[wordIndex + memberIndex + 2];
				}
			}
		} break;
		case SpvOpTypePointer: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.typeIndex = data[wordIndex + 3];
		} break;
		case SpvOpConstant: {
			Id& id = ids[data[wordIndex + 1]];
			id.op = op;
			id.typeIndex = data[wordIndex + 2];
			id.value = data[wordIndex + 3]; //Assume all constants to have maximum 32bit width
		} break;
		case SpvOpVariable: {
			Id& id = ids[data[wordIndex + 2]];
			id.op = op;
			id.typeIndex = data[wordIndex + 1];
			id.storageClass = (SpvStorageClass)data[wordIndex + 3];
		} break;
		}

		wordIndex += wordCount;
	}

	for (U32 idIndex = 0; idIndex < ids.Size(); ++idIndex)
	{
		Id& id = ids[idIndex];

		if (id.op == SpvOpVariable)
		{
			switch (id.storageClass)
			{
			case SpvStorageClassUniform:
			case SpvStorageClassUniformConstant: {
				if (id.set == 1 && (id.binding == Renderer::bindlessTextureBinding || id.binding == (Renderer::bindlessTextureBinding + 1))) { continue; }

				Id& uniformType = ids[ids[id.typeIndex].typeIndex];

				DescriptorSetLayoutCreation& setLayout = shaderState->sets[id.set];
				setLayout.SetSetIndex(id.set);

				DescriptorBinding binding{};
				binding.start = id.binding;
				binding.count = 1;

				switch (uniformType.op)
				{
				case SpvOpTypeStruct: {
					binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					binding.name = uniformType.name;
				} break;
				case SpvOpTypeSampledImage: {
					binding.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					binding.name = id.name;
				} break;
				}

				setLayout.AddBindingAtIndex(binding, id.binding);

				shaderState->setCount = Math::Max(shaderState->setCount, (id.set + 1));
			} break;
			case SpvStorageClassInput: {
				if (stage == SpvExecutionModelVertex || stage == SpvExecutionModelKernel)
				{
					Id& type = ids[ids[id.typeIndex].typeIndex];
					
					VertexAttribute attribute{};
					attribute.binding = id.location;
					attribute.location = id.location;
					attribute.offset = 0;
					attribute.count = 1;

					VertexStream stream{};
					stream.binding = id.location;
					stream.inputRate = VERTEX_INPUT_RATE_VERTEX;

					switch (type.op)
					{
					case SpvOpTypeVector: {
						attribute.count = type.count;
						attribute.format = (ScalarType)type.typeIndex;
						if (attribute.format == SCALAR_TYPE_DOUBLE) { stream.stride = 8 * type.count; }
						else { stream.stride = 4 * type.count; }
					} break;
					case SpvOpConstant: {
						//TODO:
					} break;
					}

					++shaderState->vertexAttributeCount;
					++shaderState->vertexStreamCount;
					shaderState->vertexAttributes[id.location] = attribute;
					shaderState->vertexStreams[id.location] = stream;
				}
			} break;
			}
		}

		id.members.Destroy();
	}
}
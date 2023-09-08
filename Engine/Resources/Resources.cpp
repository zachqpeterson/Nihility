#include "Resources.hpp"

#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Core\Logger.hpp"
#include "Core\File.hpp"
#include "Math\Color.hpp"
#include "Rendering\Pipeline.hpp"

#include "External\zlib\zlib.h"
#include "External\Assimp\cimport.h"
#include "External\Assimp\scene.h"
#include "External\Assimp\postprocess.h"
#include <LunarG\glslang\Include\intermediate.h>

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "External\stb_image.h"
#include "External\LunarG\glslang\Public\ShaderLang.h"

#undef near
#undef far

#define BYTECAST(x) ((U8)((x) & 255))

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
    aiProcess_EmbedTextures)

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

enum KTXFormatType
{
	KTX_FORMAT_TYPE_NONE = 0x00000000,
	KTX_FORMAT_TYPE_PACKED = 0x00000001,
	KTX_FORMAT_TYPE_COMPRESSED = 0x00000002,
	KTX_FORMAT_TYPE_PALETTIZED = 0x00000004,
	KTX_FORMAT_TYPE_DEPTH = 0x00000008,
	KTX_FORMAT_TYPE_STENCIL = 0x00000010,
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

Sampler* Resources::dummySampler;
Texture* Resources::dummyTexture;
Sampler* Resources::defaultSampler;
Shader* Resources::meshProgram;
Shader* Resources::swapchainProgram;
Pipeline* Resources::renderPipeline;
Pipeline* Resources::swapchainPipeline;

Hashmap<String, Sampler>		Resources::samplers{ 32, {} };
Hashmap<String, Texture>		Resources::textures{ 512, {} };
Hashmap<String, Renderpass>		Resources::renderpasses{ 256, {} };
Hashmap<String, Shader>			Resources::shaders{ 128, {} };
Hashmap<String, Pipeline>		Resources::pipelines{ 128, {} };
Hashmap<String, Model>			Resources::models{ 128, {} };
Hashmap<String, Skybox>			Resources::skyboxes{ 32, {} };
Hashmap<String, Scene>			Resources::scenes{ 128, {} };

Queue<ResourceUpdate>			Resources::resourceDeletionQueue{};
Queue<ResourceUpdate>			Resources::bindlessTexturesToUpdate;

Pool<DescriptorSetLayout, 256>	Resources::descriptorSetLayouts;
VkDescriptorPool				Resources::bindlessDescriptorPool;
VkDescriptorSet					Resources::bindlessDescriptorSet;
DescriptorSetLayout				Resources::bindlessDescriptorSetLayout;

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	glslang::InitializeProcess();

	descriptorSetLayouts.Create();

	TextureInfo dummyTextureInfo{};
	dummyTextureInfo.SetName("dummy_texture");
	dummyTextureInfo.SetFormatType(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D);
	dummyTextureInfo.SetSize(1, 1, 1);
	U32 zero = 0;
	dummyTextureInfo.SetData(&zero);
	dummyTexture = CreateTexture(dummyTextureInfo);

	SamplerInfo dummySamplerInfo{};
	dummySamplerInfo.SetName("dummy_sampler");
	dummySamplerInfo.SetAddressModeUV(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	dummySamplerInfo.SetMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST);
	dummySampler = CreateSampler(dummySamplerInfo);

	VkPushConstantRange pushConstant{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GlobalData) };
	meshProgram = CreateShader("shaders/MeshPbr.shader", 1, &pushConstant);

	swapchainProgram = CreateShader("shaders/Swapchain.shader");

	PipelineInfo info{};
	info.name = "render_pipeline";
	info.shader = meshProgram;
	info.attachmentFinalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	renderPipeline = CreatePipeline(info);

	info.name = "swapchain_pipeline";
	info.shader = swapchainProgram;
	info.renderpass = &Renderer::swapchain.renderpass;
	swapchainPipeline = CreatePipeline(info);

	return true;
}

void Resources::CreateDefaults()
{
	SamplerInfo defaultSamplerInfo{};
	defaultSamplerInfo.SetName("default_sampler");
	defaultSamplerInfo.SetAddressModeUVW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	defaultSamplerInfo.SetMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST);
	defaultSampler = Resources::CreateSampler(defaultSamplerInfo);
}

bool Resources::CreateBindless()
{
	VkDescriptorPoolSize bindlessPoolSizes[]
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxBindlessResources },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxBindlessResources },
	};

	VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
	poolInfo.maxSets = maxBindlessResources * CountOf32(bindlessPoolSizes);
	poolInfo.poolSizeCount = CountOf32(bindlessPoolSizes);
	poolInfo.pPoolSizes = bindlessPoolSizes;

	VkValidateFR(vkCreateDescriptorPool(Renderer::device, &poolInfo, Renderer::allocationCallbacks, &bindlessDescriptorPool));

	const U32 poolCount = CountOf32(bindlessPoolSizes);
	VkDescriptorSetLayoutBinding vkBinding[4];

	// Actual descriptor set layout
	VkDescriptorSetLayoutBinding& imageSamplerBinding = vkBinding[0];
	imageSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageSamplerBinding.descriptorCount = maxBindlessResources;
	imageSamplerBinding.binding = bindlessTextureBinding;
	imageSamplerBinding.stageFlags = VK_SHADER_STAGE_ALL;
	imageSamplerBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding& storageImageBinding = vkBinding[1];
	storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	storageImageBinding.descriptorCount = maxBindlessResources;
	storageImageBinding.binding = bindlessTextureBinding + 1;
	storageImageBinding.stageFlags = VK_SHADER_STAGE_ALL;
	storageImageBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = poolCount;
	layoutInfo.pBindings = vkBinding;
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	// TODO: reenable variable descriptor count
	// Binding flags
	VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
	VkDescriptorBindingFlags bindingFlags[4];

	bindingFlags[0] = bindlessFlags;
	bindingFlags[1] = bindlessFlags;

	VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
	extendedInfo.bindingCount = poolCount;
	extendedInfo.pBindingFlags = bindingFlags;

	layoutInfo.pNext = &extendedInfo;

	VkValidateFR(vkCreateDescriptorSetLayout(Renderer::device, &layoutInfo, Renderer::allocationCallbacks, &bindlessDescriptorSetLayout.descriptorSetLayout));

	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = bindlessDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &bindlessDescriptorSetLayout.descriptorSetLayout;

	VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
	U32 maxBinding = maxBindlessResources - 1;
	countInfo.descriptorSetCount = 1;
	// This number is the max allocatable count
	countInfo.pDescriptorCounts = &maxBinding;
	//allocInfo.pNext = &countInfo;

	VkValidateFR(vkAllocateDescriptorSets(Renderer::device, &allocInfo, &bindlessDescriptorSet));

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	while (resourceDeletionQueue.Size())
	{
		ResourceUpdate resourceDeletion;
		resourceDeletionQueue.Pop(resourceDeletion);

		if (resourceDeletion.currentFrame == -1) { continue; }

		switch (resourceDeletion.type)
		{
		case RESOURCE_UPDATE_TYPE_SAMPLER: { Renderer::DestroySamplerInstant(&samplers.Obtain(resourceDeletion.handle)); samplers.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_TEXTURE: { Renderer::DestroyTextureInstant(&textures.Obtain(resourceDeletion.handle)); textures.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_RENDER_PASS: { Renderer::DestroyRenderPassInstant(&renderpasses.Obtain(resourceDeletion.handle)); renderpasses.Remove(resourceDeletion.handle); } break;
		case RESOURCE_UPDATE_TYPE_PIPELINE: { pipelines.Obtain(resourceDeletion.handle).Destroy(); pipelines.Remove(resourceDeletion.handle); } break;
		}
	}

	CleanupHashmap(samplers, Renderer::DestroySamplerInstant);
	CleanupHashmap(textures, Renderer::DestroyTextureInstant);
	CleanupHashmap(renderpasses, Renderer::DestroyRenderPassInstant);
	CleanupHashmap(shaders, nullptr);
	CleanupHashmap(pipelines, nullptr);
	CleanupHashmap(models, nullptr);
	CleanupHashmap(skyboxes, nullptr);
	CleanupHashmap(scenes, nullptr);

	samplers.Destroy();
	textures.Destroy();
	descriptorSetLayouts.Destroy();
	renderpasses.Destroy();
	shaders.Destroy();
	pipelines.Destroy();
	models.Destroy();
	skyboxes.Destroy();
	scenes.Destroy();

	resourceDeletionQueue.Destroy();
	bindlessTexturesToUpdate.Destroy();

	vkDestroyDescriptorSetLayout(Renderer::device, bindlessDescriptorSetLayout.descriptorSetLayout, Renderer::allocationCallbacks);
	vkDestroyDescriptorPool(Renderer::device, bindlessDescriptorPool, Renderer::allocationCallbacks);
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
			case RESOURCE_UPDATE_TYPE_RENDER_PASS: { Renderer::DestroyRenderPassInstant(&renderpasses.Obtain(resourceDeletion.handle)); renderpasses.Remove(resourceDeletion.handle); } break;
			case RESOURCE_UPDATE_TYPE_PIPELINE: { pipelines.Obtain(resourceDeletion.handle).Destroy(); pipelines.Remove(resourceDeletion.handle); } break;
			}
		}
	}

	if (bindlessTexturesToUpdate.Size())
	{
		VkWriteDescriptorSet bindlessDescriptorWrites[maxBindlessResources];
		VkDescriptorImageInfo bindlessImageInfo[maxBindlessResources];

		Texture* dummyTexture = Resources::AccessDummyTexture();
		 
		U32 currentWriteIndex = 0;

		while (bindlessTexturesToUpdate.Size())
		{
			ResourceUpdate textureToUpdate;
			bindlessTexturesToUpdate.Pop(textureToUpdate);

			//TODO: Maybe check frame
			{
				Texture* texture = Resources::AccessTexture(textureToUpdate.handle);

				VkWriteDescriptorSet& descriptorWrite = bindlessDescriptorWrites[currentWriteIndex];
				descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.dstArrayElement = (U32)textureToUpdate.handle;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.dstSet = bindlessDescriptorSet;
				descriptorWrite.dstBinding = bindlessTextureBinding;

				Sampler* defaultSampler = Resources::AccessDefaultSampler();
				VkDescriptorImageInfo& descriptorImageInfo = bindlessImageInfo[currentWriteIndex];

				if (texture->sampler != nullptr) { descriptorImageInfo.sampler = texture->sampler->sampler; }
				else { descriptorImageInfo.sampler = defaultSampler->sampler; }

				descriptorImageInfo.imageView = texture->format != VK_FORMAT_UNDEFINED ? texture->imageView : dummyTexture->imageView;
				descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptorWrite.pImageInfo = &descriptorImageInfo;

				++currentWriteIndex;
			}
		}

		if (currentWriteIndex) { vkUpdateDescriptorSets(Renderer::device, currentWriteIndex, bindlessDescriptorWrites, 0, nullptr); }
	}
}

void Resources::UpdatePipelines()
{
	typename Hashmap<String, Pipeline>::Iterator end = pipelines.end();
	for (auto it = pipelines.begin(); it != end; ++it)
	{
		if (it.Valid() && !it->name.Blank()) { it->Resize(); }
	}

	for (auto it = pipelines.begin(); it != end; ++it)
	{
		if (it.Valid() && !it->name.Blank()) { it->Update(); }
	}
}

Sampler* Resources::CreateSampler(const SamplerInfo& info)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

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

Texture* Resources::CreateTexture(const TextureInfo& info)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Texture* texture = &textures.Request(info.name);

	if (!texture->name.Blank()) { return texture; }

	texture->name = info.name;
	texture->width = info.width;
	texture->height = info.height;
	texture->size = info.width * info.height * 4;
	texture->depth = info.depth;
	texture->flags = info.flags;
	texture->format = info.format;
	texture->mipmapCount = info.mipmapCount;
	texture->type = info.type;
	texture->handle = textures.GetHandle(info.name);

	Renderer::CreateTexture(texture, info.initialData);

	return texture;
}

Texture* Resources::CreateSwapchainTexture(VkImage image, VkFormat format, U8 index)
{
	String name{ "SwapchainTexture{}", index };

	Texture* texture = &textures.Request(name);

	texture->name = name;
	texture->swapchainImage = true;
	texture->format = format;
	texture->image = image;
	texture->handle = textures.GetHandle(name);

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

	if (vkCreateImageView(Renderer::device, &viewInfo, Renderer::allocationCallbacks, &texture->imageView) != VK_SUCCESS) { return nullptr; }

	Renderer::SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, "Swapchain_ImageView");

	return texture;
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
	viewInfo.format = texture->format;
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

	Renderer::SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, "Swapchain_ImageView");

	return true;
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
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }

	TextureInfo info{};

	File file(name, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		texture->name = name;
		texture->format = VK_FORMAT_R8G8B8A8_UNORM;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->handle = textures.GetHandle(name);

		I64 extIndex = name.LastIndexOf('.') + 1;

		bool success = false;

		if (name.CompareN("bmp", extIndex)) { success = LoadBMP(texture, file, generateMipMaps); }
		else if (name.CompareN("png", extIndex)) { success = LoadPNG(texture, file, generateMipMaps); }
		else if (name.CompareN("jpg", extIndex) || name.CompareN("jpeg", extIndex)) { success = LoadJPG(texture, file, generateMipMaps); }
		else if (name.CompareN("psd", extIndex)) { success = LoadPSD(texture, file, generateMipMaps); }
		else if (name.CompareN("tiff", extIndex)) { success = LoadTIFF(texture, file, generateMipMaps); }
		else if (name.CompareN("tga", extIndex)) { success = LoadTGA(texture, file, generateMipMaps); }
		else if (name.CompareN("ktx", extIndex) || name.CompareN("ktx2", extIndex) || name.CompareN("ktx1", extIndex)) { success = LoadKTX(texture, file, generateMipMaps); }
		else { Logger::Error("Unknown Texture Extension {}!", name); textures.Remove(name); return nullptr; }

		if (!success)
		{
			textures.Remove(name);
			file.Close();
			return nullptr;
		}

		file.Close();
		return texture;
	}

	Logger::Error("Failed To Find Or Open File: {}", name);

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

bool Resources::LoadBMP(Texture* texture, File& file, bool generateMipMaps)
{
	BMPHeader header{};
	BMPInfo info{};
	file.Read(header);

	if (header.signature != 0x4D42)
	{
		Logger::Error("Texture Is Not a BMP!");
		return false;
	}

	file.Read(info.infoSize);
	info.extraRead = 14;
	U32 notRead = 12;

	if (info.infoSize == 12) { file.Read((I16&)info.imageWidth); file.Read((I16&)info.imageHeight); notRead = 8; }
	else { file.Read(info.imageWidth); file.Read(info.imageHeight); }

	file.Read(&info.imagePlanes, info.infoSize - notRead);

	if (info.imagePlanes != 1) { Logger::Error("Invalid BMP!"); return false; }

	if (info.imageCompression != 0 && (info.imageCompression != 3 || (info.imageBitCount != 16 && info.imageBitCount != 32)))
	{
		Logger::Error("RLE Compressed BMPs Not Yet Supported!");
		return false;
	}

	texture->width = info.imageWidth;
	texture->height = info.imageHeight;
	texture->size = info.imageWidth * info.imageHeight * 4;

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
			return false;
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
			return false;
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
				return false;
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
				return false;
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

	texture->mipmapsGenerated = false;
	texture->mipmapCount = mipLevels;

	Renderer::CreateTexture(texture, data);

	free(data);

	return true;
}

bool Resources::LoadPNG(Texture* texture, File& file, bool generateMipMaps)
{
	Logger::Error("PNG Texture Format Not Yet Supported!");

	return false;
}

bool Resources::LoadJPG(Texture* texture, File& file, bool generateMipMaps)
{
	Logger::Error("JPG Texture Format Not Yet Supported!");

	return false;
}

bool Resources::LoadPSD(Texture* texture, File& file, bool generateMipMaps)
{
	Logger::Error("PSD Texture Format Not Yet Supported!");

	return false;
}

bool Resources::LoadTIFF(Texture* texture, File& file, bool generateMipMaps)
{
	Logger::Error("TIFF Texture Format Not Yet Supported!");

	return false;
}

bool Resources::LoadTGA(Texture* texture, File& file, bool generateMipMaps)
{
	Logger::Error("TGA Texture Format Not Yet Supported!");

	return false;
}

bool Resources::LoadKTX(Texture* texture, File& file, bool generateMipMaps)
{
	static constexpr U8 FileIdentifier11[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr U8 FileIdentifier20[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	static constexpr U32 EndiannessIdentifier = 0x04030201;

	U8 identifier[12]{};

	file.Read(identifier);

	if (Memory::Compare(identifier, FileIdentifier11, 12))
	{
		U32 endianness;
		file.Read(endianness);

		if (endianness != EndiannessIdentifier)
		{
			//TODO: Don't be lazy
			Logger::Error("Too Lazy to Flip Endianness!");
			return false;
		}

		KTXHeader11 header;
		file.Read(header);

		U8 compression = 0;
		if (header.type == 0 || header.format == 0)
		{
			if (header.type + header.format != 0) { return false; }
			compression = 1;

			Logger::Error("KTX Textures With Compression Not Yet Supported!");
			return false;
		}

		if (header.format == header.internalFormat) { return false; }
		if (header.pixelWidth == 0 || (header.pixelDepth > 0 && header.pixelHeight == 0)) { return false; }

		U16 dimension = 0;
		if (header.pixelDepth > 0)
		{
			if (header.arrayElementCount > 0)
			{
				Logger::Error("3D Array Textures Are Not Yet Supported!");
				return false;
			}
			dimension = 3;
		}
		else if (header.pixelHeight > 0) { dimension = 2; }
		else { dimension = 1; }

		if (header.faceCount == 6) { if (dimension != 2) { return false; } }
		else if (header.faceCount != 1) { return false; }

		if (header.mipmapLevelCount == 0)
		{
			texture->mipmapsGenerated = false;
			header.mipmapLevelCount = 1;
		}
		else
		{
			texture->mipmapsGenerated = true;
		}

		KTXInfo info{};
		GetKTXInfo(header.internalFormat, info);

		file.Seek(header.keyValueDataSize);

		U32 elementCount = Math::Max(1u, header.arrayElementCount);
		U32 depth = Math::Max(1u, header.pixelDepth);
		U32 dataSize = (U32)(file.Size() - file.Pointer() - header.mipmapLevelCount * sizeof(U32));

		U8* data = (U8*)calloc(1, dataSize); //TODO: Go through Memory
		U32 dataPtr = 0;

		U32 levelSizes[14];

		for (U32 mipLevel = 0; mipLevel < header.mipmapLevelCount; ++mipLevel)
		{
			U32 faceLodSize;
			file.Read(faceLodSize);

			levelSizes[mipLevel] = faceLodSize;

			for (U32 face = 0; face < header.faceCount; ++face)
			{
				file.Read(data + dataPtr, faceLodSize); //TODO: This can be read in one read
				dataPtr += faceLodSize;
			}
		}

		texture->format = VK_FORMAT_R16G16B16A16_SFLOAT;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->width = header.pixelWidth;
		texture->height = header.pixelHeight;
		texture->depth = 1;
		texture->size = dataSize;
		texture->mipmapCount = header.mipmapLevelCount;

		Renderer::CreateCubeMap(texture, data, levelSizes);

		free(data);
	}
	else if (Memory::Compare(identifier, FileIdentifier20, 12))
	{
		KTXHeader20 header;
		file.Read(header);

		KTXLevel level[14];

		file.Read(level, sizeof(KTXLevel) * header.levelCount);

		U32 dfdTotalSize;
		file.Read(dfdTotalSize);

		file.Seek(dfdTotalSize + header.kvdByteLength);

		if (header.superCompressionScheme != 0)
		{
			Logger::Error("KTX Textures With Compression Not Yet Supported!");
			return false;
		}

		if (header.levelCount == 0)
		{
			texture->mipmapsGenerated = false;
			header.levelCount = 1;
		}
		else
		{
			texture->mipmapsGenerated = true;
		}

		U32 dataSize = (U32)level[0].uncompressedByteLength;
		U8* data = (U8*)calloc(1, dataSize); //TODO: Go through Memory

		file.Read(data, dataSize);

		texture->format = header.format;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->width = header.pixelWidth;
		texture->height = header.pixelHeight;
		texture->depth = 1;
		texture->size = dataSize;
		texture->mipmapCount = 1;

		U32 layerSize = dataSize / header.faceCount;
		Renderer::CreateCubeMap(texture, data, &layerSize);

		free(data);
	}
	else
	{
		Logger::Error("Texture Is Not a KTX!");
		return false;
	}

	return true;
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

Texture* Resources::AssimpToNhimg(const String& name, const aiTexture* textureInfo)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }
	
	if (Memory::Compare(textureInfo->achFormatHint, "exr", 3))
	{
		Logger::Error("Exr Images Are Not Yet Supported!");
		BreakPoint;
	}

	U8* data = stbi_load_from_memory((U8*)textureInfo->pcData, textureInfo->mWidth, (I32*)&texture->width, (I32*)&texture->height, nullptr, 4);

	if (!data)
	{
		Logger::Error("Failed To Convert Image!");
		textures.Remove(name);
		return nullptr;
	}

	texture->name = name;
	texture->depth = 1;
	texture->size = texture->width * texture->height * 4;
	texture->format = VK_FORMAT_R8G8B8A8_UNORM;
	texture->mipmapCount = (U8)Math::Min(DegreeOfTwo(texture->width), DegreeOfTwo(texture->height));
	texture->type = VK_IMAGE_TYPE_2D;
	texture->handle = textures.GetHandle(name);

	if (Renderer::CreateTexture(texture, data))
	{
		return texture;
	}

	Logger::Error("Failed To Convert Image!");
	textures.Remove(name);
	return nullptr;
}

Texture* Resources::ConvertToNhimg(const String& name, const U8* data)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Texture* texture = &textures.Request(name);

	if (!texture->name.Blank()) { return texture; }

	texture->name = name;

	return nullptr;
}

DescriptorSetLayout* Resources::CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& info)
{
	U64 handle;
	DescriptorSetLayout* descriptorSetLayout = descriptorSetLayouts.Request(handle);

	descriptorSetLayout->bindingCount = info.bindingCount;
	descriptorSetLayout->setIndex = info.setIndex;
	descriptorSetLayout->handle = handle;
	
	Memory::Copy(descriptorSetLayout->bindings, info.bindings, info.bindingCount * sizeof(VkDescriptorSetLayoutBinding));

	Renderer::CreateDescriptorSetLayout(descriptorSetLayout);

	return descriptorSetLayout;
}

Renderpass* Resources::CreateRenderpass(const RenderpassInfo& info)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Renderpass* renderpass = &renderpasses.Request(info.name);

	if (!renderpass->name.Blank()) { return renderpass; }

	renderpass->name = info.name;
	renderpass->width = info.width;
	renderpass->height = info.height;
	renderpass->renderpass = nullptr;
	renderpass->renderTargetCount = (U8)info.renderTargetCount;
	renderpass->outputDepth = info.depthStencilTexture;
	renderpass->handle = renderpasses.GetHandle(info.name);
	renderpass->output.colorOperation = info.colorOperation;
	renderpass->output.depthOperation = info.depthOperation;
	renderpass->output.stencilOperation = info.stencilOperation;
	renderpass->output.colorFormatCount = info.renderTargetCount;
	renderpass->output.attachmentFinalLayout = info.attachmentFinalLayout;
	renderpass->clearCount = info.clearCount;
	if (info.depthStencilTexture) { renderpass->output.depthStencilFormat = info.depthStencilTexture->format; }

	for (U32 i = 0; i < info.clearCount; ++i)
	{
		renderpass->clears[i] = info.clears[i];
	}

	for (U32 i = 0; i < info.renderTargetCount; ++i)
	{
		renderpass->outputTextures[i] = info.outputTextures[i];
		renderpass->output.colorFormats[i] = info.outputTextures[i]->format;
	}

	Renderer::CreateRenderpass(renderpass);

	return renderpass;
}

Shader* Resources::CreateShader(const String& name, U8 pushConstantCount, VkPushConstantRange* pushConstants)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Shader* shader = &shaders.Request(name);

	if (!shader->name.Blank()) { return shader; }

	*shader = {};

	shader->name = name;
	shader->handle = shaders.GetHandle(name);

	if (!shader->Create(name, pushConstantCount, pushConstants))
	{
		shaders.Remove(shader->handle);
		shader->handle = U64_MAX;
	}

	return shader;
}

Pipeline* Resources::CreatePipeline(const PipelineInfo& info, const SpecializationInfo& specializationInfo)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Pipeline* pipeline = &pipelines.Request(info.name);

	if (!pipeline->name.Blank()) { return pipeline; }

	pipeline->name = info.name;
	pipeline->handle = pipelines.GetHandle(info.name);

	if (!pipeline->Create(info, specializationInfo))
	{
		pipelines.Remove(pipeline->handle);
		pipeline->handle = U64_MAX;
	}

	return pipeline;
}

Model* Resources::LoadModel(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Model* model = &models.Request(name);

	if (!model->name.Blank()) { return model; }

	const aiScene* scene = aiImportFile(name.Data(), ASSIMP_IMPORT_FLAGS);

	if (!scene)
	{
		Logger::Error("Failed To Import Model: {}!", name);
		models.Remove(name);
		return nullptr;
	}

	model->name = scene->mName.data;

	if (model->name.Blank())
	{
		//TODO: this should work for paths with '\\' also
		//TODO: this should work for paths without '/' or '\\'
		name.SubString(model->name, name.LastIndexOf('/') + 1);
	}

	Vector<Mesh*> meshes(scene->mNumMeshes);

	for (U32 i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* tempMesh = scene->mMeshes[i];
		aiMaterial* tempMaterial = scene->mMaterials[tempMesh->mMaterialIndex];

		model->meshes[model->meshCount++] = CreateMesh(i, tempMesh, tempMaterial, scene);
	}

	aiReleaseImport(scene);

	return model;
}

Mesh Resources::CreateMesh(U32 meshNumber, const aiMesh* meshInfo, const aiMaterial* materialInfo, const aiScene* scene)
{
	Mesh mesh{};

	aiReturn ret;

	for (U32 i = 0; i <= AI_TEXTURE_TYPE_MAX; ++i)
	{
		aiString texturePath;
		ret = materialInfo->GetTexture((aiTextureType)i, 0, &texturePath, nullptr, nullptr, nullptr, nullptr, nullptr);
		if (ret == aiReturn_SUCCESS)
		{
			Texture* texture = AssimpToNhimg(String::RandomString(16), scene->GetEmbeddedTexture(texturePath.C_Str()));

			switch (i)
			{
			case aiTextureType_DIFFUSE: { mesh.diffuseTextureIndex = texture->handle; } break;
				case aiTextureType_EMISSIVE: { mesh.emissivityTextureIndex = texture->handle; } break;
				case aiTextureType_NORMALS: { mesh.normalTextureIndex = texture->handle; } break;
				case aiTextureType_SHININESS: {  } break;
				case aiTextureType_BASE_COLOR: { mesh.diffuseTextureIndex = texture->handle; } break;
				case aiTextureType_EMISSION_COLOR: { mesh.emissivityTextureIndex = texture->handle; } break;
				case aiTextureType_METALNESS: {  } break;
				case aiTextureType_DIFFUSE_ROUGHNESS: {  } break;
				case aiTextureType_AMBIENT_OCCLUSION: {  } break;

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
				default: {
					Logger::Warn("Unknown Texture Usage '{}'!", i);
				}
			}
		}
	}
	
	aiColor4D color{ 1.0f, 1.0f, 1.0f, 1.0f };
	ret = materialInfo->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	ai_real roughness{ 0.5f };
	ret = materialInfo->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
	ai_real metallic{ 0.5f };
	ret = materialInfo->Get(AI_MATKEY_METALLIC_FACTOR, metallic);

	mesh.baseColorFactor.x = color.r;
	mesh.baseColorFactor.y = color.g;
	mesh.baseColorFactor.z = color.b;
	mesh.baseColorFactor.w = color.a;

	mesh.metalRoughFactor.x = metallic;
	mesh.metalRoughFactor.y = roughness;

	U32 vertexCount = meshInfo->mNumVertices;
	Vertex* vertexData = (Vertex*)malloc(vertexCount * sizeof(Vertex));

	if (meshInfo->HasTangentsAndBitangents())
	{
		if (meshInfo->HasTextureCoords(0))
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				vertexData[i].position = *(Vector3*)&meshInfo->mVertices[i];
				vertexData[i].normal = *(Vector3*)&meshInfo->mNormals[i];
				vertexData[i].tangent = *(Vector3*)&meshInfo->mTangents[i];
				vertexData[i].bitangent = *(Vector3*)&meshInfo->mBitangents[i];
				vertexData[i].texcoord = *(Vector2*)&meshInfo->mTextureCoords[0][i];
			}
		}
		else
		{
			mesh.flags |= MATERIAL_FLAG_NO_TEXTURE_COORDS;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				vertexData[i].position = *(Vector3*)&meshInfo->mVertices[i];
				vertexData[i].normal = *(Vector3*)&meshInfo->mNormals[i];
				vertexData[i].tangent = *(Vector3*)&meshInfo->mTangents[i];
				vertexData[i].bitangent = *(Vector3*)&meshInfo->mBitangents[i];
			}
		}
	}
	else
	{
		mesh.flags |= MATERIAL_FLAG_NO_TANGENTS;

		if (meshInfo->HasTextureCoords(0))
		{
			for (U32 i = 0; i < vertexCount; ++i)
			{
				vertexData[i].position = *(Vector3*)&meshInfo->mVertices[i];
				vertexData[i].normal = *(Vector3*)&meshInfo->mNormals[i];
				vertexData[i].texcoord = *(Vector2*)&meshInfo->mTextureCoords[0][i];
			}
		}
		else
		{
			mesh.flags |= MATERIAL_FLAG_NO_TEXTURE_COORDS;

			for (U32 i = 0; i < vertexCount; ++i)
			{
				vertexData[i].position = *(Vector3*)&meshInfo->mVertices[i];
				vertexData[i].normal = *(Vector3*)&meshInfo->mNormals[i];
			}
		}
	}

	U32 faceSize = meshInfo->mFaces[0].mNumIndices * sizeof(U32);

	U32* indexData = (U32*)malloc(meshInfo->mNumFaces * faceSize);

	U8* it = (U8*)indexData;

	for (U32 i = 0; i < meshInfo->mNumFaces; ++i)
	{
		Memory::Copy(it, meshInfo->mFaces[i].mIndices, faceSize);
		it += faceSize;
	}

	Renderer::UploadDraw(mesh, meshInfo->mNumFaces * meshInfo->mFaces[0].mNumIndices, indexData, vertexCount, vertexData);

	free(vertexData);
	free(indexData);

	return Move(mesh);
}

Skybox* Resources::LoadSkybox(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Skybox* skybox = &skyboxes.Request(name);

	if (!skybox->name.Blank()) { return skybox; }

	skybox->name = name;
	skybox->handle = skyboxes.GetHandle(name);
	skybox->texture = LoadTexture(name, false);

	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	//BufferCreation bufferCreation{};
	//bufferCreation.SetName("binaries/Skybox.bin").Set(flags, RESOURCE_USAGE_IMMUTABLE, 0);
	//skybox->buffer = LoadBuffer(bufferCreation);

	return skybox;
}

Scene* Resources::CreateScene(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Scene* scene = &scenes.Request(name);

	if (!scene->name.Blank()) { return scene; }

	scene->name = name;
	scene->drawSkybox = false;

	scene->camera.SetPerspective(0.00001f, 1000.0f, 45.0f, (F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());

	//scene->Create();

	return scene;
}

Scene* Resources::LoadScene(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Scene* scene = &scenes.Request(name);

	if (!scene->name.Blank()) { return scene; }

	scene->name = name;
	scene->updatePostProcess = true;
	scene->drawSkybox = true; //TODO: save in scene file

	File file(name, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		I64 extIndex = name.LastIndexOf('.') + 1;

		bool success = false;

		if (name.CompareN("nhscn", extIndex)) { success = LoadNHSCN(scene, file); }
		else if (name.CompareN("gltf", extIndex)) { success = LoadGLTF(scene, file); }
		else if (name.CompareN("glb", extIndex)) { success = LoadGLB(scene, file); }
		else { Logger::Error("Unknown Texture Extension {}!", name); textures.Remove(name); return nullptr; }

		if (!success)
		{
			textures.Remove(name);
			file.Close();
			return nullptr;
		}

		file.Close();
		return scene;
	}

	Logger::Error("Failed To Find Or Open File: {}", name);

	scenes.Remove(name);
	return nullptr;
}

bool Resources::LoadNHSCN(Scene* scene, File& file)
{
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
	
	return true;
}

bool Resources::LoadGLB(Scene* scene, File& file)
{
	Logger::Error("GLB File Format Not Yet Supported!");

	return false;
}

bool Resources::LoadGLTF(Scene* scene, File& file)
{
	Logger::Error("GLTF File Format Not Yet Supported!");

	return false;
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

Sampler* Resources::AccessDummySampler()
{
	return dummySampler;
}

Texture* Resources::AccessDummyTexture()
{
	return dummyTexture;
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

Renderpass* Resources::AccessRenderpass(const String& name)
{
	Renderpass* renderpass = &renderpasses.Request(name);

	if (!renderpass->name.Blank()) { return renderpass; }

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

Renderpass* Resources::AccessRenderpass(HashHandle handle)
{
	return &renderpasses.Obtain(handle);
}

Pipeline* Resources::AccessPipeline(HashHandle handle)
{
	return &pipelines.Obtain(handle);
}

void Resources::DestroySampler(Sampler* sampler)
{
	HashHandle handle = sampler->handle;

	if (handle != U64_MAX)
	{
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_SAMPLER;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
}

void Resources::DestroyTexture(Texture* texture)
{
	HashHandle handle = texture->handle;

	if (handle != U64_MAX)
	{
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_TEXTURE;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
}

void Resources::DestroyDescriptorSetLayout(DescriptorSetLayout* layout)
{
	if (layout)
	{
		vkDestroyDescriptorSetLayout(Renderer::device, layout->descriptorSetLayout, Renderer::allocationCallbacks);

		if (layout->updateTemplate) { vkDestroyDescriptorUpdateTemplate(Renderer::device, layout->updateTemplate, Renderer::allocationCallbacks); layout->updateTemplate = nullptr; }
	}
}

void Resources::DestroyDescriptorSet(DescriptorSet* set)
{
	HashHandle handle = set->handle;

	if (handle != U64_MAX)
	{
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_DESCRIPTOR_SET;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
}

void Resources::DestroyRenderpass(Renderpass* renderpass)
{
	HashHandle handle = renderpass->handle;

	if (handle != U64_MAX)
	{
		ResourceUpdate deletion{};
		deletion.handle = handle;
		deletion.type = RESOURCE_UPDATE_TYPE_RENDER_PASS;
		deletion.currentFrame = Renderer::currentFrame;
		resourceDeletionQueue.Push(deletion);
	}
}

bool Resources::LoadBinary(const String& name, String& result)
{
	File file{ name, FILE_OPEN_RESOURCE_READ };
	if (file.Opened())
	{
		file.ReadAll(result);
		file.Close();

		return true;
	}

	Logger::Error("Failed To Find Or Open File: {}", name);

	return false;
}

U32 Resources::LoadBinary(const String& name, void** result)
{
	File file{ name, FILE_OPEN_RESOURCE_READ };
	if (file.Opened())
	{
		U32 read = file.ReadAll(result);
		file.Close();

		return read;
	}

	Logger::Error("Failed To Find Or Open File: {}", name);

	return false;
}

void Resources::SaveBinary(const String& name, const String& data)
{
	File file{ name, FILE_OPEN_RESOURCE_WRITE };
	if (file.Opened())
	{
		file.Write(data);
		file.Close();
	}

	Logger::Error("Failed To Find Or Open File: {}", name);
}

void Resources::SaveBinary(const String& name, void* data, U64 length)
{
	File file{ name, FILE_OPEN_RESOURCE_WRITE };
	if (file.Opened())
	{
		file.Write(data, (U32)length);
		file.Close();
	}

	Logger::Error("Failed To Find Or Open File: {}", name);
}

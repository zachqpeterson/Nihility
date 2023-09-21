#include "Resources.hpp"

#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Core\Logger.hpp"
#include "Core\DataReader.hpp"
#include "Math\Color.hpp"
#include "Rendering\Pipeline.hpp"
#include "Containers\Stack.hpp"

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
	aiProcess_FlipUVs						|   \
    aiProcess_EmbedTextures)

//nhtex
//nhaud
//nhmat
//nhmsh
//nhmdl
//nhshd
//nhscn
//nhfnt
//nhbin

constexpr U32 TEXTURE_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 AUDIO_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MATERIAL_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MESH_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 MODEL_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SHADER_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 SCENE_VERSION = MakeVersionNumber(0, 1, 0);
constexpr U32 FONT_VERSION = MakeVersionNumber(0, 1, 0);

Sampler* Resources::dummySampler;
Texture* Resources::dummyTexture;
Sampler* Resources::defaultSampler;
Shader* Resources::meshProgram;
Pipeline* Resources::renderPipeline;
Shader* Resources::uiProgram;
Pipeline* Resources::uiPipeline;

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
	meshProgram = CreateShader("shaders/Pbr.shader", 1, &pushConstant);
	meshProgram->AddDescriptor({ Renderer::materialBuffer.vkBuffer });

	uiProgram = CreateShader("shaders/UI.shader");

	PipelineInfo info{};
	info.name = "render_pipeline";
	info.shader = meshProgram;
	info.attachmentFinalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	renderPipeline = CreatePipeline(info);

	info.name = "ui_pipeline";
	info.shader = uiProgram;
	info.renderpass = renderPipeline->renderpass;
	uiPipeline = CreatePipeline(info);

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
	texture->handle = textures.GetHandle(info.name);

	Renderer::CreateTexture(texture, info.initialData);

	return texture;
}

Texture* Resources::CreateSwapchainTexture(VkImage image, VkFormat format, U8 index)
{
	String name{ "SwapchainTexture{}", index };

	Texture* texture = &textures.Request(name);

	*texture = {};

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

	texture->mipmaps[0] = texture->imageView;

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

	texture->width = width;
	texture->height = height;
	texture->depth = depth;

	Renderer::CreateTexture(texture, nullptr);

	Renderer::DestroyTextureInstant(&deleteTexture);

	return true;
}

Texture* Resources::LoadTexture(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Texture* texture = &textures.Request(path, handle);

	if (!texture->name.Blank()) { return texture; }

	*texture = {};

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		texture->name = path;
		texture->type = VK_IMAGE_TYPE_2D;
		texture->flags = 0;
		texture->depth = 1;
		texture->handle = textures.GetHandle(path);

		DataReader reader{ file };
		file.Close();

		if (!reader.Compare("NH Texture"))
		{
			Logger::Error("Asset '{}' Is Not A Nihility Texture!", path);
			textures.Remove(handle);
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		reader.Read(texture->width);
		reader.Read(texture->height);
		reader.Read(texture->format);
		reader.Read(texture->mipmapCount);
		texture->size = texture->width * texture->height * 4;

		if (!Renderer::CreateTexture(texture, reader.Pointer()))
		{
			Logger::Error("Failed To Create Texture: {}!", path);
			textures.Remove(path);
			return nullptr;
		}

		return texture;
	}

	Logger::Error("Failed To Find Or Open File: {}!", path);

	textures.Remove(path);
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
	renderpass->colorOperation = info.colorOperation;
	renderpass->depthOperation = info.depthOperation;
	renderpass->stencilOperation = info.stencilOperation;
	renderpass->clearCount = info.clearCount;

	for (U32 i = 0; i < info.clearCount; ++i)
	{
		renderpass->clears[i] = info.clears[i];
	}

	for (U32 i = 0; i < info.renderTargetCount; ++i)
	{
		renderpass->outputTextures[i] = info.outputTextures[i];
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

Model* Resources::LoadModel(const String& path)
{
	if (path.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Model* model = &models.Request(path, handle);

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

			materialIndices[i] = (U32)(Renderer::UploadToBuffer(Renderer::materialBuffer, &material, sizeof(Material)) / sizeof(Material));
		}

		U32 meshCount;
		reader.Read(meshCount);

		model->meshes.Resize(meshCount);

		for (U32 i = 0; i < meshCount; ++i)
		{
			DrawCall& draw = model->meshes[i];

			U32 verticesSize;
			reader.Read(verticesSize);
			draw.vertexOffset = (U32)(Renderer::UploadToBuffer(Renderer::vertexBuffer, reader.Pointer(), verticesSize) / sizeof(Vertex));
			reader.Seek(verticesSize);

			U32 indicesSize;
			reader.Read(indicesSize);
			draw.indexCount = indicesSize / sizeof(U32);
			draw.indexOffset = (U32)(Renderer::UploadToBuffer(Renderer::indexBuffer, reader.Pointer(), indicesSize) / sizeof(U32));
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

			U32 instanceOffset = (U32)(Renderer::UploadToBuffer(Renderer::instanceBuffer, draw.instances.Data(), sizeof(MeshInstance) * draw.instances.Size()) / sizeof(MeshInstance));
			Renderer::UploadDrawCall(draw.indexCount, draw.indexOffset, draw.vertexOffset, draw.instances.Size(), instanceOffset);
			++Renderer::meshDrawCount;
		}

		return model;
	}

	Logger::Error("Failed To Find Or Open File: {}", path);
	models.Remove(handle);
	return nullptr;
}

Skybox* Resources::LoadSkybox(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Skybox* skybox = &skyboxes.Request(name);

	if (!skybox->name.Blank()) { return skybox; }

	skybox->name = name;
	skybox->handle = skyboxes.GetHandle(name);

	return skybox;
}

Scene* Resources::CreateScene(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Scene* scene = &scenes.Request(name);

	if (!scene->name.Blank()) { return scene; }

	*scene = {};

	scene->name = name;
	scene->drawSkybox = false;

	scene->Create();

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

U8 Resources::MipmapCount(U16 width, U16 height)
{
	return (U8)DegreeOfTwo(Math::Min(width, height));
}

String Resources::UploadFont(const String& path)
{
	//TODO: 
	return {};
}

String Resources::UploadAudio(const String& path)
{
	//TODO: 
	return {};
}

String Resources::UploadTexture(const String& path)
{
	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		U32 fileSize = (U32)file.Size();
		U8* fileData = (U8*)malloc(fileSize);
		file.ReadCount(fileData, fileSize);
		file.Close();

		I32 width;
		I32 height;

		U8* textureData = stbi_load_from_memory(fileData, fileSize, &width, &height, nullptr, 4);
		free(fileData);

		if (!textureData)
		{
			Logger::Error("Failed To Convert Image!");
			return {};
		}

		String newPath = path.GetFileName().Surround("textures/", ".nhtex");

		file.Open(newPath, FILE_OPEN_RESOURCE_WRITE);

		file.Write("NH Texture");
		file.Write(TEXTURE_VERSION);

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(VK_FORMAT_R8G8B8A8_UNORM);
		file.Write(MipmapCount(width, height));
		file.Write(textureData, width * height * 4);
		free(textureData);

		file.Close();

		return Move(newPath);
	}

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

		file.Write((U16)width);
		file.Write((U16)height);
		file.Write(VK_FORMAT_R8G8B8A8_UNORM);
		file.Write(MipmapCount(width, height));
		file.Write(textureData, width * height * 4);
		free(textureData);

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
		bool success = false;

		if (path.CompareN("ktx", extIndex) || path.CompareN("ktx2", extIndex) || path.CompareN("ktx1", extIndex)) { success = LoadKTX(file); }

		//TODO: 
	}

	return {};
}

bool Resources::LoadKTX(File& file)
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

		KTXInfo info{};
		GetKTXInfo(header.internalFormat, info);

		file.Seek(header.keyValueDataSize);

		U32 elementCount = Math::Max(1u, header.arrayElementCount);
		U32 depth = Math::Max(1u, header.pixelDepth);
		U32 dataSize = (U32)(file.Size() - file.Pointer() - header.mipmapLevelCount * sizeof(U32));

		U8* data = (U8*)malloc(dataSize); //TODO: Go through Memory


		U32 faceLodSize;
		file.Read(faceLodSize);

		file.Read(data, faceLodSize * header.faceCount);

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

		U32 dataSize = (U32)level[0].uncompressedByteLength;
		U8* data = (U8*)malloc(dataSize); //TODO: Go through Memory

		file.Read(data, dataSize);

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
			model.textures[model.textureCount++] = UploadTexture(scene->GetEmbeddedTexture(texturePath.C_Str()));

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
	mesh.vertices = (Vertex*)malloc(mesh.verticesSize);

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
	mesh.indices = (U32*)malloc(mesh.indicesSize);

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
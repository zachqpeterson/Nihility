#include "Resources.hpp"

#include "Font.hpp"
#include "Platform\Audio.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Core\Logger.hpp"
#include "Core\DataReader.hpp"
#include "Math\Color.hpp"
#include "Rendering\Pipeline.hpp"
#include "Containers\Stack.hpp"

#include "External\Assimp\cimport.h"
#include "External\Assimp\scene.h"
#include "External\Assimp\postprocess.h"

#include "External\LunarG\glslang\Public\ShaderLang.h"
#include "External\LunarG\glslang\Include\intermediate.h"

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
Sampler* Resources::defaultPointSampler;
Sampler* Resources::defaultLinearSampler;
Pipeline* Resources::meshPipeline;
Pipeline* Resources::skyboxPipeline;
Pipeline* Resources::postProcessPipeline;
Renderpass* Resources::geometryRenderpass;
Renderpass* Resources::postProcessRenderpass;
Texture* Resources::geometryBuffer;
Texture* Resources::geometryDepth;
RenderGraph Resources::defaultRenderGraph;

Hashmap<String, Sampler>		Resources::samplers{ 32, {} };
Hashmap<String, Texture>		Resources::textures{ 512, {} };
Hashmap<String, Font>			Resources::fonts{ 32, {} };
Hashmap<String, AudioClip>		Resources::audioClips{ 512, {} };
Hashmap<String, Renderpass>		Resources::renderpasses{ 32, {} };
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

	SamplerInfo defaultSamplerInfo{};
	defaultSamplerInfo.SetName("default_point_sampler");
	defaultSamplerInfo.SetAddressModeUVW(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	defaultSamplerInfo.SetMinMagMip(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST);
	defaultPointSampler = Resources::CreateSampler(defaultSamplerInfo);

	defaultSamplerInfo.SetName("default_linear_sampler");
	defaultSamplerInfo.SetMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);
	defaultLinearSampler = Resources::CreateSampler(defaultSamplerInfo);

	TextureInfo textureInfo{};
	textureInfo.name = "geometry_buffer";
	textureInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureInfo.width = Settings::WindowWidth();
	textureInfo.height = Settings::WindowHeight();
	textureInfo.depth = 1;
	textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
	textureInfo.type = VK_IMAGE_TYPE_2D;
	geometryBuffer = Resources::CreateTexture(textureInfo);

	textureInfo.name = "geometry_depth";
	textureInfo.format = VK_FORMAT_D32_SFLOAT;
	textureInfo.flags = TEXTURE_FLAG_RENDER_TARGET;
	geometryDepth = Resources::CreateTexture(textureInfo);

	RenderpassInfo renderPassInfo{};
	renderPassInfo.name = "geometry_pass";
	renderPassInfo.AddRenderTarget(geometryBuffer);
	renderPassInfo.AddClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	renderPassInfo.SetDepthStencilTarget(geometryDepth);
	renderPassInfo.AddClearDepth(1.0f);

	geometryRenderpass = Resources::CreateRenderpass(renderPassInfo);

	VkPushConstantRange pushConstant{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(CameraData) };
	Shader* meshProgram = CreateShader("shaders/Pbr.nhshd", 1, &pushConstant);
	meshProgram->AddDescriptor({ Renderer::materialBuffer.vkBuffer });

	PipelineInfo info{};
	info.name = "mesh_pipeline";
	info.shader = meshProgram;
	info.attachmentFinalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	info.renderpass = geometryRenderpass;
	meshPipeline = CreatePipeline(info);

	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	info.name = "skybox_pipeline";
	info.shader = CreateShader("shaders/Skybox.nhshd", 1, &pushConstant);
	info.vertexBufferSize = sizeof(F32) * CountOf32(skyboxVertices);
	info.instanceBufferSize = sizeof(I32) * 128;
	info.indexBufferSize = sizeof(U32) * CountOf32(skyboxIndices);
	info.drawBufferSize = sizeof(VkDrawIndexedIndirectCommand);
	skyboxPipeline = CreatePipeline(info);

	skyboxPipeline->UploadVertices(sizeof(F32) * CountOf32(skyboxVertices), skyboxVertices);
	skyboxPipeline->UploadIndices(sizeof(U32) * CountOf32(skyboxIndices), skyboxIndices);

	pushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstant.size = sizeof(PostProcessData);
	info.shader = CreateShader("shaders/PostProcess.nhshd", 1, &pushConstant);
	info.name = "postprocess_pipeline";
	info.vertexBufferSize = 0;
	info.instanceBufferSize = 0;
	info.indexBufferSize = 0;
	info.drawBufferSize = 0;
	postProcessPipeline = CreatePipeline(info);

	Renderer::postProcessData.textureIndex = (U32)geometryBuffer->handle;

	defaultRenderGraph.AddPipeline(postProcessPipeline);
	defaultRenderGraph.AddPipeline(skyboxPipeline);
	defaultRenderGraph.AddPipeline(meshPipeline);

	Renderer::renderGraph = &defaultRenderGraph;

	return true;
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
	CleanupHashmap(fonts, nullptr);
	CleanupHashmap(audioClips, nullptr);
	CleanupHashmap(shaders, nullptr);
	CleanupHashmap(pipelines, nullptr);
	CleanupHashmap(models, nullptr);
	CleanupHashmap(skyboxes, nullptr);
	CleanupHashmap(scenes, nullptr);

	samplers.Destroy();
	textures.Destroy();
	descriptorSetLayouts.Destroy();
	renderpasses.Destroy();
	fonts.Destroy();
	audioClips.Destroy();
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

				Sampler* defaultSampler = Resources::AccessDefaultSampler(SAMPLER_TYPE_LINEAR);
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

void Resources::Resize()
{
	typename Hashmap<String, Renderpass>::Iterator end = renderpasses.end();
	for (auto it = renderpasses.begin(); it != end; ++it)
	{
		if (it.Valid() && !it->name.Blank()) { it->Resize(); }
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

Texture* Resources::CreateTexture(const TextureInfo& info)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Texture* texture = &textures.Request(info.name, handle);

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

	Renderer::CreateTexture(texture, info.initialData);

	return texture;
}

Texture* Resources::CreateSwapchainTexture(VkImage image, VkFormat format, U8 index)
{
	String name{ "SwapchainTexture{}", index };

	HashHandle handle;
	Texture* texture = &textures.Request(name, handle);

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

	if (vkCreateImageView(Renderer::device, &viewInfo, Renderer::allocationCallbacks, &texture->imageView) != VK_SUCCESS) { textures.Remove(handle); return nullptr; }

	texture->mipmaps[0] = texture->imageView;

	Renderer::SetResourceName(VK_OBJECT_TYPE_IMAGE_VIEW, (U64)texture->imageView, "Swapchain_ImageView");

	return texture;
}

Sampler* Resources::CreateSampler(const SamplerInfo& info)
{
	if (info.name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	HashHandle handle;
	Sampler* sampler = &samplers.Request(info.name, handle);

	if (!sampler->name.Blank()) { return sampler; }

	*sampler = {};

	sampler->addressModeU = info.addressModeU;
	sampler->addressModeV = info.addressModeV;
	sampler->addressModeW = info.addressModeW;
	sampler->minFilter = info.minFilter;
	sampler->magFilter = info.magFilter;
	sampler->mipFilter = info.mipFilter;
	sampler->name = info.name;
	sampler->handle = handle;

	Renderer::CreateSampler(sampler);

	return sampler;
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

	HashHandle handle;
	Renderpass* renderpass = &renderpasses.Request(info.name, handle);

	if (!renderpass->name.Blank()) { return renderpass; }

	*renderpass = {};

	renderpass->name = info.name;
	renderpass->renderTargetCount = (U8)info.renderTargetCount;
	renderpass->depthStencilTarget = info.depthStencilTarget;
	renderpass->handle = handle;
	renderpass->colorLoadOp = info.colorLoadOp;
	renderpass->depthLoadOp = info.depthLoadOp;
	renderpass->stencilLoadOp = info.stencilLoadOp;
	renderpass->clearCount = info.clearCount;
	renderpass->renderOrder = info.renderOrder;

	for (U32 i = 0; i < info.clearCount; ++i)
	{
		renderpass->clears[i] = info.clears[i];
	}

	for (U32 i = 0; i < info.renderTargetCount; ++i)
	{
		renderpass->renderTargets[i] = info.renderTargets[i];
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

	HashHandle handle;
	Pipeline* pipeline = &pipelines.Request(info.name, handle);

	if (!pipeline->name.Blank()) { return pipeline; }

	*pipeline = {};

	pipeline->name = info.name;
	pipeline->handle = handle;

	if (!pipeline->Create(info, specializationInfo))
	{
		pipelines.Remove(handle);
		pipeline->handle = U64_MAX;
	}

	return pipeline;
}

Scene* Resources::CreateScene(const String& name)
{
	if (name.Blank()) { Logger::Error("Resources Must Have Names!"); return nullptr; }

	Scene* scene = &scenes.Request(name);

	if (!scene->name.Blank()) { return scene; }

	*scene = {};

	scene->name = name;

	scene->Create();

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
	Font* font = &fonts.Request(path, handle);

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

		Texture* texture = &textures.Request(textureName, handle);
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

		if (!Renderer::CreateTexture(texture, reader.Pointer()))
		{
			Logger::Error("Failed To Create Texture: {}!", texture->name);
			fonts.Remove(font->handle);
			textures.Remove(handle);
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
	AudioClip* audioClip = &audioClips.Request(path, handle);

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
		texture->handle = handle;

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
			textures.Remove(handle);
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
	Skybox* skybox = &skyboxes.Request(path, handle);

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
			return nullptr;
		}

		reader.Seek(4); //Skip version number for now, there is only one

		U32 faceCount;
		U32 faceSize;

		reader.Read(faceCount);
		reader.Read(faceSize);

		String textureName = path.GetFileName().Appended("_texture");

		Texture* texture = &textures.Request(textureName, handle);
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

	Scene* scene = &scenes.Request(path);

	if (!scene->name.Blank()) { return scene; }

	scene->name = path;

	File file(path, FILE_OPEN_RESOURCE_READ);
	if (file.Opened())
	{
		I64 extIndex = path.LastIndexOf('.') + 1;

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
			scenes.Remove(path);
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

Sampler* Resources::AccessDefaultSampler(SamplerType type)
{
	if (type == SAMPLER_TYPE_LINEAR) { return defaultLinearSampler; }

	return defaultPointSampler;
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

String Resources::UploadTexture(const String& path)
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
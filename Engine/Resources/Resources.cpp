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
#define SCENES_PATH "scenes/"

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
	textures.Remove(name);

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
	glTF* gltf = &scenes.Request(name);

	if (!gltf->name.Blank()) { return gltf; }

	String path(SCENES_PATH, name);

	File file(path, FILE_OPEN_RESOURCE);
	if (file.Opened())
	{
		gltf->name = name;

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
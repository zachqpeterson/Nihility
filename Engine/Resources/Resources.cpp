#include "Resources.hpp"

#include "Core/Logger.hpp"
#include "Rendering/Renderer.hpp"
#include "Containers/Stack.hpp"
#include "Containers/Pair.hpp"

#include "vma/vk_mem_alloc.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

DescriptorSet Resources::dummySet;
DescriptorSet Resources::bindlessTexturesSet;
PipelineLayout Resources::spritePipelineLayout;
Shader Resources::spriteVertexShader;
Shader Resources::spriteFragmentShader;
Pipeline Resources::spritePipeline;
Buffer Resources::spriteVertexBuffer;
Buffer Resources::spriteIndexBuffer;
Buffer Resources::spriteInstanceBuffers[MaxSwapchainImages];
Vector<SpriteInstance> Resources::instances(10000);

ResourceRef<Texture> Resources::whiteTexture;
ResourceRef<Texture> Resources::placeholderTexture;

Hashmap<String, Resource<Texture>> Resources::textures(1024);

Queue<ResourceRef<Texture>> Resources::bindlessTexturesToUpdate(128);

bool Resources::Initialize()
{
	Logger::Trace("Initializing Resources...");

	whiteTexture = LoadTexture("textures/white.png");
	placeholderTexture = LoadTexture("textures/missing_texture.png");

	DescriptorBinding textureBinding{};
	textureBinding.type = BindingType::CombinedImageSampler;
	textureBinding.count = 1024;
	textureBinding.stages = ShaderStage::All;

	DescriptorBinding texture3DBinding{};
	texture3DBinding.type = BindingType::StorageImage;
	texture3DBinding.count = 1024;
	texture3DBinding.stages = ShaderStage::All;

	bindlessTexturesSet.Create({ textureBinding, texture3DBinding }, 10, true);

	dummySet.Create({});

	VkPushConstantRange pushConstant{};
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(GlobalPushConstant);

	spritePipelineLayout.Create({ dummySet, bindlessTexturesSet }, { pushConstant });

	spriteVertexShader.Create("shaders/sprite.vert.spv", ShaderStage::Vertex);
	spriteFragmentShader.Create("shaders/sprite.frag.spv", ShaderStage::Fragment);

	Vector<VkVertexInputBindingDescription> inputs = {
		{ 0, sizeof(SpriteVertex), VK_VERTEX_INPUT_RATE_VERTEX },
		{ 1, sizeof(SpriteInstance), VK_VERTEX_INPUT_RATE_INSTANCE}
	};

	Vector<VkVertexInputAttributeDescription> attributes = {
		{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, position) },
		{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteVertex, texcoord) },

		{ 2, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Transform, position) },
		{ 3, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Transform, scale) },
		{ 4, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Transform, rotation) },
		{ 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SpriteInstance, instColor) },
		{ 6, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, instTexcoord) },
		{ 7, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SpriteInstance, instTexcoordScale) },
		{ 8, 1, VK_FORMAT_R32_UINT, offsetof(SpriteInstance, textureIndex) },
	};

	spritePipeline.Create(spritePipelineLayout, { PolygonMode::Fill }, { spriteVertexShader, spriteFragmentShader }, inputs, attributes);

	spriteVertexBuffer.Create(BufferType::Vertex, sizeof(SpriteVertex) * 4);
	spriteIndexBuffer.Create(BufferType::Index, sizeof(U32) * 6);
	for (U32 i = 0; i < Renderer::swapchain.imageCount; ++i)
	{
		spriteInstanceBuffers[i].Create(BufferType::Vertex, sizeof(SpriteInstance) * instances.Capacity());
	}

	Vector2 position = Vector2::Zero;
	Vector2 uv = Vector2::Zero;

	SpriteVertex vertices[4] = {
		{ { -0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ { -0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ {  0.5f, -0.5f }, { 1.0f, 1.0f } }
	};

	U32 indices[6] = { 0, 1, 2, 2, 3, 0 };

	spriteVertexBuffer.UploadVertexData(vertices, sizeof(SpriteVertex) * 4, 0);
	spriteIndexBuffer.UploadIndexData(indices, sizeof(U32) * 6, 0);

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	DestroyResources(textures, Renderer::DestroyTexture);

	vkDeviceWaitIdle(Renderer::device);

	spriteVertexBuffer.Destroy();
	spriteIndexBuffer.Destroy();
	for (U32 i = 0; i < Renderer::swapchain.imageCount; ++i)
	{
		spriteInstanceBuffers[i].Destroy();
	}

	spritePipeline.Destroy();

	spriteVertexShader.Destroy();
	spriteFragmentShader.Destroy();

	spritePipelineLayout.Destroy();

	bindlessTexturesSet.Destroy();
	dummySet.Destroy();
}

void Resources::Update()
{
	if (bindlessTexturesToUpdate.Size())
	{
		Vector<VkWriteDescriptorSet> writes(bindlessTexturesToUpdate.Size());
		Vector<VkDescriptorImageInfo> textureData(bindlessTexturesToUpdate.Size());

		ResourceRef<Texture> texture;
		while (bindlessTexturesToUpdate.Pop(texture))
		{
			VkDescriptorImageInfo descriptorImageInfo{};
			descriptorImageInfo.sampler = texture->sampler;
			descriptorImageInfo.imageView = texture->imageView;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			descriptorWrite.pNext = nullptr;
			descriptorWrite.dstSet = bindlessTexturesSet;
			descriptorWrite.dstBinding = 10;
			descriptorWrite.dstArrayElement = (U32)texture.Handle();
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.pImageInfo = &textureData.Push(descriptorImageInfo);
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			writes.Push(descriptorWrite);
		}

		vkUpdateDescriptorSets(Renderer::device, (U32)writes.Size(), writes.Data(), 0, nullptr);
	}

	spriteInstanceBuffers[Renderer::frameIndex].UploadVertexData(instances.Data(), instances.Size() * sizeof(SpriteInstance), 0, Renderer::vertexInputFinished[Renderer::previousFrame]);
}

ResourceRef<Texture> Resources::LoadTexture(const String& path, const Sampler& sampler, bool generateMipmaps, bool flipImage)
{
	if (path.Blank()) { Logger::Error("Blank Path Passed To LoadTexture!"); return nullptr; }

	U64 handle;
	Resource<Texture>& texture = *textures.Request(path, handle);

	if (!texture->name.Blank()) { return { texture, handle }; }

	I32 texWidth;
	I32 texHeight;
	I32 numberOfChannels;

	File file{ path, FILE_OPEN_RESOURCE_READ };
	if (file.Opened())
	{
		String data = file.ReadAll();

		stbi_set_flip_vertically_on_load(flipImage);
		U8* textureData = stbi_load_from_memory((U8*)data.Data(), (I32)data.Size(), &texWidth, &texHeight, &numberOfChannels, STBI_rgb_alpha);

		if (!textureData)
		{
			Logger::Error("Failed To Load Texture Data!");
			stbi_image_free(textureData);
			return nullptr;
		}

		texture->name = path; //TODO: Maybe get file name
		texture->width = texWidth;
		texture->height = texHeight;
		texture->depth = 1;
		texture->size = texWidth * texHeight * 4;
		texture->mipmapLevels = generateMipmaps ? (U32)Math::Floor(Math::Log2((F32)Math::Max(texWidth, texHeight))) + 1 : 1;

		if (!Renderer::UploadTexture(texture, textureData, sampler))
		{
			stbi_image_free(textureData);
			Logger::Error("Failed To Upload Texture Data!");
			return nullptr;
		}

		stbi_image_free(textureData);

		return { texture, handle };
	}

	Logger::Error("Failed To Open File: ", path, '!');
	return nullptr;
}

ResourceRef<Texture>& Resources::WhiteTexture()
{
	return whiteTexture;
}

ResourceRef<Texture>& Resources::PlaceholderTexture()
{
	return placeholderTexture;
}

void Resources::CreateSprite(ResourceRef<Texture>& texture, const Transform& transform, const Vector4& color, const Vector2& textureCoord, const Vector2& textureScale)
{
	if (instances.Full())
	{
		Logger::Error("Max Instances Reached!");
		return;
	}

	SpriteInstance instance{};
	instance.transform = transform;
	instance.instColor = color;
	instance.instTexcoord = textureCoord;
	instance.instTexcoordScale = textureScale;
	instance.textureIndex = texture.Handle();

	instances.Push(instance);

	if (!texture->inBindless)
	{
		texture->inBindless = true;

		bindlessTexturesToUpdate.Push(texture);
	}
}
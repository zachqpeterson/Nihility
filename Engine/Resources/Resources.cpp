#include "Resources.hpp"

#include "Core/Logger.hpp"
#include "Rendering/Renderer.hpp"
#include "Containers/Stack.hpp"
#include "Containers/Pair.hpp"

#include "tracy/Tracy.hpp"
#include "vma/vk_mem_alloc.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

DescriptorSet Resources::dummySet;
DescriptorSet Resources::bindlessTexturesSet;

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

	return true;
}

void Resources::Shutdown()
{
	Logger::Trace("Cleaning Up Resources...");

	DestroyResources(textures, Renderer::DestroyTexture);

	vkDeviceWaitIdle(Renderer::device);

	bindlessTexturesSet.Destroy();
	dummySet.Destroy();
}

void Resources::Update()
{
	ZoneScopedN("ResourcesUpdate");

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

		ResourceRef<Texture> textureRef = { texture, handle };

		bindlessTexturesToUpdate.Push(textureRef);

		return textureRef;
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

const DescriptorSet& Resources::DummyDescriptorSet()
{
	return dummySet;
}

const DescriptorSet& Resources::BindlessTexturesDescriptorSet()
{
	return bindlessTexturesSet;
}

#include "Texture.hpp"

#include "Rendering/VulkanInclude.hpp"
#include "Rendering/Renderer.hpp"

bool Sampler::Create()
{
	if (vkSampler)
	{
		vkDestroySampler(Renderer::device, vkSampler, Renderer::allocationCallbacks);
	}

	VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.magFilter = (VkFilter)filterMode;
	samplerInfo.minFilter = (VkFilter)filterMode;
	samplerInfo.addressModeU = (VkSamplerAddressMode)edgeSampleMode;
	samplerInfo.addressModeV = (VkSamplerAddressMode)edgeSampleMode;
	samplerInfo.addressModeW = (VkSamplerAddressMode)edgeSampleMode;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = (VkBorderColor)borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = (VkSamplerMipmapMode)mipMapSampleMode;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkValidateR(vkCreateSampler(Renderer::device, &samplerInfo, Renderer::allocationCallbacks, &vkSampler));

	return true;
}

void Sampler::Destroy()
{
	if (vkSampler)
	{
		vkDestroySampler(Renderer::device, vkSampler, Renderer::allocationCallbacks);
		vkSampler = nullptr;
	}
}

Sampler::operator VkSampler () const { return vkSampler; }
VkSampler* Sampler::operator&() const { return (VkSampler*)&vkSampler; }

Texture::Texture(const Texture& other) : name(other.name), id(other.id), width(other.width), height(other.height), depth(other.depth), size(other.size), mipmapLevels(other.mipmapLevels),
format(other.format), sampler(other.sampler), image(other.image), imageView(other.imageView), allocation(other.allocation)
{}
Texture::Texture(Texture&& other) : name(Move(other.name)), id(other.id), width(other.width), height(other.height), depth(other.depth), size(other.size), mipmapLevels(other.mipmapLevels),
format(other.format), sampler(other.sampler), image(other.image), imageView(other.imageView), allocation(other.allocation)
{}
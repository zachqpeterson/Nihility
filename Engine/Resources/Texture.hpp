#pragma once

#include "Defines.hpp"

#include "Rendering/VulkanInclude.hpp"

struct VmaAllocation_T;

struct NH_API Texture
{
	const String& Name() const { return name; }
	const U32& Width() const { return width; }
	const U32& Height() const { return height; }
	const U64& Size() const { return size; }
	const U8& MipmapLevels() const { return mipmapLevels; }

private:
	String name;
	U32 width;
	U32 height;
	U32	depth;
	U64	size;
	U8 mipmapLevels;

	VkImage image = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VmaAllocation_T* allocation = nullptr;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	friend class Renderer;
	friend class Resources;
};
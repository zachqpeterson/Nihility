#pragma once

#include "Defines.hpp"

#include "Resources/Texture.hpp"

enum VkFormat;
struct VmaAllocation_T;
struct VkCommandBuffer_T;
struct VkImage_T;
struct VkImageView_T;
struct VkSampler_T;

struct NH_API RenderTarget
{
public:
	void Create(U32 width, U32 height, VkFormat format, Sampler sampler = {}, bool storage = false, bool ignoreAspect = false);
	void Recreate(U32 width, U32 height, VkFormat format, Sampler sampler = {}, bool storage = false, bool ignoreAspect = false);
	void Destroy();

	void TransitionToStorage(VkCommandBuffer_T* commandBuffer) const;
	void TransitionToSampled(VkCommandBuffer_T* commandBuffer) const;

	void StartRender(VkCommandBuffer_T* commandBuffer) const;
	void EndRender(VkCommandBuffer_T* commandBuffer) const;

	U32 Width() const;
	U32 Height() const;
	U32 Id() const;

private:
	U32 width = 0;
	U32 height = 0;
	U32	id = 0;
	bool storage = false;
	bool ignoreAspect = false;

	bool externalSampler = false;
	VkSampler_T* vkSampler = nullptr;
	VkImage_T* image = nullptr;
	VkImageView_T* imageView = nullptr;
	VmaAllocation_T* allocation = nullptr;

	friend class Renderer;
};
#pragma once

#include "Defines.hpp"

#include "Rendering/VulkanInclude.hpp"

struct VmaAllocation_T;

enum class NH_API FilterMode
{
	Point = 0,
	Linear = 1,
	Cubic = 1000015000
};

enum class NH_API MipMapSampleMode
{
	Single = 0,
	Multiple = 1,
};

enum class NH_API EdgeSampleMode
{
	Repeat = 0,
	MirroredRepeat = 1,
	ClampToEdge = 2,
	ClampToBorder = 3,
	MirrorClampToEdge = 4,
};

enum class NH_API BorderColor
{
	Clear = 1,
	Black = 3,
	White = 5,
};

struct NH_API Sampler
{
	FilterMode filterMode = FilterMode::Linear;
	MipMapSampleMode mipMapSampleMode = MipMapSampleMode::Multiple;
	EdgeSampleMode edgeSampleMode = EdgeSampleMode::ClampToEdge;
	BorderColor borderColor = BorderColor::Clear;
};

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
	bool inBindless = false;

	VkImage image = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
	VmaAllocation_T* allocation = nullptr;

	friend class Renderer;
	friend class Resources;
	friend struct FrameBuffer;
};
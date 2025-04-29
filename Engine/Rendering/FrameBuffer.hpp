#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Containers/Vector.hpp"

struct FrameBuffer
{
private:
	bool Create();
	void Destroy();

	Vector<VkImage> vkImages;
	Vector<VkImageView> vkImageViews;
	Vector<VkFramebuffer> vkFramebuffers;

	friend class Renderer;
};
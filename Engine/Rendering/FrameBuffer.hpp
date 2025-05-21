#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "Containers/Vector.hpp"

struct VkFramebuffer_T;

struct FrameBuffer
{
private:
	bool Create();
	void Destroy();

	operator VkFramebuffer_T*() const;

	Vector<VkFramebuffer_T*> vkFramebuffers;

	friend class Renderer;
	friend struct CommandBuffer;
};
#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

struct Renderpass
{
private:
	bool Create();
	void Destroy();

	operator VkRenderPass() const;

	VkRenderPass vkRenderpass;

	friend class Renderer;
	friend struct Pipeline;
	friend struct FrameBuffer;
};
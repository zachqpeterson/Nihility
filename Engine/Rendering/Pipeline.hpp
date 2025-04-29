#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "PipelineLayout.hpp"
#include "Shader.hpp"

#include "Containers/Vector.hpp"

struct Pipeline
{
private:
	bool Create(const PipelineLayout& layout, Vector<Shader> shaders);
	void Destroy();

	operator VkPipeline() const;

	VkPipeline vkPipeline;

	friend class Renderer;
};
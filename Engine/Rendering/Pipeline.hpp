#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

#include "PipelineLayout.hpp"
#include "Shader.hpp"

#include "Containers/Vector.hpp"

enum NH_API PolygonMode
{
	Fill = 0,
	Line = 1,
	Point = 2
};

struct NH_API PipelineSettings
{
	PolygonMode polygonMode = PolygonMode::Fill;
};

struct NH_API Pipeline
{
	bool Create(const PipelineLayout& layout, const PipelineSettings& settings, const Vector<Shader>& shaders,
		const Vector<VkVertexInputBindingDescription>& bindings, const Vector<VkVertexInputAttributeDescription>& attributes);
	void Destroy();

	operator VkPipeline() const;

private:
	VkPipeline vkPipeline;

	friend class Renderer;
};
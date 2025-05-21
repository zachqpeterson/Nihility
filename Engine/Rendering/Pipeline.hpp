#pragma once

#include "Defines.hpp"

#include "PipelineLayout.hpp"
#include "Shader.hpp"

#include "Containers/Vector.hpp"

enum class NH_API PolygonMode
{
	Fill = 0,
	Line = 1,
	Point = 2
};

enum class NH_API BindPoint
{
	Graphics = 0,
	Compute = 1,
	RayTracing = 1000165000
};

struct NH_API PipelineSettings
{
	PolygonMode polygonMode = PolygonMode::Fill;
	BindPoint bindPoint = BindPoint::Graphics;
};

struct VkPipeline_T;
struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

struct NH_API Pipeline
{
	bool Create(const PipelineLayout& layout, const PipelineSettings& settings, const Vector<Shader>& shaders,
		const Vector<VkVertexInputBindingDescription>& bindings, const Vector<VkVertexInputAttributeDescription>& attributes);
	void Destroy();

	operator VkPipeline_T* () const;

private:
	VkPipeline_T* vkPipeline;
	BindPoint bindPoint;

	friend class Renderer;
	friend struct CommandBuffer;
};
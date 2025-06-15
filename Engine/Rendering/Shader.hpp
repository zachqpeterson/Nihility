#pragma once

#include "Defines.hpp"

#include "Containers/String.hpp"

struct VkShaderModule_T;

enum class NH_API ShaderStage
{
	Vertex = 0x00000001,
	TessellationControl = 0x00000002,
	TessellationEvaluation = 0x00000004,
	Geometry = 0x00000008,
	Fragment = 0x00000010,
	Compute = 0x00000020,
	AllGraphics = 0x0000001F,
	All = 0x7FFFFFFF,
	Raygen = 0x00000100,
	AnyHit = 0x00000200,
	ClosestHit = 0x00000400,
	Miss = 0x00000800,
	Intersection = 0x00001000,
	Callable = 0x00002000,
	Task = 0x00000040,
	Mesh = 0x00000080
};

struct NH_API Shader
{
	bool Create(const String& path, ShaderStage type, const StringView& entryPoint = "main");
	void Destroy();

	operator VkShaderModule_T* () const;

private:
	ShaderStage type;
	StringView entryPoint;
	VkShaderModule_T* vkShaderModule;

	friend class Renderer;
	friend struct Pipeline;
};
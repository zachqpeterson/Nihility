#pragma once

#include "Defines.hpp"

#include "VulkanInclude.hpp"

struct NH_API Shader
{
	bool Create(const String& path, ShaderStage type);
	void Destroy();

private:
	ShaderStage type;
	VkShaderModule vkShaderModule;

	friend class Renderer;
	friend struct Pipeline;
};
#include "Shader.hpp"

#include "Renderer.hpp"

bool Shader::Create(const String& path, ShaderStage type)
{
	this->type = type;

	File file(path, FILE_OPEN_RESOURCE_READ);
	String data = file.ReadAll();

	VkShaderModuleCreateInfo shaderCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = data.Size();
	shaderCreateInfo.pCode = (U32*)data.Data();

	VkValidateFR(vkCreateShaderModule(Renderer::device, &shaderCreateInfo, Renderer::allocationCallbacks, &vkShaderModule));

	return true;
}

void Shader::Destroy()
{
	if (vkShaderModule)
	{
		vkDestroyShaderModule(Renderer::device, vkShaderModule, Renderer::allocationCallbacks);
	}
}
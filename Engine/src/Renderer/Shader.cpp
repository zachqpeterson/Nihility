#include "Shader.hpp"

#include "Core/Logger.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Resources/Resources.hpp"
#include "Renderer/Camera.hpp"

void Shader::Destroy()
{
	RendererFrontend::DestroyShader(this);

	for (TextureMap* map : globalTextureMaps)
	{
		if (map) { RendererFrontend::ReleaseTextureMapResources(*map); }
	}

	for (String& s : stageFilenames)
	{
		s.Destroy();
	}

	stageFilenames.Destroy();
	stages.Destroy();
	name.Destroy();
}

void Shader::AddAttribute(Attribute attribute)
{
	attributeStride += attribute.size;
	attributes.Push(attribute);
}

bool Shader::AddUniform(Uniform uniform)
{
	if (uniform.name.Blank())
	{
		Logger::Error("Uniform name can't be blank!");
		return false;
	}

	if (uniform.setIndex == SHADER_SCOPE_INSTANCE && !useInstances)
	{
		Logger::Error("Shader cannot add an instance uniform when useInstances is false.");
		return false;
	}

	if (uniform.setIndex == SHADER_SCOPE_GLOBAL)
	{
		if (uniform.fieldType == FIELD_TYPE_SAMPLER)
		{
			uniform.location = (U16)globalTextureMaps.Size();
			globalTextureMaps.Push({});
		}

		uniform.offset = globalUboSize;
		globalUboSize += uniform.size;
	}
	else if (uniform.setIndex == SHADER_SCOPE_INSTANCE)
	{
		if (uniform.fieldType == FIELD_TYPE_SAMPLER)
		{
			uniform.location = instanceTextureCount;
			++instanceTextureCount;
		}

		uniform.offset = instanceUboSize;
		instanceUboSize += uniform.size;
	}

	uniforms[uniform.setIndex].Push(uniform);

	return true;
}

bool Shader::AddPushConstant(PushConstant pushConstant)
{
	if (!useLocals)
	{
		Logger::Error("Shader cannot add a local push constant when useLocals is false.");
		return false;
	}

	Range r = AlignRange(pushConstantSize, pushConstant.size, 4ull);
	pushConstantRanges.Push(r);
	pushConstant.offset = (U32)r.offset;
	pushConstantSize += r.size;

	pushConstants.Push(pushConstant);

	return true;
}

void Shader::ApplyMaterialLocals(void* pushes)
{
	if (useLocals)
	{
		for (PushConstant& pushConstant : pushConstants)
		{
			RendererFrontend::SetPushConstant(this, pushConstant, (U8*)pushes + pushConstant.offset);
		}
	}
}
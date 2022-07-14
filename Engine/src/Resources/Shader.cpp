#include "Shader.hpp"

#include "Core/Logger.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Resources/Resources.hpp"
#include "Renderer/Camera.hpp"

void Shader::Destroy()
{
	RendererFrontend::DestroyRenderpass(renderpass);
	RendererFrontend::DestroyShader(this);

	for (TextureMap* map : globalTextureMaps)
	{
		RendererFrontend::ReleaseTextureMapResources(*map);
	}

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
		if (uniform.type == FIELD_TYPE_SAMPLER)
		{
			uniform.location = globalTextureMaps.Size();

			TextureMap defaultMap = {};
			defaultMap.filterMagnify = TEXTURE_FILTER_MODE_NEAREST;
			defaultMap.filterMinify = TEXTURE_FILTER_MODE_NEAREST;
			defaultMap.repeatU = defaultMap.repeatV = defaultMap.repeatW = TEXTURE_REPEAT_REPEAT;

			if (!RendererFrontend::AcquireTextureMapResources(defaultMap))
			{
				Logger::Error("Failed to acquire resources for global texture map during shader creation.");
				return false;
			}

			TextureMap* map = (TextureMap*)Memory::Allocate(sizeof(TextureMap), MEMORY_TAG_RENDERER);
			*map = defaultMap;
			map->texture = Resources::DefaultTexture();
			globalTextureMaps.Push(map);
		}

		uniform.offset = globalUboSize;
		globalUboSize += uniform.size;
	}
	else if (uniform.setIndex == SHADER_SCOPE_INSTANCE)
	{
		if (uniform.type == FIELD_TYPE_SAMPLER)
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

bool Shader::ApplyGlobals(Material* material, Camera* camera)
{
	for (Uniform& uniform : uniforms[SHADER_SCOPE_GLOBAL])
	{
		U32 mode = 0;
		if (uniform.name == "projection")
		{
			if (!RendererFrontend::SetUniform(this, uniform, camera->Projection().Data()))
			{
				Logger::Error("Failed to set uniform");
				return false;
			}
		}
		else if (uniform.name == "view")
		{
			if (!RendererFrontend::SetUniform(this, uniform, camera->View().Data()))
			{
				Logger::Error("Failed to set uniform");
				return false;
			}
		}
		else if (uniform.name == "ambientColor")
		{
			if (!RendererFrontend::SetUniform(this, uniform, camera->AmbientColor().Data()))
			{
				Logger::Error("Failed to set uniform");
				return false;
			}
		}
		else if (uniform.type == FIELD_TYPE_SAMPLER)
		{
			if (!RendererFrontend::SetUniform(this, uniform, &material->globalTextureMaps[uniform.location]))
			{
				Logger::Error("Failed to set uniform");
				return false;
			}
		}
		else if (uniform.name == "viewPosition")
		{
			if (!RendererFrontend::SetUniform(this, uniform, camera->Position().Data()))
			{
				Logger::Error("Failed to set uniform");
				return false;
			}
		}
		else if (uniform.name == "mode")
		{
			if (!RendererFrontend::SetUniform(this, uniform, &mode))
			{
				Logger::Error("Failed to set uniform");
				return false;
			}
		}
	}

	return RendererFrontend::ApplyShaderGlobals(this);
}

bool Shader::ApplyMaterialInstances(Material& material, bool needsUpdate)
{
	if (!useInstances) { return true; }

	boundInstanceId = material.instance;

	if (needsUpdate)
	{
		for (Uniform& uniform : uniforms[SHADER_SCOPE_INSTANCE])
		{
			if (uniform.name == "diffuseColor")
			{
				if (!RendererFrontend::SetUniform(this, uniform, material.diffuseColor.Data()))
				{
					Logger::Error("Failed to set uniform");
					return false;
				}
			}
			else if (uniform.type == FIELD_TYPE_SAMPLER)
			{
				if (!RendererFrontend::SetUniform(this, uniform, &material.instanceTextureMaps[uniform.location]))
				{
					Logger::Error("Failed to set uniform");
					return false;
				}
			}
			else if (uniform.name == "shininess")
			{
				if (!RendererFrontend::SetUniform(this, uniform, &material.shininess))
				{
					Logger::Error("Failed to set uniform");
					return false;
				}
			}
		}
	}

	return RendererFrontend::ApplyShaderInstance(this, needsUpdate);
}

bool Shader::ApplyMaterialLocals(const Matrix4& model)
{
	if (!useLocals) { return true; }

	for (PushConstant& pushConstant : pushConstants)
	{
		if (pushConstant.name == "model" && !RendererFrontend::SetPushConstant(this, pushConstant, &model))
		{
			Logger::Error("Failed to set push constant");
			return false;
		}
	}

	return true;
}
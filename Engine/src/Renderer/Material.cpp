#include "Material.hpp"

#include "Shader.hpp"
#include "Camera.hpp"
#include "RendererFrontend.hpp"

#include "Resources/Resources.hpp"

void Material::ApplyGlobals(Camera* camera)
{
	for (Uniform& uniform : shader->uniforms[SHADER_SCOPE_GLOBAL])
	{
		switch (uniform.uniformType)
		{
		case UNIFORM_TYPE_CAMERA_PROJECTION: RendererFrontend::SetGlobalUniform(shader, uniform, camera->Projection().Data()); break;
		case UNIFORM_TYPE_CAMERA_VIEW: RendererFrontend::SetGlobalUniform(shader, uniform, camera->View().Data()); break;
		case UNIFORM_TYPE_CAMERA_POSITION: RendererFrontend::SetGlobalUniform(shader, uniform, camera->Position().Data()); break;
		case UNIFORM_TYPE_CAMERA_COLOR: RendererFrontend::SetGlobalUniform(shader, uniform, camera->AmbientColor().Data()); break;
		case UNIFORM_TYPE_TEXTURE: RendererFrontend::SetGlobalUniform(shader, uniform, &globalTextureMaps[uniform.location]); break;
		default: break;
		}
	}

	RendererFrontend::ApplyShaderGlobals(shader);
}

void Material::ApplyInstances(bool update)
{
	if (shader->useInstances)
	{
		shader->boundInstanceId = instance;

		if (update)
		{
			for (Uniform& uniform : shader->uniforms[SHADER_SCOPE_INSTANCE])
			{
				switch (uniform.uniformType)
				{
				case UNIFORM_TYPE_DIFFUSE_COLOR: RendererFrontend::SetInstanceUniform(shader, uniform, diffuseColor.Data()); break;
				case UNIFORM_TYPE_SHININESS: RendererFrontend::SetInstanceUniform(shader, uniform, &shininess); break;
				case UNIFORM_TYPE_TEXTURE: RendererFrontend::SetInstanceUniform(shader, uniform, &instanceTextureMaps[uniform.location]); break;
				default: break;
				}
			}
		}

		RendererFrontend::ApplyShaderInstance(shader, update);
	}
}
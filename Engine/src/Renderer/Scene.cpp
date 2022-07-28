#include "Scene.hpp"

#include "Camera.hpp"
#include "RendererFrontend.hpp"
#include "RendererDefines.hpp"

#include "Resources/Resources.hpp"
#include "Core/Settings.hpp"

void Scene::Create(CameraType cameraType)
{
	for (Material* material : Resources::GetMaterials())
	{
		MaterialList list{};
		list.material = material;
		meshes.Push(list);
	}

	if (cameraType == CAMERA_TYPE_PERSPECTIVE)
	{
		camera = new Camera(45.0f, 0.1f, 1000.0f, { 0.0f, 0.0f, 10.0f });
	}
	else
	{
		if (Settings::WindowWidth > (Settings::WindowHeight * 1.77777777778f))
		{
			F32 camHeight = 0.9375f;
			F32 camWidth = camHeight * ((F32)Settings::WindowWidth / (F32)Settings::WindowHeight);

			camera = new Camera({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f, { 0.0f, 0.0f, 10.0f });
		}
		else
		{
			F32 camWidth = 1.66666666667f;
			F32 camHeight = camWidth * ((F32)Settings::WindowHeight / (F32)Settings::WindowWidth);

			camera = new Camera({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f, { 0.0f, 0.0f, 10.0f });
		}
	}
}

void Scene::Destroy()
{

}

void Scene::OnResize()
{
	//TODO: Configure
	if (Settings::WindowWidth > (Settings::WindowHeight * 1.77777777778f))
	{
		F32 camHeight = 0.9375f;
		F32 camWidth = camHeight * ((F32)Settings::WindowWidth / (F32)Settings::WindowHeight);

		camera->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);
	}
	else
	{
		F32 camWidth = 1.66666666667f;
		F32 camHeight = camWidth * ((F32)Settings::WindowHeight / (F32)Settings::WindowWidth);

		camera->ChangeProjection({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f);
	}
}

bool Scene::OnRender(U64 frameNumber, U64 renderTargetIndex)
{
	for (GameObject2D* go : gameObjects)
	{
		const Matrix4& model = go->transform ? go->transform->World() : Matrix4::IDENTITY;

		for (Mesh* m : go->model->meshes)
		{
			MeshRenderData data;
			data.mesh = m;
			data.model = model;

			meshes[m->material.id].renderData.PushBack(data);
		}
	}

	String lastShaderName;

	for (MaterialList& list : meshes)
	{
		if (list.material->shader->name != lastShaderName)
		{
			if (!lastShaderName.Blank())
			{
				if (!RendererFrontend::EndRenderpass(list.material->shader->renderpass))
				{
					Logger::Error("renderpass '{}' failed to end.", list.material->shader->renderpass->name);
					return false;
				}
			}

			lastShaderName = list.material->shader->name;

			if (!RendererFrontend::BeginRenderpass(list.material->shader->renderpass))
			{
				Logger::Error("renderpass '{}' failed to start.", list.material->shader->renderpass->name);
				return false;
			}

			if (list.renderData.Size())
			{
				if (!RendererFrontend::UseShader(list.material->shader))
				{
					Logger::Error("Failed to use material shader. Render frame failed.");
					return false;
				}

				if (!list.material->shader->ApplyGlobals(list.material, camera))
				{
					Logger::Error("Failed to use apply globals for material shader. Render frame failed.");
					return false;
				}
			}
		}

		U64 size = list.renderData.Size();

		for (U64 i = 0; i < size; ++i)
		{
			MeshRenderData data = list.renderData.PopFront();
			Material& material = data.mesh->material;

			bool needsUpdate = material.renderFrameNumber != frameNumber;

			if (!material.shader->ApplyMaterialInstances(material, needsUpdate))
			{
				Logger::Warn("Failed to apply material '{}'. Skipping draw.", material.name);
				continue;
			}

			material.renderFrameNumber = frameNumber;

			if (!material.shader->ApplyMaterialLocals(data.model))
			{
				Logger::Warn("Failed to apply material '{}'. Skipping draw.", material.name);
				continue;
			}

			RendererFrontend::DrawMesh(data);
		}
	}

	if (!RendererFrontend::EndRenderpass(meshes.Back().material->shader->renderpass))
	{
		Logger::Error("renderpass '{}' failed to end.", meshes.Back().material->shader->renderpass->name);
		return false;
	}

	return true;
}

void Scene::DrawGameObject(GameObject2D* gameObject)
{
	if (gameObject->model)
	{
		gameObjects.PushBack(gameObject);
	}
}
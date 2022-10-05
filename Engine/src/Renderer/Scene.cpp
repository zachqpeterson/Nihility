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
		F32 camHeight = 0.9375f;
		F32 camWidth = 1.66666666667f;

		camera = new Camera({ -camWidth, camWidth, -camHeight, camHeight }, 0.1f, 1000.0f, { 0.0f, 0.0f, 10.0f });
	}
}

void Scene::Destroy()
{

}

void Scene::OnResize()
{

}

bool Scene::OnRender(U64 frameNumber, U64 renderTargetIndex)
{
	for (GameObject2D* go : gameObjects)
	{
		if (go->enabled)
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
	}

	for (Model* model : models)
	{
		for (Mesh* m : model->meshes)
		{
			MeshRenderData data;
			data.mesh = m;
			data.model = Matrix4::IDENTITY;

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
			MeshRenderData&& dataTemp = Move(list.renderData.PopFront());
			MeshRenderData data = dataTemp;
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
	if (gameObject && gameObject->model)
	{
		gameObjects.PushBack(gameObject);
	}
}

void Scene::UndrawGameObject(GameObject2D* gameObject)
{
	gameObjects.Remove(gameObject);
}

void Scene::DrawModel(Model* model)
{
	if (model)
	{
		models.PushBack(model);
	}
}

void Scene::UndrawModel(Model* model)
{
	models.Remove(model);
}

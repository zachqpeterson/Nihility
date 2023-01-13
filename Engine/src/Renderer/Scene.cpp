#include "Scene.hpp"

#include "Camera.hpp"
#include "RendererFrontend.hpp"
#include "RendererDefines.hpp"
#include "Material.hpp"

#include "Resources/Resources.hpp"
#include "Core/Settings.hpp"
#include "Core/Time.hpp"

void Scene::Create(CameraType cameraType)
{
	for (Material* material : Resources::GetMaterials())
	{
		MaterialList list{};
		list.material = material;
		list.meshCount = 0;
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

bool Scene::OnRender(U64 frameNumber, U64 renderTargetIndex)
{
	Timer timer{};
	for (MaterialList& ml : meshes)
	{
		if (ml.renderData.Capacity() < ml.meshCount) { ml.renderData.Reserve(ml.meshCount); }
	}

	for (GameObject2D* go : gameObjects)
	{
		if (go->enabled && go->model)
		{
			const Matrix4& model = go->transform ? go->transform->World() : Matrix4::IDENTITY;

			for (Mesh* m : go->model->meshes)
			{
				MeshRenderData data;
				data.mesh = m;
				data.model = model;

				meshes[m->material.id].renderData.Push(data);
			}
		}
	}

	for (Model* model : models)
	{
		for (Mesh* m : model->meshes)
		{
			meshes[m->material.id].renderData.Push({ Matrix4::IDENTITY, m });
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
				RendererFrontend::UseShader(list.material->shader);
				list.material->ApplyGlobals(camera);
			}
		}

		U64 size = list.renderData.Size();

		if (list.renderData.Size() && !list.material->shader->useInstances && !list.material->shader->useLocals)
		{
			for (MeshRenderData& data : list.renderData)
			{
				RendererFrontend::DrawMesh(data);
			}

			list.renderData.Clear();
		}
		else
		{
			for (U64 i = 0; i < size; ++i)
			{
				MeshRenderData& data = list.renderData[i];

				if (data.mesh)
				{
					Material& material = data.mesh->material;

					material.ApplyInstances(material.renderFrameNumber != frameNumber);

					material.renderFrameNumber = frameNumber;

					material.shader->ApplyMaterialLocals(data.mesh->pushConstant);

					RendererFrontend::DrawMesh(data);
				}
			}

			list.renderData.Clear();
		}
	}

	//Logger::Debug(timer.CurrentTime());
	if (!RendererFrontend::EndRenderpass(meshes.Back().material->shader->renderpass))
	{
		Logger::Error("renderpass '{}' failed to end.", meshes.Back().material->shader->renderpass->name);
		return false;
	}

	return true;
}

void Scene::DrawGameObject(GameObject2D* gameObject)
{
	gameObjects.PushFront(gameObject);

	if (gameObject->model)
	{
		for (Mesh* mesh : gameObject->model->meshes)
		{
			++meshes[mesh->material.id].meshCount;
		}
	}
}

void Scene::UndrawGameObject(GameObject2D* gameObject)
{
	gameObjects.Remove(gameObject);

	if (gameObject->model)
	{
		for (Mesh* mesh : gameObject->model->meshes)
		{
			--meshes[mesh->material.id].meshCount;
		}
	}
}

void Scene::DrawModel(Model* model)
{
	if (model)
	{
		models.PushFront(model);
		for (Mesh* mesh : model->meshes)
		{
			++meshes[mesh->material.id].meshCount;
		}
	}
}

void Scene::UndrawModel(Model* model)
{
	if (model)
	{
		models.Remove(model);
		for (Mesh* mesh : model->meshes)
		{
			--meshes[mesh->material.id].meshCount;
		}
	}
}

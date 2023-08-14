#include "Scene.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"

Scene::~Scene() { Destroy(); }

void Scene::Destroy()
{
	name.Destroy();
}

void Scene::Update()
{
	if (Settings::Resized())
	{
		camera.SetAspectRatio((F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());
	}

	camera.Update();

	//TODO: Upload Camera

	//
}

void Scene::AddModel(Model* model)
{
	models.Push(model);

	//TODO: Upload model

	for (U32 i = 0; i < model->meshCount; ++i)
	{
		Mesh* mesh = model->meshes[i];
		Material* material = mesh->material;

		MeshData data{};
		data.model = {};
		data.meshIndex = mesh->handle;
		data.vertexOffset = mesh->vertexOffset;
		data.meshletVisibilityOffset = 0; //TODO:

		data.diffuseTextureIndex = material->diffuseTextureIndex;
		data.metalRoughOcclTextureIndex = material->metalRoughOcclTextureIndex;
		data.normalTextureIndex = material->normalTextureIndex;
		data.emissivityTextureIndex = material->emissivityTextureIndex;

		data.baseColorFactor = material->baseColorFactor;
		data.metalRoughFactor = material->metalRoughFactor;
		data.emissiveFactor = material->emissiveFactor;
		data.alphaCutoff = material->alphaCutoff;
		data.flags = material->flags;

		draws.Push(data);
	}
}

void Scene::SetSkybox(const String& name)
{
	skybox = Resources::LoadSkybox(name);
	drawSkybox = true;

	//TODO: Upload skybox
}
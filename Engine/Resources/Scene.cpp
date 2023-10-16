#include "Scene.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"

void Scene::Create()
{
	camera.SetPerspective(0.01f, 1000.0f, 45.0f, 1.7777778f);
}

Scene::~Scene() { Destroy(); }

void Scene::Destroy()
{
	name.Destroy();

	models.Destroy();
}

bool Scene::Update()
{
	return camera.Update();
}

void Scene::AddModel(Model* model)
{
	models.Push(model);
}

void Scene::SetSkybox(Skybox* newSkybox)
{
	skybox = newSkybox;

	Resources::UseSkybox(skybox);
}
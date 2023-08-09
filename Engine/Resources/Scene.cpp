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
}

void Scene::SetSkybox(const String& name)
{
	skybox = Resources::LoadSkybox(name);
	drawSkybox = true;

	//TODO: Upload skybox
}
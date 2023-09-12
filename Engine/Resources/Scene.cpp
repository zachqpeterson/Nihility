#include "Scene.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"

void Scene::Create()
{
	camera.SetPerspective(0.0001f, 1000.0f, 45.0f, (F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());
}

Scene::~Scene() { Destroy(); }

void Scene::Destroy()
{
	name.Destroy();

	models.Destroy();
}

void Scene::Update()
{
	if (Settings::Resized())
	{
		camera.SetAspectRatio((F32)Settings::WindowWidth() / (F32)Settings::WindowHeight());
	}

	camera.Update();
}

void Scene::AddModel(Model* model)
{
	models.Push(model);
}

void Scene::SetSkybox(const String& name)
{
	skybox = Resources::LoadSkybox(name);
	drawSkybox = true;

	//TODO: Upload skybox
}
#include "Scene.hpp"
#include "Resources.hpp"
#include "Settings.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"

void Scene::Create(CameraType cameraType)
{
	switch (cameraType)
	{
	case CAMERA_TYPE_PERSPECTIVE: { camera.SetPerspective(0.01f, 1000.0f, 45.0f, 1.7777778f); } break;
	case CAMERA_TYPE_ORTHOGRAPHIC: { camera.SetOrthograpic(-100.0f, 100.0f, 240.0f, 135.0f, 1.0f); } break;
	}
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
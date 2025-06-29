#include "Scene.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

#include "tracy/Tracy.hpp"

Vector<Scene> Scene::scenes(16, {});
Freelist Scene::freeScenes(16);

Event<U32, Camera&, Vector<Entity>&> Scene::UpdateFns;
Event<U32, CommandBuffer> Scene::RenderFns;
Event<> Scene::InitializeFns;
Event<> Scene::ShutdownFns;

bool Scene::Initialize()
{
	InitializeFns();

	return true;
}

void Scene::Shutdown()
{
	vkDeviceWaitIdle(Renderer::device);

	ShutdownFns();
}

SceneRef Scene::CreateScene(CameraType type)
{
	U32 sceneId = freeScenes.GetFree();
	Scene& scene = scenes[sceneId];
	scene.camera.Create(type);
	scene.sceneId = sceneId;

	return sceneId;
}

Entity* Scene::GetEntity(U32 sceneId, U32 entityId)
{
	return &scenes[sceneId].entities[entityId];
}

void Scene::Destroy()
{
	vkDeviceWaitIdle(Renderer::device);
}

void Scene::Update()
{
	ZoneScopedN("Scene");

	camera.Update();
	UpdateFns(sceneId, camera, entities);
}

void Scene::Render(CommandBuffer commandBuffer) const
{
	RenderFns(sceneId, commandBuffer);
}

bool Scene::LoadScene()
{
	Renderer::SetScene(this);

	return true;
}

EntityRef Scene::CreateEntity(Vector2 position, Vector2 scale, Quaternion2 rotation)
{
	Entity entity{};
	entity.position = position;
	entity.scale = scale;
	entity.rotation = rotation;
	entity.prevPosition = position;
	entity.prevRotation = rotation;

	EntityRef id{};
	id.entityId = (U32)entities.Size();
	id.sceneId = sceneId;

	entities.Push(entity);

	return id;
}

const Camera& Scene::GetCamera() const
{
	return camera;
}
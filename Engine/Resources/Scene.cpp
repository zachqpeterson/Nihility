#include "Scene.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

#include "tracy/Tracy.hpp"

Vector<Scene> Scene::scenes(16, {});
Freelist Scene::freeScenes(16);

Event<Camera&, Vector<Entity>&> Scene::UpdateFns;
Event<CommandBuffer> Scene::RenderFns;
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
	scene.entities.Resize(128);
	scene.entities.Resize(scene.entities.Capacity());
	scene.freeEntities.Resize(128);

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
	UpdateFns(camera, entities);
}

void Scene::Render(CommandBuffer commandBuffer) const
{
	RenderFns(commandBuffer);
}

bool Scene::LoadScene()
{
	Renderer::SetScene(this);

	return true;
}

EntityRef Scene::CreateEntity(Vector2 position, Vector2 scale, Quaternion2 rotation)
{
	U32 index = freeEntities.GetFree();

	if (index == U32_MAX)
	{
		entities.Resize(entities.Size() + 1);
		entities.Resize(entities.Capacity());
		freeEntities.Resize(entities.Capacity());

		index = freeEntities.GetFree();
	}

	Entity& entity = entities[index];
	entity.position = position;
	entity.scale = scale;
	entity.rotation = rotation;
	entity.prevPosition = position;
	entity.prevRotation = rotation;

	EntityRef id{};
	id.entityId = index;
	id.sceneId = sceneId;

	return id;
}

void Scene::DestroyEntity(const EntityRef& ref)
{
	freeEntities.Release(ref.EntityId());
}

const Camera& Scene::GetCamera() const
{
	return camera;
}

Vector2 Scene::ScreenToWorld(const Vector2& position) const
{
	Matrix4 inv = camera.ViewProjection().Inverse();
	Vector4Int area = Renderer::RenderSize();

	F32 x = 2.0f * position.x / area.z - 1.0f;
	F32 y = 2.0f * position.y / area.w - 1.0f;

	return { x * inv.a.x + y * inv.a.y + inv.d.x, x * inv.b.x + y * inv.b.y + inv.d.y };
}
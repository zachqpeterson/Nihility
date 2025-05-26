#include "Scene.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

#include "tracy/Tracy.hpp"

U32 Scene::SceneID = 0;

bool Scene::Create(CameraType type)
{
	static bool spriteInit = SpriteComponent::Initialize();
	static bool tilemapInit = TilemapComponent::Initialize();

	sceneId = SceneID++; //TODO: Use freelist

	camera.Create(type);
	SpriteComponent::AddScene(sceneId);
	TilemapComponent::AddScene(sceneId);

	return true;
}

void Scene::Destroy()
{
	vkDeviceWaitIdle(Renderer::device);

	SpriteComponent::RemoveScene(sceneId);
	SpriteComponent::Shutdown();
	TilemapComponent::RemoveScene(sceneId);
	TilemapComponent::Shutdown();
}

void Scene::Update()
{
	ZoneScopedN("Scene");

	camera.Update();

	RigidBodyComponent::Update(entities);
	TilemapComponent::Update(sceneId, camera, entities);
	SpriteComponent::Update(sceneId, entities);
}

void Scene::Render(CommandBuffer commandBuffer) const
{
	TilemapComponent::Render(sceneId, commandBuffer);
	SpriteComponent::Render(sceneId, commandBuffer);
}

EntityRef Scene::CreateEntity(Vector2 position, Quaternion2 rotation)
{
	Entity entity{};
	entity.position = position;
	entity.rotation = rotation;

	EntityRef id{};
	id.scene = this;
	id.entityId = (U32)entities.Size();
	id.sceneId = sceneId;

	entities.Push(entity);

	return id;
}
#include "Scene.hpp"

#include "Resources.hpp"

#include "Rendering/Renderer.hpp"

#include "box2d/box2d.h"

#include "tracy/Tracy.hpp"

U32 Scene::SceneID = 0;

bool Scene::Create(CameraType type)
{
	static bool spriteInit = SpriteComponent::Initialize();

	sceneId = SceneID++; //TODO: Use freelist

	camera.Create(type);
	SpriteComponent::AddScene(sceneId);

	return true;
}

void Scene::Destroy()
{
	vkDeviceWaitIdle(Renderer::device);

	SpriteComponent::RemoveScene(sceneId);
	SpriteComponent::Shutdown();
}

void Scene::Update()
{
	ZoneScopedN("Scene");
	for (Entity& entity : entities)
	{
		if (entity.bodyId.index != 0)
		{
			b2Transform transform = b2Body_GetTransform(TypePun<b2BodyId>(entity.bodyId));

			entity.position.x = transform.p.x;
			entity.position.y = transform.p.y;
			entity.rotation.x = transform.q.s;
			entity.rotation.y = transform.q.c;
		}
	}

	camera.Update();

	SpriteComponent::Update(sceneId, entities);
}

void Scene::Render(CommandBuffer commandBuffer) const
{
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

void Scene::AddRigidBody(const EntityRef& id, BodyType type)
{
	Entity& entity = entities[id.entityId];

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.position.x = entity.position.x;
	bodyDef.position.y = entity.position.y;
	bodyDef.rotation.c = entity.rotation.y;
	bodyDef.rotation.s = entity.rotation.x;
	bodyDef.type = (b2BodyType)type;

	entity.bodyId = TypePun<BodyId>(b2CreateBody(Physics::WorldID(), &bodyDef));
}

void Scene::AddCollider(const EntityRef& id, const Vector2& scale)
{
	Entity& entity = entities[id.entityId];

	b2Polygon box = b2MakeBox(scale.x, scale.y);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(TypePun<b2BodyId>(entity.bodyId), &shapeDef, &box);
}

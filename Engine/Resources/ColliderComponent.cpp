#include "ColliderComponent.hpp"

#include "Scene.hpp"
#include "RigidBodyComponent.hpp"

#include "box2d/box2d.h"

Vector<Vector<Collider>> Collider::components;
bool Collider::initialized = false;

bool Collider::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool Collider::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<Collider> Collider::AddTo(EntityRef entity, const ComponentRef<RigidBody>& rigidBody)
{
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<Collider>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max Collider Count Reached!");
		return {};
	}

	U32 instanceId = (U32)instances.Size();

	b2Polygon box = b2MakeBox(entity->scale.x, entity->scale.y);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(TypePun<b2BodyId>(rigidBody->GetBodyId()), &shapeDef, &box);

	Collider collider{};
	collider.entityIndex = entity.EntityId();

	instances.Push(collider);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}

bool Collider::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	return false;
}

bool Collider::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	return false;
}
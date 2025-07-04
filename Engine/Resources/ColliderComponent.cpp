#include "ColliderComponent.hpp"

#include "Scene.hpp"

#include "Math/Physics.hpp"

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

ComponentRef<Collider> Collider::AddTo(EntityRef entity)
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

	Collider collider{};
	collider.entityIndex = entity.EntityId();

	Physics::AddCollider({ entity->position + entity->scale, entity->position - entity->scale });

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
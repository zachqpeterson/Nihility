#include "ColliderComponent.hpp"

#include "World.hpp"

#include "Math/Physics.hpp"
#include "Rendering/LineRenderer.hpp"

Vector<Collider> Collider::components(1000, {});
Freelist Collider::freeComponents(1000);
bool Collider::initialized = false;

bool Collider::Initialize()
{
	if (!initialized)
	{
		World::UpdateFns += Update;
		World::RenderFns += Render;

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
	U32 instanceId;
	Collider& collider = Create(instanceId);
	collider.entityIndex = entity.EntityId();
	collider.upperBound = entity->position + entity->scale;
	collider.lowerBound = entity->position - entity->scale;

	Physics::AddCollider({ entity->position + entity->scale, entity->position - entity->scale });

	return { entity.EntityId(), instanceId };
}

bool Collider::Update(Camera& camera, Vector<Entity>& entities)
{
#ifdef NH_DEBUG
	for (const Collider& collider : components)
	{
		LineRenderer::DrawLine({ collider.lowerBound, { collider.lowerBound.x, collider.upperBound.y }, collider.upperBound, { collider.upperBound.x, collider.lowerBound.y } }, true, { 0.0f, 1.0f, 0.0f, 1.0f });
	}
#endif

	return false;
}

bool Collider::Render(CommandBuffer commandBuffer)
{
	return false;
}
#include "RigidBodyComponent.hpp"

#include "Scene.hpp"

#include "box2d/box2d.h"

Vector<Vector<RigidBody>> RigidBody::components;
bool RigidBody::initialized = false;

bool RigidBody::Initialize()
{
	if (!initialized)
	{
		Scene::UpdateFns += Update;
		Scene::RenderFns += Render;

		initialized = true;
	}

	return false;
}

bool RigidBody::Shutdown()
{
	if (initialized) { initialized = false; }

	return false;
}

ComponentRef<RigidBody> RigidBody::AddTo(const EntityRef& entity, BodyType type)
{
	if (entity.SceneId() >= components.Size())
	{
		AddScene(entity.SceneId());
	}

	Vector<RigidBody>& instances = components[entity.SceneId()];

	if (instances.Full())
	{
		Logger::Error("Max RigidBody Count Reached!");
		return {};
	}

	U32 instanceId = (U32)instances.Size();

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.position.x = entity->position.x;
	bodyDef.position.y = entity->position.y;
	bodyDef.rotation.c = entity->rotation.y;
	bodyDef.rotation.s = entity->rotation.x;
	bodyDef.type = (b2BodyType)type;

	RigidBody body{};
	body.bodyId = TypePun<BodyId>(b2CreateBody(Physics::WorldID(), &bodyDef));
	body.entityIndex = entity.EntityId();

	instances.Push(body);

	return { entity.EntityId(), entity.SceneId(), instanceId };
}

bool RigidBody::Update(U32 sceneId, Camera& camera, Vector<Entity>& entities)
{
	if (sceneId >= components.Size()) { return false; }

	Vector<RigidBody>& instances = components[sceneId];

	if (Physics::updated)
	{
		for (RigidBody& rigidBody : instances)
		{
			Entity& entity = entities[rigidBody.entityIndex];

			b2Transform transform = b2Body_GetTransform(TypePun<b2BodyId>(rigidBody.bodyId));

			entity.prevPosition = entity.position;
			entity.prevRotation = entity.rotation;

			Vector2 targetPosition = { transform.p.x, transform.p.y };
			Quaternion2 targetRotation = { transform.q.s, transform.q.c };

			entity.position = Math::Lerp(entity.prevPosition, targetPosition, Physics::interpolation);
			entity.rotation = entity.prevRotation.Slerp(targetRotation, Physics::interpolation);
		}

		Physics::updated = false;
	}
	else
	{
		for (RigidBody& rigidBody : instances)
		{
			Entity& entity = entities[rigidBody.entityIndex];

			b2Transform transform = b2Body_GetTransform(TypePun<b2BodyId>(rigidBody.bodyId));

			Vector2 targetPosition = { transform.p.x, transform.p.y };
			Quaternion2 targetRotation = { transform.q.s, transform.q.c };

			entity.position = Math::Lerp(entity.prevPosition, targetPosition, Physics::interpolation);
			entity.rotation = entity.prevRotation.Slerp(targetRotation, Physics::interpolation);
		}
	}

	return false;
}

bool RigidBody::Render(U32 sceneId, CommandBuffer commandBuffer)
{
	return false;
}

const BodyId& RigidBody::GetBodyId() const
{
	return bodyId;
}
#include "RigidBodyComponent.hpp"

#include "box2d/box2d.h"

void RigidBodyComponent::Update(Vector<Entity>& entities)
{
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
}

BodyId RigidBodyComponent::AddComponent(const Vector2& position, const Quaternion2& rotation, BodyType type)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.position.x = position.x;
	bodyDef.position.y = position.y;
	bodyDef.rotation.c = rotation.y;
	bodyDef.rotation.s = rotation.x;
	bodyDef.type = (b2BodyType)type;

	return TypePun<BodyId>(b2CreateBody(Physics::WorldID(), &bodyDef));
}
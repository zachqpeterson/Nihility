#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"
#include "Math/Physics.hpp"

class NH_API RigidBodyComponent
{
private:
	static void Update(Vector<Entity>& entities);

	static BodyId AddComponent(const Vector2& position = Vector2::Zero, const Quaternion2& rotation = Quaternion2::Identity, BodyType type = BodyType::Dynamic);

	STATIC_CLASS(RigidBodyComponent);
	friend struct Scene;
	friend struct EntityRef;
};
#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"
#include "Math/Physics.hpp"

class NH_API ColliderComponent
{
private:
	static void AddComponent(BodyId bodyId, const Vector2& scale = Vector2::One);

	STATIC_CLASS(ColliderComponent);
	friend struct Scene;
	friend struct EntityRef;
};
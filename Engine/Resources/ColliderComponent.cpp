#include "ColliderComponent.hpp"

#include "box2d/box2d.h"

void ColliderComponent::AddComponent(BodyId bodyId, const Vector2& scale)
{
	b2Polygon box = b2MakeBox(scale.x, scale.y);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(TypePun<b2BodyId>(bodyId), &shapeDef, &box);
}
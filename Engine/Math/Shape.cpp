#include "Shape.hpp"

#include "PhysicsDefines.hpp"
#include "Math.hpp"

bool Filter::ShouldShapesCollide(const Filter& other)
{
	if (groupIndex == other.groupIndex && groupIndex != 0) { return groupIndex > 0; }
	return (layerMask & other.layers) != 0 && (layers & other.layerMask) != 0;
}

ConvexPolygon ConvexPolygon::operator*(const Transform2D& transform) const
{
	ConvexPolygon polygon;
	polygon.count = count;
	polygon.radius = radius;

	for (int i = 0; i < count; ++i)
	{
		polygon.vertices[i] = vertices[i] * transform;
		polygon.normals[i] = normals[i] * transform.rotation;
	}

	polygon.centroid = centroid * transform;

	return polygon;
}

ConvexPolygon& ConvexPolygon::operator*=(const Transform2D& transform)
{
	for (int i = 0; i < count; ++i)
	{
		vertices[i] *= transform;
		normals[i] *= transform.rotation;
	}

	centroid *= transform;

	return *this;
}

Shape::Shape(const ShapeDef& def)
{
	density = def.density;
	friction = def.friction;
	restitution = def.restitution;
	filter = def.filter;
	userData = def.userData;
	customColor = def.customColor;
	isSensor = def.isSensor;
	enlargedAABB = false;
	enableSensorEvents = def.enableSensorEvents;
	enableContactEvents = def.enableContactEvents;
	enableHitEvents = def.enableHitEvents;
	enablePreSolveEvents = def.enablePreSolveEvents;
	isFast = false;
	proxyKey = NullIndex;
	localCentroid = GetShapeCentroid();
	aabb = { Vector2Zero, Vector2Zero };
	fatAABB = { Vector2Zero, Vector2Zero };
}

Shape::Shape(const Shape& other)
{

}

Shape::Shape(Shape&& other) noexcept
{

}

Shape& Shape::operator=(const Shape& other)
{

	return *this;
}

Shape& Shape::operator=(Shape&& other) noexcept
{

	return *this;
}

AABB Shape::ComputeShapeAABB(const Transform2D& transform)
{
	switch (type)
	{
	case SHAPE_TYPE_CAPSULE: return ComputeCapsuleAABB(transform);
	case SHAPE_TYPE_CIRCLE: return ComputeCircleAABB(transform);
	case SHAPE_TYPE_POLYGON: return ComputePolygonAABB(transform);
	case SHAPE_TYPE_SEGMENT: return ComputeSegmentAABB(transform);
	case SHAPE_TYPE_CHAIN_SEGMENT: return ComputeChainSegmentAABB(transform);
	default: return { transform.position, transform.position };
	}
}

AABB Shape::ComputeCircleAABB(const Transform2D& transform)
{
	Vector2 p = circle.center * transform;
	float r = circle.radius;

	AABB aabb = { { p.x - r, p.y - r }, { p.x + r, p.y + r } };
	return aabb;
}

AABB Shape::ComputeCapsuleAABB(const Transform2D& transform)
{
	Vector2 v1 = capsule.center1 * transform;
	Vector2 v2 = capsule.center2 * transform;

	Vector2 r = { capsule.radius, capsule.radius };
	Vector2 lower = Math::Min(v1, v2) - r;
	Vector2 upper = Math::Max(v1, v2) + r;

	AABB aabb = { lower, upper };
	return aabb;
}

AABB Shape::ComputePolygonAABB(const Transform2D& transform)
{
	Vector2 lower = polygon.vertices[0] * transform;
	Vector2 upper = lower;

	for (I32 i = 1; i < polygon.count; ++i)
	{
		Vector2 v = polygon.vertices[i] * transform;
		lower = Math::Min(lower, v);
		upper = Math::Max(upper, v);
	}

	Vector2 r = { polygon.radius, polygon.radius };
	lower = lower - r;
	upper = upper + r;

	AABB aabb = { lower, upper };
	return aabb;
}

AABB Shape::ComputeSegmentAABB(const Transform2D& transform)
{
	Vector2 v1 = segment.point1 * transform;
	Vector2 v2 = segment.point2 * transform;

	Vector2 lower = Math::Min(v1, v2);
	Vector2 upper = Math::Max(v1, v2);

	AABB aabb = { lower, upper };
	return aabb;
}

AABB Shape::ComputeChainSegmentAABB(const Transform2D& transform)
{
	Vector2 v1 = chainSegment.segment.point1 * transform;
	Vector2 v2 = chainSegment.segment.point2 * transform;

	Vector2 lower = Math::Min(v1, v2);
	Vector2 upper = Math::Max(v1, v2);

	AABB aabb = { lower, upper };
	return aabb;
}

Vector2 Shape::GetShapeCentroid()
{
	switch (type)
	{
	case SHAPE_TYPE_CAPSULE: return Math::Lerp(capsule.center1, capsule.center2, 0.5f);
	case SHAPE_TYPE_CIRCLE: return circle.center;
	case SHAPE_TYPE_POLYGON: return polygon.centroid;
	case SHAPE_TYPE_SEGMENT: return Math::Lerp(segment.point1, segment.point2, 0.5f);
	case SHAPE_TYPE_CHAIN_SEGMENT: return Math::Lerp(chainSegment.segment.point1, chainSegment.segment.point2, 0.5f);
	default: return Vector2Zero;
	}
}

void Shape::UpdateShapeAABBs(const Transform2D& transform, BodyType proxyType)
{
	aabb = ComputeShapeAABB(transform);
	aabb.lowerBound.x -= SpeculativeDistance;
	aabb.lowerBound.y -= SpeculativeDistance;
	aabb.upperBound.x += SpeculativeDistance;
	aabb.upperBound.y += SpeculativeDistance;

	// Smaller margin for static bodies. Cannot be zero due to TOI tolerance.
	F32 margin = proxyType == BODY_TYPE_STATIC ? SpeculativeDistance : AABBMargin;
	fatAABB.lowerBound.x = aabb.lowerBound.x - margin;
	fatAABB.lowerBound.y = aabb.lowerBound.y - margin;
	fatAABB.upperBound.x = aabb.upperBound.x + margin;
	fatAABB.upperBound.y = aabb.upperBound.y + margin;
}
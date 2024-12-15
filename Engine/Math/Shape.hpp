#pragma once

#include "Defines.hpp"
#include "Math.hpp"

static constexpr inline U8 MaxPolygonVertices = 8;

enum NH_API ShapeType
{
	SHAPE_TYPE_CIRCLE,
	SHAPE_TYPE_CAPSULE,
	SHAPE_TYPE_SEGMENT,
	SHAPE_TYPE_POLYGON,
	SHAPE_TYPE_CHAIN_SEGMENT,

	SHAPE_TYPE_COUNT
};

struct Circle
{
	Vector2 center;
	F32 radius;
};

struct Capsule
{
	Vector2 center1;
	Vector2 center2;
	F32 radius;
};

struct ConvexPolygon
{
	ConvexPolygon operator*(const Transform2D& transform) const;
	ConvexPolygon& operator*=(const Transform2D& transform);

	Vector2 vertices[MaxPolygonVertices];
	Vector2 normals[MaxPolygonVertices];
	Vector2 centroid;
	F32 radius;
	I32 count;
};

struct Segment
{
	Vector2 point1;
	Vector2 point2;
};

struct ChainSegment
{
	Vector2 ghost1;
	Segment segment;
	Vector2 ghost2;
	I32 chainId;
};

struct Hull
{
	Vector2 points[MaxPolygonVertices];
	I32 count;
};

struct Filter
{
	bool ShouldShapesCollide(const Filter& other);

	U64 layers;
	U64 layerMask;
	I32 groupIndex;
};

struct AABB
{
	static AABB Combine(const AABB& a, const AABB& b)
	{
		return { Math::Min(a.lowerBound, b.lowerBound), Math::Max(a.upperBound, b.upperBound) };
	}

	static bool Enlarge(AABB& a, const AABB& b)
	{
		bool changed = false;
		if (b.lowerBound.x < a.lowerBound.x)
		{
			a.lowerBound.x = b.lowerBound.x;
			changed = true;
		}

		if (b.lowerBound.y < a.lowerBound.y)
		{
			a.lowerBound.y = b.lowerBound.y;
			changed = true;
		}

		if (a.upperBound.x < b.upperBound.x)
		{
			a.upperBound.x = b.upperBound.x;
			changed = true;
		}

		if (a.upperBound.y < b.upperBound.y)
		{
			a.upperBound.y = b.upperBound.y;
			changed = true;
		}

		return changed;
	}

	void Combine(const AABB& other)
	{
		lowerBound = Math::Min(lowerBound, other.lowerBound);
		upperBound = Math::Max(upperBound, other.upperBound);
	}

	Vector2 Center() const
	{
		return { (lowerBound.x + upperBound.x) * 0.5f, (lowerBound.y + upperBound.y) * 0.5f };
	}

	F32 Perimeter() const
	{
		F32 wx = upperBound.x - lowerBound.x;
		F32 wy = upperBound.y - lowerBound.y;
		return (wx + wy) * 2.0f;
	}

	bool Contains(const AABB& aabb) const
	{
		bool result = true;
		result = result && lowerBound.x <= aabb.lowerBound.x;
		result = result && lowerBound.y <= aabb.lowerBound.y;
		result = result && aabb.upperBound.x <= upperBound.x;
		result = result && aabb.upperBound.y <= upperBound.y;
		return result;
	}

	Vector2 lowerBound;
	Vector2 upperBound;
};

struct ShapeDef
{
	/// Use this to store application specific shape data.
	void* userData = nullptr;

	/// The Coulomb (dry) friction coefficient, usually in the range [0,1].
	F32 friction = 0.6f;

	/// The restitution (bounce) usually in the range [0,1].
	F32 restitution = 0.0f;

	/// The density, usually in kg/m^2.
	F32 density = 1.0f;

	/// Collision filtering data.
	Filter filter = { 0x0001ULL, U64_MAX, 0 };

	/// Custom debug draw color.
	U32 customColor = 0;

	/// A sensor shape generates overlap events but never generates a collision response.
	///	Sensors do not collide with other sensors and do not have continuous collision.
	///	Instead use a ray or shape cast for those scenarios.
	bool isSensor = false;

	/// Enable sensor events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
	bool enableSensorEvents = true;

	/// Enable contact events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
	bool enableContactEvents = true;

	/// Enable hit events for this shape. Only applies to kinematic and dynamic bodies. Ignored for sensors.
	bool enableHitEvents = false;

	/// Enable pre-solve contact events for this shape. Only applies to dynamic bodies. These are expensive
	///	and must be carefully handled due to threading. Ignored for sensors.
	bool enablePreSolveEvents = false;

	/// Normally shapes on static bodies don't invoke contact creation when they are added to the world. This overrides
	///	that behavior and causes contact creation. This significantly slows down static body creation which can be important
	///	when there are many static shapes.
	/// This is implicitly always true for sensors.
	bool forceContactCreation = false;
};

enum BodyType;

struct Shape
{
	Shape(const ShapeDef& def);

	Shape(const Shape&);
	Shape(Shape&&) noexcept;

	Shape& operator=(const Shape&);
	Shape& operator=(Shape&&) noexcept;

	Vector2 GetShapeCentroid();
	void UpdateShapeAABBs(const Transform2D& transform, BodyType proxyType);
	AABB ComputeShapeAABB(const Transform2D& transform);
	AABB ComputeCircleAABB(const Transform2D& transform);
	AABB ComputeCapsuleAABB(const Transform2D& transform);
	AABB ComputePolygonAABB(const Transform2D& transform);
	AABB ComputeSegmentAABB(const Transform2D& transform);
	AABB ComputeChainSegmentAABB(const Transform2D& transform);

	I32 id;
	I32 bodyId;
	I32 prevShapeId;
	I32 nextShapeId;
	ShapeType type;
	F32 density;
	F32 friction;
	F32 restitution;

	AABB aabb;
	AABB fatAABB;
	Vector2 localCentroid;
	I32 proxyKey;

	Filter filter;
	void* userData;
	U32 customColor;

	union
	{
		Capsule capsule;
		Circle circle;
		ConvexPolygon polygon;
		Segment segment;
		ChainSegment chainSegment;
	};

	bool isSensor;
	bool enableSensorEvents;
	bool enableContactEvents;
	bool enableHitEvents;
	bool enablePreSolveEvents;
	bool enlargedAABB;
	bool isFast;
};
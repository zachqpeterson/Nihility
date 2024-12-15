#pragma once

#include "Defines.hpp"
#include "Math.hpp"

#include "Shape.hpp"

static constexpr inline U8 MaxManifoldPoints = 2;

struct ManifoldPoint
{
	Vector2 point;
	Vector2 anchorA;
	Vector2 anchorB;
	F32 separation;
	F32 normalImpulse;
	F32 tangentImpulse;
	F32 maxNormalImpulse;
	F32 normalVelocity;
	U16 id;
	bool persisted;
};

struct DistanceCache
{
	U16 count;
	U8 indexA[3];
	U8 indexB[3];
};

struct NH_API Manifold
{
	ManifoldPoint points[MaxManifoldPoints];
	Vector2 normal;
	U32 pointCount;

	static Manifold CircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold CapsuleAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold CapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold PolygonAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold PolygonAndCapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold PolygonManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold SegmentAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold SegmentAndCapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold SegmentAndPolygonManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold ChainSegmentAndCircleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold ChainSegmentAndCapsuleManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
	static Manifold ChainSegmentAndPolygonManifold(const Shape& shapeA, const Transform2D& xfA, const Shape& shapeB, const Transform2D& xfB, DistanceCache& cache);
};

typedef Manifold ManifoldFn(const Shape&, const Transform2D&, const Shape&, const Transform2D&, DistanceCache&);

struct ContactRegister
{
	ManifoldFn* fcn;
	bool primary;
};

static ContactRegister contactRegister[SHAPE_TYPE_COUNT][SHAPE_TYPE_COUNT]{
{	{ Manifold::CircleManifold, true },
	{ Manifold::CapsuleAndCircleManifold, false },
	{ Manifold::SegmentAndCircleManifold, false },
	{ Manifold::PolygonAndCircleManifold, false },
	{ Manifold::ChainSegmentAndCircleManifold, false } },

{	{ Manifold::CapsuleAndCircleManifold, true },
	{ Manifold::CapsuleManifold, true },
	{ Manifold::SegmentAndCapsuleManifold, false },
	{ Manifold::PolygonAndCapsuleManifold, false },
	{ Manifold::ChainSegmentAndCapsuleManifold, false } },

{	{ Manifold::SegmentAndCircleManifold, true },
	{ Manifold::SegmentAndCapsuleManifold, true },
	{ nullptr, true },
	{ Manifold::SegmentAndPolygonManifold, true },
	{ nullptr, true } },

{	{ Manifold::PolygonAndCircleManifold, true },
	{ Manifold::PolygonAndCapsuleManifold, true },
	{ Manifold::SegmentAndPolygonManifold, false },
	{ Manifold::PolygonManifold, true },
	{ Manifold::ChainSegmentAndPolygonManifold, false } },

{	{ Manifold::ChainSegmentAndCircleManifold, true },
	{ Manifold::ChainSegmentAndCapsuleManifold, true },
	{ nullptr, true },
	{ Manifold::ChainSegmentAndPolygonManifold, true },
	{ nullptr, true } }
};
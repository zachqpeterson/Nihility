#pragma once

#include "PhysicsDefines.hpp"

// Friction mixing law. The idea is to allow either shape to drive the friction to zero.
// For example, anything slides on ice.
static constexpr F32 MixFriction(F32 friction1, F32 friction2)
{
	return Math::Sqrt(friction1 * friction2);
}

// Restitution mixing law. The idea is allow for anything to bounce off an inelastic surface.
// For example, a superball bounces on anything.
static constexpr F32 MixRestitution(F32 restitution1, F32 restitution2)
{
	return restitution1 > restitution2 ? restitution1 : restitution2;
}

typedef Manifold ManifoldFn(const Shape&, const Transform2D&, const Shape&, const Transform2D&, DistanceCache&);

struct ContactRegister
{
	ManifoldFn* fcn;
	bool primary;
};

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

static ContactRegister contactRegister[SHAPE_TYPE_COUNT][SHAPE_TYPE_COUNT]{
{	{ CircleManifold, true },
	{ CapsuleAndCircleManifold, false },
	{ SegmentAndCircleManifold, false },
	{ PolygonAndCircleManifold, false },
	{ ChainSegmentAndCircleManifold, false } },

{	{ CapsuleAndCircleManifold, true },
	{ CapsuleManifold, true },
	{ SegmentAndCapsuleManifold, false },
	{ PolygonAndCapsuleManifold, false },
	{ ChainSegmentAndCapsuleManifold, false } },

{	{ SegmentAndCircleManifold, true },
	{ SegmentAndCapsuleManifold, true },
	{ nullptr, true },
	{ SegmentAndPolygonManifold, true },
	{ nullptr, true } },

{	{ PolygonAndCircleManifold, true },
	{ PolygonAndCapsuleManifold, true },
	{ SegmentAndPolygonManifold, false },
	{ PolygonManifold, true },
	{ ChainSegmentAndPolygonManifold, false } },

{	{ ChainSegmentAndCircleManifold, true },
	{ ChainSegmentAndCapsuleManifold, true },
	{ nullptr, true },
	{ ChainSegmentAndPolygonManifold, true },
	{ nullptr, true } }
};
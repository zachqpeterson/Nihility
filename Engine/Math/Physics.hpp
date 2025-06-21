#pragma once

#include "Defines.hpp"

#include "Math.hpp"

enum class BodyType
{
	Static = 0,
	Kinematic = 1,
	Dynamic = 2
};

enum CollisionBits : U64
{
	StaticBit = 0x0001,
	MoverBit = 0x0002,
	DynamicBit = 0x0004,
	DebrisBit = 0x0008,

	AllBits = ~0u,
};

struct BodyId
{
	I32 index;
	U16 world;
	U16 generation;
};

struct CastResult
{
	Vector2 point;
	BodyId bodyId;
	F32 fraction;
	bool hit;
};

struct ShapeProxy
{
	Vector2 points[8];
	I32 count;
	F32 radius;
};

struct QueryFilter
{
	U64 categoryBits;
	U64 maskBits;
};

struct Capsule
{
	Vector2 center1;
	Vector2 center2;
	F32 radius;
};

struct Plane
{
	Vector2 normal;
	F32 offset;
};

struct CollisionPlane
{
	Plane plane;
	F32 pushLimit;
	F32 push;
	bool clipVelocity;
};

struct PlaneSolverResult
{
	Vector2 position;
	I32 iterationCount;
};

struct b2PlaneResult;
struct b2WorldId;
struct b2ShapeId;
struct b2Vec2;

using PlaneResultFn = bool(b2ShapeId, const b2PlaneResult*, void*);

class NH_API Physics
{
public:
	static b2WorldId WorldID();

	static CastResult ShapeCast(ShapeProxy proxy, Vector2 translation, QueryFilter filter);
	static void CollideMover(const Capsule& mover, QueryFilter filter, PlaneResultFn fn, void* context);
	static PlaneSolverResult SolvePlanes(Vector2 position, CollisionPlane* planes, I32 count);
	static F32 CastMover(const Capsule& mover, Vector2 translation, QueryFilter filter);
	static Vector2 ClipVector(Vector2 vector, const CollisionPlane* planes, I32 count);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static F32 CastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context);

	static bool updated;

	static F32 interpolation;
	static constexpr F32 Timestep = 0.033333335f;

	STATIC_CLASS(Physics);

	friend class Engine;
	friend class RigidBody;
	friend struct Scene;
};
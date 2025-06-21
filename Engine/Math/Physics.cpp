#include "Physics.hpp"

#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Platform/Memory.hpp"
#include "Resources/Settings.hpp"

#include "box2d/box2d.h"

#include "tracy/Tracy.hpp"

b2WorldId worldId;

F32 Physics::interpolation = 0.0f;
bool Physics::updated = true;

void* AllocFcn(U32 size, I32 alignment)
{
	U8* buffer = nullptr;
	Memory::Allocate(&buffer, size);
	return buffer;
}

void FreeFcn(void* mem)
{
	Memory::Free(&mem);
}

I32 AssertFcn(const C8* condition, const C8* fileName, I32 lineNumber)
{
	Logger::Fatal("Assertion: '", condition, "' In File: '", fileName, "' On Line: ", lineNumber);
	return 1;
}

bool Physics::Initialize()
{
	b2SetAllocator(AllocFcn, FreeFcn);
	b2SetAssertFcn(AssertFcn);

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldId = b2CreateWorld(&worldDef);

	return true;
}

void Physics::Shutdown()
{
	b2DestroyWorld(worldId);
}

void Physics::Update()
{
	ZoneScopedN("Physics");
	b2World_Step(worldId, Timestep, 4);
	updated = true;
}

CastResult Physics::ShapeCast(ShapeProxy proxy, Vector2 translation, QueryFilter filter)
{
	CastResult castResult{};

	b2World_CastShape(worldId, (b2ShapeProxy*)&proxy, TypePun<b2Vec2>(translation), TypePun<b2QueryFilter>(filter), CastCallback, &castResult);

	return castResult;
}

void Physics::CollideMover(const Capsule& mover, QueryFilter filter, PlaneResultFn fn, void* context)
{
	b2World_CollideMover(worldId, (b2Capsule*)&mover, TypePun<b2QueryFilter>(filter), fn, context);
}

PlaneSolverResult Physics::SolvePlanes(Vector2 position, CollisionPlane* planes, I32 count)
{
	return TypePun<PlaneSolverResult>(b2SolvePlanes(TypePun<b2Vec2>(position), (b2CollisionPlane*)planes, count));
}

F32 Physics::CastMover(const Capsule& mover, Vector2 translation, QueryFilter filter)
{
	return b2World_CastMover(worldId, (b2Capsule*)&mover, TypePun<b2Vec2>(translation), TypePun<b2QueryFilter>(filter));
}

Vector2 Physics::ClipVector(Vector2 vector, const CollisionPlane* planes, I32 count)
{
	return TypePun<Vector2>(b2ClipVector(TypePun<b2Vec2>(vector), (b2CollisionPlane*)planes, count));
}

b2WorldId Physics::WorldID()
{
	return worldId;
}

float Physics::CastCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
{
	CastResult* result = (CastResult*)context;
	result->point = TypePun<Vector2>(point);
	result->bodyId = TypePun<BodyId>(b2Shape_GetBody(shapeId));
	result->fraction = fraction;
	result->hit = true;
	return fraction;
}
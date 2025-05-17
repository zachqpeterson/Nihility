#include "Physics.hpp"

#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Platform/Memory.hpp"
#include "Resources/Settings.hpp"

#include "box2d/box2d.h"

#include "tracy/Tracy.hpp"

b2WorldId worldId;

F64 Physics::timeStep;

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

	timeStep = Settings::targetFrametime;

	return true;
}

void Physics::Shutdown()
{
	b2DestroyWorld(worldId);
}

void Physics::Update()
{
	ZoneScopedN("Physics");
	b2World_Step(worldId, (F32)timeStep, 4);
}

b2WorldId Physics::WorldID()
{
	return worldId;
}
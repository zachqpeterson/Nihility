#include "Physics.hpp"

#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Platform/Memory.hpp"
#include "Resources/Settings.hpp"

#include "tracy/Tracy.hpp"

Vector<AABB> Physics::colliders;

I32 AssertFcn(const C8* condition, const C8* fileName, I32 lineNumber)
{
	Logger::Fatal("Assertion: '", condition, "' In File: '", fileName, "' On Line: ", lineNumber);
	return 1;
}

bool Physics::Initialize()
{
	return true;
}

void Physics::Shutdown()
{

}

void Physics::Update()
{
	ZoneScopedN("Physics");
}

void Physics::AddCollider(const AABB& collider)
{
	colliders.Push(collider);
}

Collision Physics::CheckCollision(const AABB& collider)
{
	for (const AABB& col : colliders)
	{
		if (collider.lowerBound.x < col.upperBound.x && collider.upperBound.x > col.lowerBound.x &&
			collider.lowerBound.y < col.upperBound.y && collider.upperBound.y > col.lowerBound.y)
		{
			return { col, true };
		}
	}

	return { {}, false };
}
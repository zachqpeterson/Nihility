#include "Physics.hpp"

#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Platform/Memory.hpp"
#include "Resources/Settings.hpp"
#include "Resources/TilemapComponent.hpp"

#include "tracy/Tracy.hpp"

Vector<AABB> Physics::colliders;
Vector<GridCollider> Physics::tilemapColliders;

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

U32 Physics::AddCollider(const AABB& collider)
{
	U32 index = (U32)colliders.Size();
	colliders.Push(collider);

	return index;
}

U32 Physics::AddTilemapCollider(const TileType* tileArray, const TilemapData& data)
{
	GridCollider collider{};
	collider.width = data.width;
	collider.height = data.height;
	collider.tileSize = data.tileSize;
	collider.offset = data.offset;
	collider.tileArray = tileArray;

	U32 index = (U32)tilemapColliders.Size();
	tilemapColliders.Push(collider);

	return index;
}

void Physics::RemoveCollider(U32 index)
{

}

void Physics::RemoveTilemapCollider(U32 index)
{

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
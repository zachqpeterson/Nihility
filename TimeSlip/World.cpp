#include "World.hpp"

#include "GridBroadphase.hpp"
#include "Tile.hpp"
#include "Chunk.hpp"
#include "TimeSlip.hpp"

#include <Physics/Physics.hpp>
#include <Memory/Memory.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/Camera.hpp>
#include <Containers/Array.hpp>
#include <Core/Time.hpp>
#include <Core/Input.hpp>
#include <Platform/Platform.hpp>

#define VIEW_DISTANCE_X 6
#define VIEW_DISTANCE_Y 3

World::World(I64 seed, WorldSize size, Vector2& spawnPoint) : SEED{ seed }, TILES_X{ (U16)size }, TILES_Y{ (U16)(TILES_X / 3.5f) }, CHUNKS_X{ (U16)(TILES_X / CHUNK_SIZE) }, CHUNKS_Y{ (U16)(TILES_Y / CHUNK_SIZE) }
{
	Math::SeedRandom((U32)seed);
	Chunk::world = this;

	tiles = (Tile**)Memory::LinearAllocate(sizeof(Tile*) * TILES_X);

	for (U16 i = 0; i < TILES_X; ++i)
	{
		tiles[i] = (Tile*)Memory::LinearAllocate(sizeof(Tile) * TILES_Y);
	}

	F32 spawnHeight = GenerateWorld();

	chunks = (Chunk**)Memory::LinearAllocate(sizeof(Chunk*) * CHUNKS_X);

	for (U16 x = 0; x < CHUNKS_X; ++x)
	{
		chunks[x] = (Chunk*)Memory::LinearAllocate(sizeof(Chunk) * CHUNKS_Y);

		U16 tileX = x * CHUNK_SIZE;

		for (U16 y = 0; y < CHUNKS_Y; ++y)
		{
			U16 tileY = y * CHUNK_SIZE;

			Array<Tile*, CHUNK_SIZE>& chunkTiles = chunks[x][y].SetTiles();

			for (U16 i = 0; i < CHUNK_SIZE; ++i)
			{
				chunkTiles[i] = tiles[tileX + i] + tileY;
			}
		}
	}

	spawnPoint = { TILES_X * 0.5f, spawnHeight };

	GridBroadphase* bp = new GridBroadphase(tiles, TILES_X, TILES_Y);
	Physics::SetBroadphase(bp);
}

World::~World()
{
	Destroy();
}

void World::Destroy()
{
	for (U16 x = 0; x < CHUNKS_X; ++x)
	{
		for (U16 y = 0; y < CHUNKS_Y; ++y)
		{
			chunks[x][y].Destroy();
		}
	}
}

void* World::operator new(U64 size) { return Memory::Allocate(sizeof(World), MEMORY_TAG_GAME); }

void World::operator delete(void* ptr) { Memory::Free(ptr, sizeof(World), MEMORY_TAG_GAME); }

void World::Update()
{
	static Vector3Int lastPos{ 10, 10, 0 };
	Vector3 pos = (RendererFrontend::CurrentScene()->GetCamera()->Position() / 8.0f);
	Vector3Int posI = (Vector3Int)pos;

	if (posI != lastPos)
	{
		I32 leftMin = Math::Max(lastPos.x - VIEW_DISTANCE_X, 0);
		I32 leftMax = Math::Max(posI.x - VIEW_DISTANCE_X, 0);
		I32 rightMin = Math::Min(posI.x + VIEW_DISTANCE_X, (I32)CHUNKS_X - 1);
		I32 rightMax = Math::Min(lastPos.x + VIEW_DISTANCE_X, (I32)CHUNKS_X - 1);
		I32 bottomMin = Math::Max(lastPos.y - VIEW_DISTANCE_Y, 0);
		I32 bottomMax = Math::Max(posI.y - VIEW_DISTANCE_Y, 0);
		I32 topMin = Math::Min(posI.y + VIEW_DISTANCE_Y, (I32)CHUNKS_Y - 1);
		I32 topMax = Math::Min(lastPos.y + VIEW_DISTANCE_Y, (I32)CHUNKS_Y - 1);

		for (I32 y = bottomMin; y < bottomMax; ++y)
		{
			for (I32 x = leftMin; x <= rightMax; ++x)
			{
				chunks[x][y].Unload();
			}
		}

		for (I32 y = topMax; y > topMin; --y)
		{
			for (I32 x = leftMin; x <= rightMax; ++x)
			{
				chunks[x][y].Unload();
			}
		}

		for (I32 x = leftMin; x < leftMax; ++x)
		{
			for (I32 y = bottomMin; y <= topMax; ++y)
			{
				chunks[x][y].Unload();
			}
		}

		for (I32 x = rightMax; x > rightMin; --x)
		{
			for (I32 y = bottomMin; y <= topMax; ++y)
			{
				chunks[x][y].Unload();
			}
		}

		for (U16 x = leftMax; x <= rightMin; ++x)
		{
			for (U16 y = bottomMax; y <= topMin; ++y)
			{
				chunks[x][y].Load({ x, y });
			}
		}
	}

	lastPos = posI;
}

Vector2 World::BlockUV(const Vector2Int& pos)
{
	return { 3.0f * (pos.y + 1 == TILES_Y || tiles[pos.x][pos.y + 1].blockID) + ((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3,
		(F32)((pos.y - 1 < 0 || tiles[pos.x][pos.y - 1].blockID) +
		((pos.x - 1 < 0 || tiles[pos.x - 1][pos.y].blockID) << 1) +
		((pos.x + 1 == TILES_X || tiles[pos.x + 1][pos.y].blockID) << 2)) };
}

Vector2 World::WallUV(const Vector2Int& pos)
{
	return { 3.0f * (pos.y + 1 == TILES_Y || tiles[pos.x][pos.y + 1].wallID) + ((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3,
		(F32)((pos.y - 1 < 0 || tiles[pos.x][pos.y - 1].wallID) +
		((pos.x - 1 < 0 || tiles[pos.x - 1][pos.y].wallID) << 1) +
		((pos.x + 1 == TILES_X || tiles[pos.x + 1][pos.y].wallID) << 2)) };
}

Vector2 World::DecorationUV(const Vector2Int& pos, U8 id)
{
	if (id < 5) //TODO: don't hardcode
	{
		return { (F32)(((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3), (F32)tiles[pos.x][pos.y].biome };
	}

	return Vector2((F32)((pos.x > 0 && pos.y > 0 && tiles[pos.x - 1][pos.y - 1].decID == id) +
		((pos.x + 1 < TILES_X && pos.y > 0 && tiles[pos.x + 1][pos.y - 1].decID == id) << 1) +
		((pos.x > 0 && pos.y + 1 < TILES_Y && tiles[pos.x - 1][pos.y + 1].decID == id) << 2) +
		((pos.x + 1 < TILES_X && pos.y + 1 < TILES_Y && tiles[pos.x + 1][pos.y + 1].decID == id) << 3)),
		(F32)((pos.y > 0 && tiles[pos.x][pos.y - 1].decID == id) +
			((pos.x > 0 && tiles[pos.x - 1][pos.y].decID == id) << 1) +
			((pos.x + 1 < TILES_X && tiles[pos.x + 1][pos.y].decID == id) << 2) +
			((pos.y + 1 < TILES_Y && tiles[pos.x][pos.y + 1].decID == id) << 3)));
}

Vector2 World::LiquidUV(const Vector2Int& pos)
{
	return Vector2::ZERO;
}

void World::TileLight(const Vector2Int& pos, Vector3& color, Vector3& globalColor)
{
	Vector3 c{ 1.0f, 1.0f, 1.0f };

	color = c * tiles[pos.x][pos.y].lightSource;
	globalColor = Vector3::ONE * tiles[pos.x][pos.y].globalLightSource;

	U16 maxLightDistance = 16;

	U16 xStart = Math::Max(pos.x - maxLightDistance, 0);
	U16 xEnd = pos.x + maxLightDistance;
	U16 yStart = Math::Max(pos.y - maxLightDistance, 0);
	U16 yEnd = pos.y + maxLightDistance;

	for (U16 x = xStart; x < xEnd && x < TILES_X; ++x)
	{
		for (U16 y = yStart; y < yEnd && y < TILES_Y; ++y)
		{
			if ((x == pos.x && y == pos.y) || (!tiles[x][y].lightSource && !tiles[x][y].globalLightSource)) { continue; }

			Vector2Int start{ x, y };
			F32 length = (pos - start).Magnitude();
			Vector2 dir = (Vector2)(pos - start) / length;
			Vector2 unitStepSize = { 1.0f / dir.x * Math::Sign(dir.x), 1.0f / dir.y * Math::Sign(dir.y) };

			Vector2 length1D{ 0.5f, 0.5f };
			Vector2Int step{ (I32)Math::Sign(dir.x), (I32)Math::Sign(dir.y) };

			F32 decr = 1.5f / 20; // TODO: Intensity
			F32 distance = 0.0f;
			F32 brightness = 1.0f;

			bool checkX = !Math::NaN(unitStepSize.x);
			bool checkY = !Math::NaN(unitStepSize.y);

			while (brightness > 0.0f)
			{
				if (((length1D.x < length1D.y && checkX) || !checkY) && length1D.x < length)
				{
					start.x += step.x;
					distance = length1D.x;
					length1D.x += unitStepSize.x;

					brightness -= (unitStepSize.x * decr) * (1 + (tiles[start.x][start.y].blockID > 0) * 2 + !checkY) * (distance < length);
				}
				else if (checkY && length1D.y < length)
				{
					start.y += step.y;
					distance = length1D.y;
					length1D.y += unitStepSize.y;

					brightness -= (unitStepSize.y * decr) * (1 + (tiles[start.x][start.y].blockID > 0) * 2 + !checkX) * (distance < length);
				}
				else { break; }
			}

			color += c * Math::Max(brightness, 0.0f) * tiles[x][y].lightSource;
			globalColor += Vector3::ONE * Math::Max(brightness * 0.9f, 0.0f) * tiles[x][y].globalLightSource;
		}
	}
}

void World::BreakBlock(const Vector2Int& pos)
{
	if (tiles[pos.x][pos.y].blockID)
	{
		Vector2Int chunkPos = pos / 8;

		bool globalLight = !tiles[pos.x][pos.y].wallID;

		TimeSlip::PickupItem(tiles[pos.x][pos.y].blockID, 1);
		if (tiles[pos.x][pos.y].decID > 1) { TimeSlip::PickupItem(tiles[pos.x][pos.y].decID + 8, 1); }

		tiles[pos.x][pos.y].blockID = 0;
		tiles[pos.x][pos.y].decID = 0;
		chunks[chunkPos.x][chunkPos.y].EditBlock(pos, pos - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].EditDecoration(pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[left.x][left.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(left, left - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(left, left - chunkPos * 8);

			if (pos.y > 0)
			{
				Vector2Int leftDown = left + Vector2Int::DOWN;
				chunkPos = leftDown / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(leftDown, leftDown - chunkPos * 8);
			}

			if (pos.y < TILES_Y - 1)
			{
				Vector2Int leftUp = left + Vector2Int::UP;
				chunkPos = leftUp / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(leftUp, leftUp - chunkPos * 8);
			}
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[right.x][right.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(right, right - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(right, right - chunkPos * 8);

			if (pos.y > 0)
			{
				Vector2Int rightDown = right + Vector2Int::DOWN;
				chunkPos = rightDown / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(rightDown, rightDown - chunkPos * 8);
			}

			if (pos.y < TILES_Y - 1)
			{
				Vector2Int rightUp = right + Vector2Int::UP;
				chunkPos = rightUp / 8;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(rightUp, rightUp - chunkPos * 8);
			}
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			Tile& downTile = tiles[down.x][down.y];
			chunkPos = down / 8;
			downTile.globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(down, down - chunkPos * 8);
			if (downTile.decID > 1 && downTile.decID < 5)
			{
				if (downTile.decID > 2) { TimeSlip::PickupItem(downTile.decID + 8, 1); }
				downTile.decID = 0;
			}
			chunks[chunkPos.x][chunkPos.y].EditDecoration(down, down - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[up.x][up.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditBlock(up, up - chunkPos * 8);
			chunks[chunkPos.x][chunkPos.y].EditDecoration(up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		Vector<Mesh*> meshes{ 36 };

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes(meshes);
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes(meshes);
		}

		RendererFrontend::BatchCreateMeshes(meshes);
	}
	else if (tiles[pos.x][pos.y].decID)
	{
		Vector2Int chunkPos = pos / 8;
		Vector<Mesh*> meshes{ 36 };
		U8 id = tiles[pos.x][pos.y].decID;
		tiles[pos.x][pos.y].decID = 0;
		chunks[chunkPos.x][chunkPos.y].EditDecoration(pos, pos - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		RendererFrontend::BatchCreateMeshes(meshes);

		if (id > 2) { TimeSlip::PickupItem(id + 8, 1); }
	}
}

void World::PlaceBlock(const Vector2Int& pos, U8 id)
{
	if (!tiles[pos.x][pos.y].blockID && !tiles[pos.x][pos.y].decID)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].blockID = id;
		bool globalLight = !tiles[pos.x][pos.y].wallID;
		chunks[chunkPos.x][chunkPos.y].EditBlock(pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[left.x][left.y].blockID && !tiles[left.x][left.y].wallID);
			tiles[left.x][left.y].globalLightSource -= globalLight * (tiles[left.x][left.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(left, left - chunkPos * 8);
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[right.x][right.y].blockID && !tiles[right.x][right.y].wallID);
			tiles[right.x][right.y].globalLightSource -= globalLight * (tiles[right.x][right.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(right, right - chunkPos * 8);
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[down.x][down.y].blockID && !tiles[down.x][down.y].wallID);
			tiles[down.x][down.y].globalLightSource -= globalLight * (tiles[down.x][down.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(down, down - chunkPos * 8);
			if (tiles[down.x][down.y].decID == 1) //It's grass
			{
				tiles[down.x][down.y].decID = 0;
				chunks[chunkPos.x][chunkPos.y].EditDecoration(down, down - chunkPos * 8);
			}
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[up.x][up.y].blockID && !tiles[up.x][up.y].wallID);
			tiles[up.x][up.y].globalLightSource -= globalLight * (tiles[up.x][up.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditBlock(up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		Vector<Mesh*> meshes{ 36 };

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes(meshes);
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes(meshes);
		}

		RendererFrontend::BatchCreateMeshes(meshes);
	}
}

void World::BreakWall(const Vector2Int& pos)
{
	if (tiles[pos.x][pos.y].wallID)
	{
		Vector2Int chunkPos = pos / 8;

		bool globalLight = !tiles[pos.x][pos.y].blockID;

		TimeSlip::PickupItem(tiles[pos.x][pos.y].wallID, 1);

		tiles[pos.x][pos.y].wallID = 0;
		chunks[chunkPos.x][chunkPos.y].EditWall(pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[left.x][left.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(left, left - chunkPos * 8);
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[right.x][right.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(right, right - chunkPos * 8);
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[down.x][down.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(down, down - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[up.x][up.y].globalLightSource += globalLight;
			chunks[chunkPos.x][chunkPos.y].EditWall(up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		Vector<Mesh*> meshes{ 36 };

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes(meshes);
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes(meshes);
		}

		RendererFrontend::BatchCreateMeshes(meshes);
	}
}

void World::PlaceWall(const Vector2Int& pos, U8 id)
{
	if (!tiles[pos.x][pos.y].wallID)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].wallID = id;
		bool globalLight = !tiles[pos.x][pos.y].blockID;
		chunks[chunkPos.x][chunkPos.y].EditWall(pos, pos - chunkPos * 8);

		if (pos.x > 0)
		{
			Vector2Int left = pos + Vector2Int::LEFT;
			chunkPos = left / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[left.x][left.y].blockID && !tiles[left.x][left.y].wallID);
			tiles[left.x][left.y].globalLightSource -= globalLight * (tiles[left.x][left.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(left, left - chunkPos * 8);
		}

		if (pos.x < TILES_X - 1)
		{
			Vector2Int right = pos + Vector2Int::RIGHT;
			chunkPos = right / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[right.x][right.y].blockID && !tiles[right.x][right.y].wallID);
			tiles[right.x][right.y].globalLightSource -= globalLight * (tiles[right.x][right.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(right, right - chunkPos * 8);
		}

		if (pos.y > 0)
		{
			Vector2Int down = pos + Vector2Int::DOWN;
			chunkPos = down / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[down.x][down.y].blockID && !tiles[down.x][down.y].wallID);
			tiles[down.x][down.y].globalLightSource -= globalLight * (tiles[down.x][down.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(down, down - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int up = pos + Vector2Int::UP;
			chunkPos = up / 8;
			tiles[pos.x][pos.y].globalLightSource += globalLight * (!tiles[up.x][up.y].blockID && !tiles[up.x][up.y].wallID);
			tiles[up.x][up.y].globalLightSource -= globalLight * (tiles[up.x][up.y].globalLightSource > 0);
			chunks[chunkPos.x][chunkPos.y].EditWall(up, up - chunkPos * 8);
		}

		chunkPos = pos / 8;

		Vector<Mesh*> meshes{ 36 };

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes(meshes);
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes(meshes);
		}

		RendererFrontend::BatchCreateMeshes(meshes);
	}
}

void World::PlaceLight(const Vector2Int& pos)
{
	if (!tiles[pos.x][pos.y].lightSource)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].lightSource = 1;

		Vector<Mesh*> meshes{ 36 };

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes(meshes);
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes(meshes);
		}

		RendererFrontend::BatchCreateMeshes(meshes);
	}
}

void World::RemoveLight(const Vector2Int& pos)
{
	if (tiles[pos.x][pos.y].lightSource)
	{
		Vector2Int chunkPos = pos / 8;
		tiles[pos.x][pos.y].lightSource = 0;

		Vector<Mesh*> meshes{ 36 };

		chunks[chunkPos.x][chunkPos.y].UpdateLighting(chunkPos);
		chunks[chunkPos.x][chunkPos.y].UpdateMeshes(meshes);

		if (chunkPos.x < CHUNKS_X - 1)
		{
			chunks[chunkPos.x + 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::RIGHT);
			chunks[chunkPos.x + 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::RIGHT + Vector2Int::UP);
				chunks[chunkPos.x + 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN + Vector2Int::RIGHT);
				chunks[chunkPos.x + 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.x > 1)
		{
			chunks[chunkPos.x - 1][chunkPos.y].UpdateLighting(chunkPos + Vector2Int::LEFT);
			chunks[chunkPos.x - 1][chunkPos.y].UpdateMeshes(meshes);

			if (chunkPos.y > 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::LEFT + Vector2Int::DOWN);
				chunks[chunkPos.x - 1][chunkPos.y - 1].UpdateMeshes(meshes);
			}

			if (chunkPos.y < CHUNKS_Y - 1)
			{
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP + Vector2Int::LEFT);
				chunks[chunkPos.x - 1][chunkPos.y + 1].UpdateMeshes(meshes);
			}
		}

		if (chunkPos.y < CHUNKS_Y - 1)
		{
			chunks[chunkPos.x][chunkPos.y + 1].UpdateLighting(chunkPos + Vector2Int::UP);
			chunks[chunkPos.x][chunkPos.y + 1].UpdateMeshes(meshes);
		}

		if (chunkPos.y > 1)
		{
			chunks[chunkPos.x][chunkPos.y - 1].UpdateLighting(chunkPos + Vector2Int::DOWN);
			chunks[chunkPos.x][chunkPos.y - 1].UpdateMeshes(meshes);
		}

		RendererFrontend::BatchCreateMeshes(meshes);
	}
}

F32 World::GenerateWorld()
{
	static const F64 terrainLowFreq = 0.005;
	static const F64 terrainHighFreq = 0.05;
	static const F64 terrainLowAmplitude = 5.0;
	static const F64 terrainHighAmplitude = 25.0;
	static const F64 caveLowFreq = 0.01333333333;
	static const F64 caveHighFreq = 0.02;
	static const F64 caveThresholdMin = 0.01;
	static const F64 caveThresholdMax = 0.04;
	static const F64 oreFreq = 0.07;
	static const F64 oreAmplitude = 0.13;
	static const F64 oreThreshold = 0.1;

	Timer timer;
	timer.Start();

	F32 spawnHeight = 0.0f;

	U16 biomeLengths[BIOME_COUNT] = {};

	U16 lastLength = 0;

	for (U8 i = 0; i < BIOME_COUNT - 1; ++i)
	{
		F64 mod = TILES_X * 0.98 / (F64)BIOME_COUNT - TILES_X * 0.97 / (F64)BIOME_COUNT;
		biomeLengths[i] = (U16)(Math::Mod((F64)Math::RandomF(), mod) + TILES_X * 0.97 / (F64)BIOME_COUNT) + lastLength;
		lastLength = biomeLengths[i];
	}

	biomeLengths[BIOME_COUNT - 1] = TILES_X;

	for (U16 y = 0; y < TILES_Y; ++y)
	{
		U16 length = 0;
		F64 variation = (Math::Simplex1(y * 0.1 + SEED + length) * 5.0);

		for (U8 i = 0; i < BIOME_COUNT - 1; ++i)
		{
			U16 prevLength = length;
			length = (U16)(biomeLengths[i] + variation);

			for (U16 x = prevLength; x < length; ++x)
			{
				tiles[x][y].biome = i;
			}
		}

		U16 prevLength = length;

		for (U16 x = prevLength; x < TILES_X; ++x)
		{
			tiles[x][y].biome = BIOME_COUNT - 1;
		}
	}

	F64 tempSimplex = Math::Simplex1((F64)SEED);
	U16 prevHeight = (U16)((tempSimplex * terrainHighAmplitude) + (tempSimplex * terrainLowAmplitude) + (TILES_Y * 0.5));

	for (U16 x = 0; x < TILES_X; ++x)
	{
		U16 height = (U16)((Math::Simplex1(x * terrainLowFreq + SEED) * terrainHighAmplitude) +
			(Math::Simplex1(x * terrainHighFreq + SEED) * terrainLowAmplitude) + (TILES_Y * 0.5));

		tiles[x][height].globalLightSource = 1 + (height > prevHeight);
		if (x > 0) { tiles[x - 1][prevHeight].globalLightSource += prevHeight > height; };

		prevHeight = height;

		for (U16 y = height; y < TILES_Y; ++y)
		{
			Tile& tile = tiles[x][y];

			bool cave = Math::Abs(Math::Simplex2(x * caveHighFreq + SEED, y * caveHighFreq + SEED) +
				Math::Simplex2(x * caveLowFreq + SEED * 2.0, y * caveLowFreq + SEED * 2.0)) >
				(caveThresholdMax * (height + y) / (F64)height + caveThresholdMin);

			F64 oreNoise = Math::Simplex2(x * oreFreq + SEED * 2.0, y * oreFreq + SEED * 2.0) * oreAmplitude;
			U8 ore = (oreNoise > oreThreshold) * (5 + tile.biome);

			tile.decID = ((y == height) + (y > height) * ore) * cave;
			tile.blockID = ((1 + (y > height + 10)) + biomeTileMods[tile.biome]) * cave;
			tile.wallID = 1 + (y > height + 10) + biomeTileMods[tile.biome];
		}

		if (tiles[x][height].blockID)
		{
			//TODO: Place Fauna
			tiles[x][height - 1].decID = 3;
		}
	}

	Logger::Debug("World Generation Time: {}", timer.CurrentTime());

	return (F32)(U16)((Math::Simplex1((TILES_X >> 1) * terrainLowFreq + SEED) * terrainHighAmplitude) +
		(Math::Simplex1((TILES_X >> 1) * terrainHighFreq + SEED) * terrainLowAmplitude) + (TILES_Y * 0.5)) - 1.5f;
}

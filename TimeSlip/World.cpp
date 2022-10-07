#include "World.hpp"

#include "GridBroadphase.hpp"
#include "Tile.h"
#include "Chunk.hpp"

#include <Physics/Physics.hpp>
#include <Memory/Memory.hpp>
#include <Renderer/RendererFrontend.hpp>
#include <Renderer/Scene.hpp>
#include <Renderer/Camera.hpp>
#include <Containers/Array.hpp>

#define VIEW_DISTANCE_X 3
#define VIEW_DISTANCE_Y 2

World::World(I64 seed, WorldSize size) : SEED{ seed }, TILES_X{ (U16)size }, TILES_Y{ (U16)(TILES_X / 3.5f) }, CHUNKS_X{ (U16)(TILES_X / CHUNK_SIZE) }, CHUNKS_Y{ (U16)(TILES_Y / CHUNK_SIZE) }
{
	Math::SeedRandom((U32)seed);
	Chunk::world = this;

	tiles = (Tile**)Memory::LinearAllocate(sizeof(Tile*) * TILES_X);

	for (U16 i = 0; i < TILES_X; ++i)
	{
		tiles[i] = (Tile*)Memory::LinearAllocate(sizeof(Tile) * TILES_Y);
	}

	GenerateWorld();

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

	U16 xPos = CHUNKS_X / 2;
	U16 yPos = 2;

	RendererFrontend::CurrentScene()->GetCamera()->SetPosition(Vector3{TILES_X / 2.0f, 16.0f, 10.0f}); //TODO: set player position and set player as camera target

	GridBroadphase* bp = new GridBroadphase(TILES_X, TILES_Y);
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
	static Vector3Int lastPos{10, 10, 0};
	Vector3 pos = (RendererFrontend::CurrentScene()->GetCamera()->Position() / 8.0f);
	Vector3Int posI = (Vector3Int)pos;

	if (posI != lastPos)
	{
		for (U16 x = Math::Max(lastPos.x - VIEW_DISTANCE_X, 0); x <= Math::Min(lastPos.x + VIEW_DISTANCE_X, (I32)CHUNKS_X - 1); ++x)
		{
			for (U16 y = Math::Max(lastPos.y - VIEW_DISTANCE_Y, 0); y <= Math::Min(lastPos.y + VIEW_DISTANCE_Y, (I32)CHUNKS_Y - 1); ++y)
			{
				chunks[x][y].Unload();
			}
		}

		for (U16 x = Math::Max(posI.x - VIEW_DISTANCE_X, 0); x <= Math::Min(posI.x + VIEW_DISTANCE_X, (I32)CHUNKS_X - 1); ++x)
		{
			for (U16 y = Math::Max(posI.y - VIEW_DISTANCE_Y, 0); y <= Math::Min(posI.y + VIEW_DISTANCE_Y, (I32)CHUNKS_Y - 1); ++y)
			{
				chunks[x][y].Load(Vector2{ (F32)x, (F32)y });
			}
		}
	}

	lastPos = posI;
}

Vector2 World::BlockUV(const Vector2Int& pos)
{
	return { 3.0f * (pos.y + 1 == TILES_Y || tiles[pos.x][pos.y + 1].blockID) + ((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3,
		(pos.y - 1 < 0 || tiles[pos.x][pos.y - 1].blockID) +
		((pos.x - 1 < 0 || tiles[pos.x - 1][pos.y].blockID) * 2.0f) +
		((pos.x + 1 == TILES_X || tiles[pos.x + 1][pos.y].blockID) * 4.0f) };
}

Vector2 World::WallUV(const Vector2Int& pos)
{
	return { 3.0f * (pos.y + 1 == TILES_Y || tiles[pos.x][pos.y + 1].wallID) + ((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3,
		(pos.y - 1 < 0 || tiles[pos.x][pos.y - 1].wallID) +
		((pos.x - 1 < 0 || tiles[pos.x - 1][pos.y].wallID) * 2.0f) +
		((pos.x + 1 == TILES_X || tiles[pos.x + 1][pos.y].wallID) * 4.0f) };
}

Vector2 World::DecorationUV(const Vector2Int& pos)
{
	return Vector2::ZERO;
}

Vector2 World::LiquidUV(const Vector2Int& pos)
{
	return Vector2::ZERO;
}


void World::GenerateWorld()
{
	for (U16 x = 0; x < TILES_X; ++x)
	{
		U16 height = (U16)((Math::Simplex1(x * 0.005 + SEED) * 25.0) +
			(Math::Simplex1(x * 0.05 + SEED) * 5.0) + (TILES_Y / 2.0));

		tiles[x][height].decID = 1;

		for (U16 y = height; y < TILES_Y; ++y)
		{
			tiles[x][y].blockID = 1 + (y > height + 10);
			tiles[x][y].wallID = 1 + (y > height + 10);
		}
	}
}

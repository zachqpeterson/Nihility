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
				chunks[x][y].Load(Vector2{ (F32)x, (F32)y });
			}
		}
	}

	//TODO: Block mouse click with the ui
	if (Input::ButtonDown(LEFT_CLICK))
	{
		Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
		Vector2 mousePos = (Vector2)Input::MousePos();
		Vector2 screenSize = (Vector2)Platform::ScreenSize();
		Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
		Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();

		if (Input::ButtonDown(CONTROL))
		{
			BreakWall(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
		}
		else
		{
			BreakBlock(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f });
		}
	}
	
	if (Input::ButtonDown(RIGHT_CLICK))
	{
		Vector2 cameraPos = (Vector2)RendererFrontend::CurrentScene()->GetCamera()->Position();
		Vector2 mousePos = (Vector2)Input::MousePos();
		Vector2 screenSize = (Vector2)Platform::ScreenSize();
		Vector2 windowSize = (Vector2)RendererFrontend::WindowSize();
		Vector2 windowOffset = (Vector2)RendererFrontend::WindowOffset();

		if (Input::ButtonDown(CONTROL))
		{
			PlaceWall(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f }, 1);
		}
		else
		{
			PlaceBlock(Vector2Int{ ((mousePos - windowSize * 0.5f - windowOffset) / (windowSize.x * 0.0125f)) + cameraPos + 0.5f }, 1);
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
	if (id < 3)
	{
		return { (F32)(((I16)pos.x ^ 2 * (I16)pos.y + SEED) % 3), 0.0f };
	}

	return Vector2((F32)((tiles[pos.x - 1][pos.y - 1].decID == id) +
		((tiles[pos.x + 1][pos.y - 1].decID == id) << 1) +
		((tiles[pos.x - 1][pos.y + 1].decID == id) << 2) +
		((tiles[pos.x + 1][pos.y + 1].decID == id) << 3)),
		(F32)((tiles[pos.x][pos.y - 1].decID == id) +
			((tiles[pos.x - 1][pos.y].decID == id) << 1) +
			((tiles[pos.x + 1][pos.y].decID == id) << 2) +
			((tiles[pos.x][pos.y + 1].decID == id) << 3)));
}

Vector2 World::LiquidUV(const Vector2Int& pos)
{
	return Vector2::ZERO;
}

void World::BreakBlock(const Vector2Int& pos)
{
	Vector2Int chunkPos = pos / 8;

	if (tiles[pos.x][pos.y].blockID) { TimeSlip::PickupItem(tiles[pos.x][pos.y].blockID, 1); }
	if (tiles[pos.x][pos.y].decID > 2) { TimeSlip::PickupItem(tiles[pos.x][pos.y].decID, 1); }

	tiles[pos.x][pos.y].blockID = 0;
	tiles[pos.x][pos.y].decID = 0;
	chunks[chunkPos.x][chunkPos.y].EditBlock(0, pos, pos - chunkPos * 8);
	chunks[chunkPos.x][chunkPos.y].EditDecoration(0, pos, pos - chunkPos * 8);

	if (pos.x > 0)
	{
		Vector2Int left = pos - Vector2Int::LEFT;
		chunkPos = left / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[left.x][left.y].blockID, left, left - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[left.x][left.y].decID, left, left - chunkPos * 8);

		if (pos.y > 0)
		{
			Vector2Int leftDown = left - Vector2Int::DOWN;
			chunkPos = leftDown / 8;
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[leftDown.x][leftDown.y].decID, leftDown, leftDown - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int leftUp = left - Vector2Int::UP;
			chunkPos = leftUp / 8;
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[leftUp.x][leftUp.y].decID, leftUp, leftUp - chunkPos * 8);
		}
	}

	if (pos.x < TILES_X - 1)
	{
		Vector2Int right = pos - Vector2Int::RIGHT;
		chunkPos = right / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[right.x][right.y].blockID, right, right - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[right.x][right.y].decID, right, right - chunkPos * 8);

		if (pos.y > 0)
		{
			Vector2Int rightDown = right - Vector2Int::DOWN;
			chunkPos = rightDown / 8;
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[rightDown.x][rightDown.y].decID, rightDown, rightDown - chunkPos * 8);
		}

		if (pos.y < TILES_Y - 1)
		{
			Vector2Int rightUp = right - Vector2Int::UP;
			chunkPos = rightUp / 8;
			chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[rightUp.x][rightUp.y].decID, rightUp, rightUp - chunkPos * 8);
		}
	}

	if (pos.y > 0)
	{
		Vector2Int down = pos - Vector2Int::DOWN;
		chunkPos = down / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[down.x][down.y].blockID, down, down - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[down.x][down.y].decID, down, down - chunkPos * 8);
	}

	if (pos.y < TILES_Y - 1)
	{
		Vector2Int up = pos - Vector2Int::UP;
		chunkPos = up / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[up.x][up.y].blockID, up, up - chunkPos * 8);
		chunks[chunkPos.x][chunkPos.y].EditDecoration(tiles[up.x][up.y].decID, up, up - chunkPos * 8);
	}
}

void World::PlaceBlock(const Vector2Int& pos, U8 id)
{
	Vector2Int chunkPos = pos / 8;
	tiles[pos.x][pos.y].blockID = id;
	chunks[chunkPos.x][chunkPos.y].EditBlock(id, pos, pos - chunkPos * 8);

	if (pos.x > 0)
	{
		Vector2Int left = pos - Vector2Int::LEFT;
		chunkPos = left / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[left.x][left.y].blockID, left, left - chunkPos * 8);
	}

	if (pos.x < TILES_X - 1)
	{
		Vector2Int right = pos - Vector2Int::RIGHT;
		chunkPos = right / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[right.x][right.y].blockID, right, right - chunkPos * 8);
	}

	if (pos.y > 0)
	{
		Vector2Int down = pos - Vector2Int::DOWN;
		chunkPos = down / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[down.x][down.y].blockID, down, down - chunkPos * 8);
		if (tiles[down.x][down.y].decID < 3) //TODO: This will change was we add different decorations
		{
			tiles[down.x][down.y].decID = 0;
			chunks[chunkPos.x][chunkPos.y].EditDecoration(0, down, down - chunkPos * 8);
		}
	}

	if (pos.y < TILES_Y - 1)
	{
		Vector2Int up = pos - Vector2Int::UP;
		chunkPos = up / 8;
		chunks[chunkPos.x][chunkPos.y].EditBlock(tiles[up.x][up.y].blockID, up, up - chunkPos * 8);
	}
}

void World::BreakWall(const Vector2Int& pos)
{
	Vector2Int chunkPos = pos / 8;

	if (tiles[pos.x][pos.y].wallID) { TimeSlip::PickupItem(tiles[pos.x][pos.y].wallID, 1); }

	tiles[pos.x][pos.y].wallID = 0;
	chunks[chunkPos.x][chunkPos.y].EditWall(0, pos, pos - chunkPos * 8);

	if (pos.x > 0)
	{
		Vector2Int left = pos - Vector2Int::LEFT;
		chunkPos = left / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[left.x][left.y].wallID, left, left - chunkPos * 8);
	}

	if (pos.x < TILES_X - 1)
	{
		Vector2Int right = pos - Vector2Int::RIGHT;
		chunkPos = right / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[right.x][right.y].wallID, right, right - chunkPos * 8);
	}

	if (pos.y > 0)
	{
		Vector2Int down = pos - Vector2Int::DOWN;
		chunkPos = down / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[down.x][down.y].wallID, down, down - chunkPos * 8);
	}

	if (pos.y < TILES_Y - 1)
	{
		Vector2Int up = pos - Vector2Int::UP;
		chunkPos = up / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[up.x][up.y].wallID, up, up - chunkPos * 8);
	}
}

void World::PlaceWall(const Vector2Int& pos, U8 id)
{
	Vector2Int chunkPos = pos / 8;
	tiles[pos.x][pos.y].wallID = id;
	chunks[chunkPos.x][chunkPos.y].EditWall(id, pos, pos - chunkPos * 8);

	if (pos.x > 0)
	{
		Vector2Int left = pos - Vector2Int::LEFT;
		chunkPos = left / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[left.x][left.y].wallID, left, left - chunkPos * 8);
	}

	if (pos.x < TILES_X - 1)
	{
		Vector2Int right = pos - Vector2Int::RIGHT;
		chunkPos = right / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[right.x][right.y].wallID, right, right - chunkPos * 8);
	}

	if (pos.y > 0)
	{
		Vector2Int down = pos - Vector2Int::DOWN;
		chunkPos = down / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[down.x][down.y].wallID, down, down - chunkPos * 8);
	}

	if (pos.y < TILES_Y - 1)
	{
		Vector2Int up = pos - Vector2Int::UP;
		chunkPos = up / 8;
		chunks[chunkPos.x][chunkPos.y].EditWall(tiles[up.x][up.y].wallID, up, up - chunkPos * 8);
	}
}

F32 World::GenerateWorld()
{
	Timer timer;
	timer.Start();

	F32 spawnHeight = 0.0f;

	for (U16 x = 0; x < TILES_X; ++x)
	{
		U16 height = (U16)((Math::Simplex1(x * 0.005 + SEED) * 25.0) +
			(Math::Simplex1(x * 0.05 + SEED) * 5.0) + (TILES_Y * 0.5));

		for (U16 y = height; y < TILES_Y; ++y)
		{
			bool cave = Math::Abs(Math::Simplex2(x * 0.02 + SEED, y * 0.02 + SEED) +
				Math::Simplex2(x * 0.01333333333 + SEED * 2.0, y * 0.01333333333 + SEED * 2.0)) > (0.04 * (height + y) / (F64)height + 0.01);

			F64 oreNoise0 = Math::Simplex2(x * 0.07 + SEED * 2.0, y * 0.07 + SEED * 2.0) * 0.13;
			F64 oreNoise1 = Math::Simplex2(x * 0.07 + SEED * 3.0, y * 0.07 + SEED * 3.0) * 0.13;
			U8 ore = Math::Max((oreNoise0 > 0.1) * 3 + (oreNoise0 < -0.1) * 4, (oreNoise1 > 0.1) * 5 + (oreNoise1 < -0.1) * 6);

			tiles[x][y].decID = ((y == height) + (y > height) * ore) * cave;
			tiles[x][y].blockID = (1 + (y > height + 10)) * cave;
			tiles[x][y].wallID = 1 + (y > height + 10);
		}
	}

	Logger::Debug("World Generation Time: {}", timer.CurrentTime());

	return (F32)(U16)((Math::Simplex1(TILES_X * 0.0025 + SEED) * 25.0) +
		(Math::Simplex1(TILES_X * 0.025 + SEED) * 5.0) + (TILES_Y * 0.5)) - 1.5f;
}

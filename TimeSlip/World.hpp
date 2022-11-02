#pragma once

#include <Defines.hpp>


const enum Biomes
{
	Grasslands,	//Copper
	Mesa,		//Tin
	Desert,		//Iron
	Jungle,		//Coal
	Marsh,		//?

	BIOME_COUNT
};

enum WorldSize
{
	WS_TEST = 280,
	WS_SMALL = 2800,
	WS_MEDIUM = 4200,
	WS_LARGE = 5600
};

static const U8 biomeTileMods[BIOME_COUNT] =
{
	0,	//Grasslands
	2,	//Mesa
	4,	//Desert
	2,	//Jungle
	4,	//Marsh
};

struct Tile;
struct Vector2;
struct Vector3;
struct Vector2Int;
class Chunk;

class World
{
public:
	World(I64 seed, WorldSize size, Vector2& spawnPoint);
	~World();
	void Destroy();
	void* operator new(U64 size);
	void operator delete(void* ptr);

	void Update();

	Vector2 BlockUV(const Vector2Int& pos);
	Vector2 WallUV(const Vector2Int& pos);
	Vector2 DecorationUV(const Vector2Int& pos, U8 id);
	Vector2 LiquidUV(const Vector2Int& pos);
	void TileLight(const Vector2Int& pos, Vector3& color, Vector3& globalColor);

	void BreakBlock(const Vector2Int& pos);
	bool PlaceBlock(const Vector2Int& pos, U8 id);
	void BreakWall(const Vector2Int& pos);
	bool PlaceWall(const Vector2Int& pos, U8 id);
	void PlaceLight(const Vector2Int& pos);
	void RemoveLight(const Vector2Int& pos);

	F32 GenerateWorld();

private:
	const I64 SEED;
	const U16 TILES_X;
	const U16 TILES_Y;
	const U16 CHUNKS_X;
	const U16 CHUNKS_Y;

	Tile** tiles;
	Chunk** chunks;

	friend class TimeSlip;
};
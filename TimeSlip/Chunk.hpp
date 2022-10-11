#pragma once

#include <Defines.hpp>
#include <Containers/Array.hpp>
#include <Math/Math.hpp>

#define CHUNK_SIZE 8

struct Model;
struct Tile;

class World;

class Chunk
{
	struct Vertex
	{
		Vector3 position;
		Vector2 uv;
		U32 texId;
	};

public:
	Chunk();
	~Chunk();

	void Destroy();

	void Load(const Vector2& pos);
	void Unload();

	void EditBlock(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);
	void EditWall(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);
	void EditDecoration(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);
	void EditLiquid(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);

private:
	Array<Tile*, CHUNK_SIZE>& SetTiles();

	bool loaded;

	Model* model;
	Array<Tile*, CHUNK_SIZE> tiles;

	static const Vector3 VERTEX_POSITIONS[4];
	static const Vector2 UV_POSITIONS[4];
	static const U8 INDEX_SEQUENCE[6];

	static World* world;

	friend class World;
};
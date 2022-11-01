#pragma once

#include <Defines.hpp>
#include <Containers/Array.hpp>
#include <Containers/Vector.hpp>
#include <Math/Math.hpp>

#define CHUNK_SIZE 8

struct Model;
struct Mesh;
struct Tile;

class World;

class Chunk
{
	struct Vertex
	{
		Vector3 position;
		Vector2 uv;
		Vector3 color;
		Vector3 globalColor;
		U32 texId;
	};

public:
	Chunk();
	~Chunk();

	void Destroy();

	void Load(const Vector2Int& pos);
	void Unload();

	void EditBlock(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);
	void EditWall(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);
	void EditDecoration(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);
	void EditLiquid(U8 id, const Vector2Int& worldPos, const Vector2Int& tilePos);

	void UpdateLighting(const Vector2Int& pos);

	void UpdateMeshes(Vector<Mesh*>& meshes);

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
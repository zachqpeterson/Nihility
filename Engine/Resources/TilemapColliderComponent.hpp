#pragma once

#include "Component.hpp"
#include "TilemapComponent.hpp"

struct ChainId
{
	I32 index;
	U16 world;
	U16 generation;
};

class NH_API TilemapCollider
{
	enum Dir
	{
		Right,
		DownRight,
		Down,
		DownLeft,
		Left,
		UpLeft,
		Up,
		UpRight,
	};

public:
	ComponentRef<Tilemap> tilemap;
	Vector<Vector2> points;
	Vector2Int dimensions;
	Vector2 offset;
	Vector2 tileSize;
	Vector2Int current;
	Vector2Int startPos;
	Vector2 position;
	const TileType* tiles;
	Dir dir;
	Dir endDir;
	bool start;

	static bool Initialize();
	static bool Shutdown();

	static ComponentRef<TilemapCollider> AddTo(EntityRef entity, const ComponentRef<Tilemap>& tilemap);

private:
	void GenerateCollision();
	bool CheckRight();
	bool CheckDownRight();
	bool CheckDown();
	bool CheckDownLeft();
	bool CheckLeft();
	bool CheckUpLeft();
	bool CheckUp();
	bool CheckUpRight();

	static bool Update(Camera& camera, Vector<Entity>& entities);
	static bool Render(CommandBuffer commandBuffer);

	static bool initialized;

	COMPONENT(TilemapCollider);
	friend struct EntityRef;
};
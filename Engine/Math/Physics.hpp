#pragma once

#include "Defines.hpp"

#include "Math.hpp"

#include "Resources/Component.hpp"
#include "Containers/Vector.hpp"

enum class NH_API BodyType
{
	Static = 0,
	Kinematic = 1,
	Dynamic = 2
};

struct NH_API AABB
{
	Vector2 upperBound;
	Vector2 lowerBound;

	AABB operator+(const Vector2& v) const { return { upperBound + v, lowerBound + v }; }
	AABB operator-(const Vector2& v) const { return { upperBound - v, lowerBound - v }; }
};

enum class TileType;
class TilemapCollider;

struct NH_API GridCollider
{
	Vector2Int dimensions;
	Vector2 tileSize;
	Vector2 offset;
	const TileType* tileArray;
};

//struct NH_API Collider
//{
//	AABB aabb;
//	Material material; //restitution, friction
//	U64 mask;
//	bool trigger;
//};

struct NH_API Collision
{
	AABB aabb;
	bool valid;

	operator bool() const { return valid; }
};

class NH_API Physics
{
public:
	static U32 AddCollider(const AABB& collider);
	static U32 AddTilemapCollider(const ComponentRef<TilemapCollider>& tilemapCollider);
	static void RemoveCollider(U32 index);
	static void RemoveTilemapCollider(U32 index);

	static Collision CheckCollision(const AABB& collider);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static Vector<AABB> colliders;
	static Vector<GridCollider> tilemapColliders;

	STATIC_CLASS(Physics);

	friend class Engine;
	friend class RigidBody;
};
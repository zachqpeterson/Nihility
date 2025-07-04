#pragma once

#include "Defines.hpp"

#include "Math.hpp"

#include "Containers/Vector.hpp"

enum class BodyType
{
	Static = 0,
	Kinematic = 1,
	Dynamic = 2
};

struct AABB
{
	Vector2 upperBound;
	Vector2 lowerBound;

	AABB operator+(const Vector2& v) const { return { upperBound + v, lowerBound + v }; }
	AABB operator-(const Vector2& v) const { return { upperBound - v, lowerBound - v }; }
};

class NH_API Physics
{
public:
	static void AddCollider(AABB collider);
	static bool CheckCollision(AABB collider);

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static Vector<AABB> colliders;

	STATIC_CLASS(Physics);

	friend class Engine;
	friend class RigidBody;
	friend struct Scene;
};
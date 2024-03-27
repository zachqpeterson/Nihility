#pragma once

#include "Math.hpp"

struct RigidBody
{
	Vector3 position;
	Vector3 velocity;

	Quaternion3 rotation;
	Vector3 angularMomentum;
};

class NH_API Physics
{
private:
	static bool Initialize();
	static void Shutdown();

	static void Update(F64 step);

	STATIC_CLASS(Physics);
	friend class Engine;
};
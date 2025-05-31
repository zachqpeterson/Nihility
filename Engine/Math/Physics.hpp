#pragma once

#include "Defines.hpp"

enum class BodyType
{
	Static = 0,
	Kinematic = 1,
	Dynamic = 2
};

struct b2WorldId;

class NH_API Physics
{
public:
	static b2WorldId WorldID();

private:
	static bool Initialize();
	static void Shutdown();

	static void Update();

	static F64 timeStep;

	STATIC_CLASS(Physics);

	friend class Engine;
	friend class RigidBodyComponent;
	friend struct Scene;
};
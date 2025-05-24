#pragma once

#include "Defines.hpp"

#include "Math/Math.hpp"

struct BodyId
{
	I32 index;
	U16 world;
	U16 generation;
};

struct NH_API Entity
{
	Vector2 position;
	Quaternion2 rotation;
	BodyId bodyId;
	U32 spriteId = U32_MAX;
};

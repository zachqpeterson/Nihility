#pragma once

#include "ResourceDefines.hpp"
#include "Texture.hpp"

struct Vector2;

class NH_API Particles
{
public:
	static void Spawn(const Vector2& position, ResourceRef<Texture> texture);

	STATIC_CLASS(Particles);
};
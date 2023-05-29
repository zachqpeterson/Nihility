#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct Scene
{
public:
	String				name{ NO_INIT };

	Vector<MeshDraw>	meshDraws;

	Vector<Texture*>	images;
	Vector<Sampler*>	samplers;
	Vector<Buffer*>		buffers;
};
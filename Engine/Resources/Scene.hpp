#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct Scene
{
public:

private:

public:
	String				name{ NO_INIT };

	Vector<MeshDraw>	meshDraws;

	Vector<Texture*>	images;
	Vector<Sampler*>	samplers;
	Vector<Buffer*>		buffers; //These should never be child buffers, most should be parent buffers
};

//bool SetupScene(Vector<MeshDraw>& draw);
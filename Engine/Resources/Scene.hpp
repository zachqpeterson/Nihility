#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct Scene
{
public:


public:
	String				name{ NO_INIT };
	HashHandle			handle;

	Vector<MeshDraw>	meshDraws;	//TODO: Just call these Mesh, not MeshDraw

	Vector<Texture*>	images;		//TODO: Probably don't need to store textures here, just handles which might be more useful
	Vector<Sampler*>	samplers;	//TODO: Probably don't need samplers here at all, textures have access to samplers
	Vector<Buffer*>		buffers;	//These are parent buffers, meshes will hold child buffers to these
};

//bool SetupScene(Vector<MeshDraw>& draw);
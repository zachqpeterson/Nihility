#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct Scene
{
public:


public:
	String				name{ NO_INIT };
	HashHandle			handle;

	//TODO: Add Camera

	Vector<MeshDraw>	meshDraws;	//TODO: Just call these Mesh, not MeshDraw

	Vector<Texture*>	textures;	//TODO: Probably don't need to store textures here, just handles which might be more useful
	Vector<Sampler*>	samplers;
	Vector<Buffer*>		buffers;	//These are parent buffers, meshes will hold child buffers to these
};

//bool SetupScene(Vector<MeshDraw>& draw);
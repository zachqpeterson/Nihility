#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct CommandBuffer;

struct NH_API Scene
{
public:
	void Update();

private:
	void UploadMaterial(MeshData& meshData, const MeshDraw& meshDraw);
	void DrawMesh(CommandBuffer* commands, MeshDraw& meshDraw);

public:
	String				name{ NO_INIT };
	HashHandle			handle;

	Camera camera;
	Buffer* constantBuffer;

	Vector<MeshDraw>	meshDraws;	//TODO: Just call these Mesh, not MeshDraw

	Vector<Texture*>	textures;	//TODO: Probably don't need to store textures here, just handles which might be more useful
	Vector<Sampler*>	samplers;
	Vector<Buffer*>		buffers;	//These are parent buffers, meshes will hold child buffers to these
};

//bool SetupScene(Vector<MeshDraw>& draw);
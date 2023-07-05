#pragma once

#include "ResourceDefines.hpp"

#include "Math\Math.hpp"

struct CommandBuffer;

struct NH_API Scene
{
public:
	void Create();
	~Scene();
	void Destroy();

	void Update();

private:
	void UploadMaterial(MeshData& meshData, const Mesh& mesh);

public:
	String				name{ NO_INIT };
	HashHandle			handle;

	Camera				camera;
	Buffer*				constantBuffer;
	Skybox*				skybox;

	Vector<Mesh>		meshes{ NO_INIT };

	Vector<Texture*>	textures{ NO_INIT };
	Vector<Sampler*>	samplers{ NO_INIT };
	Vector<Buffer*>		buffers{ NO_INIT };	//These are parent buffers, meshes will hold child buffers to these
};
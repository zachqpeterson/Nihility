#pragma once

#include "ResourceDefines.hpp"

#include "Entity.hpp"

struct MeshLocation
{
	U32 instanceOffset{ 0 };
	U32 instanceCount{ 0 };
	U32 drawOffset{ 0 };
};

struct MeshDraw
{
	U32 indexOffset{ 0 };
	U32 indexCount{ 0 };
	I32 vertexOffset{ 0 };

	MeshLocation locations[RENDER_STAGE_COUNT][MAX_PIPELINES_PER_STAGE]{ };
};

struct NH_API Scene
{
	void AddEntity(Entity* entity);

	const String& Name() { return name; }

	Camera* GetCamera() { return &camera; }

private:
	void Create(CameraType cameraType, Rendergraph* rendergraph);
	void Destroy();

	bool Render(CommandBuffer* commandBuffer);
	void Resize();

	String							name{};
	HashHandle						handle;

	Camera							camera{};

	Buffer							vertexBuffer;
	Buffer							instanceBuffer;
	Buffer							indexBuffer;
	Buffer							drawBuffer;
	Buffer							countsBuffer;

	U32								vertexOffset{ 0 };
	U32								instanceOffset{ 0 };
	U32								indexOffset{ 0 };
	U32								countsOffset{ 0 };
	U32								drawOffset{ 0 };

	Rendergraph*					rendergraph{ nullptr };
	Vector<Entity*>					entities{};

	Hashmap<HashHandle, HashHandle>	handles{ 1024 };
	Vector<MeshDraw>				meshDraws;

#ifdef NH_DEBUG
	FlyCamera				flyCamera{};
#endif

	friend class Renderer;
	friend class Resources;
};
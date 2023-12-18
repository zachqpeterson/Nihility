#pragma once

#include "ResourceDefines.hpp"

#include "Material.hpp"

struct Entity;

struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texcoord;
};

struct NH_API Mesh
{
	void Destroy() { name.Destroy(); }

	String		name{};
	HashHandle	handle;

	U8* indices;
	U32 indicesSize;
	U8* vertices;
	U32 verticesSize;
};

struct InstanceData
{
	U8 data[256];
};

//TODO: Any changes after loaded in the scene needs to notify the scene
struct NH_API MeshInstance
{
	Material*		material;
	Mesh*			mesh;

	InstanceData	instanceData;

private:
	HashHandle		handle;
	U32				instanceOffset;

	friend struct Scene;
};

struct NH_API Model
{
	void Destroy() { name.Destroy(); meshes.Destroy(); handle = U64_MAX; }

	String		name{};
	HashHandle	handle;

	Vector<MeshInstance> meshes;
};
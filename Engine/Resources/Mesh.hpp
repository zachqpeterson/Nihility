#pragma once

#include "ResourceDefines.hpp"

#include "Material.hpp"
#include "Component.hpp"

struct Entity;
struct Scene;

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

struct NH_API MeshInstance
{
	MeshInstance() = default;
	MeshInstance(MeshInstance&& other) noexcept : material{ other.material }, mesh{ other.mesh }, instanceData{ other.instanceData }, handle{ other.handle }, instanceOffset{ other.instanceOffset } {}

	MeshInstance& operator=(MeshInstance&& other) noexcept
	{
		material = other.material;
		mesh = other.mesh;
		instanceData = other.instanceData;
		handle = other.handle;
		instanceOffset = other.instanceOffset;

		return *this;
	}

	Material*		material;
	Mesh*			mesh;

	InstanceData	instanceData;

private:
	HashHandle		handle;
	U32				instanceOffset;

	friend struct Scene;
};

struct NH_API Model //TODO: model instance
{
	void Destroy() { name.Destroy(); meshes.Destroy(); handle = U64_MAX; }

	String		name{};
	HashHandle	handle;

	Vector<MeshInstance> meshes;
};

struct NH_API MeshComponent : public Component
{
	MeshComponent(Mesh* mesh, Material* material)
	{
		meshInstance.mesh = mesh;
		meshInstance.material = material;
	}

	MeshComponent(MeshComponent&& other) noexcept : meshInstance{ Move(other.meshInstance) } {}

	MeshComponent& operator=(MeshComponent&& other) noexcept { meshInstance = Move(other.meshInstance); return *this; }

	virtual void Update() final {}

	virtual void Load(Scene* scene) final;

	MeshInstance meshInstance;
};

struct NH_API ModelComponent : public Component
{
	ModelComponent(Model* model) : model{ model } {}

	ModelComponent(ModelComponent&& other) noexcept : model{ other.model } {}

	ModelComponent& operator=(ModelComponent&& other) noexcept { model = other.model; return *this; }

	virtual void Update() final {}

	virtual void Load(Scene* scene) final;

	Model* model; //TODO: Use an instance of a model
};
#pragma once

#include "ResourceDefines.hpp"

#include "Material.hpp"
#include "Component.hpp"

struct Entity;
struct Scene;

struct VertexBuffer
{
	VertexType type{ VERTEX_TYPE_COUNT };
	U32 stride{ 0 };
	U32 size{ 0 };
	U8* buffer{ nullptr };
};

struct NH_API Mesh
{
	void Destroy() { name.Destroy(); }

	String		name{};
	HashHandle	handle;

	U8* indices;
	U32 indicesSize;

	U32 vertexCount;

	Vector<VertexBuffer> buffers;
};

struct InstanceData
{
	U32 materialID;
	Matrix4 model;
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

	Material* material;
	Mesh* mesh;

	InstanceData	instanceData;

private:
	HashHandle		handle{ U64_MAX };
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
	COMPONENT(MeshComponent);

	MeshComponent(Mesh* mesh, Material* material)
	{
		meshInstance.mesh = mesh;
		meshInstance.material = material;
	}
	MeshComponent(MeshComponent&& other) noexcept : meshInstance{ Move(other.meshInstance) } {}

	MeshComponent& operator=(MeshComponent&& other) noexcept { meshInstance = Move(other.meshInstance); return *this; }

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;

	MeshInstance meshInstance;
};

struct NH_API ModelComponent : public Component
{
	COMPONENT(ModelComponent);

	ModelComponent(Model* model) : model{ model } {}
	ModelComponent(ModelComponent&& other) noexcept : model{ other.model } {}

	ModelComponent& operator=(ModelComponent&& other) noexcept { model = other.model; return *this; }

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;

	Model* model; //TODO: Use an instance of a model
};
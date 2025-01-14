#pragma once

#include "ResourceDefines.hpp"
#include "Material.hpp"

#include "Resources\Scene.hpp"
#include "Memory\Memory.hpp"

struct Entity;
struct Scene;

struct VertexBuffer
{
	VertexType type = VERTEX_TYPE_COUNT;
	U32 stride = 0;
	U32 size = 0;
	U8* buffer = nullptr;
};

struct NH_API Mesh : public Resource
{
	U8* indices;
	U32 indicesSize;

	U32 vertexCount;

	Vector<VertexBuffer> buffers;

private:
	U32 index = U32_MAX;

	F32 mass;
	Vector3 centerOfMass;
	Matrix3 invInertiaMatrix;

	friend struct Scene;
	friend class Resources;
};

struct InstanceData
{
	U8 data[128];
};

struct NH_API MeshInstance
{
	MeshInstance() {};
	MeshInstance(const MeshInstance& other) : mesh(other.mesh), material(other.material)
	{
		Copy(instanceData.data, other.instanceData.data, sizeof(InstanceData));
	}
	MeshInstance(MeshInstance&& other) noexcept : mesh(other.mesh), material(other.material),
		handle(other.handle), instanceOffset(other.instanceOffset)
	{
		Copy(instanceData.data, other.instanceData.data, sizeof(InstanceData));
	}

	MeshInstance& operator=(MeshInstance&& other) noexcept
	{
		mesh = other.mesh;
		material = other.material;
		Copy(instanceData.data, other.instanceData.data, sizeof(InstanceData));
		handle = other.handle;
		instanceOffset = other.instanceOffset;

		return *this;
	}
	MeshInstance& operator=(const MeshInstance& other)
	{
		mesh = other.mesh;
		material = other.material;
		Copy(instanceData.data, other.instanceData.data, sizeof(InstanceData));

		return *this;
	}

	ResourceRef<Mesh>		mesh = nullptr;
	ResourceRef<Material>	material = nullptr;

	InstanceData			instanceData;

private:
	HashHandle				handle = U64_MAX;
	U32						instanceOffset = 0;

	friend struct Scene;
};

struct NH_API MeshInstanceCluster
{
	MeshInstanceCluster() = default;
	MeshInstanceCluster(MeshInstanceCluster&& other) noexcept : mesh(other.mesh), material(other.material),
		handle(other.handle), instanceOffset(other.instanceOffset), instances(Move(other.instances))
	{
	}

	MeshInstanceCluster& operator=(MeshInstanceCluster&& other) noexcept
	{
		mesh = other.mesh;
		material = other.material;
		instances = Move(other.instances);
		handle = other.handle;
		instanceOffset = other.instanceOffset;

		return *this;
	}

	ResourceRef<Mesh>		mesh = nullptr;
	ResourceRef<Material>	material = nullptr;

	Vector<InstanceData>	instances;

private:
	HashHandle				handle = U64_MAX;
	U32						instanceOffset = 0;

	friend struct Scene;
};

struct NH_API Model : public Resource //TODO: model instance
{
	void Destroy() { name.Destroy(); meshes.Destroy(); handle = U64_MAX; }

	Vector<Matrix4> matrices;
	Vector<MeshInstance> meshes;
};

struct NH_API MeshComponent
{
	MeshComponent(const ResourceRef<Mesh>& mesh, const ResourceRef<Material>& material)
	{
		modelMatrix = Matrix4Identity;
		meshInstance.mesh = mesh;
		meshInstance.material = material;
		Copy((U32*)meshInstance.instanceData.data, (U32*)&material->Handle(), 1);
	}
	MeshComponent(MeshComponent&& other) noexcept : meshInstance(Move(other.meshInstance)), modelMatrix(other.modelMatrix) {}
	MeshComponent& operator=(MeshComponent&& other) noexcept
	{
		meshInstance = Move(other.meshInstance);
		modelMatrix = other.modelMatrix;
		return *this;
	}

	//virtual void Update(Scene* scene) final;
	//virtual void Load(Scene* scene) final;
	//virtual void Cleanup(Scene* scene) final {}

	Matrix4		 modelMatrix;
	MeshInstance meshInstance;
};

struct NH_API ModelComponent
{
	ModelComponent(const ResourceRef<Model>& model) : model(model) {}
	ModelComponent(ModelComponent&& other) noexcept :  model(other.model), modelMatrix(other.modelMatrix) {}
	ModelComponent& operator=(ModelComponent&& other) noexcept
	{
		model = other.model;
		modelMatrix = other.modelMatrix;
		return *this;
	}

	//virtual void Update(Scene* scene) final;
	//virtual void Load(Scene* scene) final;
	//virtual void Cleanup(Scene* scene) final {}

	Matrix4 modelMatrix;
	ResourceRef<Model> model;
};
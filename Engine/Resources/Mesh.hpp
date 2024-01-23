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

struct NH_API Mesh : public Resource
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
	U32 materialId;
	Matrix4 model;
};

struct NH_API MeshInstance
{
	MeshInstance() = default;
	MeshInstance(MeshInstance&& other) noexcept : mesh{ other.mesh }, material{ other.material },
		handle{ other.handle }, instanceOffset{ other.instanceOffset }
	{
		Memory::Copy(&instanceData, &other.instanceData, sizeof(InstanceData));
	}

	MeshInstance& operator=(MeshInstance&& other) noexcept
	{
		mesh = other.mesh;
		material = other.material;
		Memory::Copy(&instanceData, &other.instanceData, sizeof(InstanceData));
		handle = other.handle;
		instanceOffset = other.instanceOffset;

		return *this;
	}

	ResourceRef<Mesh>		mesh{ nullptr };
	ResourceRef<Material>	material{ nullptr };

	InstanceData			instanceData{};

private:
	HashHandle				handle{ U64_MAX };
	U32						instanceOffset{ 0 };

	friend struct Scene;
};

struct NH_API Model : public Resource //TODO: model instance
{
	void Destroy() { name.Destroy(); meshes.Destroy(); handle = U64_MAX; }

	String		name{};
	HashHandle	handle;

	Vector<Matrix4> matrices;
	Vector<MeshInstance> meshes;
};

struct NH_API MeshComponent : public Component
{
	MeshComponent(const ResourceRef<Mesh>& mesh, const ResourceRef<Material>& material)
	{
		modelMatrix = Matrix4Identity;
		meshInstance.mesh = mesh;
		meshInstance.material = material;
		meshInstance.instanceData.materialId = (U32)material->Handle();
	}
	MeshComponent(MeshComponent&& other) noexcept : meshInstance{ Move(other.meshInstance) }, modelMatrix{ other.modelMatrix } {}

	MeshComponent& operator=(MeshComponent&& other) noexcept { meshInstance = Move(other.meshInstance); modelMatrix = other.modelMatrix; return *this; }

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;

	Matrix4		 modelMatrix{};
	MeshInstance meshInstance;
};

struct NH_API ModelComponent : public Component
{
	ModelComponent(const ResourceRef<Model>& model) : model{ model } {}
	ModelComponent(ModelComponent&& other) noexcept : model{ other.model }, modelMatrix{ other.modelMatrix } {}

	ModelComponent& operator=(ModelComponent&& other) noexcept { model = other.model; modelMatrix = other.modelMatrix; return *this; }

	virtual void Update(Scene* scene) final;
	virtual void Load(Scene* scene) final;

	Matrix4 modelMatrix{};
	ResourceRef<Model> model;
};
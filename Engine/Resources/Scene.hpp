#pragma once

#include "ResourceDefines.hpp"

#include "ComponentRegistry.hpp"

struct ComponentPool
{
	virtual void Update(Scene* scene) = 0;
	virtual void Load(Scene* scene) = 0;

	virtual U64 Count() = 0;
};

template <class Type>
struct ComponentPoolInternal : public ComponentPool
{
	Type* AddComponent(Type&& component)
	{
		components.Push(Move(component));
		return &components.Back();
	}

	virtual void Update(Scene* scene) final
	{
		for (Type& component : components)
		{
			component.Update(scene);
		}
	}

	virtual void Load(Scene* scene) final
	{
		for (Type& component : components)
		{
			component.Load(scene);
		}
	}

	virtual U64 Count() final { return components.Size(); }

	Vector<Type> components;
};

struct ComponentReference
{
	U64 type;
	U64 id;
};

struct MeshDraw
{
	U32 indexOffset{ 0 };
	U32 indexCount{ 0 };
	I32 vertexOffset{ 0 };

	U32 instanceOffset{ 0 };
	U32 instanceCount{ 0 };
	U32 drawOffset{ 0 };
};

struct Entity;
struct MeshInstance;
struct Rendergraph;
struct CommandBuffer;
struct VkBufferCopy;

struct NH_API Scene : public Resource
{
	Entity* AddEntity();
	Entity* GetEntity(U32 id);

	const String& Name() { return name; }
	Camera* GetCamera() { return &camera; }

	void AddMesh(MeshInstance& instance);
	void UpdateMesh(MeshInstance& instance);
	void SetSkybox(const ResourceRef<Skybox>& skybox);

private:
	void Create(CameraType cameraType);
	void Destroy();

	template<ComponentType Type, typename... Args>
	Type* AddComponent(ComponentReference& reference, const Args&... args)
	{
		reference.type = IndexOf<RegisteredComponents, Type>;
		reference.id = componentPools[reference.type]->Count();

		return ((ComponentPoolInternal<Type>*)componentPools[reference.type])->AddComponent(Move(Type{ args... }));
	}

	template<ComponentType Type>
	void CreatePool()
	{
		componentPools[IndexOf<RegisteredComponents, Type>] = new ComponentPoolInternal<Type>();
	}

	void Load(ResourceRef<Rendergraph>& rendergraph);
	void Unload();
	void Update();

	VkBufferCopy CreateWrite(U64 dstOffset, U64 srcOffset, U64 size, void* data);

	String							name{};
	HashHandle						handle;
	bool							loaded{ false };

	Camera							camera{};

	Buffer							stagingBuffer;
	BufferData						buffers;

	Vector<VkBufferCopy>			vertexWrites[VERTEX_TYPE_COUNT - 1];
	Vector<VkBufferCopy>			instanceWrites;
	Vector<VkBufferCopy>			indexWrites;
	Vector<VkBufferCopy>			drawWrites;
	Vector<VkBufferCopy>			countsWrites;

	U32								vertexOffset{ 0 };
	U32								instanceOffset{ 0 };
	U32								indexOffset{ 0 };
	U32								drawOffset{ 0 };
	U32								countsOffset{ 0 };

	Hashmap<HashHandle, HashHandle>	handles{ 1024 };
	Vector<MeshDraw>				meshDraws{};
	Vector<Entity>					entities{};

	ComponentPool*					componentPools[RegisteredComponents::Size];

#ifdef NH_DEBUG
	FlyCamera						flyCamera{};
#endif

	friend class Renderer;
	friend class Resources;
	friend struct Entity;
};